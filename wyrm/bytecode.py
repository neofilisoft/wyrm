"""
Bytecode compiler and virtual machine for Wyrm language.
"""

from typing import List, Any, Optional
from .ast import *

# Bytecode opcodes
class OpCode:
    # Constants and loading
    LOAD_CONST = 0
    LOAD_NONE = 1
    LOAD_TRUE = 2
    LOAD_FALSE = 3
    LOAD_VAR = 4
    STORE_VAR = 5
    LOAD_GLOBAL = 6
    STORE_GLOBAL = 7

    # Arithmetic
    ADD = 10
    SUB = 11
    MUL = 12
    DIV = 13
    MOD = 14
    NEG = 15
    POS = 16

    # Comparison
    EQ = 20
    NE = 21
    LT = 22
    GT = 23
    LE = 24
    GE = 25

    # Boolean logic
    AND = 30
    OR = 31
    NOT = 32

    # Control flow
    JUMP = 40
    JUMP_IF_FALSE = 41
    JUMP_IF_TRUE = 42

    # Function calls
    CALL = 50
    RETURN = 51
    YIELD = 52

    # Print and I/O
    PRINT = 60

    # Stack operations
    POP = 70
    DUP = 71

    # Lists
    BUILD_LIST = 80
    LIST_APPEND = 81
    STORE_SUBSCR = 82
    LOAD_SUBSCR = 83

# Instruction format: (opcode, arg)
# arg can be None for instructions that don't need arguments

class Bytecode:
    def __init__(self):
        self.instructions: List[tuple] = []  # List of (opcode, arg)
        self.constants: List[Any] = []       # Constants pool
        self.varnames: List[str] = []        # Variable names (locals)
        self.names: List[str] = []           # Global names

    def add_instruction(self, opcode: int, arg: Any = None):
        self.instructions.append((opcode, arg))

    def add_constant(self, value: Any) -> int:
        self.constants.append(value)
        return len(self.constants) - 1

    def add_varname(self, name: str) -> int:
        if name not in self.varnames:
            self.varnames.append(name)
        return self.varnames.index(name)

    def add_name(self, name: str) -> int:
        if name not in self.names:
            self.names.append(name)
        return self.names.index(name)


