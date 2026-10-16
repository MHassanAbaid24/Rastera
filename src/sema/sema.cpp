#include "sema/sema.hpp"

bool Sema::check(Program& program) {
    program.accept(*this);
    return !diag_.hasErrors();
}

void Sema::visit(Program& n) {
    for (auto& f : n.funcs) functions_.emplace(f->name, f.get());

    if (!functions_.count("main")) diag_.error(n.loc, "program has no @main() function");

    for (auto& f : n.funcs) f->accept(*this);
}

void Sema::visit(FuncDecl& n) { n.body->accept(*this); }

void Sema::visit(Block& n) {
    symbols_.push();
    for (auto& s : n.stmts) s->accept(*this);
    symbols_.pop();
}

const std::unordered_map<std::string, int> Sema::builtins_ = {
    {"print", 1},
};

void Sema::visit(ReturnStmt& n) { n.value->accept(*this); }

void Sema::visit(AssignStmt& n) {
    n.value->accept(*this);
    if (auto* sym = symbols_.lookup(n.name)) {
        n.sym = sym;
        n.declares = false;
    } else {
        n.sym = symbols_.declare(n.name, Type::Int, n.loc);
        n.declares = true;
    }
}

void Sema::visit(VarRef& n) {
    n.sym = symbols_.lookup(n.name);
    if (n.sym == nullptr) diag_.error(n.loc, "undefined variable '" + n.name + "'");
}

void Sema::visit(ExprStmt& n) { n.expr->accept(*this); }

void Sema::visit(CallExpr& n) {
    for (auto& a : n.args) a->accept(*this);

    auto it = builtins_.find(n.name);
    if (it == builtins_.end()) {
        diag_.error(n.loc, "unknown function '" + n.name + "'");
        return;
    }

    if (static_cast<int>(n.args.size()) != it->second) {
        diag_.error(n.loc, "wrong argument count for '" + n.name + "': expected " +
                               std::to_string(it->second) + ", got " +
                               std::to_string(n.args.size()));
        return;
    }
}

void Sema::visit(IntLit&) {}

void Sema::visit(BinaryExpr& n) {
    n.lhs->accept(*this);
    n.rhs->accept(*this);
}

void Sema::visit(UnaryExpr& n) {
    n.operand->accept(*this);
}
