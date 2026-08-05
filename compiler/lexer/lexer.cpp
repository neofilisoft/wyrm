#include "lexer.hpp"
#include <stdexcept>
#include <cctype>
#include <algorithm>

namespace wyrm {

const std::unordered_set<std::string_view> Lexer::KEYWORDS = {
    "fn", "true", "false", "null",
    "if", "elif", "else",
    "repeat", "do", "til",
    "break", "continue",
    "print", "input",
    "int", "float", "str",
    "len", "append", "pop",
    "return",
    "var", "dec", "use",
    "unsafe", "owned", "arena"
};

const std::unordered_set<char> Lexer::OPERATOR_CHARS = {
    '+', '-', '*', '/', '%', '=', '!', '<', '>', '&', '|'
};

const std::unordered_set<std::string_view> Lexer::OPERATOR_KEYS = {
    "+", "-", "*", "/", "//", "%",
    "=", "+=", "-=", "*=", "/=", "%=",
    "==", "!=", "<", ">", "<=", ">=",
    "&", "&&", "|", "||",
    "!",
    "and", "or", "not"
};

const std::unordered_set<char> Lexer::DELIMITERS = {
    '(', ')', '{', '}', '[', ']',
    ',', ':', ';', '.'
};

Lexer::Lexer(std::string_view source)
    : source_(source), pos_(0), line_(1), column_(1) {}

char Lexer::current_char() const {
    if (pos_ >= source_.size()) {
        return '\0';
    }
    return source_[pos_];
}

char Lexer::peek(size_t offset) const {
    if (pos_ + offset >= source_.size()) {
        return '\0';
    }
    return source_[pos_ + offset];
}

void Lexer::advance() {
    char curr = current_char();
    if (curr == '\0') {
        return;
    }
    if (curr == '\n') {
        line_++;
        column_ = 1;
    } else {
        column_++;
    }
    pos_++;
}

void Lexer::skip_whitespace() {
    while (true) {
        char curr = current_char();
        if (curr != '\0' && curr != '\n' && std::isspace(static_cast<unsigned char>(curr))) {
            advance();
        } else {
            break;
        }
    }
}

void Lexer::skip_comment() {
    while (true) {
        char curr = current_char();
        if (curr != '\0' && curr != '\n') {
            advance();
        } else {
            break;
        }
    }
}

Token Lexer::read_number() {
    size_t start_col = column_;
    std::string result;
    size_t dot_count = 0;

    while (true) {
        char curr = current_char();
        if (curr == '\0') {
            break;
        }
        if (std::isdigit(static_cast<unsigned char>(curr))) {
            result += curr;
            advance();
        } else if (curr == '.') {
            // Check if next character is a digit to differentiate member access like 'obj.method' from float '.5'
            char next = peek(1);
            if (std::isdigit(static_cast<unsigned char>(next)) && dot_count == 0) {
                dot_count++;
                result += curr;
                advance();
            } else {
                break; // Delimiter '.' or invalid dot sequence
            }
        } else {
            break;
        }
    }

    if (dot_count == 1) {
        return Token(TokenType::FLOAT, std::move(result), line_, start_col);
    } else {
        return Token(TokenType::INT, std::move(result), line_, start_col);
    }
}

Token Lexer::read_string() {
    size_t start_col = column_;
    char quote_char = current_char(); // Either " or '
    advance(); // Skip quote

    std::string result;
    while (true) {
        char curr = current_char();
        if (curr == '\0') {
            throw std::runtime_error("SyntaxError: Unterminated string literal at line " + std::to_string(line_) + ", col " + std::to_string(start_col));
        }
        if (curr == quote_char) {
            advance(); // Skip closing quote
            break;
        }
        if (curr == '\\') {
            advance();
            char escaped = current_char();
            if (escaped == '\0') {
                throw std::runtime_error("SyntaxError: Unterminated string escape sequence at line " + std::to_string(line_) + ", col " + std::to_string(column_));
            }
            if (escaped == 'n') {
                result += '\n';
            } else if (escaped == 't') {
                result += '\t';
            } else if (escaped == 'r') {
                result += '\r';
            } else if (escaped == '\\') {
                result += '\\';
            } else if (escaped == quote_char) {
                result += quote_char;
            } else {
                result += escaped; // Unknown escape sequence, keep as is
            }
        } else {
            result += curr;
        }
        advance();
    }

    return Token(TokenType::STRING, std::move(result), line_, start_col);
}

Token Lexer::read_identifier() {
    size_t start_col = column_;
    std::string result;

    while (true) {
        char curr = current_char();
        if (curr == '\0') {
            break;
        }
        if (std::isalnum(static_cast<unsigned char>(curr)) || curr == '_') {
            result += curr;
            advance();
        } else {
            break;
        }
    }

    if (KEYWORDS.contains(result)) {
        return Token(TokenType::KEYWORD, std::move(result), line_, start_col);
    } else {
        return Token(TokenType::IDENTIFIER, std::move(result), line_, start_col);
    }
}

Token Lexer::read_operator() {
    size_t start_col = column_;
    std::string op;

    // Greedy search for longest valid operator starting at current position
    for (size_t i = 1; i <= 10; ++i) {
        if (pos_ + i > source_.size()) {
            break;
        }
        std::string_view sub = source_.substr(pos_, i);
        if (OPERATOR_KEYS.contains(sub)) {
            op = std::string(sub);
        } else {
            break;
        }
    }

    if (!op.empty()) {
        for (size_t k = 0; k < op.size(); ++k) {
            advance();
        }
        return Token(TokenType::OPERATOR, std::move(op), line_, start_col);
    }

    // Fallback if no operator matches but character is in OPERATOR_CHARS
    std::string fallback(1, current_char());
    advance();
    return Token(TokenType::OPERATOR, std::move(fallback), line_, start_col);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (true) {
        char curr = current_char();
        if (curr == '\0') {
            break;
        }

        if (curr != '\n' && std::isspace(static_cast<unsigned char>(curr))) {
            skip_whitespace();
        } else if (curr == '\n') {
            tokens.emplace_back(TokenType::NEWLINE, "\n", line_, column_);
            advance();
        } else if (std::isdigit(static_cast<unsigned char>(curr))) {
            tokens.push_back(read_number());
        } else if (curr == '"' || curr == '\'') {
            tokens.push_back(read_string());
        } else if (std::isalpha(static_cast<unsigned char>(curr)) || curr == '_') {
            // Read identifier, check if it's a word operator 'and', 'or', 'not' first
            Token tok = read_identifier();
            if (tok.type == TokenType::KEYWORD && (tok.value == "and" || tok.value == "or" || tok.value == "not")) {
                tok.type = TokenType::OPERATOR;
            }
            tokens.push_back(std::move(tok));
        } else if (curr == '/' && peek(1) == '/') {
            // Single-line comment: check prefix rules
            if (!tokens.empty() && tokens.back().type != TokenType::NEWLINE) {
                throw std::runtime_error("SyntaxError: Unexpected comment start '//' at line " + std::to_string(line_) + ", col " + std::to_string(column_));
            }
            skip_comment();
        } else if (OPERATOR_CHARS.contains(curr)) {
            tokens.push_back(read_operator());
        } else if (DELIMITERS.contains(curr)) {
            std::string delim(1, curr);
            tokens.emplace_back(TokenType::DELIMITER, std::move(delim), line_, column_);
            advance();
        } else {
            throw std::runtime_error(std::string("SyntaxError: Unknown character: ") + curr + " at line " + std::to_string(line_) + ", col " + std::to_string(column_));
        }
    }

    tokens.emplace_back(TokenType::END_OF_FILE, "", line_, column_);
    return tokens;
}

} // namespace wyrm
