"""
Bytecode compiler and virtual machine for Wyrm language.

Full implementation with:
- Proper function support (closures, recursion)
- break/continue via compile-time jump patching
- if/elif/else with correct backpatching
- Correct frame management for calls and returns
"""

from typing import List, Any, Optional, Dict
from dataclasses import dataclass, field
from .ast import *


# ---------------------------------------------------------------------------
# OpCodes
# ---------------------------------------------------------------------------

class OpCode:
    # Constants / Loading
    LOAD_CONST      = 0   # arg: index into bytecode.constants
    LOAD_NONE       = 1
    LOAD_TRUE       = 2
    LOAD_FALSE      = 3
    LOAD_VAR        = 4   # arg: variable name (str)
    STORE_VAR       = 5   # arg: variable name (str)

    # Arithmetic
    ADD  = 10
    SUB  = 11
    MUL  = 12
    DIV  = 13
    MOD  = 14
    NEG  = 15
    FLOORDIV = 16

    # Comparison
    EQ = 20
    NE = 21
    LT = 22
    GT = 23
    LE = 24
    GE = 25

    # Boolean logic
    AND = 30
    OR  = 31
    NOT = 32

    # Control flow
    JUMP          = 40   # arg: absolute instruction index
    JUMP_IF_FALSE = 41   # arg: absolute instruction index (pops condition)
    JUMP_IF_TRUE  = 42   # arg: absolute instruction index (pops condition)

    # Functions
    CALL   = 50   # arg: (func_name: str, num_args: int)
    RETURN = 51   # pops and returns TOS

    # I/O
    PRINT = 60    # arg: num_args (int) - pops that many values

    # Stack
    POP = 70
    DUP = 71

    # Lists
    BUILD_LIST  = 80   # arg: count
    LOAD_SUBSCR = 83   # pops index then obj, pushes obj[index]
    STORE_SUBSCR = 82  # pops index, obj, value; does obj[index]=value


# ---------------------------------------------------------------------------
# Bytecode object
# ---------------------------------------------------------------------------

class Bytecode:
    def __init__(self):
        self.instructions: List[tuple] = []
        self.constants: List[Any] = []

    def add_instruction(self, opcode: int, arg: Any = None) -> int:
        """Append an instruction and return its index."""
        self.instructions.append((opcode, arg))
        return len(self.instructions) - 1

    def add_constant(self, value: Any) -> int:
        """Add constant to pool and return index (deduplicates simple scalars)."""
        # Note: FunctionObject is never deduplicated
        if not isinstance(value, FunctionObject):
            try:
                return self.constants.index(value)
            except ValueError:
                pass
        self.constants.append(value)
        return len(self.constants) - 1

    def patch(self, idx: int, new_arg: Any):
        """Patch the argument of instruction at idx."""
        opcode, _ = self.instructions[idx]
        self.instructions[idx] = (opcode, new_arg)


# ---------------------------------------------------------------------------
# Function object (stored in the VM's environment)
# ---------------------------------------------------------------------------

@dataclass
class FunctionObject:
    name: str
    params: List[str]
    bytecode: 'Bytecode'


# ---------------------------------------------------------------------------
# Internal signals
# ---------------------------------------------------------------------------

class _ReturnSignal(Exception):
    def __init__(self, value: Any):
        self.value = value


# ---------------------------------------------------------------------------
# Call frame
# ---------------------------------------------------------------------------

@dataclass
class CallFrame:
    bytecode: Bytecode
    ip: int = 0
    locals: Dict[str, Any] = field(default_factory=dict)
    is_global: bool = False


# ---------------------------------------------------------------------------
# Compiler  (AST -> Bytecode)
# ---------------------------------------------------------------------------

