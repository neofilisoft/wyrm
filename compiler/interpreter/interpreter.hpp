#ifndef WYRM_INTERPRETER_HPP
#define WYRM_INTERPRETER_HPP

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include "../ast/ast.hpp"
#include "environment.hpp"

namespace wyrm {

class Interpreter;

class Callable {
public:
    virtual ~Callable() = default;
    virtual Value call(const std::vector<Value>& args, Interpreter* interpreter) = 0;
    virtual bool is_builtin() const = 0;
};

class Function : public Callable {
public:
    std::string name;
    std::vector<std::string> params;
    std::vector<ASTNode*> body;
    std::shared_ptr<Environment> closure;

    Function(std::string n, std::vector<std::string> p, std::vector<ASTNode*> b, std::shared_ptr<Environment> clos)
        : name(std::move(n)), params(std::move(p)), body(std::move(b)), closure(std::move(clos)) {}

    Value call(const std::vector<Value>& args, Interpreter* interpreter) override;
    bool is_builtin() const override { return false; }
};

using BuiltinFnPtr = Value(*)(const std::vector<Value>&);

class BuiltinFunction : public Callable {
public:
    std::string name;
    BuiltinFnPtr func;

    BuiltinFunction(std::string n, BuiltinFnPtr f)
        : name(std::move(n)), func(f) {}

    Value call(const std::vector<Value>& args, Interpreter* interpreter) override;
    bool is_builtin() const override { return true; }
};

struct ReturnSignal {
    Value value;
};

struct BreakSignal {};
struct ContinueSignal {};

class Interpreter : public ASTVisitor {
public:
    std::shared_ptr<Environment> global_env;
    std::shared_ptr<Environment> current_env;
    std::string source_dir;
    bool in_unsafe;
    std::vector<std::string> cli_args;

    Interpreter(const std::string& src_dir = ".", std::vector<std::string> args = {});
    ~Interpreter() override = default;

    void execute(std::vector<ASTNodePtr>& statements);
    void interpret(std::vector<ASTNodePtr>& statements);
    
    Value evaluate(ASTNode* node);
    void execute_statement(ASTNode* node);
    void execute_block(std::vector<ASTNodePtr>& statements, std::shared_ptr<Environment> env);
    void execute_block(std::vector<ASTNode*>& statements, std::shared_ptr<Environment> env);

    // Visitor methods
    void visit(NumberNode* node) override;
    void visit(StringNode* node) override;
    void visit(BooleanNode* node) override;
    void visit(NoneNode* node) override;
    void visit(IdentifierNode* node) override;
    void visit(BinaryOpNode* node) override;
    void visit(UnaryOpNode* node) override;
    void visit(AssignmentNode* node) override;
    void visit(PrintNode* node) override;
    void visit(IfNode* node) override;
    void visit(RepeatNode* node) override;
    void visit(FunctionDefNode* node) override;
    void visit(FunctionCallNode* node) override;
    void visit(ListNode* node) override;
    void visit(IndexNode* node) override;
    void visit(IndexAssignNode* node) override;
    void visit(ReturnNode* node) override;
    void visit(UseNode* node) override;
    void visit(BreakNode* node) override;
    void visit(ContinueNode* node) override;
    void visit(SliceNode* node) override;
    void visit(UnsafeBlockNode* node) override;
    void visit(OwnedDeclNode* node) override;
    void visit(ArenaNode* node) override;
    void visit(ArenaAllocNode* node) override;
    void visit(ArenaResetNode* node) override;

private:
    Value last_value_; // temporary storage for visited node returns
    std::vector<std::unique_ptr<Callable>> callables_alive_; // memory management for callables

    void setup_builtins();
};

} // namespace wyrm

#endif // WYRM_INTERPRETER_HPP
