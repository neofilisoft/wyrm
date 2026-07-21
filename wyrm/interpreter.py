from typing import Any, List, Dict, Optional, Callable
from .parser import *

class RuntimeError(Exception):
    pass

class ReturnSignal(Exception):
    def __init__(self, value):
        self.value = value

class Function:
    def __init__(self, name: str, params: List[str], body: List[ASTNode], closure: Optional['Environment'] = None):
        self.name = name
        self.params = params
        self.body = body
        self.closure = closure or {}

    def call(self, args: List[Any], interpreter: 'Interpreter') -> Any:
        # Create a new environment for this function call
        env = Environment(self.closure)

        # Bind parameters to arguments
        for i, param in enumerate(self.params):
            if i < len(args):
                env.define(param, args[i])
            else:
                env.define(param, None)  # Default to None for missing args

        # Execute function body
        try:
            interpreter.execute_block(self.body, env)
        except ReturnSignal as ret:
            return ret.value

        return None  # Functions return None by default

class Environment:
    def __init__(self, outer: Optional[Dict[str, tuple]] = None):
        self.store: Dict[str, tuple] = {}  # Maps name to (value, is_constant)
        self.outer = outer or {}

    def define(self, name: str, value: Any, is_constant: bool = False):
        self.store[name] = (value, is_constant)

    def get(self, name: str) -> Any:
        if name in self.store:
            return self.store[name][0]  # Return just the value
        elif name in self.outer:
            # Handle outer environment - it might be in the old format or new format
            val = self.outer[name]
            if isinstance(val, tuple):
                return val[0]  # New format: (value, is_constant)
            else:
                return val  # Old format: just value
        else:
            raise RuntimeError(f"Undefined variable: '{name}'")

    def set(self, name: str, value: Any):
        if name in self.store:
            # Check if this variable is constant
            if self.store[name][1]:  # is_constant flag
                raise RuntimeError(f"Cannot assign to constant '{name}'")
            self.store[name] = (value, self.store[name][1])  # Keep the same constancy
        elif name in self.outer:
            # Handle outer environment - check if it's constant there
            val = self.outer[name]
            if isinstance(val, tuple):
                # New format: (value, is_constant)
                if val[1]:  # is_constant flag
                    raise RuntimeError(f"Cannot assign to constant '{name}'")
                self.outer[name] = (value, val[1])  # Keep the same constancy
            else:
                # Old format: just value (assume mutable for backward compatibility)
                self.outer[name] = value
        else:
            # Variable doesn't exist in either scope - treat as new mutable variable
            # In a language with declarations, this might be an error,
            # but for simplicity we'll treat it as defining a new mutable variable
            self.store[name] = (value, False)

