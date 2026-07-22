from typing import Any, List, Dict, Optional, Callable
from pathlib import Path
from .parser import *

class RuntimeError(Exception):
    pass

class ReturnSignal(Exception):
    def __init__(self, value):
        self.value = value

class BreakSignal(Exception):
    """Raised when a break statement is executed inside a loop."""
    pass

class ContinueSignal(Exception):
    """Raised when a continue statement is executed inside a loop."""
    pass

class Function:
    def __init__(self, name: str, params: List[str], body: List[ASTNode], closure: Optional['Environment'] = None):
        self.name = name
        self.params = params
        self.body = body
        self.closure = closure  # Environment snapshot for closures

    def call(self, args: List[Any], interpreter: 'Interpreter') -> Any:
        # Create a new environment chained to the closure
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
    """Lexically scoped variable environment."""

    def __init__(self, outer: Optional['Environment'] = None):
        self.store: Dict[str, tuple] = {}  # name -> (value, is_constant)
        self.outer: Optional['Environment'] = outer

    def define(self, name: str, value: Any, is_constant: bool = False):
        self.store[name] = (value, is_constant)

    def get(self, name: str) -> Any:
        if name in self.store:
            return self.store[name][0]
        elif self.outer is not None:
            return self.outer.get(name)
        else:
            raise RuntimeError(f"Undefined variable: '{name}'")

    def set(self, name: str, value: Any):
        if name in self.store:
            if self.store[name][1]:  # is_constant flag
                raise RuntimeError(f"Cannot assign to constant '{name}'")
            self.store[name] = (value, self.store[name][1])
        elif self.outer is not None:
            self.outer.set(name, value)
        else:
            # Variable not declared - create as mutable in current scope
            self.store[name] = (value, False)


