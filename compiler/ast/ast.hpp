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

    AssignmentNode(std::unique_ptr<IdentifierNode> name, ASTNodePtr val, bool is_decl = false, std::string decl_type = "")
        : var_name(std::move(name)), value(std::move(val)), is_declaration(is_decl), declaration_type(std::move(decl_type)) {}
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
    std::unique_ptr<IdentifierNode> name;
    std::vector<std::unique_ptr<IdentifierNode>> params;
    std::vector<ASTNodePtr> body;

    FunctionDefNode(std::unique_ptr<IdentifierNode> n, std::vector<std::unique_ptr<IdentifierNode>> p, std::vector<ASTNodePtr> b)
        : name(std::move(n)), params(std::move(p)), body(std::move(b)) {}
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

    OwnedDeclNode(std::unique_ptr<IdentifierNode> name, ASTNodePtr val, std::string decl_type = "var")
        : var_name(std::move(name)), value(std::move(val)), declaration_type(std::move(decl_type)) {}
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

} // namespace wyrm

#endif // WYRM_AST_HPP