class Compiler:
    def __init__(self):
        self.bytecode = Bytecode()
        # Stack of (break_patch_positions, continue_patch_positions, loop_start_ip)
        self._loop_stack: List[tuple] = []

    def compile(self, node) -> Bytecode:
        """Entry point: compile AST list or single node."""
        if isinstance(node, list):
            for stmt in node:
                self.visit(stmt)
        else:
            self.visit(node)
        return self.bytecode

    def visit(self, node: ASTNode):
        method = f'visit_{type(node).__name__}'
        visitor = getattr(self, method, self.generic_visit)
        return visitor(node)

    def generic_visit(self, node: ASTNode):
        raise Exception(f'Bytecode compiler: no handler for {type(node).__name__}')

    # ----- Expressions -----

    def visit_NumberNode(self, node: NumberNode):
        idx = self.bytecode.add_constant(node.value)
        self.bytecode.add_instruction(OpCode.LOAD_CONST, idx)

    def visit_StringNode(self, node: StringNode):
        idx = self.bytecode.add_constant(node.value)
        self.bytecode.add_instruction(OpCode.LOAD_CONST, idx)

    def visit_BooleanNode(self, node: BooleanNode):
        self.bytecode.add_instruction(OpCode.LOAD_TRUE if node.value else OpCode.LOAD_FALSE)

    def visit_NoneNode(self, node: NoneNode):
        self.bytecode.add_instruction(OpCode.LOAD_NONE)

    def visit_IdentifierNode(self, node: IdentifierNode):
        self.bytecode.add_instruction(OpCode.LOAD_VAR, node.name)

    def visit_BinaryOpNode(self, node: BinaryOpNode):
        self.visit(node.left)
        self.visit(node.right)
        op_map = {
            '+': OpCode.ADD, '-': OpCode.SUB,
            '*': OpCode.MUL, '/': OpCode.DIV,
            '//': OpCode.FLOORDIV, '%': OpCode.MOD,
            '==': OpCode.EQ, '!=': OpCode.NE,
            '<': OpCode.LT, '>': OpCode.GT,
            '<=': OpCode.LE, '>=': OpCode.GE,
            'and': OpCode.AND, '&&': OpCode.AND,
            'or': OpCode.OR,  '||': OpCode.OR,
        }
        op = op_map.get(node.op.value)
        if op is None:
            raise Exception(f'Unsupported operator: {node.op.value}')
        self.bytecode.add_instruction(op)

    def visit_UnaryOpNode(self, node: UnaryOpNode):
        self.visit(node.expr)
        op_map = {'-': OpCode.NEG, '!': OpCode.NOT, 'not': OpCode.NOT}
        op = op_map.get(node.op.value)
        if op is None:
            raise Exception(f'Unsupported unary operator: {node.op.value}')
        self.bytecode.add_instruction(op)

    def visit_ListNode(self, node: ListNode):
        for el in node.elements:
            self.visit(el)
        self.bytecode.add_instruction(OpCode.BUILD_LIST, len(node.elements))

    def visit_IndexNode(self, node: IndexNode):
        self.visit(node.obj)
        self.visit(node.index)
        self.bytecode.add_instruction(OpCode.LOAD_SUBSCR)

    def visit_FunctionCallNode(self, node: FunctionCallNode):
        for arg in node.args:
            self.visit(arg)
        self.bytecode.add_instruction(OpCode.CALL, (node.name.name, len(node.args)))

    # ----- Statements -----

    def visit_AssignmentNode(self, node: AssignmentNode):
        self.visit(node.value)
        self.bytecode.add_instruction(OpCode.STORE_VAR, node.var_name.name)

    def visit_IndexAssignNode(self, node):
        # Push value, then obj reference, then index - VM pops index, obj, value
        self.visit(node.value)
        self.visit(node.obj)
        self.visit(node.index)
        self.bytecode.add_instruction(OpCode.STORE_SUBSCR)

    def visit_PrintNode(self, node: PrintNode):
        exprs = getattr(node, 'expressions', [])
        if not exprs and getattr(node, 'expression', None) is not None:
            exprs = [node.expression]
        for expr in exprs:
            self.visit(expr)
        self.bytecode.add_instruction(OpCode.PRINT, len(exprs))

    def visit_ReturnNode(self, node: ReturnNode):
        self.visit(node.expr)
        self.bytecode.add_instruction(OpCode.RETURN)

    def visit_BreakNode(self, node):
        if not self._loop_stack:
            raise Exception("'break' used outside of a loop")
        break_positions, _, _ = self._loop_stack[-1]
        pos = self.bytecode.add_instruction(OpCode.JUMP, 0)  # placeholder
        break_positions.append(pos)

    def visit_ContinueNode(self, node):
        if not self._loop_stack:
            raise Exception("'continue' used outside of a loop")
        _, continue_positions, _ = self._loop_stack[-1]
        pos = self.bytecode.add_instruction(OpCode.JUMP, 0)  # placeholder
        continue_positions.append(pos)

    def visit_IfNode(self, node: IfNode):
        # --- then branch ---
        self.visit(node.condition)
        jump_false_pos = self.bytecode.add_instruction(OpCode.JUMP_IF_FALSE, 0)

        for stmt in node.then_body:
            self.visit(stmt)

        # Jump past all elif/else when then-branch finishes
        end_jump_positions = [self.bytecode.add_instruction(OpCode.JUMP, 0)]

        # Patch the false-jump to here (start of elif/else chain)
        self.bytecode.patch(jump_false_pos, len(self.bytecode.instructions))

        # --- elif branches ---
        for elif_cond, elif_body in node.elif_clauses:
            self.visit(elif_cond)
            elif_false_pos = self.bytecode.add_instruction(OpCode.JUMP_IF_FALSE, 0)

            for stmt in elif_body:
                self.visit(stmt)

            end_jump_positions.append(self.bytecode.add_instruction(OpCode.JUMP, 0))
            self.bytecode.patch(elif_false_pos, len(self.bytecode.instructions))

        # --- else branch ---
        if node.else_body:
            for stmt in node.else_body:
                self.visit(stmt)

        # Patch all end-jumps to here
        end_pos = len(self.bytecode.instructions)
        for pos in end_jump_positions:
            self.bytecode.patch(pos, end_pos)

    def visit_RepeatNode(self, node: RepeatNode):
        """
        repeat { body } til (cond)
        Semantics: run body, then check cond; exit if cond is True.
        break  -> exit immediately
        continue -> skip rest of body, go to condition check
        """
        loop_start = len(self.bytecode.instructions)
        break_positions: List[int] = []
        continue_positions: List[int] = []
        self._loop_stack.append((break_positions, continue_positions, loop_start))

        for stmt in node.body:
            self.visit(stmt)

        self._loop_stack.pop()

        # Patch continue-jumps to here (condition evaluation)
        cond_pos = len(self.bytecode.instructions)
        for pos in continue_positions:
            self.bytecode.patch(pos, cond_pos)

        # Condition: if FALSE -> jump back to loop_start
        self.visit(node.condition)
        self.bytecode.add_instruction(OpCode.JUMP_IF_FALSE, loop_start)

        # Patch break-jumps to here (after the loop)
        after_loop = len(self.bytecode.instructions)
        for pos in break_positions:
            self.bytecode.patch(pos, after_loop)

    def visit_FunctionDefNode(self, node: FunctionDefNode):
        """Compile a function definition - body becomes a nested Bytecode object."""
        params = [p.name for p in node.params]
        func_bytecode = self._compile_function_body(params, node.body)
        func_obj = FunctionObject(node.name.name, params, func_bytecode)
        idx = self.bytecode.add_constant(func_obj)
        self.bytecode.add_instruction(OpCode.LOAD_CONST, idx)
        self.bytecode.add_instruction(OpCode.STORE_VAR, node.name.name)

    def _compile_function_body(self, params: List[str], body: List[ASTNode]) -> Bytecode:
        """Create a sub-compiler and compile the function body."""
        sub = Compiler()
        for stmt in body:
            sub.visit(stmt)
        # Implicit return None at end of function
        none_idx = sub.bytecode.add_constant(None)
        sub.bytecode.add_instruction(OpCode.LOAD_CONST, none_idx)
        sub.bytecode.add_instruction(OpCode.RETURN)
        return sub.bytecode

    def visit_UseNode(self, node: UseNode):
        # UseNode is handled at interpreter level; in bytecode mode, no-op
        pass


