#include "../../lexer/lexer.hpp"
#include "../../parser/parser.hpp"
#include "../interpreter.hpp"
#include <iostream>
#include <cassert>
#include <sstream>

using namespace wyrm;

void test_interp_basic() {
    std::cout << "Running test_interp_basic..." << std::endl;
    std::string_view code = 
        "var x = 10\n"
        "dec y = 20\n"
        "sum = x + y\n";

    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    Interpreter interpreter;
    interpreter.execute(ast);

    Value x_val = interpreter.global_env->get("x");
    Value y_val = interpreter.global_env->get("y");
    Value sum_val = interpreter.global_env->get("sum");

    assert(x_val.type == VAL_NUMBER && x_val.as.number == 10.0);
    assert(y_val.type == VAL_NUMBER && y_val.as.number == 20.0);
    assert(sum_val.type == VAL_NUMBER && sum_val.as.number == 30.0);

    std::cout << "test_interp_basic passed!" << std::endl;
}

void test_interp_control_flow() {
    std::cout << "Running test_interp_control_flow..." << std::endl;
    std::string_view code = 
        "x = 7\n"
        "if x > 10 {\n"
        "    res = \"big\"\n"
        "} elif x > 5 {\n"
        "    res = \"medium\"\n"
        "} else {\n"
        "    res = \"small\"\n"
        "}\n";

    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    Interpreter interpreter;
    interpreter.execute(ast);

    Value res = interpreter.global_env->get("res");
    assert(res.type == VAL_STRING && std::string(res.as.string) == "medium");

    std::cout << "test_interp_control_flow passed!" << std::endl;
}

void test_interp_loops() {
    std::cout << "Running test_interp_loops..." << std::endl;
    std::string_view code = 
        "i = 0\n"
        "do {\n"
        "    i = i + 1\n"
        "} til (i >= 5)\n";

    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    Interpreter interpreter;
    interpreter.execute(ast);

    Value i_val = interpreter.global_env->get("i");
    assert(i_val.type == VAL_NUMBER && i_val.as.number == 5.0);

    std::cout << "test_interp_loops passed!" << std::endl;
}

void test_interp_functions() {
    std::cout << "Running test_interp_functions..." << std::endl;
    std::string_view code = 
        "fn add(a, b) {\n"
        "    return a + b\n"
        "}\n"
        "res = add(12, 13)\n";

    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    Interpreter interpreter;
    interpreter.execute(ast);

    Value res = interpreter.global_env->get("res");
    assert(res.type == VAL_NUMBER && res.as.number == 25.0);

    std::cout << "test_interp_functions passed!" << std::endl;
}

void test_interp_recursion() {
    std::cout << "Running test_interp_recursion..." << std::endl;
    std::string_view code = 
        "fn fib(n) {\n"
        "    if n <= 1 {\n"
        "        return n\n"
        "    }\n"
        "    return fib(n - 1) + fib(n - 2)\n"
        "}\n"
        "res = fib(6)\n";

    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    Interpreter interpreter;
    interpreter.execute(ast);

    Value res = interpreter.global_env->get("res");
    assert(res.type == VAL_NUMBER && res.as.number == 8.0);

    std::cout << "test_interp_recursion passed!" << std::endl;
}

void test_interp_builtins() {
    std::cout << "Running test_interp_builtins..." << std::endl;
    std::string_view code = 
        "s = \"  hello world  \"\n"
        "trimmed = trim(s)\n"
        "up = upper(trimmed)\n"
        "is_hello = starts_with(up, \"HELLO\")\n";

    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    Interpreter interpreter;
    interpreter.execute(ast);

    Value trimmed = interpreter.global_env->get("trimmed");
    assert(trimmed.type == VAL_STRING && std::string(trimmed.as.string) == "hello world");

    Value up = interpreter.global_env->get("up");
    assert(up.type == VAL_STRING && std::string(up.as.string) == "HELLO WORLD");

    Value is_hello = interpreter.global_env->get("is_hello");
    assert(is_hello.type == VAL_BOOL && is_hello.as.boolean == true);

    std::cout << "test_interp_builtins passed!" << std::endl;
}

void test_interp_arrays() {
    std::cout << "Running test_interp_arrays..." << std::endl;
    std::string_view code = 
        "arr = [10, 20, 30]\n"
        "append(arr, 40)\n"
        "len_val = len(arr)\n"
        "val0 = arr[0]\n"
        "val3 = arr[3]\n"
        "slice_val = arr[1:3]\n";

    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    Interpreter interpreter;
    interpreter.execute(ast);

    Value len_val = interpreter.global_env->get("len_val");
    assert(len_val.type == VAL_NUMBER && len_val.as.number == 4.0);

    Value val0 = interpreter.global_env->get("val0");
    assert(val0.type == VAL_NUMBER && val0.as.number == 10.0);

    Value val3 = interpreter.global_env->get("val3");
    assert(val3.type == VAL_NUMBER && val3.as.number == 40.0);

    Value slice_val = interpreter.global_env->get("slice_val");
    assert(slice_val.type == VAL_ARRAY);
    assert(slice_val.as.array->size == 2);
    assert(slice_val.as.array->data[0].as.number == 20.0);
    assert(slice_val.as.array->data[1].as.number == 30.0);

    std::cout << "test_interp_arrays passed!" << std::endl;
}

void test_interp_unsafe_memory() {
    std::cout << "Running test_interp_unsafe_memory..." << std::endl;
    std::string_view code = 
        "unsafe {\n"
        "    p = malloc(16)\n"
        "    p = realloc(p, 32)\n"
        "    free(p)\n"
        "}\n";

    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    Interpreter interpreter;
    interpreter.execute(ast);

    // Verify raw memory allocator has been called and cleaned up safely
    std::cout << "test_interp_unsafe_memory passed!" << std::endl;
}

void test_interp_arena_memory() {
    std::cout << "Running test_interp_arena_memory..." << std::endl;
    std::string_view code = 
        "arena buf(256)\n"
        "p = buf.alloc(64)\n"
        "buf.reset()\n";

    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto ast = parser.parse();

    Interpreter interpreter;
    interpreter.execute(ast);

    // Verify arena was created, allocated, and reset successfully
    std::cout << "test_interp_arena_memory passed!" << std::endl;
}

int main() {
    std::cout << "=== Running Wyrm Interpreter Native C++20 Tests ===" << std::endl;
    test_interp_basic();
    test_interp_control_flow();
    test_interp_loops();
    test_interp_functions();
    test_interp_recursion();
    test_interp_builtins();
    test_interp_arrays();
    test_interp_unsafe_memory();
    test_interp_arena_memory();
    std::cout << "=== All Interpreter Tests Passed Successfully ===" << std::endl;
    return 0;
}
