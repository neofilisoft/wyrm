#ifndef WYRM_LEXER_HPP
#define WYRM_LEXER_HPP

#include <vector>
#include <string_view>
#include <unordered_set>
#include "../token/token.hpp"

namespace wyrm {

class Lexer {
public:
    explicit Lexer(std::string_view source);

    std::vector<Token> tokenize();

private:
    std::string_view source_;
    size_t pos_;
    size_t line_;
    size_t column_;

    char current_char() const;
    char peek(size_t offset = 1) const;
    void advance();
    void skip_whitespace();
    void skip_comment();
    void skip_block_comment();

    Token read_number();
    Token read_string();
    Token read_identifier();
    Token read_operator();

    static const std::unordered_set<std::string_view> KEYWORDS;
    static const std::unordered_set<char> OPERATOR_CHARS;
    static const std::unordered_set<std::string_view> OPERATOR_KEYS;
    static const std::unordered_set<char> DELIMITERS;
};

} // namespace wyrm

#endif // WYRM_LEXER_HPP