class Compiler:
    def __init__(self):
        self.bytecode = Bytecode()

    def compile(self, node):
        """Compile AST to bytecode"""
        if isinstance(node, list):
            for stmt in node:
                self.visit(stmt)
        else:
            self.visit(node)
        return self.bytecode

    def visit(self, node: ASTNode):
        """Dispatch to appropriate visitor method"""
        method_name = f'visit_{type(node).__name__}'
        visitor = getattr(self, method_name, self.generic_visit)
        return visitor(node)

    def generic_visit(self, node: ASTNode):
        raise Exception(f'No visitor method for {type(node).__name__}')

    # Visitor methods for each AST node type
    def visit_NumberNode(self, node: NumberNode):
        const_idx = self.bytecode.add_constant(node.value)
        self.bytecode.add_instruction(OpCode.LOAD_CONST, const_idx)

    def visit_StringNode(self, node: StringNode):
        const_idx = self.bytecode.add_constant(node.value)
        self.bytecode.add_instruction(OpCode.LOAD_CONST, const_idx)

    def visit_BooleanNode(self, node: BooleanNode):
        if node.value:
            self.bytecode.add_instruction(OpCode.LOAD_TRUE)
        else:
            self.bytecode.add_instruction(OpCode.LOAD_FALSE)

    def visit_NoneNode(self, node: NoneNode):
        self.bytecode.add_instruction(OpCode.LOAD_NONE)

    def visit_IdentifierNode(self, node: IdentifierNode):
        # For simplicity, treat all identifiers as variables
        # In a full implementation, we'd need to handle scoping properly
        var_index = self.bytecode.add_varname(node.name)
        self.bytecode.add_instruction(OpCode.LOAD_VAR, var_index)

    def visit_BinaryOpNode(self, node: BinaryOpNode):
        self.visit(node.left)
        self.visit(node.right)

        # Map operator to opcode
        op_map = {
            '+': OpCode.ADD,
            '-': OpCode.SUB,
            '*': OpCode.MUL,
            '/': OpCode.DIV,
            '%': OpCode.MOD,
            '==': OpCode.EQ,
            '!=': OpCode.NE,
            '<': OpCode.LT,
            '>': OpCode.GT,
            '<=': OpCode.LE,
            '>=': OpCode.GE,
            'and': OpCode.AND,
            'or': OpCode.OR,
        }

        op = op_map.get(node.op.value)
        if op is None:
            raise Exception(f'Unsupported operator: {node.op.value}')

        self.bytecode.add_instruction(op)

    def visit_UnaryOpNode(self, node: UnaryOpNode):
        self.visit(node.expr)

        op_map = {
            '-': OpCode.NEG,
            '+': OpCode.POS,
            '!': OpCode.NOT,
        }

        op = op_map.get(node.op.value)
        if op is None:
            raise Exception(f'Unsupported unary operator: {node.op.value}')

        self.bytecode.add_instruction(op)

    def visit_AssignmentNode(self, node: AssignmentNode):
        self.visit(node.value)
        var_index = self.bytecode.add_varname(node.var_name.name)
        self.bytecode.add_instruction(OpCode.STORE_VAR, var_index)

    def visit_PrintNode(self, node: PrintNode):
        self.visit(node.expression)
        self.bytecode.add_instruction(OpCode.PRINT)

    def visit_IfNode(self, node: IfNode):
        # Compile condition
        self.visit(node.condition)

        # Jump if false to else block (or end if no else)
        jump_if_false_pos = len(self.bytecode.instructions)
        self.bytecode.add_instruction(OpCode.JUMP_IF_FALSE, 0)  # Placeholder

        # Then branch
        for stmt in node.then_body:
            self.visit(stmt)

        # Jump over else block
        jump_after_then_pos = len(self.bytecode.instructions)
        self.bytecode.add_instruction(OpCode.JUMP, 0)  # Placeholder

        # Else branch (if any)
        else_pos = len(self.bytecode.instructions)
        # Update the jump if false to point here
        self.bytecode.instructions[jump_if_false_pos] = (OpCode.JUMP_IF_FALSE, else_pos)

        for stmt in node.else_body:
            self.visit(stmt)

        # End of if statement
        end_pos = len(self.bytecode.instructions)
        # Update the jump after then to point here
        self.bytecode.instructions[jump_after_then_pos] = (OpCode.JUMP, end_pos)

    def visit_RepeatNode(self, node: RepeatNode):
        # For repeat/til loop: body; jump back to condition if condition is false
        loop_start = len(self.bytecode.instructions)

        # Body
        for stmt in node.body:
            self.visit(stmt)

        # Condition
        self.visit(node.condition)

        # Jump back to loop start if condition is false
        self.bytecode.add_instruction(OpCode.JUMP_IF_FALSE, loop_start)

    def visit_FunctionDefNode(self, node: FunctionDefNode):
        # For now, just define the function - full implementation would be more complex
        # We'll store the function in globals for now
        func_name = node.name.name
        func_index = self.bytecode.add_name(func_name)

        # For simplicity, we'll just store a placeholder
        # A full implementation would compile the function body separately
        none_index = self.bytecode.add_constant(None)
        self.bytecode.add_instruction(OpCode.LOAD_CONST, none_index)
        self.bytecode.add_instruction(OpCode.STORE_GLOBAL, func_index)

        # Don't compile the body yet - we'll handle function calls separately
        # For now, just note that the function exists

    def visit_FunctionCallNode(self, node: FunctionCallNode):
        # Evaluate arguments
        for arg in node.args:
            self.visit(arg)

        # Call function
        func_name = node.name.name
        func_index = self.bytecode.add_name(func_name)
        self.bytecode.add_instruction(OpCode.CALL, func_index)

    def visit_ListNode(self, node: ListNode):
        # Build list from elements
        for element in node.elements:
            self.visit(element)
        self.bytecode.add_instruction(OpCode.BUILD_LIST, len(node.elements))

    def visit_IndexNode(self, node: IndexNode):
        self.visit(node.obj)
        self.visit(node.index)
        self.bytecode.add_instruction(OpCode.LOAD_SUBSCR)


