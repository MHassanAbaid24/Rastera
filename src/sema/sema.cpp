#include "sema/sema.hpp"

bool Sema::check(Program& program) {
    program.accept(*this);
    return !diag_.hasErrors();
}

void Sema::visit(Program& n) {
    // Collect signatures before checking bodies so calls can target later functions.
    for (auto& f : n.funcs) {
        if (builtins_.count(f->name)) {
            diag_.error(f->loc, "using built-in name '" + f->name + "' as a function name");
            continue;
        }
        if (!functions_.emplace(f->name, f.get()).second)
            diag_.error(f->loc, "duplicate function '" + f->name + "'");
    }
    auto main = functions_.find("main");
    if (main == functions_.end()) {
        diag_.error(n.loc, "program has no @main() function");
    } else if (!main->second->params.empty()) {
        diag_.error(main->second->loc, "@main() must not have parameters");
    }

    for (auto& f : n.funcs) f->accept(*this);
}

void Sema::visit(FuncDecl& n) {
    // Parameter scope: encloses the body block, and starts empty, so functions
    // cannot see each other's variables (spec §4).
    symbols_.push();
    for (auto& p : n.params) {
        if (builtins_.count(p.name)) {
            diag_.error(p.loc, "using built-in name '" + p.name + "' as a parameter name");
            continue;
        }
        p.sym = symbols_.declare(p.name, Type::Int, p.loc);
        if (p.sym == nullptr) diag_.error(p.loc, "duplicate parameter '" + p.name + "'");
    }
    n.body->accept(*this);
    symbols_.pop();
}

void Sema::visit(Block& n) {
    symbols_.push();
    for (auto& s : n.stmts) s->accept(*this);
    symbols_.pop();
}

const std::unordered_map<std::string, std::size_t> Sema::builtins_ = {
    {"print", 1},
};

void Sema::visit(ReturnStmt& n) { n.value->accept(*this); }

void Sema::visit(AssignStmt& n) {
    if (builtins_.count(n.name)) {
        diag_.error(n.loc, "using built-in name '" + n.name + "' as a variable name");
        return;
    }
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

    std::size_t expected;
    auto builtin = builtins_.find(n.name);
    if (builtin != builtins_.end()) {
        expected = builtin->second;
    } else {
        auto function = functions_.find(n.name);
        if (function == functions_.end()) {
            diag_.error(n.loc, "unknown function '" + n.name + "'");
            return;
        }
        n.callee = function->second;
        expected = n.callee->params.size();
    }

    if (n.args.size() != expected) {
        diag_.error(n.loc, "wrong argument count for '" + n.name + "': expected " +
                               std::to_string(expected) + ", got " +
                               std::to_string(n.args.size()));
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
