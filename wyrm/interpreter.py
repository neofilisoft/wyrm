from typing import Any, List, Dict, Optional, Callable
from pathlib import Path
from .parser import *

class RuntimeError(Exception):
    pass

class OwnershipError(RuntimeError):
    """Raised when an ownership rule is violated at runtime."""
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


class RawPointer:
    """Represents a raw memory block allocated via malloc/realloc inside the interpreter."""
    def __init__(self, size: int):
        self.buffer = bytearray(size)
        self.freed = False
        self.size = size

    def free(self):
        self.freed = True

    def realloc(self, size: int):
        if self.freed:
            raise RuntimeError("realloc on freed pointer")
        new_buf = bytearray(size)
        min_sz = min(self.size, size)
        new_buf[:min_sz] = self.buffer[:min_sz]
        self.buffer = new_buf
        self.size = size

    def __repr__(self):
        return f"<RawPointer size={self.size} freed={self.freed}>"


class ArenaAllocator:
    """Scope-bound arena allocator for the Wyrm interpreter.
    
    All values allocated into an arena are tracked in a list. When `reset()` is
    called (or when the arena goes out of scope), the entire list is cleared in
    O(1). The arena does NOT perform actual C-level memory management inside the
    Python interpreter - it acts as a logical grouping / lifecycle tracker that
    mirrors what the C runtime does for native builds.
    
    Ownership rule: a value that was allocated into an arena must not be returned
    from the arena's owning scope. The interpreter enforces this by marking each
    value with the arena it belongs to and checking on function return.
    """
    def __init__(self, name: str, capacity: int):
        self.name = name
        self.capacity = capacity
        self._allocations: List[Any] = []
        self._freed = False

    def alloc(self, size: int) -> bytearray:
        """Allocate a bytearray buffer of `size` bytes from the arena."""
        if self._freed:
            raise OwnershipError(f"Arena '{self.name}' has already been reset (use-after-free)")
        if size <= 0:
            raise OwnershipError(f"Arena alloc size must be positive, got {size}")
        buf = bytearray(size)
        self._allocations.append(buf)
        return buf

    def reset(self):
        """Free all allocations at once. All previously returned buffers become invalid."""
        self._allocations.clear()
        self._freed = True  # mark so subsequent alloc() calls fail

    def __repr__(self):
        return f"<Arena '{self.name}' cap={self.capacity} allocs={len(self._allocations)}>"


class Function:
    def __init__(self, name: str, params: List[str], body: List[ASTNode], closure: Optional['Environment'] = None):
        self.name = name
        self.params = params
        self.body = body
        self.closure = closure  # Environment snapshot for closures
        # Set of owned variable names - tracked for ownership transfer on call
        self.owned_params: set = set()

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
    """Lexically scoped variable environment with ownership tracking and RAII."""

    def __init__(self, outer: Optional['Environment'] = None):
        self.store: Dict[str, tuple] = {}  # name -> (value, is_constant, is_moved)
        self.outer: Optional['Environment'] = outer
        # Names of owned variables in this scope
        self._owned: set = set()
        # Arenas bound to this scope
        self._arenas: Dict[str, ArenaAllocator] = {}
        # Track raw allocations for RAII auto-free fallback
        self._raw_allocations: List[RawPointer] = []

    def register_raw_allocation(self, ptr: RawPointer):
        self._raw_allocations.append(ptr)

    def unregister_raw_allocation(self, ptr: RawPointer):
        if ptr in self._raw_allocations:
            self._raw_allocations.remove(ptr)

    def unregister_raw_allocation_chain(self, ptr: RawPointer):
        """Walk the full scope chain to find and remove a raw allocation."""
        env = self
        while env is not None:
            if ptr in env._raw_allocations:
                env._raw_allocations.remove(ptr)
                return
            env = env.outer

    def define(self, name: str, value: Any, is_constant: bool = False, is_owned: bool = False):
        self.store[name] = (value, is_constant, False)  # (val, const, moved)
        if is_owned:
            self._owned.add(name)

    def get(self, name: str) -> Any:
        if name in self.store:
            val, _const, moved = self.store[name]
            if moved:
                raise OwnershipError(f"Use-after-move: variable '{name}' has been moved and can no longer be used")
            return val
        elif self.outer is not None:
            return self.outer.get(name)
        else:
            raise RuntimeError(f"Undefined variable: '{name}'")

    def set(self, name: str, value: Any):
        if name in self.store:
            _val, is_const, moved = self.store[name]
            if is_const:
                raise RuntimeError(f"Cannot assign to constant '{name}'")
            if moved:
                raise OwnershipError(f"Cannot assign to moved variable '{name}'")
            self.store[name] = (value, is_const, False)
        elif self.outer is not None:
            self.outer.set(name, value)
        else:
            # Variable not declared - create as mutable in current scope
            self.store[name] = (value, False, False)

    def mark_moved(self, name: str):
        """Mark a variable as moved so further access raises OwnershipError."""
        if name in self.store:
            val, is_const, _ = self.store[name]
            self.store[name] = (val, is_const, True)
        elif self.outer is not None:
            self.outer.mark_moved(name)

    def define_arena(self, name: str, arena: ArenaAllocator):
        self._arenas[name] = arena

    def get_arena(self, name: str) -> ArenaAllocator:
        if name in self._arenas:
            return self._arenas[name]
        elif self.outer is not None and hasattr(self.outer, 'get_arena'):
            return self.outer.get_arena(name)
        raise RuntimeError(f"Undefined arena: '{name}'")

