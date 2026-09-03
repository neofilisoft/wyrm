#ifndef WYRM_TRANSPILER_HPP
#define WYRM_TRANSPILER_HPP

#include <string>
#include <vector>
#include <unordered_set>
#include <memory>
#include "../ast/ast.hpp"

namespace wyrm {

struct TranspilerScope {
    std::vector<std::string> arenas;
    std::vector<std::string> allocations;
};

class Transpiler : public ASTVisitor {
public:
    std::vector<std::string> header_lines;
    std::vector<std::string> function_lines;
    std::vector<std::string> top_level_lines;
    
    std::unordered_set<std::string> declared_globals;
    std::unordered_set<std::string> declared_locals;
    std::unordered_set<std::string> constants;
    
    bool in_function;
    bool has_main_def;
    bool in_unsafe;
    std::vector<TranspilerScope> scopes;
    std::string source_dir;

    explicit Transpiler(const std::string& src_dir = ".");
    ~Transpiler() override = default;

    std::string transpile(std::vector<ASTNodePtr>& ast);

    void push_scope();
    std::string pop_scope();
    std::string get_all_cleanups();
    void emit(const std::string& stmt);
    void visit_statement(ASTNode* stmt);
    std::string evaluate(ASTNode* node);

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
    void visit(StructDefNode* node) override;
    void visit(MemberAccessNode* node) override;
    void visit(MemberAssignNode* node) override;
    void visit(MethodCallNode* node) override;

private:
    std::string last_result_;
    std::string load_c_runtime();
};

} // namespace wyrm

#endif // WYRM_TRANSPILER_HPP
