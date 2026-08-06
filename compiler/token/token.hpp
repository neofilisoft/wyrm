#ifndef WYRM_TOKEN_HPP
#define WYRM_TOKEN_HPP

#include <string>
#include <string_view>
#include <ostream>

namespace wyrm {

enum class TokenType {
    INT,
    FLOAT,
    STRING,
    IDENTIFIER,
    KEYWORD,
    OPERATOR,
    DELIMITER,
    NEWLINE,
    END_OF_FILE
};

inline std::string_view to_string(TokenType type) {
    switch (type) {
        case TokenType::INT:         return "INT";
        case TokenType::FLOAT:       return "FLOAT";
        case TokenType::STRING:      return "STRING";
        case TokenType::IDENTIFIER:  return "IDENTIFIER";
        case TokenType::KEYWORD:     return "KEYWORD";
        case TokenType::OPERATOR:    return "OPERATOR";
        case TokenType::DELIMITER:   return "DELIMITER";
        case TokenType::NEWLINE:     return "NEWLINE";
        case TokenType::END_OF_FILE: return "EOF";
    }
    return "UNKNOWN";
}

struct Token {
    TokenType type;
    std::string value;
    size_t line;
    size_t column;

    Token(TokenType t, std::string val, size_t l, size_t c)
        : type(t), value(std::move(val)), line(l), column(c) {}
};

inline std::ostream& operator<<(std::ostream& os, const Token& token) {
    os << "Token(" << to_string(token.type) << ", \"" << token.value << "\", line=" << token.line << ", col=" << token.column << ")";
    return os;
}

} // namespace wyrm

#endif // WYRM_TOKEN_HPP