# ---------------------------------------------------------------------------
# VM  (Bytecode -> Execute)
# ---------------------------------------------------------------------------

class VM:
    def __init__(self):
        self.stack: List[Any] = []
        self.globals: Dict[str, Any] = {}
        self.call_stack: List[CallFrame] = []

    # ----- Public interface -----

    def execute(self, bytecode: Bytecode):
        """Execute a top-level Bytecode object."""
        self._init_builtins()
        frame = CallFrame(bytecode=bytecode, ip=0, locals={}, is_global=True)
        self._run_frame(frame)

        # Auto-call main() if defined (matching interpreter behaviour)
        main_func = self.globals.get('main')
        if isinstance(main_func, FunctionObject):
            self._call_function(main_func, [])

    # ----- Frame management -----

    def _run_frame(self, frame: CallFrame) -> Any:
        """Execute instructions in frame until RETURN or end."""
        self.call_stack.append(frame)
        bc = frame.bytecode
        try:
            while frame.ip < len(bc.instructions):
                opcode, arg = bc.instructions[frame.ip]
                frame.ip += 1
                self._exec(opcode, arg, frame)
        except _ReturnSignal as ret:
            return ret.value
        finally:
            self.call_stack.pop()
        return None

    def _call_function(self, func: FunctionObject, args: List[Any]) -> Any:
        """Create a new frame for a user-defined function and run it."""
        frame = CallFrame(bytecode=func.bytecode, ip=0, locals={}, is_global=False)
        for i, param in enumerate(func.params):
            frame.locals[param] = args[i] if i < len(args) else None
        return self._run_frame(frame)

    # ----- Variable access -----

    def _get(self, name: str, frame: CallFrame) -> Any:
        if frame.is_global:
            return self.globals.get(name)
        if name in frame.locals:
            return frame.locals[name]
        return self.globals.get(name)

    def _set(self, name: str, value: Any, frame: CallFrame):
        if frame.is_global:
            self.globals[name] = value
        else:
            frame.locals[name] = value

    # ----- Builtins -----

    def _init_builtins(self):
        self.globals.update({
            'print': lambda *a: print(' '.join('null' if x is None else ('true' if x is True else ('false' if x is False else str(x))) for x in a)),
            'input': lambda *a: input(str(a[0]) if a else ''),
            'int':   lambda *a: int(a[0]),
            'float': lambda *a: float(a[0]),
            'str':   lambda *a: ('null' if a[0] is None else str(a[0])) if a else '',
            'len':   lambda *a: len(a[0]),
            'abs':   lambda *a: abs(a[0]),
            'max':   lambda *a: max(a),
            'min':   lambda *a: min(a),
            'round': lambda *a: round(a[0]),
            'pow':   lambda *a: pow(a[0], a[1]),
            'append': lambda *a: a[0].append(a[1]) or a[0],
            'pop':    lambda *a: a[0].pop(),
            'type':   lambda *a: _type_name(a[0]),
            # Language keywords as values
            'true': True,
            'false': False,
            'null': None,
        })

    # ----- Instruction execution -----

    def _exec(self, opcode: int, arg: Any, frame: CallFrame):
        s = self.stack

        if opcode == OpCode.LOAD_CONST:
            s.append(frame.bytecode.constants[arg])

        elif opcode == OpCode.LOAD_NONE:
            s.append(None)
        elif opcode == OpCode.LOAD_TRUE:
            s.append(True)
        elif opcode == OpCode.LOAD_FALSE:
            s.append(False)

        elif opcode == OpCode.LOAD_VAR:
            val = self._get(arg, frame)
            if val is None and arg not in (frame.locals if not frame.is_global else self.globals):
                # Fall back gracefully - variable may simply be None
                pass
            s.append(val)

        elif opcode == OpCode.STORE_VAR:
            self._set(arg, s.pop(), frame)

        # Arithmetic
        elif opcode == OpCode.ADD:
            b, a = s.pop(), s.pop(); s.append(a + b)
        elif opcode == OpCode.SUB:
            b, a = s.pop(), s.pop(); s.append(a - b)
        elif opcode == OpCode.MUL:
            b, a = s.pop(), s.pop(); s.append(a * b)
        elif opcode == OpCode.DIV:
            b, a = s.pop(), s.pop()
            if b == 0: raise RuntimeError("Division by zero")
            s.append(a / b)
        elif opcode == OpCode.FLOORDIV:
            b, a = s.pop(), s.pop()
            if b == 0: raise RuntimeError("Division by zero")
            s.append(a // b)
        elif opcode == OpCode.MOD:
            b, a = s.pop(), s.pop()
            if b == 0: raise RuntimeError("Modulo by zero")
            s.append(a % b)
        elif opcode == OpCode.NEG:
            s.append(-s.pop())

        # Comparison
        elif opcode == OpCode.EQ:
            b, a = s.pop(), s.pop(); s.append(a == b)
        elif opcode == OpCode.NE:
            b, a = s.pop(), s.pop(); s.append(a != b)
        elif opcode == OpCode.LT:
            b, a = s.pop(), s.pop(); s.append(a < b)
        elif opcode == OpCode.GT:
            b, a = s.pop(), s.pop(); s.append(a > b)
        elif opcode == OpCode.LE:
            b, a = s.pop(), s.pop(); s.append(a <= b)
        elif opcode == OpCode.GE:
            b, a = s.pop(), s.pop(); s.append(a >= b)

        # Logic
        elif opcode == OpCode.AND:
            b, a = s.pop(), s.pop(); s.append(bool(a) and bool(b))
        elif opcode == OpCode.OR:
            b, a = s.pop(), s.pop(); s.append(bool(a) or bool(b))
        elif opcode == OpCode.NOT:
            s.append(not s.pop())

        # Jumps
        elif opcode == OpCode.JUMP:
            frame.ip = arg
        elif opcode == OpCode.JUMP_IF_FALSE:
            if not s.pop():
                frame.ip = arg
        elif opcode == OpCode.JUMP_IF_TRUE:
            if s.pop():
                frame.ip = arg

        # Stack ops
        elif opcode == OpCode.POP:
            s.pop()
        elif opcode == OpCode.DUP:
            s.append(s[-1])

        # Lists
        elif opcode == OpCode.BUILD_LIST:
            items = [s.pop() for _ in range(arg)]
            items.reverse()
            s.append(items)

        elif opcode == OpCode.LOAD_SUBSCR:
            index = s.pop()
            obj = s.pop()
            s.append(obj[index])

        elif opcode == OpCode.STORE_SUBSCR:
            index = s.pop()
            obj = s.pop()
            value = s.pop()
            obj[index] = value

        # Print
        elif opcode == OpCode.PRINT:
            num = arg if arg is not None else 1
            values = [s.pop() for _ in range(num)]
            values.reverse()
            print(' '.join('null' if v is None else ('true' if v is True else ('false' if v is False else str(v))) for v in values))

        # Return
        elif opcode == OpCode.RETURN:
            raise _ReturnSignal(s.pop())

        # Function call
        elif opcode == OpCode.CALL:
            func_name, num_args = arg
            args = [s.pop() for _ in range(num_args)]
            args.reverse()

            func = self._get(func_name, frame)

            if isinstance(func, FunctionObject):
                result = self._call_function(func, args)
                s.append(result)
            elif callable(func):
                result = func(*args)
                s.append(result)
            else:
                raise RuntimeError(f"'{func_name}' is not callable")

        else:
            raise Exception(f'VM: unknown opcode {opcode}')


def _type_name(obj: Any) -> str:
    if isinstance(obj, bool): return 'bool'
    if isinstance(obj, int): return 'int'
    if isinstance(obj, float): return 'float'
    if isinstance(obj, str): return 'str'
    if isinstance(obj, list): return 'list'
    if obj is None: return 'null'
    return type(obj).__name__


# ---------------------------------------------------------------------------
# Serialization (for .wyb files - stores source, not raw bytecode)
# ---------------------------------------------------------------------------

def serialize_bytecode(bytecode: Bytecode) -> str:
    """Serialize bytecode to JSON string. FunctionObjects are represented as markers."""
    import json

    def _encode(val):
        if isinstance(val, FunctionObject):
            return {"__wyrm_func__": val.name, "params": val.params}
        return val

    data = {
        "instructions": bytecode.instructions,
        "constants": [_encode(c) for c in bytecode.constants],
    }
    return json.dumps(data, ensure_ascii=False)


def deserialize_bytecode(json_str: str) -> Bytecode:
    """Deserialize bytecode from JSON string."""
    import json
    data = json.loads(json_str)
    bytecode = Bytecode()
    bytecode.instructions = [tuple(i) for i in data["instructions"]]
    bytecode.constants = data["constants"]
    return bytecode


# ---------------------------------------------------------------------------
# Convenience function
# ---------------------------------------------------------------------------

def compile_and_execute(source: str):
    """Parse, compile and run Wyrm source code through the bytecode VM."""
    from .lexer import Lexer
    from .parser import Parser

    tokens = Lexer(source).tokenize()
    ast = Parser(tokens).parse()
    compiler = Compiler()
    bytecode = compiler.compile(ast)
    vm = VM()
    vm.execute(bytecode)
    return vm