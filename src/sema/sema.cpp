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

void Sema::visit(ReturnStmt& n) { n.value->accept(*this); }

void Sema::visit(IntLit&) {}
