from typing import List, Optional, Union
from .lexer import Token, TT_INT, TT_FLOAT, TT_STRING, TT_IDENTIFIER, TT_KEYWORD, TT_OPERATOR, TT_DELIMITER, TT_NEWLINE, TT_EOF
from .ast import (
    ASTNode,
    NumberNode,
    StringNode,
    BooleanNode,
    NoneNode,
    IdentifierNode,
    BinaryOpNode,
    UnaryOpNode,
    AssignmentNode,
    PrintNode,
    IfNode,
    RepeatNode,
    FunctionDefNode,
    FunctionCallNode,
    ListNode,
    IndexNode,
    IndexAssignNode,
    ReturnNode,
    UseNode,
    BreakNode,
    ContinueNode
)

class Parser:
    def __init__(self, tokens: List[Token]):
        self.tokens = tokens
        self.pos = 0
        self.current_token = self.tokens[0] if self.tokens else None

    def advance(self):
        """Move to the next token."""
        self.pos += 1
        if self.pos >= len(self.tokens):
            self.current_token = None
        else:
            self.current_token = self.tokens[self.pos]

    def peek(self) -> Optional[Token]:
        """Look at the next token without consuming."""
        if self.pos + 1 < len(self.tokens):
            return self.tokens[self.pos + 1]
        return None

    def expect(self, token_type: str, value: Optional[str] = None) -> Token:
        """Consume and return the current token if it matches expected type/value."""
        if not self.current_token:
            raise SyntaxError("Unexpected end of input")

        if self.current_token.type != token_type:
            raise SyntaxError(f"Expected {token_type}, got {self.current_token.type} at line {self.current_token.line}, column {self.current_token.column}")

        if value is not None and self.current_token.value != value:
            raise SyntaxError(f"Expected '{value}', got '{self.current_token.value}' at line {self.current_token.line}, column {self.current_token.column}")

        token = self.current_token
        self.advance()
        return token

    def parse(self) -> List[ASTNode]:
        """Parse a list of statements."""
        statements = []
        while self.current_token and self.current_token.type != TT_EOF:
            if self.current_token.type == TT_NEWLINE:
                self.advance()
                continue
            stmt = self.statement()
            if stmt:
                statements.append(stmt)
        return statements

    def statement(self) -> Optional[ASTNode]:
        """Parse a single statement."""
        if self.current_token.type == TT_KEYWORD:
            if self.current_token.value == 'use':
                return self.use_statement()
            elif self.current_token.value == 'fn':
                return self.function_def()
            elif self.current_token.value == 'return':
                return self.return_statement()
            elif self.current_token.value == 'break':
                return self.break_statement()
            elif self.current_token.value == 'continue':
                return self.continue_statement()
            elif self.current_token.value == 'print':
                return self.print_statement()
            elif self.current_token.value == 'if':
                return self.if_statement()
            elif self.current_token.value == 'repeat':
                return self.repeat_statement()
            elif self.current_token.value in ('var', 'dec'):
                # Variable declaration: var/dec identifier = expression
                var_type = self.current_token.value  # 'var' or 'dec'
                self.advance()  # Consume the var/dec token

                # Expect identifier
                if not self.current_token or self.current_token.type != TT_IDENTIFIER:
                    raise SyntaxError(f"Expected identifier after {var_type} at line {self.current_token.line if self.current_token else 'unknown'}, column {self.current_token.column if self.current_token else 'unknown'}")

                var_name = IdentifierNode(self.current_token)
                self.advance()  # Consume the identifier

                # Expect equals sign
                if not self.current_token or self.current_token.type != TT_OPERATOR or self.current_token.value != '=':
                    raise SyntaxError(f"Expected '=' after variable name in {var_type} declaration at line {self.current_token.line}, column {self.current_token.column}")

                self.advance()  # Consume the equals sign

                # Expression
                value = self.expression()

                # Create assignment node with declaration info
                return AssignmentNode(var_name, value, is_declaration=True, declaration_type=var_type)
            elif self.current_token.value in ('true', 'false', 'null'):
                # These can be used in expressions, but as statements they're just expressions
                return self.expression_statement()
        elif self.current_token.type == TT_IDENTIFIER:
            # Peek ahead: identifier[index] = value  OR  identifier = value  OR  func call
            if (self.peek() and self.peek().type == TT_OPERATOR
                    and self.peek().value in ('=', '+=', '-=', '*=', '/=', '%=')):
                return self.assignment()
            elif self.peek() and self.peek().type == TT_DELIMITER and self.peek().value == '[':
                # Could be indexed assignment: name[expr] = value
                return self.indexed_assignment_or_expr()
            else:
                return self.expression_statement()

        # If we don't recognize a statement, treat it as an expression
        return self.expression_statement()

    def expression_statement(self) -> ASTNode:
        """Parse an expression and return it as a statement node."""
        expr = self.expression()
        return expr

    def assignment(self) -> AssignmentNode:
        """Parse an assignment statement."""
        var_name = IdentifierNode(self.expect(TT_IDENTIFIER))
        op_token = self.expect(TT_OPERATOR)  # Consume '=', '+=', etc.
        value = self.expression()
        
        if op_token.value != '=':
            base_op = op_token.value[:-1]
            bin_op_token = Token(TT_OPERATOR, base_op, op_token.line, op_token.column)
            value = BinaryOpNode(var_name, bin_op_token, value)
            
        return AssignmentNode(var_name, value)

    def indexed_assignment_or_expr(self):
        """Parse indexed assignment like arr[i] = value, or fall back to expression."""
        name_token = self.current_token
        name_node = IdentifierNode(name_token)
        self.advance()  # consume identifier
        self.advance()  # consume '['
        index = self.expression()
        self.expect(TT_DELIMITER, ']')
        # Check if followed by '=' or compound operator
        if self.current_token and self.current_token.type == TT_OPERATOR and self.current_token.value in ('=', '+=', '-=', '*=', '/=', '%='):
            op_token = self.current_token
            self.advance()  # consume operator
            value = self.expression()
            
            if op_token.value != '=':
                base_op = op_token.value[:-1]
                bin_op_token = Token(TT_OPERATOR, base_op, op_token.line, op_token.column)
                current_val_node = IndexNode(name_node, index)
                value = BinaryOpNode(current_val_node, bin_op_token, value)
                
            return IndexAssignNode(name_node, index, value)
        else:
            # Not an assignment - wrap as IndexNode expression statement
            node = IndexNode(name_node, index)
            return node

    def print_statement(self) -> PrintNode:
        """Parse a print statement (supports print(a, b, c) or print a, b, c)."""
        self.expect(TT_KEYWORD, 'print')

        has_paren = False
        if self.current_token and self.current_token.type == TT_DELIMITER and self.current_token.value == '(':
            has_paren = True
            self.advance()  # Skip '('

        expressions = []
        if self.current_token and (not has_paren or self.current_token.value != ')'):
            while True:
                expressions.append(self.expression())
                if self.current_token and self.current_token.type == TT_DELIMITER and self.current_token.value == ',':
                    self.advance()
                else:
                    break

        if has_paren:
            self.expect(TT_DELIMITER, ')')

        return PrintNode(expressions=expressions)

    def return_statement(self) -> ReturnNode:
        """Parse a return statement."""
        self.expect(TT_KEYWORD, 'return')
        expr = self.expression()
        return ReturnNode(expr)

    def break_statement(self) -> BreakNode:
        """Parse a break statement."""
        self.expect(TT_KEYWORD, 'break')
        return BreakNode()

    def continue_statement(self) -> ContinueNode:
        """Parse a continue statement."""
        self.expect(TT_KEYWORD, 'continue')
        return ContinueNode()

    def if_statement(self) -> IfNode:
        """Parse an if statement."""
        self.expect(TT_KEYWORD, 'if')
        condition = self.expression()
        self.expect(TT_DELIMITER, '{')
        then_body = self.block()
        self.expect(TT_DELIMITER, '}')

        elif_clauses = []
        while self.current_token and self.current_token.type == TT_KEYWORD and self.current_token.value == 'elif':
            self.expect(TT_KEYWORD, 'elif')
            elif_condition = self.expression()
            self.expect(TT_DELIMITER, '{')
            elif_body = self.block()
            self.expect(TT_DELIMITER, '}')
            elif_clauses.append((elif_condition, elif_body))

        else_body = None
        if self.current_token and self.current_token.type == TT_KEYWORD and self.current_token.value == 'else':
            self.expect(TT_KEYWORD, 'else')
            self.expect(TT_DELIMITER, '{')
            else_body = self.block()
            self.expect(TT_DELIMITER, '}')

        return IfNode(condition, then_body, elif_clauses, else_body)

    def repeat_statement(self) -> RepeatNode:
        """Parse a repeat/til loop."""
        self.expect(TT_KEYWORD, 'repeat')
        self.expect(TT_DELIMITER, '{')
        body = self.block()
        self.expect(TT_DELIMITER, '}')
        self.expect(TT_KEYWORD, 'til')
        self.expect(TT_DELIMITER, '(')
        condition = self.expression()
        self.expect(TT_DELIMITER, ')')
        return RepeatNode(body, condition)

    def function_def(self) -> FunctionDefNode:
        """Parse a function definition."""
        self.expect(TT_KEYWORD, 'fn')
        name = IdentifierNode(self.expect(TT_IDENTIFIER))
        self.expect(TT_DELIMITER, '(')
        params = []
        if self.current_token and self.current_token.type != TT_DELIMITER:
            while True:
                params.append(IdentifierNode(self.expect(TT_IDENTIFIER)))
                if self.current_token and self.current_token.type == TT_DELIMITER and self.current_token.value == ',':
                    self.advance()
                else:
                    break
        self.expect(TT_DELIMITER, ')')
        self.expect(TT_DELIMITER, '{')
        body = self.block()
        self.expect(TT_DELIMITER, '}')
        return FunctionDefNode(name, params, body)

    def block(self) -> List[ASTNode]:
        """Parse a block of statements."""
        statements = []
        while self.current_token and self.current_token.type != TT_DELIMITER:
            if self.current_token.type == TT_NEWLINE:
                self.advance()
                continue
            stmt = self.statement()
            if stmt:
                statements.append(stmt)
        return statements

    def expression(self) -> ASTNode:
        """Parse an expression (with precedence)."""
        node = self.and_expression()

        while self.current_token and ((self.current_token.type == TT_OPERATOR and self.current_token.value in ('or', '||')) or
                                      (self.current_token.type == TT_IDENTIFIER and self.current_token.value in ('or',))):
            op_token = self.current_token
            self.advance()
            right = self.and_expression()
            node = BinaryOpNode(node, op_token, right)

        return node

    def and_expression(self) -> ASTNode:
        """Parse AND expressions."""
        node = self.comparison()

        while self.current_token and ((self.current_token.type == TT_OPERATOR and self.current_token.value in ('and', '&&')) or
                                      (self.current_token.type == TT_IDENTIFIER and self.current_token.value in ('and',))):
            op_token = self.current_token
            self.advance()
            right = self.comparison()
            node = BinaryOpNode(node, op_token, right)

        return node

    def comparison(self) -> ASTNode:
        """Parse comparison operations."""
        node = self.addition_subtraction()

        while self.current_token and self.current_token.type == TT_OPERATOR and self.current_token.value in ('==', '!=', '<', '>', '<=', '>='):
            op_token = self.current_token
            self.advance()
            right = self.addition_subtraction()
            node = BinaryOpNode(node, op_token, right)

        return node

    def addition_subtraction(self) -> ASTNode:
        """Parse addition and subtraction operations."""
        node = self.multiplication_division()

        while self.current_token and self.current_token.type == TT_OPERATOR and self.current_token.value in ('+', '-'):
            op_token = self.current_token
            self.advance()
            right = self.multiplication_division()
            node = BinaryOpNode(node, op_token, right)

        return node

    def multiplication_division(self) -> ASTNode:
        """Parse multiplication, division, and modulo operations."""
        node = self.power()

        while self.current_token and self.current_token.type == TT_OPERATOR and self.current_token.value in ('*', '/', '%'):
            op_token = self.current_token
            self.advance()
            right = self.power()
            node = BinaryOpNode(node, op_token, right)

        return node

    def power(self) -> ASTNode:
        """Parse power operation."""
        return self.unary()

    def unary(self) -> ASTNode:
        """Parse unary operations."""
        if self.current_token and ((self.current_token.type == TT_OPERATOR and self.current_token.value in ('!', '-')) or
                                  (self.current_token.type == TT_IDENTIFIER and self.current_token.value == 'not')):
            op_token = self.current_token
            self.advance()
            expr = self.unary()
            return UnaryOpNode(op_token, expr)
        return self.primary()

    def primary(self) -> ASTNode:
        """Parse primary expressions."""
        if self.current_token.type == TT_INT or self.current_token.type == TT_FLOAT:
            node = NumberNode(self.current_token)
            self.advance()
            return node
        elif self.current_token.type == TT_STRING:
            node = StringNode(self.current_token)
            self.advance()
            return node
        elif self.current_token.type == TT_KEYWORD and self.current_token.value in ('true', 'false', 'null'):
            if self.current_token.value == 'true':
                node = BooleanNode(self.current_token)
            elif self.current_token.value == 'false':
                node = BooleanNode(self.current_token)
            else:
                node = NoneNode(self.current_token)
            self.advance()
            return node
        # Handle built-in function calls (keywords that can be called as functions)
        elif self.current_token.type == TT_KEYWORD and self.current_token.value in ('int', 'float', 'str', 'len', 'input', 'append', 'pop', 'print'):
            func_name = self.current_token.value
            line = self.current_token.line
            column = self.current_token.column
            self.advance()  # Skip the function name keyword
            # Expect opening parenthesis
            self.expect(TT_DELIMITER, '(')
            # Parse arguments
            args = []
            if self.current_token and self.current_token.type != TT_DELIMITER:
                while True:
                    args.append(self.expression())
                    if self.current_token and self.current_token.type == TT_DELIMITER and self.current_token.value == ',':
                        self.advance()
                    else:
                        break
            # Expect closing parenthesis
            self.expect(TT_DELIMITER, ')')
            return FunctionCallNode(IdentifierNode(Token(TT_IDENTIFIER, func_name, line, column)), args)
        elif self.current_token.type == TT_IDENTIFIER:
            node = IdentifierNode(self.current_token)
            self.advance()
            # Check for function call
            if self.current_token and self.current_token.type == TT_DELIMITER and self.current_token.value == '(':
                self.advance()  # Skip '('
                args = []
                if self.current_token and self.current_token.type != TT_DELIMITER:
                    while True:
                        args.append(self.expression())
                        if self.current_token and self.current_token.type == TT_DELIMITER and self.current_token.value == ',':
                            self.advance()
                        else:
                            break
                self.expect(TT_DELIMITER, ')')
                return FunctionCallNode(node, args)
            # Check for list/array indexing
            elif self.current_token and self.current_token.type == TT_DELIMITER and self.current_token.value == '[':
                self.advance()  # Skip '['
                index = self.expression()
                self.expect(TT_DELIMITER, ']')
                return IndexNode(node, index)
            return node
        elif self.current_token.type == TT_DELIMITER and self.current_token.value == '(':
            self.advance()  # Skip '('
            node = self.expression()
            self.expect(TT_DELIMITER, ')')
            return node
        elif self.current_token.type == TT_DELIMITER and self.current_token.value == '[':
            # List literal
            self.advance()  # Skip '['
            elements = []
            if self.current_token and self.current_token.type != TT_DELIMITER:
                while True:
                    elements.append(self.expression())
                    if self.current_token and self.current_token.type == TT_DELIMITER and self.current_token.value == ',':
                        self.advance()
                    else:
                        break
            self.expect(TT_DELIMITER, ']')
            return ListNode(elements)
        else:
            raise SyntaxError(f"Unexpected token: {self.current_token.type} ({self.current_token.value}) at line {self.current_token.line}, column {self.current_token.column}")

    def use_statement(self) -> UseNode:
        """Parse a use (import) statement."""
        self.advance()  # Consume 'use'
        if not self.current_token or self.current_token.type in (TT_NEWLINE, TT_EOF):
            raise SyntaxError("Expected module path after 'use'")

        if self.current_token.type == TT_STRING:
            module_path = self.current_token.value
            self.advance()
        else:
            parts = []
            while self.current_token and self.current_token.type not in (TT_NEWLINE, TT_EOF) and self.current_token.value != ';':
                parts.append(str(self.current_token.value))
                self.advance()
            module_path = "".join(parts).strip()

        if self.current_token and self.current_token.value == ';':
            self.advance()

        return UseNode(module_path)