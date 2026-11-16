#ifndef WYRM_AST_HPP
#define WYRM_AST_HPP

#include <string>
#include <vector>
#include <memory>
#include "../token/token.hpp"

namespace wyrm {

class ASTVisitor;

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor* visitor) = 0;
};

using ASTNodePtr = std::unique_ptr<ASTNode>;

class NumberNode : public ASTNode {
public:
    Token token;
    double value;

    explicit NumberNode(Token t) : token(std::move(t)), value(std::stod(token.value)) {}
    void accept(ASTVisitor* visitor) override;
};

class StringNode : public ASTNode {
public:
    Token token;
    std::string value;

    explicit StringNode(Token t) : token(std::move(t)), value(token.value) {}
    void accept(ASTVisitor* visitor) override;
};

class BooleanNode : public ASTNode {
public:
    Token token;
    bool value;

    explicit BooleanNode(Token t) : token(std::move(t)), value(token.value == "true") {}
    void accept(ASTVisitor* visitor) override;
};

class NoneNode : public ASTNode {
public:
    Token token;

    explicit NoneNode(Token t) : token(std::move(t)) {}
    void accept(ASTVisitor* visitor) override;
};

class IdentifierNode : public ASTNode {
public:
    Token token;
    std::string name;

    explicit IdentifierNode(Token t) : token(std::move(t)), name(token.value) {}
    void accept(ASTVisitor* visitor) override;
};

class BinaryOpNode : public ASTNode {
public:
    ASTNodePtr left;
    Token op;
    ASTNodePtr right;

    BinaryOpNode(ASTNodePtr l, Token o, ASTNodePtr r)
        : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}
    void accept(ASTVisitor* visitor) override;
};

class UnaryOpNode : public ASTNode {
public:
    Token op;
    ASTNodePtr expr;

    UnaryOpNode(Token o, ASTNodePtr e)
        : op(std::move(o)), expr(std::move(e)) {}
    void accept(ASTVisitor* visitor) override;
};

class AssignmentNode : public ASTNode {
public:
    std::unique_ptr<IdentifierNode> var_name;
    ASTNodePtr value;
    bool is_declaration;
    std::string declaration_type; // "var", "dec", or ""
    std::string type_annotation;  // e.g. "i32", "f64", "string", or ""

    AssignmentNode(std::unique_ptr<IdentifierNode> name, ASTNodePtr val, bool is_decl = false, std::string decl_type = "", std::string type_ann = "")
        : var_name(std::move(name)), value(std::move(val)), is_declaration(is_decl), declaration_type(std::move(decl_type)), type_annotation(std::move(type_ann)) {}
    void accept(ASTVisitor* visitor) override;
};

class PrintNode : public ASTNode {
public:
    std::vector<ASTNodePtr> expressions;

    explicit PrintNode(std::vector<ASTNodePtr> exprs) : expressions(std::move(exprs)) {}
    void accept(ASTVisitor* visitor) override;
};

class IfNode : public ASTNode {
public:
    ASTNodePtr condition;
    std::vector<ASTNodePtr> then_body;
    std::vector<std::pair<ASTNodePtr, std::vector<ASTNodePtr>>> elif_clauses;
    std::vector<ASTNodePtr> else_body;

    IfNode(ASTNodePtr cond, std::vector<ASTNodePtr> then_b,
           std::vector<std::pair<ASTNodePtr, std::vector<ASTNodePtr>>> elif_c,
           std::vector<ASTNodePtr> else_b)
        : condition(std::move(cond)), then_body(std::move(then_b)),
          elif_clauses(std::move(elif_c)), else_body(std::move(else_b)) {}
    void accept(ASTVisitor* visitor) override;
};

class RepeatNode : public ASTNode {
public:
    std::vector<ASTNodePtr> body;
    ASTNodePtr condition;

    RepeatNode(std::vector<ASTNodePtr> b, ASTNodePtr cond)
        : body(std::move(b)), condition(std::move(cond)) {}
    void accept(ASTVisitor* visitor) override;
};

