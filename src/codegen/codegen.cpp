#include "codegen/codegen.hpp"

#include <cstddef>
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
    // Declare all prototypes first so generated bodies can call later functions.
    for (auto& f : n.funcs) {
        std::vector<llvm::Type*> paramTypes(f->params.size(), i32_);
        auto* type = llvm::FunctionType::get(i32_, paramTypes, /*isVarArg=*/false);
        llvm::Function::Create(type, llvm::Function::ExternalLinkage, mangle(f->name), module_.get());
    }
    for (auto& f : n.funcs) f->accept(*this);
}

void CodeGen::visit(FuncDecl& n) {
    auto* fn = module_->getFunction(mangle(n.name));

    builder_.SetInsertPoint(llvm::BasicBlock::Create(ctx_, "entry", fn));
    for (std::size_t i = 0; i < n.params.size(); ++i) {
        const auto& p = n.params[i];
        auto* arg = fn->getArg(i);
        arg->setName(p.name);
        // Parameters are mutable variables, so their entry slots are keyed by Symbol*; mem2reg removes them.
        auto* slot = builder_.CreateAlloca(i32_, nullptr, p.name + ".addr");
        builder_.CreateStore(arg, slot);
        slots_.emplace(p.sym, slot);
    }
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

void CodeGen::visit(AssignStmt& n) {
    auto* v = emit(*n.value);
    if (n.declares) {
        auto* fn = builder_.GetInsertBlock()->getParent();
        // Entry-block allocas give each variable one stack slot, enable mem2reg,
        // and remain correct when T06 introduces loop-local declarations.
        llvm::IRBuilder<> tmp(&fn->getEntryBlock(), fn->getEntryBlock().begin());
        slots_.emplace(n.sym, tmp.CreateAlloca(i32_, nullptr, n.name));
    }
    builder_.CreateStore(v, slots_.at(n.sym));
}

void CodeGen::visit(VarRef& n) { value_ = builder_.CreateLoad(i32_, slots_.at(n.sym), n.name); }

void CodeGen::visit(ExprStmt& n) { emit(*n.expr); }

void CodeGen::visit(CallExpr& n) {
    llvm::Function* fn;
    if (n.callee) {
        fn = module_->getFunction(mangle(n.name));
    } else {
        std::string rtName = "rt_" + n.name;
        fn = module_->getFunction(rtName);
        if (!fn) {
            std::vector<llvm::Type*> paramTypes(n.args.size(), i32_);
            auto* fnType = llvm::FunctionType::get(i32_, paramTypes, /*isVarArg=*/false);
            fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, rtName, module_.get());
        }
    }

    std::vector<llvm::Value*> args;
    args.reserve(n.args.size());
    for (auto& a : n.args) {
        args.push_back(emit(*a));
    }

    value_ = builder_.CreateCall(fn, args);
}

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
