#include "codegen/codegen.hpp"

#include <cstdlib>

#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

CodeGen::CodeGen(llvm::LLVMContext& ctx, const std::string& moduleName, const llvm::TargetMachine& tm)
    : ctx_(ctx),
      module_(std::make_unique<llvm::Module>(moduleName, ctx)),
      builder_(ctx),
      i32_(llvm::Type::getInt32Ty(ctx)) {
    module_->setTargetTriple(tm.getTargetTriple().str());
    module_->setDataLayout(tm.createDataLayout());
}

std::unique_ptr<llvm::Module> CodeGen::generate(Program& program) {
    program.accept(*this);
    // A broken module here is a compiler bug, not a user error: sema should
    // have rejected anything codegen cannot handle.
    if (llvm::verifyModule(*module_, &llvm::errs())) {
        llvm::errs() << "internal compiler error: invalid IR generated\n";
        module_->print(llvm::errs(), nullptr);
        std::abort();
    }
    return std::move(module_);
}

llvm::Value* CodeGen::emit(Expr& e) {
    e.accept(*this);
    return value_;
}

void CodeGen::visit(Program& n) {
    for (auto& f : n.funcs) f->accept(*this);
}

void CodeGen::visit(FuncDecl& n) {
    auto* type = llvm::FunctionType::get(i32_, /*params=*/{}, /*isVarArg=*/false);
    auto* fn = llvm::Function::Create(type, llvm::Function::ExternalLinkage, mangle(n.name), module_.get());

    builder_.SetInsertPoint(llvm::BasicBlock::Create(ctx_, "entry", fn));
    n.body->accept(*this);

    // Falling off the end of a function returns 0 (spec §5).
    if (!blockTerminated()) builder_.CreateRet(llvm::ConstantInt::get(i32_, 0));
}

void CodeGen::visit(Block& n) {
    for (auto& s : n.stmts) {
        // After `^`, the rest of the block is unreachable: emitting it would
        // put instructions after a terminator, which is invalid IR.
        if (blockTerminated()) break;
        s->accept(*this);
    }
}

void CodeGen::visit(ReturnStmt& n) { builder_.CreateRet(emit(*n.value)); }

void CodeGen::visit(IntLit& n) { value_ = llvm::ConstantInt::get(i32_, n.value, /*isSigned=*/true); }

void CodeGen::visit(BinaryExpr& n) {
    auto* l = emit(*n.lhs);
    auto* r = emit(*n.rhs);
    switch (n.op) {
        case BinaryOp::Add: value_ = builder_.CreateAdd(l, r, "add"); break;
        case BinaryOp::Sub: value_ = builder_.CreateSub(l, r, "sub"); break;
        case BinaryOp::Mul: value_ = builder_.CreateMul(l, r, "mul"); break;
        case BinaryOp::Div: value_ = builder_.CreateSDiv(l, r, "sdiv"); break;
        case BinaryOp::Mod: value_ = builder_.CreateSRem(l, r, "srem"); break;
    }
}

void CodeGen::visit(UnaryExpr& n) {
    auto* op = emit(*n.operand);
    switch (n.op) {
        case UnaryOp::Neg: value_ = builder_.CreateNeg(op, "neg"); break;
    }
}
