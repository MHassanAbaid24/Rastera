#include "codegen/codegen.hpp"

#include <cstdlib>

#include <llvm/IR/Verifier.h>

#include "sema/symbols.hpp"
#include <llvm/Support/raw_ostream.h>

CodeGen::CodeGen(llvm::LLVMContext& ctx, const std::string& moduleName, const llvm::TargetMachine& tm)
    : ctx_(ctx),
      module_(std::make_unique<llvm::Module>(moduleName, ctx)),
      builder_(ctx),
      i32_(llvm::Type::getInt32Ty(ctx)),
      i8ptr_(llvm::Type::getInt8Ty(ctx)->getPointerTo(0)) {
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

void CodeGen::visit(AssignStmt& n) {
    auto* v = emit(*n.value);
    if (n.declares) {
        auto* fn = builder_.GetInsertBlock()->getParent();
        // Entry-block allocas give each variable one stack slot, enable mem2reg,
        // and remain correct when T06 introduces loop-local declarations.
        // Array variables hold an i8* pointer to heap data; ints hold i32.
        llvm::IRBuilder<> tmp(&fn->getEntryBlock(), fn->getEntryBlock().begin());
        llvm::Type* slotType = (n.sym->type == Type::Array) ? i8ptr_ : i32_;
        slots_.emplace(n.sym, tmp.CreateAlloca(slotType, nullptr, n.name));
    }
    builder_.CreateStore(v, slots_.at(n.sym));
}

void CodeGen::visit(VarRef& n) {
    if (n.sym->type == Type::Array) {
        value_ = builder_.CreateLoad(i8ptr_, slots_.at(n.sym), n.name);
    } else {
        value_ = builder_.CreateLoad(i32_, slots_.at(n.sym), n.name);
    }
}

void CodeGen::visit(ExprStmt& n) { emit(*n.expr); }

void CodeGen::visit(CallExpr& n) {
    std::string rtName = "rt_" + n.name;
    llvm::Function* fn = module_->getFunction(rtName);
    if (!fn) {
        std::vector<llvm::Type*> paramTypes(n.args.size(), i32_);
        auto* fnType = llvm::FunctionType::get(i32_, paramTypes, /*isVarArg=*/false);
        fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, rtName, module_.get());
    }

    std::vector<llvm::Value*> args;
    args.reserve(n.args.size());
    for (auto& a : n.args) {
        args.push_back(emit(*a));
    }

    value_ = builder_.CreateCall(fn, args);
}

// Declare (or get) the runtime array allocator: rt_alloc(i32) -> i8*.
llvm::Function* CodeGen::rtAllocFn() {
    llvm::Function* fn = module_->getFunction("rt_alloc");
    if (!fn) {
        std::vector<llvm::Type*> paramTypes = {i32_};
        auto* fnType = llvm::FunctionType::get(i8ptr_, paramTypes, /*isVarArg=*/false);
        fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, "rt_alloc", module_.get());
    }
    return fn;
}

void CodeGen::visit(ArrayLit& n) {
    // <1, 2, 3>: allocate via rt_alloc(count), then store each element.
    auto* count = llvm::ConstantInt::get(i32_, static_cast<int32_t>(n.elements.size()));
    auto* arr = builder_.CreateCall(rtAllocFn(), {count}, "arr");
    auto* i32arr = builder_.CreateBitCast(arr, llvm::Type::getInt32Ty(ctx_)->getPointerTo(0), "arr.i32");
    int idx = 0;
    for (auto& e : n.elements) {
        auto* val = emit(*e);
        auto* elemPtr = builder_.CreateGEP(i32_, i32arr, {llvm::ConstantInt::get(i32_, idx++)}, "elem");
        builder_.CreateStore(val, elemPtr);
    }
    value_ = arr;  // the array reference (i8*) is the expression's value
}

void CodeGen::visit(ArrayAlloc& n) {
    // arr(n): call rt_alloc(n), result is the array reference (i8*).
    auto* size = emit(*n.size);
    value_ = builder_.CreateCall(rtAllocFn(), {size}, "arr");
}

void CodeGen::visit(IndexExpr& n) {
    // a[i]: load array ptr (i8*), bitcast to i32*, GEP by index, load i32.
    auto* arr = emit(*n.array);
    auto* idx = emit(*n.index);
    auto* i32arr = builder_.CreateBitCast(arr, llvm::Type::getInt32Ty(ctx_)->getPointerTo(0), "arr.i32");
    auto* elemPtr = builder_.CreateGEP(i32_, i32arr, {idx}, "elem");
    value_ = builder_.CreateLoad(i32_, elemPtr, "elem.val");
}

void CodeGen::visit(ArrayStoreStmt& n) {
    // a[i] := e: load array ptr, bitcast to i32*, GEP by index, store i32.
    auto* arr = emit(*n.array);
    auto* idx = emit(*n.index);
    auto* val = emit(*n.value);
    auto* i32arr = builder_.CreateBitCast(arr, llvm::Type::getInt32Ty(ctx_)->getPointerTo(0), "arr.i32");
    auto* elemPtr = builder_.CreateGEP(i32_, i32arr, {idx}, "elem");
    builder_.CreateStore(val, elemPtr);
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
