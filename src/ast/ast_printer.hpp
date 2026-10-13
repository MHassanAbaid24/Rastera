// `rasterc --ast`: prints the tree, one node per line, children indented.
#pragma once

#include <ostream>

#include "ast/ast.hpp"

class AstPrinter : public Visitor {
public:
    explicit AstPrinter(std::ostream& out) : out_(out) {}

    void visit(IntLit& n) override { line(n) << "Int " << n.value << "\n"; }

    void visit(BinaryExpr& n) override {
        line(n) << "Binary " << opString(n.op) << "\n";
        child(*n.lhs);
        child(*n.rhs);
    }

    void visit(UnaryExpr& n) override {
        line(n) << "Unary " << opString(n.op) << "\n";
        child(*n.operand);
    }

    void visit(ReturnStmt& n) override {
        line(n) << "Return\n";
        child(*n.value);
    }

    void visit(Block& n) override {
        line(n) << "Block\n";
        for (auto& s : n.stmts) child(*s);
    }

    void visit(FuncDecl& n) override {
        line(n) << "Func " << n.name << "\n";
        child(*n.body);
    }

    void visit(Program& n) override {
        line(n) << "Program\n";
        for (auto& f : n.funcs) child(*f);
    }

private:
    std::ostream& line(Node& n) {
        for (int i = 0; i < depth_; ++i) out_ << "  ";
        return out_ << "[" << n.loc.line << "] ";
    }

    void child(Node& n) {
        ++depth_;
        n.accept(*this);
        --depth_;
    }

    static const char* opString(BinaryOp op) {
        switch (op) {
            case BinaryOp::Add: return "+";
            case BinaryOp::Sub: return "-";
            case BinaryOp::Mul: return "*";
            case BinaryOp::Div: return "/";
            case BinaryOp::Mod: return "%";
        }
        return "?";
    }

    static const char* opString(UnaryOp op) {
        switch (op) {
            case UnaryOp::Neg: return "-";
        }
        return "?";
    }

    std::ostream& out_;
    int depth_ = 0;
};