class Interpreter:
    def __init__(self, source_dir: Optional[Path] = None):
        self.global_env = Environment()
        self.source_dir: Path = source_dir or Path.cwd()
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
        self.global_env.define("append", BuiltinFunction("append", self._builtin_append))
        self.global_env.define("pop", BuiltinFunction("pop", self._builtin_pop))

        # Boolean values
        self.global_env.define("true", True)
        self.global_env.define("false", False)
        self.global_env.define("null", None)

    def _builtin_print(self, args: List[Any]) -> None:
        print(" ".join(str(arg) if arg is not None else "null" for arg in args))
        return None

    def _builtin_input(self, args: List[Any]) -> str:
        prompt = args[0] if args else ""
        return input(str(prompt))

    def _builtin_int(self, args: List[Any]) -> int:
        if not args:
            raise RuntimeError("int() requires an argument")
        return int(args[0])

    def _builtin_float(self, args: List[Any]) -> float:
        if not args:
            raise RuntimeError("float() requires an argument")
        return float(args[0])

    def _builtin_str(self, args: List[Any]) -> str:
        if not args:
            raise RuntimeError("str() requires an argument")
        obj = args[0]
        if obj is None:
            return "null"
        if isinstance(obj, bool):
            return "true" if obj else "false"
        return str(obj)

    def _builtin_len(self, args: List[Any]) -> int:
        if not args:
            raise RuntimeError("len() requires an argument")
        obj = args[0]
        if hasattr(obj, '__len__'):
            return len(obj)
        raise RuntimeError(f"Object of type {type(obj).__name__} has no len()")

    def _builtin_type(self, args: List[Any]) -> str:
        if not args:
            raise RuntimeError("type() requires an argument")
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
        if not args:
            raise RuntimeError("abs() requires an argument")
        return abs(args[0])

    def _builtin_max(self, args: List[Any]) -> Any:
        if not args:
            raise RuntimeError("max() requires at least one argument")
        return max(args)

    def _builtin_min(self, args: List[Any]) -> Any:
        if not args:
            raise RuntimeError("min() requires at least one argument")
        return min(args)

    def _builtin_round(self, args: List[Any]) -> float:
        if not args:
            raise RuntimeError("round() requires an argument")
        return round(args[0])

    def _builtin_pow(self, args: List[Any]) -> float:
        if len(args) < 2:
            raise RuntimeError("pow() requires two arguments")
        return pow(args[0], args[1])

    def _builtin_append(self, args: List[Any]) -> None:
        if len(args) < 2:
            raise RuntimeError("append() requires list and value arguments")
        lst, val = args[0], args[1]
        if not isinstance(lst, list):
            raise RuntimeError(f"append() first argument must be a list, got {type(lst).__name__}")
        lst.append(val)
        return None

    def _builtin_pop(self, args: List[Any]) -> Any:
        if not args:
            raise RuntimeError("pop() requires a list argument")
        lst = args[0]
        if not isinstance(lst, list):
            raise RuntimeError(f"pop() argument must be a list, got {type(lst).__name__}")
        if not lst:
            raise RuntimeError("pop() on empty list")
        return lst.pop()

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
                    pass

    def interpret(self, ast):
        """Alias for execute method for compatibility with main.py."""
        return self.execute(ast, True)

    def execute_statement(self, stmt: ASTNode):
        """Execute a single statement."""
        if isinstance(stmt, (NumberNode, StringNode, BooleanNode, NoneNode)):
            pass  # Literals as statements are no-ops
        elif isinstance(stmt, IdentifierNode):
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
            self.visit_return(stmt)
        elif isinstance(stmt, BreakNode):
            raise BreakSignal()
        elif isinstance(stmt, ContinueNode):
            raise ContinueSignal()
        elif isinstance(stmt, ListNode):
            pass  # List literals as statements are no-ops
        elif isinstance(stmt, IndexNode):
            self.evaluate(stmt)
        elif isinstance(stmt, IndexAssignNode):
            self.visit_index_assign(stmt)
        elif isinstance(stmt, UseNode):
            self.visit_use(stmt)
        else:
            raise RuntimeError(f"Unknown statement type: {type(stmt)}")

    def visit_use(self, stmt: UseNode):
        """Execute a use (import) statement, resolving paths correctly."""
        raw_path = stmt.module_path

        # Build search paths in priority order:
        # 1. Relative to current source file's directory
        # 2. Global installed packages (~/.wyrm/packages)
        # 3. Local packages folder (CWD/packages) - dev fallback
        global_pkg_dir = Path.home() / ".wyrm" / "packages"

        possible_paths = [
            # Relative to source file
            self.source_dir / raw_path,
            self.source_dir / (raw_path + ".wyr"),
            self.source_dir / "packages" / raw_path,
            self.source_dir / "packages" / (raw_path + ".wyr"),
            self.source_dir / "packages" / raw_path / "mod.wyr",
            self.source_dir / "packages" / raw_path / "main.wyr",
            # Global installed packages
            global_pkg_dir / raw_path,
            global_pkg_dir / (raw_path + ".wyr"),
            global_pkg_dir / raw_path / "mod.wyr",
            global_pkg_dir / raw_path / "main.wyr",
            # CWD fallback
            Path(raw_path),
            Path(raw_path + ".wyr"),
            Path("packages") / raw_path,
            Path("packages") / (raw_path + ".wyr"),
            Path("packages") / raw_path / "mod.wyr",
            Path("packages") / raw_path / "main.wyr",
        ]

        target = None
        for p in possible_paths:
            if p.exists() and p.is_file():
                target = p
                break

        if not target:
            raise RuntimeError(f"Cannot find module '{raw_path}'")

        content = target.read_text(encoding="utf-8")

        # Save/restore source_dir so nested imports work correctly
        old_source_dir = self.source_dir
        self.source_dir = target.parent.resolve()

        try:
            from .lexer import Lexer
            from .parser import Parser
            tokens = Lexer(content).tokenize()
            ast = Parser(tokens).parse()
            for node in ast:
                self.execute_statement(node)
        finally:
            self.source_dir = old_source_dir

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
        return self.global_env.get(node.name)

    def visit_binary_op(self, node: BinaryOpNode) -> Any:
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
        elif op == '//':
            if right == 0:
                raise RuntimeError("Division by zero")
            return left // right
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
        elif op in ('and', '&&'):
            return bool(left) and bool(right)
        elif op in ('or', '||'):
            return bool(left) or bool(right)
        else:
            raise RuntimeError(f"Unknown operator: {op}")

    def visit_unary_op(self, node: UnaryOpNode) -> Any:
        operand = self.evaluate(node.expr)
        op = node.op.value

        if op == '-':
            return -operand
        elif op in ('!', 'not'):
            return not operand
        else:
            raise RuntimeError(f"Unknown unary operator: {op}")

    def visit_assignment(self, node: AssignmentNode):
        value = self.evaluate(node.value)
        var_name = node.var_name.name

        if node.is_declaration:
            is_constant = node.declaration_type == 'dec'
            self.global_env.define(var_name, value, is_constant=is_constant)
        else:
            self.global_env.set(var_name, value)

    def visit_print(self, node: PrintNode):
        if hasattr(node, 'expressions') and node.expressions:
            values = [self.evaluate(expr) for expr in node.expressions]
            print(" ".join("null" if v is None else ("true" if v is True else ("false" if v is False else str(v))) for v in values))
        elif hasattr(node, 'expression') and node.expression is not None:
            value = self.evaluate(node.expression)
            print("null" if value is None else str(value))

    def visit_if(self, node: IfNode):
        condition = self.evaluate(node.condition)
        if condition:
            self.execute(node.then_body, False)
        else:
            for elif_condition, elif_body in node.elif_clauses:
                if self.evaluate(elif_condition):
                    self.execute(elif_body, False)
                    return
            if node.else_body:
                self.execute(node.else_body, False)

    def visit_repeat(self, node: RepeatNode):
        """Execute a repeat/til loop - runs body then checks condition (do-while style).
        
        break: exits the loop immediately
        continue: skips rest of body, goes to condition check
        """
        while True:
            try:
                self.execute(node.body, False)
            except BreakSignal:
                return  # Exit loop
            except ContinueSignal:
                pass  # Skip to condition check

            if self.evaluate(node.condition):
                break  # condition met - stop repeating

    def visit_return(self, node: ReturnNode) -> None:
        value = self.evaluate(node.expr)
        raise ReturnSignal(value)

    def visit_function_def(self, node: FunctionDefNode):
        func = Function(
            node.name.name,
            [p.name for p in node.params],
            node.body,
            closure=self.global_env  # capture current environment as closure
        )
        self.global_env.define(node.name.name, func)

    def visit_function_call(self, node: FunctionCallNode) -> Any:
        func_name = node.name.name
        func = self.global_env.get(func_name)

        args = [self.evaluate(arg) for arg in node.args]

        if isinstance(func, Function):
            return func.call(args, self)
        elif isinstance(func, BuiltinFunction):
            return func.func(args)
        else:
            raise RuntimeError(f"'{func_name}' is not a function")

    def visit_list(self, node: ListNode) -> List[Any]:
        return [self.evaluate(element) for element in node.elements]

    def visit_index(self, node: IndexNode) -> Any:
        obj = self.evaluate(node.obj)
        index = self.evaluate(node.index)

        if isinstance(obj, (str, list)):
            try:
                return obj[index]
            except IndexError:
                raise RuntimeError(f"Index {index} out of bounds (size {len(obj)})")
        else:
            raise RuntimeError(f"Object of type {type(obj).__name__} is not subscriptable")

    def visit_index_assign(self, node) -> None:
        obj = self.global_env.get(node.obj.name)
        index = self.evaluate(node.index)
        value = self.evaluate(node.value)
        if isinstance(obj, list):
            try:
                obj[index] = value
            except IndexError:
                raise RuntimeError(f"Index {index} out of bounds (size {len(obj)})")
        elif isinstance(obj, str):
            raise RuntimeError("Strings are immutable - cannot assign to index")
        else:
            raise RuntimeError(f"Object of type {type(obj).__name__} does not support item assignment")


class BuiltinFunction:
    def __init__(self, name: str, func: Callable[[List[Any]], Any]):
        self.name = name
        self.func = func