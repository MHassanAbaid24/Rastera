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

enum class BinaryOp {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
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

// ---- Statements ----

struct ReturnStmt : Stmt {
    std::unique_ptr<Expr> value;
    explicit ReturnStmt(std::unique_ptr<Expr> e) : value(std::move(e)) {}
    void accept(Visitor& v) override;
};

// ---- Structure ----

// `[ stmt* ]` - a sequence of statements that opens a new scope.
struct Block : Node {
    std::vector<std::unique_ptr<Stmt>> stmts;
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
    virtual void visit(BinaryExpr&) = 0;
    virtual void visit(UnaryExpr&) = 0;
    virtual void visit(ReturnStmt&) = 0;
    virtual void visit(Block&) = 0;
    virtual void visit(FuncDecl&) = 0;
    virtual void visit(Program&) = 0;
};

inline void IntLit::accept(Visitor& v) { v.visit(*this); }
inline void BinaryExpr::accept(Visitor& v) { v.visit(*this); }
inline void UnaryExpr::accept(Visitor& v) { v.visit(*this); }
inline void ReturnStmt::accept(Visitor& v) { v.visit(*this); }
inline void Block::accept(Visitor& v) { v.visit(*this); }
inline void FuncDecl::accept(Visitor& v) { v.visit(*this); }
inline void Program::accept(Visitor& v) { v.visit(*this); }
