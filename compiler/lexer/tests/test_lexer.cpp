#include "../lexer.hpp"
#include <iostream>
#include <cassert>
#include <algorithm>

using namespace wyrm;

void test_basic_tokens() {
    std::cout << "Running test_basic_tokens..." << std::endl;
    std::string_view code = 
        "fn main() {\n"
        "    x = 42\n"
        "    y = 3.14\n"
        "    s = \"hello\"\n"
        "    b = true\n"
        "    print(x)\n"
        "}\n";

    Lexer lexer(code);
    auto tokens = lexer.tokenize();

    assert(!tokens.empty());
    assert(tokens.back().type == TokenType::END_OF_FILE);

    // Look for fn
    assert(tokens[0].type == TokenType::KEYWORD);
    assert(tokens[0].value == "fn");

    // Look for main
    assert(tokens[1].type == TokenType::IDENTIFIER);
    assert(tokens[1].value == "main");

    // Look for ( and )
    assert(tokens[2].type == TokenType::DELIMITER);
    assert(tokens[2].value == "(");
    assert(tokens[3].type == TokenType::DELIMITER);
    assert(tokens[3].value == ")");

    // Look for {
    assert(tokens[4].type == TokenType::DELIMITER);
    assert(tokens[4].value == "{");

    // Newline at index 5
    assert(tokens[5].type == TokenType::NEWLINE);

    std::cout << "test_basic_tokens passed!" << std::endl;
}

void test_numbers() {
    std::cout << "Running test_numbers..." << std::endl;
    std::string_view code = "x = 42\ny = 3.14\nz = 0";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();

    size_t int_count = 0;
    size_t float_count = 0;

    for (const auto& tok : tokens) {
        if (tok.type == TokenType::INT) {
            int_count++;
            if (tok.value == "42") {
                assert(tok.line == 1);
            } else if (tok.value == "0") {
                assert(tok.line == 3);
            }
        } else if (tok.type == TokenType::FLOAT) {
            float_count++;
            assert(tok.value == "3.14");
            assert(tok.line == 2);
        }
    }

    assert(int_count == 2);
    assert(float_count == 1);
    std::cout << "test_numbers passed!" << std::endl;
}

void test_strings() {
    std::cout << "Running test_strings..." << std::endl;
    std::string_view code = "s = \"hello\\nworld\"\nt = 'world'";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();

    size_t str_count = 0;
    for (const auto& tok : tokens) {
        if (tok.type == TokenType::STRING) {
            str_count++;
            if (tok.value == "hello\nworld") {
                assert(tok.line == 1);
            } else if (tok.value == "world") {
                assert(tok.line == 2);
            }
        }
    }

    assert(str_count == 2);
    std::cout << "test_strings passed!" << std::endl;
}

void test_keywords() {
    std::cout << "Running test_keywords..." << std::endl;
    std::string_view code = "fn main() { if true else false unsafe owned arena var dec use }";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();

    std::vector<std::string_view> keywords_found;
    for (const auto& tok : tokens) {
        if (tok.type == TokenType::KEYWORD) {
            keywords_found.push_back(tok.value);
        }
    }

    assert(std::find(keywords_found.begin(), keywords_found.end(), "fn") != keywords_found.end());
    assert(std::find(keywords_found.begin(), keywords_found.end(), "if") != keywords_found.end());
    assert(std::find(keywords_found.begin(), keywords_found.end(), "else") != keywords_found.end());
    assert(std::find(keywords_found.begin(), keywords_found.end(), "true") != keywords_found.end());
    assert(std::find(keywords_found.begin(), keywords_found.end(), "false") != keywords_found.end());
    assert(std::find(keywords_found.begin(), keywords_found.end(), "unsafe") != keywords_found.end());
    assert(std::find(keywords_found.begin(), keywords_found.end(), "owned") != keywords_found.end());
    assert(std::find(keywords_found.begin(), keywords_found.end(), "arena") != keywords_found.end());
    assert(std::find(keywords_found.begin(), keywords_found.end(), "var") != keywords_found.end());
    assert(std::find(keywords_found.begin(), keywords_found.end(), "dec") != keywords_found.end());
    assert(std::find(keywords_found.begin(), keywords_found.end(), "use") != keywords_found.end());

    std::cout << "test_keywords passed!" << std::endl;
}

void test_operators() {
    std::cout << "Running test_operators..." << std::endl;
    std::string_view code = "a = b + c * d / e - f && g || h != i <= j >= k += l and m or not n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();

    size_t op_count = 0;
    for (const auto& tok : tokens) {
        if (tok.type == TokenType::OPERATOR) {
            op_count++;
        }
    }

    assert(op_count == 14);
    std::cout << "test_operators passed!" << std::endl;
}

void test_delimiters() {
    std::cout << "Running test_delimiters..." << std::endl;
    std::string_view code = "if (x > 0) { return x; }";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();

    std::vector<std::string> delims;
    for (const auto& tok : tokens) {
        if (tok.type == TokenType::DELIMITER) {
            delims.push_back(tok.value);
        }
    }

    assert(delims.size() == 5);
    assert(delims[0] == "(");
    assert(delims[1] == ")");
    assert(delims[2] == "{");
    assert(delims[3] == ";");
    assert(delims[4] == "}");

    std::cout << "test_delimiters passed!" << std::endl;
}

void test_comments() {
    std::cout << "Running test_comments..." << std::endl;
    // Comments at start of line are valid
    std::string_view valid_code = "// comment at start\nx = 10\n// comment again\ny = 20";
    Lexer lexer1(valid_code);
    auto tokens1 = lexer1.tokenize();
    assert(tokens1.size() > 0);

    // Comment in middle of statement raises SyntaxError
    std::string_view invalid_code = "x = 10 // comment here";
    Lexer lexer2(invalid_code);
    bool caught = false;
    try {
        lexer2.tokenize();
    } catch (const std::runtime_error& e) {
        caught = true;
    }
    assert(caught);

    std::cout << "test_comments passed!" << std::endl;
}

int main() {
    std::cout << "=== Running Wyrm Lexer Native C++20 Tests ===" << std::endl;
    test_basic_tokens();
    test_numbers();
    test_strings();
    test_keywords();
    test_operators();
    test_delimiters();
    test_comments();
    std::cout << "=== All Tests Passed Successfully ===" << std::endl;
    return 0;
}