class VM:
    def __init__(self):
        self.stack: List[Any] = []
        self.globals: dict = {}
        self.frames: List[dict] = []  # Call stack for local variables
        self.current_frame: dict = {}

    def push_frame(self):
        self.frames.append(self.current_frame)
        self.current_frame = {}

    def pop_frame(self):
        if self.frames:
            self.current_frame = self.frames.pop()
        else:
            self.current_frame = {}

    def execute(self, bytecode: Bytecode):
        """Execute bytecode"""
        self.stack = []
        self.ip = 0  # Instruction pointer
        self.bytecode = bytecode
        self.globals = {}  # Reset globals
        self.frames = []
        self.current_frame = {}

        # Initialize built-ins
        self._init_builtins()

        while self.ip < len(bytecode.instructions):
            opcode, arg = bytecode.instructions[self.ip]
            self.ip += 1

            self._execute_instruction(opcode, arg)

    def _init_builtins(self):
        """Initialize built-in functions and values"""
        self.globals.update({
            'print': self._builtin_print,
            'input': self._builtin_input,
            'int': int,
            'float': float,
            'str': str,
            'len': len,
            'True': True,
            'False': False,
            'None': None,
        })

    def _builtin_print(self, *args):
        print(' '.join(str(arg) for arg in args))
        return None

    def _builtin_input(self, prompt=''):
        return input(str(prompt))

    def _execute_instruction(self, opcode: int, arg: Any):
        if opcode == OpCode.LOAD_CONST:
            self.stack.append(self.bytecode.constants[arg])
        elif opcode == OpCode.LOAD_NONE:
            self.stack.append(None)
        elif opcode == OpCode.LOAD_TRUE:
            self.stack.append(True)
        elif opcode == OpCode.LOAD_FALSE:
            self.stack.append(False)
        elif opcode == OpCode.LOAD_VAR:
            var_name = self.bytecode.varnames[arg]
            # Lookup in current frame (locals) first, then globals
            if var_name in self.current_frame:
                self.stack.append(self.current_frame[var_name])
            else:
                self.stack.append(self.globals.get(var_name, None))
        elif opcode == OpCode.STORE_VAR:
            value = self.stack.pop()
            var_name = self.bytecode.varnames[arg]
            self.current_frame[var_name] = value
        elif opcode == OpCode.LOAD_GLOBAL:
            name = self.bytecode.names[arg]
            self.stack.append(self.globals.get(name, None))
        elif opcode == OpCode.STORE_GLOBAL:
            value = self.stack.pop()
            name = self.bytecode.names[arg]
            self.globals[name] = value
        elif opcode == OpCode.ADD:
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a + b)
        elif opcode == OpCode.SUB:
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a - b)
        elif opcode == OpCode.MUL:
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a * b)
        elif opcode == OpCode.DIV:
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a / b)
        elif opcode == OpCode.MOD:
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a % b)
        elif opcode == OpCode.EQ:
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a == b)
        elif opcode == OpCode.NE:
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a != b)
        elif opcode == OpCode.LT:
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a < b)
        elif opcode == OpCode.GT:
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a > b)
        elif opcode == OpCode.LE:
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a <= b)
        elif opcode == OpCode.GE:
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a >= b)
        elif opcode == OpCode.AND:
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a and b)
        elif opcode == OpCode.OR:
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a or b)
        elif opcode == OpCode.NOT:
            a = self.stack.pop()
            self.stack.append(not a)
        elif opcode == OpCode.PRINT:
            value = self.stack.pop()
            print(value)
        elif opcode == OpCode.JUMP:
            self.ip = arg
        elif opcode == OpCode.JUMP_IF_FALSE:
            condition = self.stack.pop()
            if not condition:
                self.ip = arg
        elif opcode == OpCode.JUMP_IF_TRUE:
            condition = self.stack.pop()
            if condition:
                self.ip = arg
        elif opcode == OpCode.POP:
            self.stack.pop()
        elif opcode == OpCode.DUP:
            if self.stack:
                self.stack.append(self.stack[-1])
        elif opcode == OpCode.BUILD_LIST:
            count = arg
            elements = []
            for _ in range(count):
                elements.append(self.stack.pop())
            elements.reverse()  # Because we popped in reverse order
            self.stack.append(elements)
        elif opcode == OpCode.LIST_APPEND:
            item = self.stack.pop()
            lst = self.stack.pop()
            lst.append(item)
            self.stack.append(lst)
        elif opcode == OpCode.STORE_SUBSCR:
            value = self.stack.pop()
            index = self.stack.pop()
            obj = self.stack.pop()
            obj[index] = value
        elif opcode == OpCode.LOAD_SUBSCR:
            index = self.stack.pop()
            obj = self.stack.pop()
            self.stack.append(obj[index])
        elif opcode == OpCode.CALL:
            # Simple function call handling
            func_index = arg
            func_name = self.bytecode.names[func_index]
            num_args = arg if isinstance(arg, int) else 0  # Simplified

            # Pop arguments
            args = []
            for _ in range(num_args):
                args.append(self.stack.pop())
            args.reverse()

            # Look up function
            if func_name in self.globals:
                func = self.globals[func_name]
                if callable(func):
                    # Call the function
                    result = func(*args)
                    self.stack.append(result)
                else:
                    # Not a callable, push it back
                    self.stack.append(func)
            else:
                # Function not found, push None
                self.stack.append(None)
        else:
            raise Exception(f'Unknown opcode: {opcode}')


# Helper function to compile and execute
def compile_and_execute(source: str):
    from .lexer import Lexer
    from .parser import Parser

    lexer = Lexer(source)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    ast = parser.parse()

    compiler = Compiler()
    bytecode = compiler.compile(ast)

    vm = VM()
    vm.execute(bytecode)
    return vm