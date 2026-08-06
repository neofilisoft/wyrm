#include "../../lexer/lexer.hpp"
#include "../parser.hpp"
#include <iostream>
#include <cassert>

using namespace wyrm;

void test_parse_arithmetic() {
    std::cout << "Running test_parse_arithmetic..." << std::endl;
    std::string_view code = "x = 5 + 3 * 2";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto stmts = parser.parse();

    assert(stmts.size() == 1);
    auto* assign = dynamic_cast<AssignmentNode*>(stmts[0].get());
    assert(assign != nullptr);
    assert(assign->var_name->name == "x");
    assert(!assign->is_declaration);

    auto* add_op = dynamic_cast<BinaryOpNode*>(assign->value.get());
    assert(add_op != nullptr);
    assert(add_op->op.value == "+");

    auto* left = dynamic_cast<NumberNode*>(add_op->left.get());
    assert(left != nullptr);
    assert(left->value == 5.0);

    auto* mul_op = dynamic_cast<BinaryOpNode*>(add_op->right.get());
    assert(mul_op != nullptr);
    assert(mul_op->op.value == "*");

    std::cout << "test_parse_arithmetic passed!" << std::endl;
}

void test_parse_declarations() {
    std::cout << "Running test_parse_declarations..." << std::endl;
    std::string_view code = "var x = 10\ndec y = 20";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto stmts = parser.parse();

    assert(stmts.size() == 2);

    auto* decl1 = dynamic_cast<AssignmentNode*>(stmts[0].get());
    assert(decl1 != nullptr);
    assert(decl1->var_name->name == "x");
    assert(decl1->is_declaration);
    assert(decl1->declaration_type == "var");

    auto* decl2 = dynamic_cast<AssignmentNode*>(stmts[1].get());
    assert(decl2 != nullptr);
    assert(decl2->var_name->name == "y");
    assert(decl2->is_declaration);
    assert(decl2->declaration_type == "dec");

    std::cout << "test_parse_declarations passed!" << std::endl;
}

void test_parse_if_stmt() {
    std::cout << "Running test_parse_if_stmt..." << std::endl;
    std::string_view code = 
        "if x > 10 {\n"
        "    print(\"big\")\n"
        "} elif x > 5 {\n"
        "    print(\"medium\")\n"
        "} else {\n"
        "    print(\"small\")\n"
        "}";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto stmts = parser.parse();

    assert(stmts.size() == 1);
    auto* if_node = dynamic_cast<IfNode*>(stmts[0].get());
    assert(if_node != nullptr);
    assert(if_node->then_body.size() == 1);
    assert(if_node->elif_clauses.size() == 1);
    assert(if_node->else_body.size() == 1);

    std::cout << "test_parse_if_stmt passed!" << std::endl;
}

void test_parse_repeat_stmt() {
    std::cout << "Running test_parse_repeat_stmt..." << std::endl;
    std::string_view code = 
        "do {\n"
        "    x = x + 1\n"
        "} til (x >= 5)";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto stmts = parser.parse();

    assert(stmts.size() == 1);
    auto* repeat_node = dynamic_cast<RepeatNode*>(stmts[0].get());
    assert(repeat_node != nullptr);
    assert(repeat_node->body.size() == 1);
    assert(repeat_node->condition != nullptr);

    std::cout << "test_parse_repeat_stmt passed!" << std::endl;
}

void test_parse_func_def_call() {
    std::cout << "Running test_parse_func_def_call..." << std::endl;
    std::string_view code = 
        "fn add(a, b) {\n"
        "    return a + b\n"
        "}\n"
        "res = add(5, 10)";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto stmts = parser.parse();

    assert(stmts.size() == 2);
    auto* def = dynamic_cast<FunctionDefNode*>(stmts[0].get());
    assert(def != nullptr);
    assert(def->name->name == "add");
    assert(def->params.size() == 2);
    assert(def->body.size() == 1);

    auto* call = dynamic_cast<AssignmentNode*>(stmts[1].get());
    assert(call != nullptr);
    auto* call_node = dynamic_cast<FunctionCallNode*>(call->value.get());
    assert(call_node != nullptr);
    assert(call_node->name->name == "add");
    assert(call_node->args.size() == 2);

    std::cout << "test_parse_func_def_call passed!" << std::endl;
}

void test_parse_arena_stmt() {
    std::cout << "Running test_parse_arena_stmt..." << std::endl;
    std::string_view code = 
        "arena buf(256)\n"
        "p = buf.alloc(64)\n"
        "buf.reset()";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto stmts = parser.parse();

    assert(stmts.size() == 3);
    auto* arena_decl = dynamic_cast<ArenaNode*>(stmts[0].get());
    assert(arena_decl != nullptr);
    assert(arena_decl->name == "buf");

    auto* arena_alloc = dynamic_cast<AssignmentNode*>(stmts[1].get());
    assert(arena_alloc != nullptr);
    auto* alloc_node = dynamic_cast<ArenaAllocNode*>(arena_alloc->value.get());
    assert(alloc_node != nullptr);
    assert(alloc_node->arena_name == "buf");

    auto* arena_reset = dynamic_cast<ArenaResetNode*>(stmts[2].get());
    assert(arena_reset != nullptr);
    assert(arena_reset->arena_name == "buf");

    std::cout << "test_parse_arena_stmt passed!" << std::endl;
}

void test_parse_unsafe_owned() {
    std::cout << "Running test_parse_unsafe_owned..." << std::endl;
    std::string_view code = 
        "unsafe {\n"
        "    owned var x = malloc(16)\n"
        "}";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto stmts = parser.parse();

    assert(stmts.size() == 1);
    auto* unsafe_node = dynamic_cast<UnsafeBlockNode*>(stmts[0].get());
    assert(unsafe_node != nullptr);
    assert(unsafe_node->body.size() == 1);

    auto* owned_node = dynamic_cast<OwnedDeclNode*>(unsafe_node->body[0].get());
    assert(owned_node != nullptr);
    assert(owned_node->var_name->name == "x");
    assert(owned_node->declaration_type == "var");

    std::cout << "test_parse_unsafe_owned passed!" << std::endl;
}

int main() {
    std::cout << "=== Running Wyrm Parser Native C++20 Tests ===" << std::endl;
    test_parse_arithmetic();
    test_parse_declarations();
    test_parse_if_stmt();
    test_parse_repeat_stmt();
    test_parse_func_def_call();
    test_parse_arena_stmt();
    test_parse_unsafe_owned();
    std::cout << "=== All Parser Tests Passed Successfully ===" << std::endl;
    return 0;
}
