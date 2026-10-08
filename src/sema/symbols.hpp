// Scoped symbol table used by semantic analysis.
//
// scopes_ is a stack: push() on entering a block, pop() on leaving it.
// lookup() searches innermost scope first, which is what makes inner
// blocks see outer variables but not the other way round.
//
// Symbols live in storage_ (a deque: growing it never moves existing
// elements), so Symbol* stays valid after its scope is popped. Sema stores
// these pointers in the AST and codegen uses them, which means names are
// resolved exactly once, in sema.
#pragma once

#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

#include "ast/ast.hpp"

enum class Type { Int, Array };

struct Symbol {
    std::string name;
    Type type;
    Loc declared;
};

class SymbolTable {
public:
    void push() { scopes_.emplace_back(); }
    void pop() { scopes_.pop_back(); }

    // Innermost visible symbol called `name`, or nullptr.
    Symbol* lookup(const std::string& name) const {
        for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) return found->second;
        }
        return nullptr;
    }

    // Create `name` in the current (innermost) scope.
    // Returns nullptr if the current scope already has that name.
    Symbol* declare(const std::string& name, Type type, Loc loc) {
        auto& scope = scopes_.back();
        if (scope.count(name)) return nullptr;
        storage_.push_back(Symbol{name, type, loc});
        Symbol* sym = &storage_.back();
        scope.emplace(name, sym);
        return sym;
    }

private:
    std::vector<std::unordered_map<std::string, Symbol*>> scopes_;
    std::deque<Symbol> storage_;
};