class FunctionDefNode : public ASTNode {
public:
    std::unique_ptr<IdentifierNode> name; // null for lambda
    std::vector<std::unique_ptr<IdentifierNode>> params;
    std::vector<ASTNodePtr> body;
    std::vector<std::string> param_types;
    std::string return_type;
    std::vector<std::string> type_params; // M4: generic type parameters, e.g. ["T", "U"]
    bool is_async = false;                // M6: async fn flag

    FunctionDefNode(std::unique_ptr<IdentifierNode> n, std::vector<std::unique_ptr<IdentifierNode>> p, std::vector<ASTNodePtr> b,
                    std::vector<std::string> p_types = {}, std::string ret_t = "",
                    std::vector<std::string> t_params = {}, bool async_flag = false)
        : name(std::move(n)), params(std::move(p)), body(std::move(b)),
          param_types(std::move(p_types)), return_type(std::move(ret_t)),
          type_params(std::move(t_params)), is_async(async_flag) {}
    void accept(ASTVisitor* visitor) override;
};

class FunctionCallNode : public ASTNode {
public:
    std::unique_ptr<IdentifierNode> name;
    std::vector<ASTNodePtr> args;

    FunctionCallNode(std::unique_ptr<IdentifierNode> n, std::vector<ASTNodePtr> a)
        : name(std::move(n)), args(std::move(a)) {}
    void accept(ASTVisitor* visitor) override;
};

class ListNode : public ASTNode {
public:
    std::vector<ASTNodePtr> elements;

    explicit ListNode(std::vector<ASTNodePtr> elems) : elements(std::move(elems)) {}
    void accept(ASTVisitor* visitor) override;
};

class IndexNode : public ASTNode {
public:
    ASTNodePtr obj;
    ASTNodePtr index;

    IndexNode(ASTNodePtr o, ASTNodePtr idx)
        : obj(std::move(o)), index(std::move(idx)) {}
    void accept(ASTVisitor* visitor) override;
};

class IndexAssignNode : public ASTNode {
public:
    ASTNodePtr obj;
    ASTNodePtr index;
    ASTNodePtr value;

    IndexAssignNode(ASTNodePtr o, ASTNodePtr idx, ASTNodePtr val)
        : obj(std::move(o)), index(std::move(idx)), value(std::move(val)) {}
    void accept(ASTVisitor* visitor) override;
};

class ReturnNode : public ASTNode {
public:
    ASTNodePtr expr;

    explicit ReturnNode(ASTNodePtr e) : expr(std::move(e)) {}
    void accept(ASTVisitor* visitor) override;
};

class UseNode : public ASTNode {
public:
    std::string module_path;

    explicit UseNode(std::string path) : module_path(std::move(path)) {}
    void accept(ASTVisitor* visitor) override;
};

class BreakNode : public ASTNode {
public:
    void accept(ASTVisitor* visitor) override;
};

class ContinueNode : public ASTNode {
public:
    void accept(ASTVisitor* visitor) override;
};

class SliceNode : public ASTNode {
public:
    ASTNodePtr start;
    ASTNodePtr end;

    SliceNode(ASTNodePtr s, ASTNodePtr e)
        : start(std::move(s)), end(std::move(e)) {}
    void accept(ASTVisitor* visitor) override;
};

class UnsafeBlockNode : public ASTNode {
public:
    std::vector<ASTNodePtr> body;

    explicit UnsafeBlockNode(std::vector<ASTNodePtr> b) : body(std::move(b)) {}
    void accept(ASTVisitor* visitor) override;
};

class OwnedDeclNode : public ASTNode {
public:
    std::unique_ptr<IdentifierNode> var_name;
    ASTNodePtr value;
    std::string declaration_type; // "var" or "dec"
    std::string type_annotation;

    OwnedDeclNode(std::unique_ptr<IdentifierNode> name, ASTNodePtr val, std::string decl_type = "var", std::string type_ann = "")
        : var_name(std::move(name)), value(std::move(val)), declaration_type(std::move(decl_type)), type_annotation(std::move(type_ann)) {}
    void accept(ASTVisitor* visitor) override;
};

class ArenaNode : public ASTNode {
public:
    std::string name;
    ASTNodePtr size_expr;

    ArenaNode(std::string n, ASTNodePtr sz)
        : name(std::move(n)), size_expr(std::move(sz)) {}
    void accept(ASTVisitor* visitor) override;
};

