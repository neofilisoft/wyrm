"""
AsyncInterpreter - Async variant of the Wyrm interpreter for use in browser (Pyodide).
Supports interactive input() by awaiting a JS-provided coroutine.
All methods mirror the sync Interpreter but are async coroutines.
"""
from typing import Any, List, Optional
from .interpreter import (
    Environment, Function, BuiltinFunction,
    ReturnSignal, BreakSignal, ContinueSignal, RuntimeError
)
from .parser import *


class AsyncFunction:
    """Async-capable user-defined function."""
    def __init__(self, name: str, params: List[str], body: List[ASTNode], closure: Optional[Environment] = None):
        self.name = name
        self.params = params
        self.body = body
        self.closure = closure

    async def call_async(self, args: List[Any], interp: 'AsyncInterpreter') -> Any:
        env = Environment(self.closure)
        for i, param in enumerate(self.params):
            env.define(param, args[i] if i < len(args) else None)
        try:
            await interp._exec_block(self.body, env)
        except ReturnSignal as ret:
            return ret.value
        return None


class AsyncInterpreter:
    """
    Fully async tree-walk interpreter.
    Pass `input_coro` - an async callable(prompt: str) -> str
    that the web frontend provides.
    """
    def __init__(self, input_coro=None):
        self.global_env = Environment()
        self._input_coro = input_coro  # async callable
        self._setup_builtins()

    # ------------------------------------------------------------------ #
    # Builtins                                                              #
    # ------------------------------------------------------------------ #
    def _setup_builtins(self):
        env = self.global_env
        env.define("true",  True)
        env.define("false", False)
        env.define("null",  None)

        def _print(args):
            import sys
            sys.stdout.write(" ".join(
                "null" if v is None else ("true" if v is True else ("false" if v is False else str(v)))
                for v in args
            ) + "\n")

        env.define("print",  BuiltinFunction("print",  _print))
        env.define("int",    BuiltinFunction("int",    lambda a: int(a[0])))
        env.define("float",  BuiltinFunction("float",  lambda a: float(a[0])))
        env.define("str",    BuiltinFunction("str",    lambda a: ("null" if a[0] is None else ("true" if a[0] is True else ("false" if a[0] is False else str(a[0]))))))
        env.define("len",    BuiltinFunction("len",    lambda a: len(a[0])))
        env.define("type",   BuiltinFunction("type",   self._builtin_type))
        env.define("abs",    BuiltinFunction("abs",    lambda a: abs(a[0])))
        env.define("max",    BuiltinFunction("max",    lambda a: max(a)))
        env.define("min",    BuiltinFunction("min",    lambda a: min(a)))
        env.define("round",  BuiltinFunction("round",  lambda a: round(a[0])))
        env.define("pow",    BuiltinFunction("pow",    lambda a: pow(a[0], a[1])))
        env.define("append", BuiltinFunction("append", lambda a: a[0].append(a[1])))
        env.define("pop",    BuiltinFunction("pop",    lambda a: a[0].pop()))
        # input is handled specially as async
        env.define("input",  None)  # placeholder - overridden in evaluate

    def _builtin_type(self, args):
        obj = args[0]
        if isinstance(obj, bool): return "bool"
        if isinstance(obj, int): return "int"
        if isinstance(obj, float): return "float"
        if isinstance(obj, str): return "str"
        if isinstance(obj, list): return "list"
        if obj is None: return "null"
        return type(obj).__name__

    # ------------------------------------------------------------------ #
    # Entry point                                                           #
    # ------------------------------------------------------------------ #
    async def execute(self, stmts: list):
        for stmt in stmts:
            await self._exec_stmt(stmt)
        # Call main() if defined
        try:
            main_fn = self.global_env.get("main")
        except RuntimeError:
            main_fn = None
        if main_fn is not None and isinstance(main_fn, AsyncFunction):
            await main_fn.call_async([], self)

    async def _exec_block(self, stmts: list, env: Environment):
        prev = self.global_env
        self.global_env = env
        try:
            for stmt in stmts:
                await self._exec_stmt(stmt)
        finally:
            self.global_env = prev

    # ------------------------------------------------------------------ #
    # Statement dispatch                                                    #
    # ------------------------------------------------------------------ #
    async def _exec_stmt(self, stmt: ASTNode):
        if isinstance(stmt, (NumberNode, StringNode, BooleanNode, NoneNode, ListNode)):
            return
        if isinstance(stmt, AssignmentNode):
            await self._exec_assign(stmt)
        elif isinstance(stmt, PrintNode):
            await self._exec_print(stmt)
        elif isinstance(stmt, IfNode):
            await self._exec_if(stmt)
        elif isinstance(stmt, RepeatNode):
            await self._exec_repeat(stmt)
        elif isinstance(stmt, FunctionDefNode):
            self._exec_fundef(stmt)
        elif isinstance(stmt, FunctionCallNode):
            await self._eval(stmt)
        elif isinstance(stmt, ReturnNode):
            val = await self._eval(stmt.expr)
            raise ReturnSignal(val)
        elif isinstance(stmt, BreakNode):
            raise BreakSignal()
        elif isinstance(stmt, ContinueNode):
            raise ContinueSignal()
        elif isinstance(stmt, IndexAssignNode):
            await self._exec_index_assign(stmt)
        elif isinstance(stmt, UseNode):
            await self._exec_use(stmt)
        elif isinstance(stmt, (IdentifierNode, BinaryOpNode, UnaryOpNode, IndexNode)):
            await self._eval(stmt)
        else:
            raise RuntimeError(f"Unknown statement: {type(stmt)}")

    async def _exec_assign(self, node: AssignmentNode):
        val = await self._eval(node.value)
        name = node.var_name.name
        if node.is_declaration:
            self.global_env.define(name, val, is_constant=(node.declaration_type == 'dec'))
        else:
            self.global_env.set(name, val)

    async def _exec_print(self, node: PrintNode):
        import sys
        values = [await self._eval(e) for e in node.expressions]
        sys.stdout.write(" ".join(
            "null" if v is None else ("true" if v is True else ("false" if v is False else str(v)))
            for v in values
        ) + "\n")

    async def _exec_if(self, node: IfNode):
        if await self._eval(node.condition):
            for stmt in node.then_body:
                await self._exec_stmt(stmt)
        else:
            for cond, body in node.elif_clauses:
                if await self._eval(cond):
                    for stmt in body:
                        await self._exec_stmt(stmt)
                    return
            if node.else_body:
                for stmt in node.else_body:
                    await self._exec_stmt(stmt)

    async def _exec_repeat(self, node: RepeatNode):
        while True:
            try:
                for stmt in node.body:
                    await self._exec_stmt(stmt)
            except BreakSignal:
                return
            except ContinueSignal:
                pass
            if await self._eval(node.condition):
                break

    def _exec_fundef(self, node: FunctionDefNode):
        fn = AsyncFunction(
            node.name.name,
            [p.name for p in node.params],
            node.body,
            closure=self.global_env
        )
        self.global_env.define(node.name.name, fn)

    async def _exec_index_assign(self, node: IndexAssignNode):
        obj = self.global_env.get(node.obj.name)
        idx = await self._eval(node.index)
        val = await self._eval(node.value)
        if isinstance(obj, list):
            obj[idx] = val
        else:
            raise RuntimeError(f"Cannot index-assign on {type(obj).__name__}")

    async def _exec_use(self, node: UseNode):
        from pathlib import Path
        from .lexer import Lexer
        from .parser import Parser
        raw = node.module_path
        candidates = [Path(raw), Path(raw + ".wyr"), Path("packages") / raw, Path("packages") / (raw + ".wyr")]
        target = next((p for p in candidates if p.exists() and p.is_file()), None)
        if not target:
            raise RuntimeError(f"Cannot find module '{raw}'")
        content = target.read_text(encoding="utf-8")
        tokens = Lexer(content).tokenize()
        ast = Parser(tokens).parse()
        for stmt in ast:
            await self._exec_stmt(stmt)

    # ------------------------------------------------------------------ #
    # Expression evaluator                                                  #
    # ------------------------------------------------------------------ #
    async def _eval(self, expr: ASTNode) -> Any:
        if isinstance(expr, NumberNode):
            return expr.value
        if isinstance(expr, StringNode):
            return expr.value
        if isinstance(expr, BooleanNode):
            return expr.value
        if isinstance(expr, NoneNode):
            return None
        if isinstance(expr, IdentifierNode):
            return self.global_env.get(expr.name)
        if isinstance(expr, BinaryOpNode):
            return await self._eval_binop(expr)
        if isinstance(expr, UnaryOpNode):
            operand = await self._eval(expr.expr)
            op = expr.op.value
            if op == '-': return -operand
            if op in ('!', 'not'): return not operand
            raise RuntimeError(f"Unknown unary op: {op}")
        if isinstance(expr, FunctionCallNode):
            return await self._eval_call(expr)
        if isinstance(expr, ListNode):
            return [await self._eval(e) for e in expr.elements]
        if isinstance(expr, IndexNode):
            obj = await self._eval(expr.obj)
            idx = await self._eval(expr.index)
            try:
                return obj[idx]
            except IndexError:
                raise RuntimeError(f"Index {idx} out of bounds")
        raise RuntimeError(f"Cannot evaluate: {type(expr)}")

    async def _eval_binop(self, node: BinaryOpNode) -> Any:
        left = await self._eval(node.left)
        right = await self._eval(node.right)
        op = node.op.value
        if op == '+': return left + right
        if op == '-': return left - right
        if op == '*': return left * right
        if op == '/':
            if right == 0: raise RuntimeError("Division by zero")
            return left / right
        if op == '//':
            if right == 0: raise RuntimeError("Division by zero")
            return left // right
        if op == '%':
            if right == 0: raise RuntimeError("Modulo by zero")
            return left % right
        if op == '==': return left == right
        if op == '!=': return left != right
        if op == '<':  return left < right
        if op == '>':  return left > right
        if op == '<=': return left <= right
        if op == '>=': return left >= right
        if op in ('and', '&&'): return bool(left) and bool(right)
        if op in ('or',  '||'): return bool(left) or bool(right)
        raise RuntimeError(f"Unknown operator: {op}")

    async def _eval_call(self, node: FunctionCallNode) -> Any:
        name = node.name.name

        # Special case: input() is async
        if name == 'input':
            args = [await self._eval(a) for a in node.args]
            prompt = str(args[0]) if args else ""
            if self._input_coro is not None:
                result = await self._input_coro(prompt)
                return result if result is not None else ""
            raise RuntimeError("input() is not available in this context")

        fn = self.global_env.get(name)
        args = [await self._eval(a) for a in node.args]

        if isinstance(fn, AsyncFunction):
            return await fn.call_async(args, self)
        if isinstance(fn, BuiltinFunction):
            result = fn.func(args)
            # Some builtins might return coroutines (unlikely but safe)
            if hasattr(result, '__await__'):
                return await result
            return result
        raise RuntimeError(f"'{name}' is not a function")
