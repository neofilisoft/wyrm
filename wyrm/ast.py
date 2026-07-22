# AST Node classes for the Wyrm language
# These are imported by both parser and interpreter

class ASTNode:
    pass

class NumberNode(ASTNode):
    def __init__(self, token):
        self.token = token
        self.value = token.value

    def __repr__(self):
        return f"NumberNode({self.value})"

class StringNode(ASTNode):
    def __init__(self, token):
        self.token = token
        self.value = token.value

    def __repr__(self):
        return f"StringNode({self.value})"

class BooleanNode(ASTNode):
    def __init__(self, token):
        self.token = token
        self.value = token.value == 'true'

    def __repr__(self):
        return f"BooleanNode({self.value})"

class NoneNode(ASTNode):
    def __init__(self, token):
        self.token = token
        self.value = None

    def __repr__(self):
        return "NoneNode()"

class IdentifierNode(ASTNode):
    def __init__(self, token):
        self.token = token
        self.name = token.value

    def __repr__(self):
        return f"IdentifierNode({self.name})"

class BinaryOpNode(ASTNode):
    def __init__(self, left, op, right):
        self.left = left
        self.op = op
        self.right = right

    def __repr__(self):
        return f"BinaryOpNode({self.left}, {self.op.value}, {self.right})"

class UnaryOpNode(ASTNode):
    def __init__(self, op, expr):
        self.op = op
        self.expr = expr

    def __repr__(self):
        return f"UnaryOpNode({self.op.value}, {self.expr})"

class AssignmentNode(ASTNode):
    def __init__(self, var_name, value, is_declaration=False, declaration_type=None):
        self.var_name = var_name
        self.value = value
        self.is_declaration = is_declaration  # True if this is a var/dec declaration
        self.declaration_type = declaration_type  # 'var', 'dec', or None for regular assignment

    def __repr__(self):
        if self.is_declaration:
            return f"AssignmentNode({self.var_name}, {self.value}, {self.declaration_type})"
        else:
            return f"AssignmentNode({self.var_name}, {self.value})"

class PrintNode(ASTNode):
    def __init__(self, expression=None, expressions=None):
        if expressions is not None:
            self.expressions = expressions
            self.expression = expressions[0] if expressions else None
        else:
            self.expression = expression
            self.expressions = [expression] if expression is not None else []

    def __repr__(self):
        return f"PrintNode({self.expressions})"

class IfNode(ASTNode):
    def __init__(self, condition, then_body, elif_clauses, else_body):
        self.condition = condition
        self.then_body = then_body
        self.elif_clauses = elif_clauses  # List of (condition, body) tuples
        self.else_body = else_body

    def __repr__(self):
        return f"IfNode({self.condition}, {self.then_body}, {self.elif_clauses}, {self.else_body})"

class RepeatNode(ASTNode):
    def __init__(self, body, condition):
        self.body = body
        self.condition = condition

    def __repr__(self):
        return f"RepeatNode({self.body}, {self.condition})"

class FunctionDefNode(ASTNode):
    def __init__(self, name, params, body):
        self.name = name
        self.params = params
        self.body = body

    def __repr__(self):
        return f"FunctionDefNode({self.name}, {self.params}, {self.body})"

class FunctionCallNode(ASTNode):
    def __init__(self, name, args):
        self.name = name
        self.args = args

    def __repr__(self):
        return f"FunctionCallNode({self.name}, {self.args})"

class ListNode(ASTNode):
    def __init__(self, elements):
        self.elements = elements

    def __repr__(self):
        return f"ListNode({self.elements})"

class IndexNode(ASTNode):
    def __init__(self, obj, index):
        self.obj = obj
        self.index = index

    def __repr__(self):
        return f"IndexNode({self.obj}, {self.index})"

class IndexAssignNode(ASTNode):
    def __init__(self, obj, index, value):
        self.obj = obj    # IdentifierNode of the list
        self.index = index  # index expression
        self.value = value  # value to assign

    def __repr__(self):
        return f"IndexAssignNode({self.obj}, {self.index}, {self.value})"


class ReturnNode(ASTNode):
    def __init__(self, expr):
        self.expr = expr

    def __repr__(self):
        return f"ReturnNode({self.expr})"

class UseNode(ASTNode):
    def __init__(self, module_path):
        self.module_path = module_path

    def __repr__(self):
        return f"UseNode({self.module_path})"

class BreakNode(ASTNode):
    def __repr__(self):
        return "BreakNode()"

class ContinueNode(ASTNode):
    def __repr__(self):
        return "ContinueNode()"