class ArenaAllocNode : public ASTNode {
public:
    std::string arena_name;
    ASTNodePtr size_expr;

    ArenaAllocNode(std::string name, ASTNodePtr sz)
        : arena_name(std::move(name)), size_expr(std::move(sz)) {}
    void accept(ASTVisitor* visitor) override;
};

class ArenaResetNode : public ASTNode {
public:
    std::string arena_name;

    explicit ArenaResetNode(std::string name) : arena_name(std::move(name)) {}
    void accept(ASTVisitor* visitor) override;
};

class StructDefNode : public ASTNode {
public:
    std::string name;
    std::vector<std::string> fields;
    std::vector<std::string> field_types;
    std::vector<std::unique_ptr<FunctionDefNode>> methods;
    std::vector<std::string> type_params; // M4: generic type parameters

    StructDefNode(std::string n, std::vector<std::string> f, std::vector<std::unique_ptr<FunctionDefNode>> m,
                  std::vector<std::string> f_types = {}, std::vector<std::string> t_params = {})
        : name(std::move(n)), fields(std::move(f)), field_types(std::move(f_types)),
          methods(std::move(m)), type_params(std::move(t_params)) {}
    void accept(ASTVisitor* visitor) override;
};

class MemberAccessNode : public ASTNode {
public:
    ASTNodePtr obj;
    std::string member;

    MemberAccessNode(ASTNodePtr o, std::string m)
        : obj(std::move(o)), member(std::move(m)) {}
    void accept(ASTVisitor* visitor) override;
};

class MemberAssignNode : public ASTNode {
public:
    ASTNodePtr obj;
    std::string member;
    ASTNodePtr value;

    MemberAssignNode(ASTNodePtr o, std::string m, ASTNodePtr val)
        : obj(std::move(o)), member(std::move(m)), value(std::move(val)) {}
    void accept(ASTVisitor* visitor) override;
};

class MethodCallNode : public ASTNode {
public:
    ASTNodePtr obj;
    std::string method_name;
    std::vector<ASTNodePtr> args;

    MethodCallNode(ASTNodePtr o, std::string m, std::vector<ASTNodePtr> a)
        : obj(std::move(o)), method_name(std::move(m)), args(std::move(a)) {}
    void accept(ASTVisitor* visitor) override;
};

// ===========================================================================
// M2: enum + match
// ===========================================================================

struct EnumVariant {
    std::string name;
    std::vector<std::string> fields;      // positional field names (may be empty)
    std::vector<std::string> field_types; // corresponding type annotations
};

class EnumDefNode : public ASTNode {
public:
    std::string name;
    std::vector<EnumVariant> variants;

    EnumDefNode(std::string n, std::vector<EnumVariant> v)
        : name(std::move(n)), variants(std::move(v)) {}
    void accept(ASTVisitor* visitor) override;
};

struct MatchArm {
    std::string variant_name;             // "Red", "_" (wildcard)
    std::vector<std::string> bindings;    // bound variable names
    std::vector<ASTNodePtr> body;
};

class MatchNode : public ASTNode {
public:
    ASTNodePtr subject;
    std::vector<MatchArm> arms;

    MatchNode(ASTNodePtr s, std::vector<MatchArm> a)
        : subject(std::move(s)), arms(std::move(a)) {}
    void accept(ASTVisitor* visitor) override;
};

// ===========================================================================
// M3: try / catch / throw
// ===========================================================================

class ThrowNode : public ASTNode {
public:
    ASTNodePtr expr; // expression to throw (any Value)

    explicit ThrowNode(ASTNodePtr e) : expr(std::move(e)) {}
    void accept(ASTVisitor* visitor) override;
};

class TryCatchNode : public ASTNode {
public:
    std::vector<ASTNodePtr> try_body;
    std::string catch_var;               // name bound to the caught value
    std::vector<ASTNodePtr> catch_body;
    std::vector<ASTNodePtr> finally_body; // may be empty

