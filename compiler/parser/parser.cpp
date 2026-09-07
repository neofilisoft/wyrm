#include "parser.hpp"
#include <iostream>

namespace wyrm {

Parser::Parser(std::vector<Token> tokens)
    : tokens_(std::move(tokens)), pos_(0), current_token_(nullptr) {
    if (!tokens_.empty()) {
        current_token_ = &tokens_[0];
    }
}

const Token* Parser::peek(size_t offset) const {
    if (pos_ + offset >= tokens_.size()) {
        return nullptr;
    }
    return &tokens_[pos_ + offset];
}

void Parser::advance() {
    pos_++;
    if (pos_ >= tokens_.size()) {
        current_token_ = nullptr;
    } else {
        current_token_ = &tokens_[pos_];
    }
}

Token Parser::expect(TokenType type, const std::string& value) {
    if (!current_token_) {
        throw std::runtime_error(
            "Unexpected end of input, expected " + std::string(to_string(type)) +
            (value.empty() ? "" : " '" + value + "'")
        );
    }

    if (current_token_->type != type) {
        throw std::runtime_error(
            "Expected " + std::string(to_string(type)) +
            (value.empty() ? "" : " '" + value + "'") +
            " but got " + std::string(to_string(current_token_->type)) +
            " '" + current_token_->value + "'" +
            " at line " + std::to_string(current_token_->line) +
            ", column " + std::to_string(current_token_->column)
        );
    }

    if (!value.empty() && current_token_->value != value) {
        throw std::runtime_error(
            "Expected " + std::string(to_string(type)) + " '" + value +
            "' but got '" + current_token_->value + "'" +
            " at line " + std::to_string(current_token_->line) +
            ", column " + std::to_string(current_token_->column)
        );
    }

    Token token = *current_token_;
    advance();
    return token;
}

std::vector<ASTNodePtr> Parser::parse() {
    std::vector<ASTNodePtr> statements;
    while (current_token_ && current_token_->type != TokenType::END_OF_FILE) {
        if (current_token_->type == TokenType::NEWLINE) {
            advance();
            continue;
        }
        ASTNodePtr stmt = statement();
        if (stmt) {
            statements.push_back(std::move(stmt));
        }
    }
    return statements;
}

ASTNodePtr Parser::statement() {
    auto statement_inner = [this]() -> ASTNodePtr {
        if (!current_token_) return nullptr;

        if (current_token_->type == TokenType::KEYWORD) {
            if (current_token_->value == "use") {
                return use_statement();
            } else if (current_token_->value == "fn") {
                return function_def();
            } else if (current_token_->value == "return") {
                return return_statement();
            } else if (current_token_->value == "break") {
                return break_statement();
            } else if (current_token_->value == "continue") {
                return continue_statement();
            } else if (current_token_->value == "print") {
                return print_statement();
            } else if (current_token_->value == "if") {
                return if_statement();
            } else if (current_token_->value == "repeat" || current_token_->value == "do") {
                return repeat_statement();
            } else if (current_token_->value == "unsafe") {
                return unsafe_block();
            } else if (current_token_->value == "owned") {
                return owned_declaration();
            } else if (current_token_->value == "arena") {
                return arena_declaration();
            } else if (current_token_->value == "struct") {
                return struct_def();
            } else if (current_token_->value == "var" || current_token_->value == "dec") {
                return variable_declaration(current_token_->value);
            } else if (current_token_->value == "true" || current_token_->value == "false" || current_token_->value == "null") {
                return expression_statement();
            }
        } else if (current_token_->type == TokenType::IDENTIFIER) {
            // Peek ahead for assignment or method call
            const Token* p = peek(1);
            if (p && p->type == TokenType::DELIMITER && p->value == ":") {
                Token name_tok = expect(TokenType::IDENTIFIER);
                auto name_node = std::make_unique<IdentifierNode>(name_tok);
                advance(); // consume ':'
                std::string type_ann = "";
                if (current_token_ && (current_token_->type == TokenType::IDENTIFIER || current_token_->type == TokenType::KEYWORD)) {
                    type_ann = current_token_->value;
                    advance();
                }
                expect(TokenType::OPERATOR, "=");
                ASTNodePtr val = expression();
                return std::make_unique<AssignmentNode>(std::move(name_node), std::move(val), true, "var", type_ann);
            } else if (p && p->type == TokenType::OPERATOR && 
                (p->value == "=" || p->value == "+=" || p->value == "-=" || 
                 p->value == "*=" || p->value == "/=" || p->value == "%=")) {
                return assignment();
            } else if (p && p->type == TokenType::DELIMITER && p->value == "[") {
                return indexed_assignment_or_expr();
            } else if (p && p->type == TokenType::DELIMITER && p->value == ".") {
                return arena_method_or_expr();
            } else {
                return expression_statement();
            }
        }

        return expression_statement();
    };

    ASTNodePtr res = statement_inner();
    if (current_token_ && current_token_->type == TokenType::DELIMITER && current_token_->value == ";") {
        advance();
    }
    return res;
}

ASTNodePtr Parser::use_statement() {
    advance(); // consume 'use'
    std::string module_path;

    if (!current_token_) {
        throw std::runtime_error("SyntaxError: Expected module path after 'use'");
    }

    if (current_token_->type == TokenType::STRING) {
        module_path = current_token_->value;
        advance();
    } else {
        while (current_token_ && current_token_->type != TokenType::NEWLINE &&
               current_token_->type != TokenType::END_OF_FILE &&
               current_token_->value != ";") {
            module_path += current_token_->value;
            advance();
        }
    }

    if (!current_token_ || current_token_->value != ";") {
        size_t err_line = current_token_ ? current_token_->line : 0;
        size_t err_col  = current_token_ ? current_token_->column : 0;
        throw std::runtime_error(
            "SyntaxError: Expected ';' after 'use " + module_path + "' at line " +
            std::to_string(err_line) + ", col " + std::to_string(err_col) +
            ". Wyrm requires a semicolon to terminate 'use' statements."
        );
    }
    advance(); // consume ';'

    return std::make_unique<UseNode>(module_path);
}

ASTNodePtr Parser::function_def() {
    advance(); // consume 'fn'
    Token name_tok = expect(TokenType::IDENTIFIER);
    auto name_node = std::make_unique<IdentifierNode>(name_tok);

    expect(TokenType::DELIMITER, "(");
    std::vector<std::unique_ptr<IdentifierNode>> params;
    std::vector<std::string> param_types;

    if (current_token_ && !(current_token_->type == TokenType::DELIMITER && current_token_->value == ")")) {
        while (true) {
            Token p_tok = expect(TokenType::IDENTIFIER);
            params.push_back(std::make_unique<IdentifierNode>(p_tok));
            std::string p_type = "";
            if (current_token_ && current_token_->type == TokenType::DELIMITER && current_token_->value == ":") {
                advance(); // consume ':'
                if (current_token_ && (current_token_->type == TokenType::IDENTIFIER || current_token_->type == TokenType::KEYWORD)) {
                    p_type = current_token_->value;
                    advance();
                }
            }
            param_types.push_back(p_type);

            if (current_token_ && current_token_->type == TokenType::DELIMITER && current_token_->value == ",") {
                advance();
            } else {
                break;
            }
        }
    }
    expect(TokenType::DELIMITER, ")");

    std::string return_type = "";
    if (current_token_ && (current_token_->value == ":" || current_token_->value == "->")) {
        advance(); // consume ':' or '->'
        if (current_token_ && (current_token_->type == TokenType::IDENTIFIER || current_token_->type == TokenType::KEYWORD)) {
            return_type = current_token_->value;
            advance();
        }
    }

    expect(TokenType::DELIMITER, "{");

    std::vector<ASTNodePtr> body;
    while (current_token_ && !(current_token_->type == TokenType::DELIMITER && current_token_->value == "}")) {
        if (current_token_->type == TokenType::NEWLINE) {
            advance();
            continue;
        }
        ASTNodePtr stmt = statement();
        if (stmt) {
            body.push_back(std::move(stmt));
        }
    }
    expect(TokenType::DELIMITER, "}");

    return std::make_unique<FunctionDefNode>(std::move(name_node), std::move(params), std::move(body), std::move(param_types), return_type);
}

ASTNodePtr Parser::return_statement() {
    advance(); // consume 'return'
    ASTNodePtr expr = nullptr;
    if (current_token_ && current_token_->type != TokenType::NEWLINE && 
        current_token_->type != TokenType::END_OF_FILE && 
        current_token_->value != ";" && 
        current_token_->value != "}") {
        expr = expression();
    }
    if (current_token_ && current_token_->value == ";") {
        advance();
    }
    return std::make_unique<ReturnNode>(std::move(expr));
}

ASTNodePtr Parser::break_statement() {
    advance(); // consume 'break'
    if (current_token_ && current_token_->value == ";") {
        advance();
    }
    return std::make_unique<BreakNode>();
}

ASTNodePtr Parser::continue_statement() {
    advance(); // consume 'continue'
    if (current_token_ && current_token_->value == ";") {
        advance();
    }
    return std::make_unique<ContinueNode>();
}

ASTNodePtr Parser::print_statement() {
    advance(); // consume 'print'
    if (current_token_ && current_token_->type == TokenType::OPERATOR && current_token_->value == "!") {
        advance(); // consume '!'
    }
    expect(TokenType::DELIMITER, "(");
    std::vector<ASTNodePtr> exprs;

    if (current_token_ && !(current_token_->type == TokenType::DELIMITER && current_token_->value == ")")) {
        while (true) {
            exprs.push_back(expression());
            if (current_token_ && current_token_->type == TokenType::DELIMITER && current_token_->value == ",") {
                advance();
            } else {
                break;
            }
        }
    }
    expect(TokenType::DELIMITER, ")");
    return std::make_unique<PrintNode>(std::move(exprs));
}

ASTNodePtr Parser::if_statement() {
    advance(); // consume 'if'
    ASTNodePtr condition = expression();
    expect(TokenType::DELIMITER, "{");

    std::vector<ASTNodePtr> then_body;
    while (current_token_ && !(current_token_->type == TokenType::DELIMITER && current_token_->value == "}")) {
        if (current_token_->type == TokenType::NEWLINE) {
            advance();
            continue;
        }
        ASTNodePtr stmt = statement();
        if (stmt) {
            then_body.push_back(std::move(stmt));
        }
    }
    expect(TokenType::DELIMITER, "}");

    std::vector<std::pair<ASTNodePtr, std::vector<ASTNodePtr>>> elif_clauses;
    std::vector<ASTNodePtr> else_body;

    while (current_token_ && current_token_->type == TokenType::KEYWORD && current_token_->value == "elif") {
        advance(); // consume 'elif'
        ASTNodePtr elif_cond = expression();
        expect(TokenType::DELIMITER, "{");

        std::vector<ASTNodePtr> elif_body;
        while (current_token_ && !(current_token_->type == TokenType::DELIMITER && current_token_->value == "}")) {
            if (current_token_->type == TokenType::NEWLINE) {
                advance();
                continue;
            }
            ASTNodePtr stmt = statement();
            if (stmt) {
                elif_body.push_back(std::move(stmt));
            }
        }
        expect(TokenType::DELIMITER, "}");
        elif_clauses.push_back({std::move(elif_cond), std::move(elif_body)});
    }

    if (current_token_ && current_token_->type == TokenType::KEYWORD && current_token_->value == "else") {
        advance(); // consume 'else'
        expect(TokenType::DELIMITER, "{");

        while (current_token_ && !(current_token_->type == TokenType::DELIMITER && current_token_->value == "}")) {
            if (current_token_->type == TokenType::NEWLINE) {
                advance();
                continue;
            }
            ASTNodePtr stmt = statement();
            if (stmt) {
                else_body.push_back(std::move(stmt));
            }
        }
        expect(TokenType::DELIMITER, "}");
    }

    return std::make_unique<IfNode>(std::move(condition), std::move(then_body), std::move(elif_clauses), std::move(else_body));
}

ASTNodePtr Parser::repeat_statement() {
    advance(); // consume 'repeat' or 'do'
    expect(TokenType::DELIMITER, "{");

    std::vector<ASTNodePtr> body;
    while (current_token_ && !(current_token_->type == TokenType::DELIMITER && current_token_->value == "}")) {
        if (current_token_->type == TokenType::NEWLINE) {
            advance();
            continue;
        }
        ASTNodePtr stmt = statement();
        if (stmt) {
            body.push_back(std::move(stmt));
        }
    }
    expect(TokenType::DELIMITER, "}");
    expect(TokenType::KEYWORD, "til");
    
    ASTNodePtr condition = expression();
    return std::make_unique<RepeatNode>(std::move(body), std::move(condition));
}

ASTNodePtr Parser::unsafe_block() {
    advance(); // consume 'unsafe'
    expect(TokenType::DELIMITER, "{");

    std::vector<ASTNodePtr> body;
    while (current_token_ && !(current_token_->type == TokenType::DELIMITER && current_token_->value == "}")) {
        if (current_token_->type == TokenType::NEWLINE) {
            advance();
            continue;
        }
        ASTNodePtr stmt = statement();
        if (stmt) {
            body.push_back(std::move(stmt));
        }
    }
    expect(TokenType::DELIMITER, "}");
    return std::make_unique<UnsafeBlockNode>(std::move(body));
}

ASTNodePtr Parser::owned_declaration() {
    advance(); // consume 'owned'
    std::string decl_type = "var";
    if (current_token_ && current_token_->type == TokenType::KEYWORD && 
        (current_token_->value == "var" || current_token_->value == "dec")) {
        decl_type = current_token_->value;
        advance();
    }

    Token name_tok = expect(TokenType::IDENTIFIER);
    auto name_node = std::make_unique<IdentifierNode>(name_tok);

    std::string type_ann = "";
    if (current_token_ && current_token_->type == TokenType::DELIMITER && current_token_->value == ":") {
        advance(); // consume ':'
        if (current_token_ && (current_token_->type == TokenType::IDENTIFIER || current_token_->type == TokenType::KEYWORD)) {
            type_ann = current_token_->value;
            advance();
        }
    }

    expect(TokenType::OPERATOR, "=");
    ASTNodePtr val = expression();

    return std::make_unique<OwnedDeclNode>(std::move(name_node), std::move(val), decl_type, type_ann);
}

ASTNodePtr Parser::arena_declaration() {
    advance(); // consume 'arena'
    Token name_tok = expect(TokenType::IDENTIFIER);
    expect(TokenType::DELIMITER, "(");
    ASTNodePtr size_expr = expression();
    expect(TokenType::DELIMITER, ")");

    return std::make_unique<ArenaNode>(name_tok.value, std::move(size_expr));
}

ASTNodePtr Parser::struct_def() {
    advance(); // consume 'struct'
    Token name_tok = expect(TokenType::IDENTIFIER);
    std::string struct_name = name_tok.value;

    expect(TokenType::DELIMITER, "{");
    std::vector<std::string> fields;
    std::vector<std::string> field_types;
    std::vector<std::unique_ptr<FunctionDefNode>> methods;

    while (current_token_ && !(current_token_->type == TokenType::DELIMITER && current_token_->value == "}")) {
        if (current_token_->type == TokenType::NEWLINE) {
            advance();
            continue;
        }
        if (current_token_->type == TokenType::KEYWORD && current_token_->value == "fn") {
            auto fn_node = function_def();
            auto* f_def = dynamic_cast<FunctionDefNode*>(fn_node.get());
            if (f_def) {
                fn_node.release();
                methods.push_back(std::unique_ptr<FunctionDefNode>(f_def));
            }
        } else if (current_token_->type == TokenType::IDENTIFIER) {
            std::string f_name = current_token_->value;
            advance();
            std::string f_type = "";
            if (current_token_ && current_token_->type == TokenType::DELIMITER && current_token_->value == ":") {
                advance(); // consume ':'
                if (current_token_ && (current_token_->type == TokenType::IDENTIFIER || current_token_->type == TokenType::KEYWORD)) {
                    f_type = current_token_->value;
                    advance();
                }
            }
            fields.push_back(f_name);
            field_types.push_back(f_type);
            if (current_token_ && current_token_->type == TokenType::DELIMITER &&
                (current_token_->value == "," || current_token_->value == ";")) {
                advance();
            }
        } else {
            break;
        }
    }
    expect(TokenType::DELIMITER, "}");
    return std::make_unique<StructDefNode>(struct_name, std::move(fields), std::move(methods), std::move(field_types));
}

ASTNodePtr Parser::variable_declaration(const std::string& var_type) {
    advance(); // consume 'var' or 'dec'
    Token name_tok = expect(TokenType::IDENTIFIER);
    auto name_node = std::make_unique<IdentifierNode>(name_tok);

    std::string type_ann = "";
    if (current_token_ && current_token_->type == TokenType::DELIMITER && current_token_->value == ":") {
        advance(); // consume ':'
        if (current_token_ && (current_token_->type == TokenType::IDENTIFIER || current_token_->type == TokenType::KEYWORD)) {
            type_ann = current_token_->value;
            advance();
        }
    }

    expect(TokenType::OPERATOR, "=");
    ASTNodePtr val = expression();

    return std::make_unique<AssignmentNode>(std::move(name_node), std::move(val), true, var_type, type_ann);
}

ASTNodePtr Parser::assignment() {
    Token name_tok = expect(TokenType::IDENTIFIER);
    auto name_node = std::make_unique<IdentifierNode>(name_tok);
    Token op_tok = expect(TokenType::OPERATOR);
    ASTNodePtr val = expression();

    if (op_tok.value != "=") {
        std::string base_op = op_tok.value.substr(0, op_tok.value.size() - 1);
        Token bin_op_tok(TokenType::OPERATOR, base_op, op_tok.line, op_tok.column);
        val = std::make_unique<BinaryOpNode>(
            std::make_unique<IdentifierNode>(name_tok),
            bin_op_tok,
            std::move(val)
        );
    }

    return std::make_unique<AssignmentNode>(std::move(name_node), std::move(val));
}

ASTNodePtr Parser::indexed_assignment_or_expr() {
    Token name_tok = expect(TokenType::IDENTIFIER);
    ASTNodePtr obj = std::make_unique<IdentifierNode>(name_tok);

    // Collect all consecutive index levels: arr[i], arr[i][j], etc.
    std::vector<ASTNodePtr> indices;
    while (current_token_ && current_token_->type == TokenType::DELIMITER && current_token_->value == "[") {
        advance(); // consume '['
        indices.push_back(expression());
        expect(TokenType::DELIMITER, "]");
    }

    if (indices.empty()) {
        // No subscript at all: fall through to expression_statement
        return obj;
    }

    // Check for assignment
    if (current_token_ && current_token_->type == TokenType::OPERATOR && current_token_->value == "=") {
        advance(); // consume '='
        ASTNodePtr val = expression();
        if (indices.size() == 1) {
            return std::make_unique<IndexAssignNode>(std::move(obj), std::move(indices[0]), std::move(val));
        }
        // Chained: build nested IndexNode for all but last index, then IndexAssignNode
        ASTNodePtr base = std::make_unique<IndexNode>(std::move(obj), std::move(indices[0]));
        for (size_t i = 1; i + 1 < indices.size(); ++i) {
            base = std::make_unique<IndexNode>(std::move(base), std::move(indices[i]));
        }
        return std::make_unique<IndexAssignNode>(std::move(base), std::move(indices.back()), std::move(val));
    }

    // Not an assignment: build a chain of index nodes as an expression
    ASTNodePtr result = std::make_unique<IndexNode>(std::move(obj), std::move(indices[0]));
    for (size_t i = 1; i < indices.size(); ++i) {
        result = std::make_unique<IndexNode>(std::move(result), std::move(indices[i]));
    }
    return result;
}

ASTNodePtr Parser::arena_method_or_expr() {
    Token name_tok = expect(TokenType::IDENTIFIER);
    ASTNodePtr node = std::make_unique<IdentifierNode>(name_tok);

    while (current_token_ && current_token_->type == TokenType::DELIMITER && current_token_->value == ".") {
        advance(); // consume '.'
        Token member_tok = expect(TokenType::IDENTIFIER);
        std::string member = member_tok.value;

        if (current_token_ && current_token_->type == TokenType::DELIMITER && current_token_->value == "(") {
            advance(); // consume '('
            std::vector<ASTNodePtr> args;
            if (current_token_ && !(current_token_->type == TokenType::DELIMITER && current_token_->value == ")")) {
                while (true) {
                    args.push_back(expression());
                    if (current_token_ && current_token_->type == TokenType::DELIMITER && current_token_->value == ",") {
                        advance();
                    } else {
                        break;
                    }
                }
            }
            expect(TokenType::DELIMITER, ")");
            auto* id_obj = dynamic_cast<IdentifierNode*>(node.get());
            if (id_obj && member == "alloc" && args.size() == 1) {
                node = std::make_unique<ArenaAllocNode>(id_obj->name, std::move(args[0]));
            } else if (id_obj && member == "reset" && args.empty()) {
                node = std::make_unique<ArenaResetNode>(id_obj->name);
            } else {
                node = std::make_unique<MethodCallNode>(std::move(node), member, std::move(args));
            }
        } else if (current_token_ && current_token_->type == TokenType::OPERATOR && current_token_->value == "=") {
            advance(); // consume '='
            ASTNodePtr val = expression();
            return std::make_unique<MemberAssignNode>(std::move(node), member, std::move(val));
        } else {
            node = std::make_unique<MemberAccessNode>(std::move(node), member);
        }
    }
    return node;
}

ASTNodePtr Parser::expression_statement() {
    return expression();
}

ASTNodePtr Parser::expression() {
    return logical_or();
}

ASTNodePtr Parser::logical_or() {
    ASTNodePtr node = logical_and();
    while (current_token_ && (current_token_->value == "or" || current_token_->value == "||")) {
        Token op = *current_token_;
        advance();
        ASTNodePtr right = logical_and();
        node = std::make_unique<BinaryOpNode>(std::move(node), op, std::move(right));
    }
    return node;
}

ASTNodePtr Parser::logical_and() {
    ASTNodePtr node = equality();
    while (current_token_ && (current_token_->value == "and" || current_token_->value == "&&")) {
        Token op = *current_token_;
        advance();
        ASTNodePtr right = equality();
        node = std::make_unique<BinaryOpNode>(std::move(node), op, std::move(right));
    }
    return node;
}

ASTNodePtr Parser::equality() {
    ASTNodePtr node = comparison();
    while (current_token_ && (current_token_->value == "==" || current_token_->value == "!=")) {
        Token op = *current_token_;
        advance();
        ASTNodePtr right = comparison();
        node = std::make_unique<BinaryOpNode>(std::move(node), op, std::move(right));
    }
    return node;
}

ASTNodePtr Parser::comparison() {
    ASTNodePtr node = addition_subtraction();
    while (current_token_ && (current_token_->value == "<" || current_token_->value == ">" || 
                              current_token_->value == "<=" || current_token_->value == ">=")) {
        Token op = *current_token_;
        advance();
        ASTNodePtr right = addition_subtraction();
        node = std::make_unique<BinaryOpNode>(std::move(node), op, std::move(right));
    }
    return node;
}

ASTNodePtr Parser::addition_subtraction() {
    ASTNodePtr node = multiplication_division();
    while (current_token_ && (current_token_->value == "+" || current_token_->value == "-")) {
        Token op = *current_token_;
        advance();
        ASTNodePtr right = multiplication_division();
        node = std::make_unique<BinaryOpNode>(std::move(node), op, std::move(right));
    }
    return node;
}

ASTNodePtr Parser::multiplication_division() {
    ASTNodePtr node = power();
    while (current_token_ && (current_token_->value == "*" || current_token_->value == "/" || 
                              current_token_->value == "%" || current_token_->value == "//")) {
        Token op = *current_token_;
        advance();
        ASTNodePtr right = power();
        node = std::make_unique<BinaryOpNode>(std::move(node), op, std::move(right));
    }
    return node;
}

ASTNodePtr Parser::power() {
    ASTNodePtr node = unary();
    while (current_token_ && current_token_->value == "**") {
        Token op = *current_token_;
        advance();
        ASTNodePtr right = unary();
        node = std::make_unique<BinaryOpNode>(std::move(node), op, std::move(right));
    }
    return node;
}

ASTNodePtr Parser::unary() {
    if (current_token_ && current_token_->type == TokenType::OPERATOR && 
        (current_token_->value == "not" || current_token_->value == "!" || 
         current_token_->value == "-" || current_token_->value == "+")) {
        Token op = *current_token_;
        advance();
        ASTNodePtr expr = unary();
        return std::make_unique<UnaryOpNode>(op, std::move(expr));
    }
    return primary();
}

ASTNodePtr Parser::primary() {
    if (!current_token_) {
        throw std::runtime_error("Unexpected end of input");
    }

    if (current_token_->type == TokenType::INT || current_token_->type == TokenType::FLOAT) {
        Token tok = *current_token_;
        advance();
        return std::make_unique<NumberNode>(tok);
    } else if (current_token_->type == TokenType::STRING) {
        Token tok = *current_token_;
        advance();
        return std::make_unique<StringNode>(tok);
    } else if (current_token_->type == TokenType::KEYWORD && 
               (current_token_->value == "true" || current_token_->value == "false" || current_token_->value == "null")) {
        Token tok = *current_token_;
        advance();
        if (tok.value == "null") {
            return std::make_unique<NoneNode>(tok);
        } else {
            return std::make_unique<BooleanNode>(tok);
        }
    } else if (current_token_->type == TokenType::KEYWORD && 
               (current_token_->value == "int" || current_token_->value == "float" || current_token_->value == "str" || 
                current_token_->value == "len" || current_token_->value == "input" || current_token_->value == "append" || 
                current_token_->value == "pop" || current_token_->value == "print")) {
        std::string func_name = current_token_->value;
        Token name_tok(TokenType::IDENTIFIER, func_name, current_token_->line, current_token_->column);
        advance(); // consume builtin keyword
        if (current_token_ && current_token_->type == TokenType::OPERATOR && current_token_->value == "!") {
            advance(); // consume '!'
        }
        expect(TokenType::DELIMITER, "(");
        std::vector<ASTNodePtr> args;
        if (current_token_ && !(current_token_->type == TokenType::DELIMITER && current_token_->value == ")")) {
            while (true) {
                args.push_back(expression());
                if (current_token_ && current_token_->type == TokenType::DELIMITER && current_token_->value == ",") {
                    advance();
                } else {
                    break;
                }
            }
        }
        expect(TokenType::DELIMITER, ")");
        return std::make_unique<FunctionCallNode>(std::make_unique<IdentifierNode>(name_tok), std::move(args));
    } else if (current_token_->type == TokenType::IDENTIFIER) {
        Token id_tok = *current_token_;
        ASTNodePtr node = std::make_unique<IdentifierNode>(id_tok);
        advance();
        if (current_token_ && current_token_->type == TokenType::OPERATOR && current_token_->value == "!") {
            const Token* next_tok = peek(1);
            if (next_tok && next_tok->type == TokenType::DELIMITER && next_tok->value == "(") {
                advance(); // consume '!'
            }
        }
        // Postfix operators: function call, indexing, or member access
        while (current_token_) {
            if (current_token_->type == TokenType::DELIMITER && current_token_->value == "(") {
                advance(); // consume '('
                std::vector<ASTNodePtr> args;
                if (current_token_ && !(current_token_->type == TokenType::DELIMITER && current_token_->value == ")")) {
                    while (true) {
                        args.push_back(expression());
                        if (current_token_ && current_token_->type == TokenType::DELIMITER && current_token_->value == ",") {
                            advance();
                        } else {
                            break;
                        }
                    }
                }
                expect(TokenType::DELIMITER, ")");
                auto* id_node = dynamic_cast<IdentifierNode*>(node.get());
                if (id_node) {
                    node = std::make_unique<FunctionCallNode>(std::make_unique<IdentifierNode>(id_node->token), std::move(args));
                } else {
                    break;
                }
            } else if (current_token_->type == TokenType::DELIMITER && current_token_->value == "[") {
                advance(); // consume '['
                if (current_token_ && current_token_->type == TokenType::DELIMITER && current_token_->value == ":") {
                    advance(); // consume ':'
                    ASTNodePtr end = nullptr;
                    if (current_token_ && !(current_token_->type == TokenType::DELIMITER && current_token_->value == "]")) {
                        end = expression();
                    }
                    expect(TokenType::DELIMITER, "]");
                    auto slice = std::make_unique<SliceNode>(nullptr, std::move(end));
                    node = std::make_unique<IndexNode>(std::move(node), std::move(slice));
                } else {
                    ASTNodePtr first_expr = expression();
                    if (current_token_ && current_token_->type == TokenType::DELIMITER && current_token_->value == ":") {
                        advance(); // consume ':'
                        ASTNodePtr end = nullptr;
                        if (current_token_ && !(current_token_->type == TokenType::DELIMITER && current_token_->value == "]")) {
                            end = expression();
                        }
                        expect(TokenType::DELIMITER, "]");
                        auto slice = std::make_unique<SliceNode>(std::move(first_expr), std::move(end));
                        node = std::make_unique<IndexNode>(std::move(node), std::move(slice));
                    } else {
                        expect(TokenType::DELIMITER, "]");
                        node = std::make_unique<IndexNode>(std::move(node), std::move(first_expr));
                    }
                }
            } else if (current_token_->type == TokenType::DELIMITER && current_token_->value == ".") {
                advance(); // consume '.'
                Token member_tok = expect(TokenType::IDENTIFIER);
                std::string member = member_tok.value;
                if (current_token_ && current_token_->type == TokenType::DELIMITER && current_token_->value == "(") {
                    advance(); // consume '('
                    std::vector<ASTNodePtr> args;
                    if (current_token_ && !(current_token_->type == TokenType::DELIMITER && current_token_->value == ")")) {
                        while (true) {
                            args.push_back(expression());
                            if (current_token_ && current_token_->type == TokenType::DELIMITER && current_token_->value == ",") {
                                advance();
                            } else {
                                break;
                            }
                        }
                    }
                    expect(TokenType::DELIMITER, ")");
                    auto* id_obj = dynamic_cast<IdentifierNode*>(node.get());
                    if (id_obj && member == "alloc" && args.size() == 1) {
                        node = std::make_unique<ArenaAllocNode>(id_obj->name, std::move(args[0]));
                    } else if (id_obj && member == "reset" && args.empty()) {
                        node = std::make_unique<ArenaResetNode>(id_obj->name);
                    } else {
                        node = std::make_unique<MethodCallNode>(std::move(node), member, std::move(args));
                    }
                } else {
                    node = std::make_unique<MemberAccessNode>(std::move(node), member);
                }
            } else {
                break;
            }
        }
        return node;
    } else if (current_token_->type == TokenType::DELIMITER && current_token_->value == "(") {
        advance(); // consume '('
        ASTNodePtr node = expression();
        expect(TokenType::DELIMITER, ")");
        return node;
    } else if (current_token_->type == TokenType::DELIMITER && current_token_->value == "[") {
        advance(); // consume '['
        std::vector<ASTNodePtr> elements;
        if (current_token_ && !(current_token_->type == TokenType::DELIMITER && current_token_->value == "]")) {
            while (true) {
                elements.push_back(expression());
                if (current_token_ && current_token_->type == TokenType::DELIMITER && current_token_->value == ",") {
                    advance();
                } else {
                    break;
                }
            }
        }
        expect(TokenType::DELIMITER, "]");
        return std::make_unique<ListNode>(std::move(elements));
    }

    std::string context = "";
    int start_pos = std::max(0, (int)pos_ - 5);
    for (int i = start_pos; i < std::min((int)tokens_.size(), (int)pos_ + 5); ++i) {
        if (i == (int)pos_) {
            context += " >>>" + tokens_[i].value + "<<< ";
        } else {
            context += " " + tokens_[i].value + " ";
        }
    }
    throw std::runtime_error(std::string("Unexpected token: ") + std::string(to_string(current_token_->type)) + 
                             " (" + current_token_->value + ") at line " + 
                             std::to_string(current_token_->line) + ", column " + 
                             std::to_string(current_token_->column) + ". Context:" + context);
    return nullptr;
}

} // namespace wyrm