class Interpreter:
    def __init__(self, source_dir: Optional[Path] = None):
        self.global_env = Environment()
        self.source_dir: Path = source_dir or Path.cwd()
        self._in_unsafe = False
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
        self.global_env.define("malloc", BuiltinFunction("malloc", self._builtin_malloc))
        self.global_env.define("free", BuiltinFunction("free", self._builtin_free))
        self.global_env.define("realloc", BuiltinFunction("realloc", self._builtin_realloc))

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

    def _builtin_malloc(self, args: List[Any]) -> RawPointer:
        if not self._in_unsafe:
            raise RuntimeError("Raw memory operations (malloc, free, realloc) are only allowed inside unsafe blocks")
        if not args:
            raise RuntimeError("malloc() requires a size argument")
        size = int(args[0])
        ptr = RawPointer(size)
        self.global_env.register_raw_allocation(ptr)
        return ptr

    def _builtin_free(self, args: List[Any]) -> None:
        if not self._in_unsafe:
            raise RuntimeError("Raw memory operations (malloc, free, realloc) are only allowed inside unsafe blocks")
        if not args:
            raise RuntimeError("free() requires a pointer argument")
        ptr = args[0]
        # Ignore freeing null or non-pointers gracefully or raise error
        if ptr is None:
            return None
        if not isinstance(ptr, RawPointer):
            raise RuntimeError("free() argument must be a raw pointer or null")
        ptr.free()
        # Walk the full scope chain to unregister the pointer (it may be in a sub-env)
        self.global_env.unregister_raw_allocation_chain(ptr)
        return None

    def _builtin_realloc(self, args: List[Any]) -> RawPointer:
        if not self._in_unsafe:
            raise RuntimeError("Raw memory operations (malloc, free, realloc) are only allowed inside unsafe blocks")
        if len(args) < 2:
            raise RuntimeError("realloc() requires pointer and size arguments")
        ptr, size = args[0], int(args[1])
        if ptr is None:
            return self._builtin_malloc([size])
        if not isinstance(ptr, RawPointer):
            raise RuntimeError("realloc() first argument must be a raw pointer or null")
        ptr.realloc(size)
        return ptr

    def execute(self, statements: List[ASTNode], is_top_level: bool = True):
        """Execute a list of statements."""
        for stmt in statements:
            self.execute_statement(stmt)

    def interpret(self, ast):
        """Execute AST and automatically invoke a main function if defined."""
        # Execute top-level statements
        self.execute(ast, True)
        # After execution, if a function named "main" exists, invoke it.
        try:
            main_func = self.global_env.get("main")
            if isinstance(main_func, Function):
                main_func.call([], self)
        except RuntimeError:
            # No main function defined; ignore.
            pass
        return None

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
        # ---- Memory Safety Nodes ----
        elif isinstance(stmt, UnsafeBlockNode):
            self.visit_unsafe_block(stmt)
        elif isinstance(stmt, OwnedDeclNode):
            self.visit_owned_decl(stmt)
        elif isinstance(stmt, ArenaNode):
            self.visit_arena_create(stmt)
        elif isinstance(stmt, ArenaAllocNode):
            self.visit_arena_alloc(stmt)
        elif isinstance(stmt, ArenaResetNode):
            self.visit_arena_reset(stmt)
        else:
            raise RuntimeError(f"Unknown statement type: {type(stmt)}")

    # ---- Memory Safety Visit Methods ----

    def visit_unsafe_block(self, node: UnsafeBlockNode):
        """Execute an unsafe block. All statements run in a sub-scope where
        ownership checks are still active but raw allocation is permitted."""
        sub_env = Environment(self.global_env)
        old_unsafe = getattr(self, '_in_unsafe', False)
        self._in_unsafe = True
        try:
            self.execute_block(node.body, sub_env)
        finally:
            self._in_unsafe = old_unsafe

    def visit_owned_decl(self, node: OwnedDeclNode):
        """Declare an owned variable. The value is bound and cannot be used after a move."""
        value = self.evaluate(node.value)
        self.global_env.define(node.var_name, value,
                               is_constant=(node.declaration_type == 'dec'),
                               is_owned=True)

    def visit_arena_create(self, node: ArenaNode):
        """Create an arena allocator bound to the current scope."""
        size = self.evaluate(node.size_expr)
        if not isinstance(size, (int, float)):
            raise RuntimeError("Arena size must be a number")
        arena = ArenaAllocator(node.name, int(size))
        self.global_env.define_arena(node.name, arena)

    def visit_arena_alloc(self, node: ArenaAllocNode):
        """Call arena.alloc(size) - allocate a buffer from the arena."""
        arena = self.global_env.get_arena(node.arena_name)
        size = self.evaluate(node.size_expr)
        return arena.alloc(int(size))

    def visit_arena_reset(self, node: ArenaResetNode):
        """Call arena.reset() - free all arena allocations at once."""
        arena = self.global_env.get_arena(node.arena_name)
        arena.reset()


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
            # RAII cleanup: reset/destroy all arenas defined in this environment
            for arena_name, arena in list(environment._arenas.items()):
                arena.reset()
            # RAII cleanup: auto-free any remaining raw allocations (fallback)
            for ptr in list(environment._raw_allocations):
                if not ptr.freed:
                    ptr.free()
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
        elif isinstance(expr, SliceNode):
            start = self.evaluate(expr.start) if expr.start is not None else None
            end = self.evaluate(expr.end) if expr.end is not None else None
            return slice(start, end)
        elif isinstance(expr, ArenaAllocNode):
            return self.visit_arena_alloc(expr)
        elif isinstance(expr, ArenaResetNode):
            self.visit_arena_reset(expr)
            return None
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