// Code generation: walks a sema-checked AST and builds an LLVM IR module.
//
// Expressions return their result through value_: visiting an Expr leaves
// the llvm::Value* holding its result there; emit() wraps that pattern.
// Statements append instructions at the builder's insertion point.
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Target/TargetMachine.h>

#include "ast/ast.hpp"

class CodeGen : public Visitor {
public:
    // The target machine fixes the module's triple and data layout (type sizes
    // and alignments) before any IR is built.
    CodeGen(llvm::LLVMContext& ctx, const std::string& moduleName, const llvm::TargetMachine& tm);

    std::unique_ptr<llvm::Module> generate(Program& program);

    void visit(IntLit&) override;
    void visit(VarRef&) override;
    void visit(BinaryExpr&) override;
    void visit(UnaryExpr&) override;
    void visit(CallExpr&) override;
    void visit(ArrayLit&) override;
    void visit(ArrayAlloc&) override;
    void visit(IndexExpr&) override;
    void visit(ArrayStoreStmt&) override;
    void visit(ReturnStmt&) override;
    void visit(AssignStmt&) override;
    void visit(ExprStmt&) override;
    void visit(Block&) override;
    void visit(FuncDecl&) override;
    void visit(Program&) override;

    // Symbol name used for user function `name` (avoids clashes with libc).
    static std::string mangle(const std::string& name) { return "rs_" + name; }

private:
    llvm::Value* emit(Expr& e);
    bool blockTerminated() const { return builder_.GetInsertBlock()->getTerminator() != nullptr; }

    // Runtime array allocator: rt_alloc(i32) -> i8*.
    llvm::Function* rtAllocFn();

    llvm::LLVMContext& ctx_;
    std::unique_ptr<llvm::Module> module_;
    llvm::IRBuilder<> builder_;
    llvm::Type* i32_;
    llvm::Type* i8ptr_;          // i8* — the type of an array reference
    llvm::Value* value_ = nullptr;
    std::unordered_map<const Symbol*, llvm::AllocaInst*> slots_;
};