class Interpreter:
    def __init__(self):
        self.global_env = Environment()
        self.setup_builtins()

    def setup_builtins(self):
        """Set up built-in functions and values."""
        self.global_env.define("print", BuiltinFunction("print", self._builtin_print))
        self.global_env.define("input", BuiltinFunction("input", self._builtin_input))
        self.global_env.define("int", BuiltinFunction("int", self._builtin_int))
        self.global_env.define("float", BuiltinFunction("float", self._builtin_float))
        self.global_env.define("str", BuiltinFunction("str", self._builtin_str))
        self.global_env.define("len", BuiltinFunction("len", self._builtin_len))
        self.global_env.define("type", BuiltinFunction("type", self._builtin_type))
        self.global_env.define("abs", BuiltinFunction("abs", self._builtin_abs))
        self.global_env.define("max", BuiltinFunction("max", self._builtin_max))
        self.global_env.define("min", BuiltinFunction("min", self._builtin_min))
        self.global_env.define("round", BuiltinFunction("round", self._builtin_round))
        self.global_env.define("pow", BuiltinFunction("pow", self._builtin_pow))

        # Boolean values
        self.global_env.define("true", True)
        self.global_env.define("false", False)
        self.global_env.define("null", None)

    def _builtin_print(self, args: List[Any]) -> None:
        print(" ".join(str(arg) for arg in args))
        return None

    def _builtin_input(self, args: List[Any]) -> str:
        prompt = args[0] if args else ""
        return input(str(prompt))

    def _builtin_int(self, args: List[Any]) -> int:
        return int(args[0])

    def _builtin_float(self, args: List[Any]) -> float:
        return float(args[0])

    def _builtin_str(self, args: List[Any]) -> str:
        obj = args[0]
        if obj is None:
            return "null"
        return str(obj)

    def _builtin_len(self, args: List[Any]) -> int:
        obj = args[0]
        if hasattr(obj, '__len__'):
            return len(obj)
        else:
            raise RuntimeError(f"Object of type {type(obj).__name__} has no len()")

    def _builtin_type(self, args: List[Any]) -> str:
        obj = args[0]
        if isinstance(obj, bool):
            return "bool"
        elif isinstance(obj, int):
            return "int"
        elif isinstance(obj, float):
            return "float"
        elif isinstance(obj, str):
            return "str"
        elif isinstance(obj, list):
            return "list"
        elif obj is None:
            return "null"
        else:
            return type(obj).__name__

    def _builtin_abs(self, args: List[Any]) -> float:
        return abs(args[0])

    def _builtin_max(self, args: List[Any]) -> Any:
        return max(args)

    def _builtin_min(self, args: List[Any]) -> Any:
        return min(args)

    def _builtin_round(self, args: List[Any]) -> float:
        return round(args[0])

    def _builtin_pow(self, args: List[Any]) -> float:
        return pow(args[0], args[1])

    def execute(self, statements: List[ASTNode], is_top_level: bool = True):
        """Execute a list of statements."""
        for stmt in statements:
            self.execute_statement(stmt)
        # Call main function if defined (only for top-level execution)
        if is_top_level:
            try:
                main_func = self.global_env.get("main")
            except RuntimeError:
                main_func = None
            if main_func is not None and isinstance(main_func, Function):
                try:
                    main_func.call([], self)
                except ReturnSignal:
                    # Ignore return value from main
                    pass


    def interpret(self, ast):
        """Alias for execute method for compatibility with main.py."""
        return self.execute(ast, True)

    def execute_statement(self, stmt: ASTNode):
        """Execute a single statement."""
        if isinstance(stmt, NumberNode):
            # Number literals as statements don't do anything
            pass
        elif isinstance(stmt, StringNode):
            # String literals as statements don't do anything
            pass
        elif isinstance(stmt, BooleanNode):
            # Boolean literals as statements don't do anything
            pass
        elif isinstance(stmt, NoneNode):
            # None literals as statements don't do anything
            pass
        elif isinstance(stmt, IdentifierNode):
            # Variable access as statement - just evaluate but don't store result
            self.evaluate(stmt)
        elif isinstance(stmt, BinaryOpNode):
            self.evaluate(stmt)
        elif isinstance(stmt, UnaryOpNode):
            self.evaluate(stmt)
        elif isinstance(stmt, AssignmentNode):
            self.visit_assignment(stmt)
        elif isinstance(stmt, PrintNode):
            self.visit_print(stmt)
        elif isinstance(stmt, IfNode):
            self.visit_if(stmt)
        elif isinstance(stmt, RepeatNode):
            self.visit_repeat(stmt)
        elif isinstance(stmt, FunctionDefNode):
            self.visit_function_def(stmt)
        elif isinstance(stmt, FunctionCallNode):
            self.visit_function_call(stmt)
        elif isinstance(stmt, ReturnNode):
            return self.visit_return(stmt)
        elif isinstance(stmt, ListNode):
            # List literals as statements don't do anything
            pass
        elif isinstance(stmt, IndexNode):
            self.evaluate(stmt)
        elif isinstance(stmt, UseNode):
            self.visit_use(stmt)
        else:
            raise RuntimeError(f"Unknown statement type: {type(stmt)}")

    def visit_use(self, stmt: UseNode):
        from pathlib import Path
        raw_path = stmt.module_path
        possible_paths = [
            Path(raw_path),
            Path(raw_path + ".wyr"),
            Path("packages") / raw_path,
            Path("packages") / (raw_path + ".wyr"),
            Path("packages") / raw_path / "mod.wyr",
            Path("packages") / raw_path / "main.wyr"
        ]
        target = None
        for p in possible_paths:
            if p.exists() and p.is_file():
                target = p
                break
        if not target:
            raise RuntimeError(f"Cannot find module '{raw_path}'")

        content = target.read_text(encoding="utf-8")
        from .lexer import Lexer
        from .parser import Parser
        tokens = Lexer(content).tokenize()
        ast = Parser(tokens).parse()
        for node in ast:
            self.execute_statement(node)

    def execute_block(self, statements: List[ASTNode], environment: Environment):

        """Execute a block of statements in a specific environment."""
        previous_env = self.global_env
        self.global_env = environment
        try:
            self.execute(statements, False)
        finally:
            self.global_env = previous_env

    def evaluate(self, expr: ASTNode) -> Any:
        """Evaluate an expression and return its value."""
        if isinstance(expr, NumberNode):
            return expr.value
        elif isinstance(expr, StringNode):
            return expr.value
        elif isinstance(expr, BooleanNode):
            return expr.value
        elif isinstance(expr, NoneNode):
            return expr.value
        elif isinstance(expr, IdentifierNode):
            return self.visit_identifier(expr)
        elif isinstance(expr, BinaryOpNode):
            return self.visit_binary_op(expr)
        elif isinstance(expr, UnaryOpNode):
            return self.visit_unary_op(expr)
        elif isinstance(expr, FunctionCallNode):
            return self.visit_function_call(expr)
        elif isinstance(expr, ListNode):
            return self.visit_list(expr)
        elif isinstance(expr, IndexNode):
            return self.visit_index(expr)
        else:
            raise RuntimeError(f"Cannot evaluate expression type: {type(expr)}")

    def visit_identifier(self, node: IdentifierNode) -> Any:
        """Visit an identifier node."""
        return self.global_env.get(node.name)

    def visit_binary_op(self, node: BinaryOpNode) -> Any:
        """Visit a binary operation node."""
        left = self.evaluate(node.left)
        right = self.evaluate(node.right)
        op = node.op.value

        if op == '+':
            return left + right
        elif op == '-':
            return left - right
        elif op == '*':
            return left * right
        elif op == '/':
            if right == 0:
                raise RuntimeError("Division by zero")
            return left / right
        elif op == '%':
            if right == 0:
                raise RuntimeError("Modulo by zero")
            return left % right
        elif op == '==':
            return left == right
        elif op == '!=':
            return left != right
        elif op == '<':
            return left < right
        elif op == '>':
            return left > right
        elif op == '<=':
            return left <= right
        elif op == '>=':
            return left >= right
        elif op == 'and':
            return bool(left) and bool(right)
        elif op == 'or':
            return bool(left) or bool(right)
        elif op == '&&':
            return bool(left) and bool(right)
        elif op == '||':
            return bool(left) or bool(right)
        else:
            raise RuntimeError(f"Unknown operator: {op}")

    def visit_unary_op(self, node: UnaryOpNode) -> Any:
        """Visit a unary operation node."""
        operand = self.evaluate(node.expr)
        op = node.op.value

        if op == '-':
            return -operand
        elif op == '!':
            return not operand
        elif op == 'not':
            return not operand
        else:
            raise RuntimeError(f"Unknown unary operator: {op}")

    def visit_assignment(self, node: AssignmentNode):
        """Visit an assignment node."""
        value = self.evaluate(node.value)
        var_name = node.var_name.name

        if node.is_declaration:
            # This is a variable declaration (var or dec)
            if node.declaration_type == 'dec':
                # 'dec' means constant/immutable
                self.global_env.define(var_name, value, is_constant=True)
            else:
                # 'var' or default means mutable
                self.global_env.define(var_name, value, is_constant=False)
        else:
            # This is a regular assignment (reassignment)
            self.global_env.set(var_name, value)

    def visit_print(self, node: PrintNode):
        """Visit a print node."""
        value = self.evaluate(node.expression)
        print(value)

    def visit_if(self, node: IfNode):
        """Visit an if node."""
        condition = self.evaluate(node.condition)
        if condition:
            self.execute(node.then_body, False)
        else:
            # Check elif clauses
            for condition, body in node.elif_clauses:
                if self.evaluate(condition):
                    self.execute(body, False)
                    return
            # If no elif matched, execute else block if present
            if node.else_body:
                self.execute(node.else_body, False)

    def visit_repeat(self, node: RepeatNode):
        """Visit a repeat/til loop node."""
        while True:
            self.execute(node.body, False)
            if self.evaluate(node.condition):
                break


    def visit_return(self, node: ReturnNode) -> None:
        """Handle return statement by raising ReturnSignal."""
        value = self.evaluate(node.expr)
        raise ReturnSignal(value)

    def visit_function_def(self, node: FunctionDefNode):
        """Visit a function definition node."""
        func = Function(node.name.name, [p.name for p in node.params], node.body, self.global_env.store)
        self.global_env.define(node.name.name, func)

    def visit_function_call(self, node: FunctionCallNode) -> Any:
        """Visit a function call node."""
        func_name = node.name.name
        func = self.global_env.get(func_name)

        if isinstance(func, Function):
            args = [self.evaluate(arg) for arg in node.args]
            return func.call(args, self)
        elif isinstance(func, BuiltinFunction):
            args = [self.evaluate(arg) for arg in node.args]
            return func.func(args)
        else:
            raise RuntimeError(f"'{func_name}' is not a function")

    def visit_list(self, node: ListNode) -> List[Any]:
        """Visit a list node."""
        return [self.evaluate(element) for element in node.elements]

    def visit_index(self, node: IndexNode) -> Any:
        """Visit an index node."""
        obj = self.evaluate(node.obj)
        index = self.evaluate(node.index)

        if isinstance(obj, (str, list)):
            return obj[index]
        else:
            raise RuntimeError(f"Object of type {type(obj).__name__} is not subscriptable")

class BuiltinFunction:
    def __init__(self, name: str, func: Callable[[List[Any]], Any]):
        self.name = name
        self.func = func