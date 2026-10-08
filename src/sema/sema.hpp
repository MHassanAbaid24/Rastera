// Semantic analysis: checks the rules the grammar cannot express
// (main exists, names are defined, types match, ...) and annotates the AST
// with what codegen needs. Codegen assumes a program that passed sema.
#pragma once

#include <string>
#include <unordered_map>

#include "ast/ast.hpp"
#include "diag.hpp"
#include "sema/symbols.hpp"

class Sema : public Visitor {
public:
    explicit Sema(Diagnostics& diag) : diag_(diag) {}

    // Returns true if the program is valid. Keep the Sema object alive while
    // the AST is in use: annotations point into its symbol table.
    bool check(Program& program);

    void visit(IntLit&) override;
    void visit(ReturnStmt&) override;
    void visit(Block&) override;
    void visit(FuncDecl&) override;
    void visit(Program&) override;

private:
    Diagnostics& diag_;
    SymbolTable symbols_;
    std::unordered_map<std::string, FuncDecl*> functions_;
};
