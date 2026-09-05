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

void test_parse_struct() {
    std::cout << "Running test_parse_struct..." << std::endl;
    std::string_view code = 
        "struct Vector2 {\n"
        "    x,\n"
        "    y,\n"
        "    fn length_sq(self) {\n"
        "        return self.x * self.x + self.y * self.y\n"
        "    }\n"
        "}\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto stmts = parser.parse();

    assert(stmts.size() == 1);
    auto* s_def = dynamic_cast<StructDefNode*>(stmts[0].get());
    assert(s_def != nullptr);
    assert(s_def->name == "Vector2");
    assert(s_def->fields.size() == 2);
    assert(s_def->fields[0] == "x");
    assert(s_def->fields[1] == "y");
    assert(s_def->methods.size() == 1);
    assert(s_def->methods[0]->name->name == "length_sq");
    assert(s_def->methods[0]->params.size() == 1);
    assert(s_def->methods[0]->params[0]->name == "self");

    std::cout << "test_parse_struct passed!" << std::endl;
}

void test_parse_type_annotations() {
    std::cout << "Running test_parse_type_annotations..." << std::endl;
    std::string_view code = 
        "var count: i32 = 42\n"
        "var big_num: i64 = 9000000000\n"
        "var small_val: u8 = 255\n"
        "var flag: bool = true\n"
        "var flt: f32 = 3.14\n"
        "owned var ptr_data: i64 = 1000\n"
        "fn add(a: i32, b: i32): i32 {\n"
        "    return a + b\n"
        "}\n"
        "struct Particle {\n"
        "    x: f64,\n"
        "    y: f64,\n"
        "    fn move(self, dx: f64, dy: f64) {\n"
        "        self.x = self.x + dx\n"
        "    }\n"
        "}\n";
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    auto stmts = parser.parse();

    assert(stmts.size() == 8);

    // 1. var count: i32 = 42
    auto* var_decl = dynamic_cast<AssignmentNode*>(stmts[0].get());
    assert(var_decl != nullptr);
    assert(var_decl->var_name->name == "count");
    assert(var_decl->type_annotation == "i32");

    // 2. var big_num: i64 = 9000000000
    auto* i64_decl = dynamic_cast<AssignmentNode*>(stmts[1].get());
    assert(i64_decl != nullptr);
    assert(i64_decl->var_name->name == "big_num");
    assert(i64_decl->type_annotation == "i64");

    // 3. var small_val: u8 = 255
    auto* u8_decl = dynamic_cast<AssignmentNode*>(stmts[2].get());
    assert(u8_decl != nullptr);
    assert(u8_decl->var_name->name == "small_val");
    assert(u8_decl->type_annotation == "u8");

    // 4. var flag: bool = true
    auto* bool_decl = dynamic_cast<AssignmentNode*>(stmts[3].get());
    assert(bool_decl != nullptr);
    assert(bool_decl->var_name->name == "flag");
    assert(bool_decl->type_annotation == "bool");

    // 5. var flt: f32 = 3.14
    auto* f32_decl = dynamic_cast<AssignmentNode*>(stmts[4].get());
    assert(f32_decl != nullptr);
    assert(f32_decl->var_name->name == "flt");
    assert(f32_decl->type_annotation == "f32");

    // 6. owned var ptr_data: i64 = 1000
    auto* owned_decl = dynamic_cast<OwnedDeclNode*>(stmts[5].get());
    assert(owned_decl != nullptr);
    assert(owned_decl->var_name->name == "ptr_data");
    assert(owned_decl->type_annotation == "i64");

    // 7. fn add(a: i32, b: i32): i32
    auto* fn_def = dynamic_cast<FunctionDefNode*>(stmts[6].get());
    assert(fn_def != nullptr);
    assert(fn_def->name->name == "add");
    assert(fn_def->params.size() == 2);
    assert(fn_def->params[0]->name == "a");
    assert(fn_def->param_types.size() == 2);
    assert(fn_def->param_types[0] == "i32");
    assert(fn_def->param_types[1] == "i32");
    assert(fn_def->return_type == "i32");

    // 8. struct Particle
    auto* s_def = dynamic_cast<StructDefNode*>(stmts[7].get());
    assert(s_def != nullptr);
    assert(s_def->name == "Particle");
    assert(s_def->fields.size() == 2);
    assert(s_def->fields[0] == "x");
    assert(s_def->field_types.size() == 2);
    assert(s_def->field_types[0] == "f64");
    assert(s_def->field_types[1] == "f64");

    std::cout << "test_parse_type_annotations passed!" << std::endl;
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
    test_parse_struct();
    test_parse_type_annotations();
    std::cout << "=== All Parser Tests Passed Successfully ===" << std::endl;
    return 0;
}
