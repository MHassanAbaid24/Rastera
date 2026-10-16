// Abstract syntax tree: the parser builds it, every later stage walks it.
//
// Ownership: a parent node owns its children through std::unique_ptr, so
// destroying the Program frees the whole tree.
//
// Traversal: the Visitor pattern. Each concrete node implements accept(),
// which calls back the visit() overload for its own type ("double dispatch").
// A new compiler pass (printer, sema, codegen) is a new Visitor subclass;
// a new node type means adding one visit() to Visitor and every pass.
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

struct Loc {
    int line = 0;
};

struct Symbol;
struct Visitor;

struct Node {
    Loc loc;
    virtual ~Node() = default;
    virtual void accept(Visitor& v) = 0;
};

struct Expr : Node {};
struct Stmt : Node {};

// ---- Expressions ----

struct IntLit : Expr {
    int32_t value;
    explicit IntLit(int32_t v) : value(v) {}
    void accept(Visitor& v) override;
};

struct VarRef : Expr {
    std::string name;
    Symbol* sym = nullptr;
    explicit VarRef(std::string n) : name(std::move(n)) {}
    void accept(Visitor& v) override;
};

enum class BinaryOp {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Equal,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
};

struct BinaryExpr : Expr {
    BinaryOp op;
    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;
    BinaryExpr(BinaryOp op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs)
        : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    void accept(Visitor& v) override;
};

enum class UnaryOp {
    Neg,
};

struct UnaryExpr : Expr {
    UnaryOp op;
    std::unique_ptr<Expr> operand;
    UnaryExpr(UnaryOp op, std::unique_ptr<Expr> operand)
        : op(op), operand(std::move(operand)) {}
    void accept(Visitor& v) override;
};

// `name(args)` - call to a built-in (T03) or user function (T07).
struct CallExpr : Expr {
    std::string name;
    std::vector<std::unique_ptr<Expr>> args;
    CallExpr(std::string n, std::vector<std::unique_ptr<Expr>> a)
        : name(std::move(n)), args(std::move(a)) {}
    void accept(Visitor& v) override;
};

// ---- Statements ----

struct ReturnStmt : Stmt {
    std::unique_ptr<Expr> value;
    explicit ReturnStmt(std::unique_ptr<Expr> e) : value(std::move(e)) {}
    void accept(Visitor& v) override;
};

struct AssignStmt : Stmt {
    std::string name;
    std::unique_ptr<Expr> value;
    Symbol* sym = nullptr;
    bool declares = false;
    AssignStmt(std::string n, std::unique_ptr<Expr> v) : name(std::move(n)), value(std::move(v)) {}
    void accept(Visitor& v) override;
};

// `expr;` - evaluate expression for its side effects (spec §5).
struct ExprStmt : Stmt {
    std::unique_ptr<Expr> expr;
    explicit ExprStmt(std::unique_ptr<Expr> e) : expr(std::move(e)) {}
    void accept(Visitor& v) override;
};

// ---- Structure ----

// `[ stmt* ]` - a sequence of statements that opens a new scope.
struct Block : Stmt {
    std::vector<std::unique_ptr<Stmt>> stmts;
    void accept(Visitor& v) override;
};

// `? (c) [ ... ]` / `? (c) [ ... ] : [ ... ]`
struct IfStmt : Stmt {
    std::unique_ptr<Expr> cond;
    std::unique_ptr<Block> then_branch;
    std::unique_ptr<Stmt> else_branch;  // nullptr, Block, or chained IfStmt
    IfStmt(std::unique_ptr<Expr> c, std::unique_ptr<Block> t, std::unique_ptr<Stmt> e = nullptr)
        : cond(std::move(c)), then_branch(std::move(t)), else_branch(std::move(e)) {}
    void accept(Visitor& v) override;
};

// `@name() [ ... ]`
struct FuncDecl : Node {
    std::string name;
    std::unique_ptr<Block> body;
    FuncDecl(std::string n, std::unique_ptr<Block> b) : name(std::move(n)), body(std::move(b)) {}
    void accept(Visitor& v) override;
};

struct Program : Node {
    std::vector<std::unique_ptr<FuncDecl>> funcs;
    void accept(Visitor& v) override;
};

struct Visitor {
    virtual ~Visitor() = default;
    virtual void visit(IntLit&) = 0;
    virtual void visit(VarRef&) = 0;
    virtual void visit(BinaryExpr&) = 0;
    virtual void visit(UnaryExpr&) = 0;
    virtual void visit(CallExpr&) = 0;
    virtual void visit(ReturnStmt&) = 0;
    virtual void visit(AssignStmt&) = 0;
    virtual void visit(ExprStmt&) = 0;
    virtual void visit(IfStmt&) = 0;
    virtual void visit(Block&) = 0;
    virtual void visit(FuncDecl&) = 0;
    virtual void visit(Program&) = 0;
};

inline void IntLit::accept(Visitor& v) { v.visit(*this); }
inline void VarRef::accept(Visitor& v) { v.visit(*this); }
inline void BinaryExpr::accept(Visitor& v) { v.visit(*this); }
inline void UnaryExpr::accept(Visitor& v) { v.visit(*this); }
inline void CallExpr::accept(Visitor& v) { v.visit(*this); }
inline void ReturnStmt::accept(Visitor& v) { v.visit(*this); }
inline void AssignStmt::accept(Visitor& v) { v.visit(*this); }
inline void ExprStmt::accept(Visitor& v) { v.visit(*this); }
inline void IfStmt::accept(Visitor& v) { v.visit(*this); }
inline void Block::accept(Visitor& v) { v.visit(*this); }
inline void FuncDecl::accept(Visitor& v) { v.visit(*this); }
inline void Program::accept(Visitor& v) { v.visit(*this); }
