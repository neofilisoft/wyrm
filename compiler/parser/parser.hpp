#ifndef WYRM_PARSER_HPP
#define WYRM_PARSER_HPP

#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include "../token/token.hpp"
#include "../ast/ast.hpp"

namespace wyrm {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    std::vector<ASTNodePtr> parse();

private:
    std::vector<Token> tokens_;
    size_t pos_;
    const Token* current_token_;

    const Token* peek(size_t offset = 1) const;
    void advance();
    Token expect(TokenType type, const std::string& value = "");

    ASTNodePtr statement();
    ASTNodePtr use_statement();
    ASTNodePtr function_def();
    ASTNodePtr return_statement();
    ASTNodePtr break_statement();
    ASTNodePtr continue_statement();
    ASTNodePtr print_statement();
    ASTNodePtr if_statement();
    ASTNodePtr repeat_statement();
    ASTNodePtr unsafe_block();
    ASTNodePtr owned_declaration();
    ASTNodePtr arena_declaration();
    ASTNodePtr variable_declaration(const std::string& var_type);
    
    ASTNodePtr assignment();
    ASTNodePtr indexed_assignment_or_expr();
    ASTNodePtr arena_method_or_expr();
    ASTNodePtr expression_statement();

    ASTNodePtr expression();
    ASTNodePtr logical_or();
    ASTNodePtr logical_and();
    ASTNodePtr equality();
    ASTNodePtr comparison();
    ASTNodePtr addition_subtraction();
    ASTNodePtr multiplication_division();
    ASTNodePtr power();
    ASTNodePtr unary();
    ASTNodePtr primary();

    ASTNodePtr finish_call(ASTNodePtr name);
    ASTNodePtr finish_subscript(ASTNodePtr obj);
};

} // namespace wyrm

#endif // WYRM_PARSER_HPP