    TryCatchNode(std::vector<ASTNodePtr> tb, std::string cv,
                 std::vector<ASTNodePtr> cb, std::vector<ASTNodePtr> fb)
        : try_body(std::move(tb)), catch_var(std::move(cv)),
          catch_body(std::move(cb)), finally_body(std::move(fb)) {}
    void accept(ASTVisitor* visitor) override;
};

// ===========================================================================
// M5: Lambda / Closure
// ===========================================================================

class LambdaNode : public ASTNode {
public:
    std::vector<std::unique_ptr<IdentifierNode>> params;
    std::vector<std::string> param_types;
    std::vector<ASTNodePtr> body;

    LambdaNode(std::vector<std::unique_ptr<IdentifierNode>> p,
               std::vector<std::string> pt,
               std::vector<ASTNodePtr> b)
        : params(std::move(p)), param_types(std::move(pt)), body(std::move(b)) {}
    void accept(ASTVisitor* visitor) override;
};

// ===========================================================================
// M6: await expression
// ===========================================================================

class AwaitNode : public ASTNode {
public:
    ASTNodePtr expr;

    explicit AwaitNode(ASTNodePtr e) : expr(std::move(e)) {}
    void accept(ASTVisitor* visitor) override;
};

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    virtual void visit(NumberNode* node) = 0;
    virtual void visit(StringNode* node) = 0;
    virtual void visit(BooleanNode* node) = 0;
    virtual void visit(NoneNode* node) = 0;
    virtual void visit(IdentifierNode* node) = 0;
    virtual void visit(BinaryOpNode* node) = 0;
    virtual void visit(UnaryOpNode* node) = 0;
    virtual void visit(AssignmentNode* node) = 0;
    virtual void visit(PrintNode* node) = 0;
    virtual void visit(IfNode* node) = 0;
    virtual void visit(RepeatNode* node) = 0;
    virtual void visit(FunctionDefNode* node) = 0;
    virtual void visit(FunctionCallNode* node) = 0;
    virtual void visit(ListNode* node) = 0;
    virtual void visit(IndexNode* node) = 0;
    virtual void visit(IndexAssignNode* node) = 0;
    virtual void visit(ReturnNode* node) = 0;
    virtual void visit(UseNode* node) = 0;
    virtual void visit(BreakNode* node) = 0;
    virtual void visit(ContinueNode* node) = 0;
    virtual void visit(SliceNode* node) = 0;
    virtual void visit(UnsafeBlockNode* node) = 0;
    virtual void visit(OwnedDeclNode* node) = 0;
    virtual void visit(ArenaNode* node) = 0;
    virtual void visit(ArenaAllocNode* node) = 0;
    virtual void visit(ArenaResetNode* node) = 0;
    virtual void visit(StructDefNode* node) = 0;
    virtual void visit(MemberAccessNode* node) = 0;
    virtual void visit(MemberAssignNode* node) = 0;
    virtual void visit(MethodCallNode* node) = 0;
    // M2
    virtual void visit(EnumDefNode* node) = 0;
    virtual void visit(MatchNode* node) = 0;
    // M3
    virtual void visit(ThrowNode* node) = 0;
    virtual void visit(TryCatchNode* node) = 0;
    // M5
    virtual void visit(LambdaNode* node) = 0;
    // M6
    virtual void visit(AwaitNode* node) = 0;
};

inline void NumberNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void StringNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void BooleanNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void NoneNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void IdentifierNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void BinaryOpNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void UnaryOpNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void AssignmentNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void PrintNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void IfNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void RepeatNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void FunctionDefNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void FunctionCallNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void ListNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void IndexNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void IndexAssignNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void ReturnNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void UseNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void BreakNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void ContinueNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void SliceNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void UnsafeBlockNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void OwnedDeclNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void ArenaNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void ArenaAllocNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void ArenaResetNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void StructDefNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void MemberAccessNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void MemberAssignNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void MethodCallNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
// M2
inline void EnumDefNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void MatchNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
// M3
inline void ThrowNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
inline void TryCatchNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
// M5
inline void LambdaNode::accept(ASTVisitor* visitor) { visitor->visit(this); }
// M6
inline void AwaitNode::accept(ASTVisitor* visitor) { visitor->visit(this); }

} // namespace wyrm

#endif // WYRM_AST_HPP
