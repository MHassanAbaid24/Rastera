#include "codegen/codegen.hpp"

#include <cstddef>
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
    // Declare all prototypes first so generated bodies can call later functions.
    for (auto& f : n.funcs) {
        std::vector<llvm::Type*> paramTypes;
        for (const auto& p : f->params) paramTypes.push_back(p.type == Type::Array ? i8ptr_ : i32_);
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
        // An array parameter holds the caller's array pointer (header-backed, D027), so stores and bounds checks reach the caller's data.
        auto* slot = builder_.CreateAlloca(p.type == Type::Array ? i8ptr_ : i32_, nullptr, p.name + ".addr");
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


void CodeGen::visit(IfStmt& n) {
    auto* fn = builder_.GetInsertBlock()->getParent();

    auto* condVal = emit(*n.cond);
    auto* condBool = builder_.CreateICmpNE(condVal, llvm::ConstantInt::get(condVal->getType(), 0), "ifcond");

    auto* thenBB = llvm::BasicBlock::Create(ctx_, "then", fn);
    auto* elseBB = n.else_branch ? llvm::BasicBlock::Create(ctx_, "else") : nullptr;
    auto* mergeBB = llvm::BasicBlock::Create(ctx_, "ifmerge");

    builder_.CreateCondBr(condBool, thenBB, elseBB ? elseBB : mergeBB);

    // Emit 'then' branch
    builder_.SetInsertPoint(thenBB);
    n.then_branch->accept(*this);
    bool thenTerminated = blockTerminated();
    if (!thenTerminated) {
        builder_.CreateBr(mergeBB);
    }

    // Emit 'else' branch if present
    bool elseTerminated = false;
    if (n.else_branch) {
        fn->insert(fn->end(), elseBB);
        builder_.SetInsertPoint(elseBB);
        n.else_branch->accept(*this);
        elseTerminated = blockTerminated();
        if (!elseTerminated) {
            builder_.CreateBr(mergeBB);
        }
    }

    if (!thenTerminated || (n.else_branch && !elseTerminated) || (!n.else_branch)) {
        fn->insert(fn->end(), mergeBB);
        builder_.SetInsertPoint(mergeBB);
    } else {
        delete mergeBB;
    }
}

void CodeGen::visit(WhileStmt& n) {
    auto* fn = builder_.GetInsertBlock()->getParent();
    auto* condBB = llvm::BasicBlock::Create(ctx_, "while.cond", fn);
    auto* bodyBB = llvm::BasicBlock::Create(ctx_, "while.body", fn);
    auto* exitBB = llvm::BasicBlock::Create(ctx_, "while.exit", fn);

    builder_.CreateBr(condBB);
    builder_.SetInsertPoint(condBB);
    auto* condVal = emit(*n.cond);
    auto* condBool = builder_.CreateICmpNE(
        condVal, llvm::ConstantInt::get(condVal->getType(), 0), "whilecond");
    builder_.CreateCondBr(condBool, bodyBB, exitBB);

    builder_.SetInsertPoint(bodyBB);
    n.body->accept(*this);
    if (!blockTerminated()) builder_.CreateBr(condBB);

    builder_.SetInsertPoint(exitBB);
}

void CodeGen::visit(RangeStmt& n) {
    auto* fn = builder_.GetInsertBlock()->getParent();

    // Both bounds are evaluated once, left to right, before the first iteration.
    auto* from = emit(*n.from);
    auto* to = emit(*n.to);

    llvm::IRBuilder<> entry(&fn->getEntryBlock(), fn->getEntryBlock().begin());
    auto* slot = entry.CreateAlloca(i32_, nullptr, n.var);
    slots_.emplace(n.sym, slot);
    builder_.CreateStore(from, slot);

    auto* bodyBB = llvm::BasicBlock::Create(ctx_, "range.body", fn);
    auto* exitBB = llvm::BasicBlock::Create(ctx_, "range.exit");

    // `a > b` skips the loop. This is the only comparison that tests a bound.
    builder_.CreateCondBr(builder_.CreateICmpSLE(from, to, "range.nonempty"), bodyBB, exitBB);

    builder_.SetInsertPoint(bodyBB);
    n.body->accept(*this);
    // A body that ended in `^` has no back-edge (same rule as while).
    if (!blockTerminated()) {
        // Test `i == b` before incrementing: `i + 1` never runs when b is INT_MAX, so it cannot wrap.
        auto* stepBB = llvm::BasicBlock::Create(ctx_, "range.step", fn);
        auto* nextBB = llvm::BasicBlock::Create(ctx_, "range.next", fn);
        builder_.CreateBr(stepBB);

        builder_.SetInsertPoint(stepBB);
        auto* current = builder_.CreateLoad(i32_, slot, n.var);
        builder_.CreateCondBr(builder_.CreateICmpEQ(current, to, "range.last"), exitBB, nextBB);

        builder_.SetInsertPoint(nextBB);
        builder_.CreateStore(builder_.CreateAdd(current, llvm::ConstantInt::get(i32_, 1), n.var + ".next"), slot);
        builder_.CreateBr(bodyBB);
    }

    fn->insert(fn->end(), exitBB);
    builder_.SetInsertPoint(exitBB);
}

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

llvm::Function* CodeGen::rtAllocFn() {
    llvm::Function* fn = module_->getFunction("rt_alloc");
    if (!fn) {
        std::vector<llvm::Type*> paramTypes = {i32_};
        auto* fnType = llvm::FunctionType::get(i8ptr_, paramTypes, /*isVarArg=*/false);
        fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, "rt_alloc", module_.get());
    }
    return fn;
}

llvm::Function* CodeGen::rtPanicFn() {
    llvm::Function* fn = module_->getFunction("rt_panic");
    if (!fn) {
        std::vector<llvm::Type*> paramTypes = {i8ptr_};
        auto* fnType = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx_), paramTypes, /*isVarArg=*/false);
        fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, "rt_panic", module_.get());
        fn->addFnAttr(llvm::Attribute::NoReturn);
    }
    return fn;
}

void CodeGen::emitRuntimeCheck(llvm::Value* condition, const char* message) {
    auto* fn = builder_.GetInsertBlock()->getParent();
    auto* panic = llvm::BasicBlock::Create(ctx_, "panic", fn);
    auto* cont = llvm::BasicBlock::Create(ctx_, "check.cont", fn);
    builder_.CreateCondBr(condition, panic, cont);

    builder_.SetInsertPoint(panic);
    builder_.CreateCall(rtPanicFn(), {builder_.CreateGlobalStringPtr(message, "panic.msg")});
    builder_.CreateUnreachable();
    builder_.SetInsertPoint(cont);
}

llvm::Value* CodeGen::checkedArrayData(llvm::Value* arr, llvm::Value* idx) {
    auto* i32arr = builder_.CreateBitCast(arr, i32_->getPointerTo(0), "arr.i32");
    auto* lengthPtr = builder_.CreateGEP(i32_, i32arr, {llvm::ConstantInt::get(i32_, -1, true)}, "arr.len");
    auto* length = builder_.CreateLoad(i32_, lengthPtr, "arr.length");
    auto* negative = builder_.CreateICmpSLT(idx, llvm::ConstantInt::get(i32_, 0), "index.negative");
    auto* tooHigh = builder_.CreateICmpSGE(idx, length, "index.high");
    emitRuntimeCheck(builder_.CreateOr(negative, tooHigh, "index.invalid"), "array index out of range");
    return i32arr;
}

void CodeGen::visit(ArrayLit& n) {
    // <1, 2, 3>: allocate via rt_alloc(count), then store each element.
    auto* count = llvm::ConstantInt::get(i32_, static_cast<int32_t>(n.elements.size()));
    auto* arr = builder_.CreateCall(rtAllocFn(), {count}, "arr");
    auto* i32arr = builder_.CreateBitCast(arr, i32_->getPointerTo(0), "arr.i32");
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
    auto* arr = emit(*n.array);
    auto* idx = emit(*n.index);
    auto* elemPtr = builder_.CreateGEP(i32_, checkedArrayData(arr, idx), {idx}, "elem");
    value_ = builder_.CreateLoad(i32_, elemPtr, "elem.val");
}

void CodeGen::visit(ArrayStoreStmt& n) {
    auto* arr = emit(*n.array);
    auto* idx = emit(*n.index);
    auto* val = emit(*n.value);
    auto* elemPtr = builder_.CreateGEP(i32_, checkedArrayData(arr, idx), {idx}, "elem");
    builder_.CreateStore(val, elemPtr);
}

void CodeGen::visit(IntLit& n) { value_ = llvm::ConstantInt::get(i32_, n.value, /*isSigned=*/true); }

void CodeGen::visit(BinaryExpr& n) {
    auto* l = emit(*n.lhs);

    if (n.op == BinaryOp::And || n.op == BinaryOp::Or) {
        auto* lhsBB = builder_.GetInsertBlock();
        auto* fn = lhsBB->getParent();
        auto* lhsBool = builder_.CreateICmpNE(l, llvm::ConstantInt::get(i32_, 0), "logic.lhs");
        auto* rhsBB = llvm::BasicBlock::Create(ctx_, "logic.rhs", fn);
        auto* mergeBB = llvm::BasicBlock::Create(ctx_, "logic.merge");

        if (n.op == BinaryOp::And) {
            builder_.CreateCondBr(lhsBool, rhsBB, mergeBB);
        } else {
            builder_.CreateCondBr(lhsBool, mergeBB, rhsBB);
        }

        builder_.SetInsertPoint(rhsBB);
        auto* r = emit(*n.rhs);
        auto* rhsBool = builder_.CreateICmpNE(r, llvm::ConstantInt::get(i32_, 0), "logic.rhs");
        auto* rhsValue = builder_.CreateZExt(rhsBool, i32_, "logic.value");
        auto* rhsEndBB = builder_.GetInsertBlock();
        builder_.CreateBr(mergeBB);

        fn->insert(fn->end(), mergeBB);
        builder_.SetInsertPoint(mergeBB);
        auto* phi = builder_.CreatePHI(i32_, 2, "logic");
        phi->addIncoming(llvm::ConstantInt::get(i32_, n.op == BinaryOp::And ? 0 : 1), lhsBB);
        phi->addIncoming(rhsValue, rhsEndBB);
        value_ = phi;
        return;
    }

    auto* r = emit(*n.rhs);
    switch (n.op) {
        case BinaryOp::Add: value_ = builder_.CreateAdd(l, r, "add"); break;
        case BinaryOp::Sub: value_ = builder_.CreateSub(l, r, "sub"); break;
        case BinaryOp::Mul: value_ = builder_.CreateMul(l, r, "mul"); break;
        case BinaryOp::Div:
            emitRuntimeCheck(builder_.CreateICmpEQ(r, llvm::ConstantInt::get(i32_, 0), "div.zero"),
                             "division by zero");
            emitRuntimeCheck(builder_.CreateAnd(
                                 builder_.CreateICmpEQ(l, llvm::ConstantInt::get(i32_, 0x80000000U), "div.min"),
                                 builder_.CreateICmpEQ(r, llvm::ConstantInt::get(i32_, -1, true), "div.minus_one"),
                                 "div.overflow"),
                             "integer division overflow");
            value_ = builder_.CreateSDiv(l, r, "sdiv");
            break;
        case BinaryOp::Mod:
            emitRuntimeCheck(builder_.CreateICmpEQ(r, llvm::ConstantInt::get(i32_, 0), "mod.zero"),
                             "modulo by zero");
            emitRuntimeCheck(builder_.CreateAnd(
                                 builder_.CreateICmpEQ(l, llvm::ConstantInt::get(i32_, 0x80000000U), "mod.min"),
                                 builder_.CreateICmpEQ(r, llvm::ConstantInt::get(i32_, -1, true), "mod.minus_one"),
                                 "mod.overflow"),
                             "integer modulo overflow");
            value_ = builder_.CreateSRem(l, r, "srem");
            break;
        case BinaryOp::Equal: value_ = builder_.CreateZExt(builder_.CreateICmpEQ(l, r, "eq"), i32_); break;
        case BinaryOp::NotEqual: value_ = builder_.CreateZExt(builder_.CreateICmpNE(l, r, "ne"), i32_); break;
        case BinaryOp::Less: value_ = builder_.CreateZExt(builder_.CreateICmpSLT(l, r, "lt"), i32_); break;
        case BinaryOp::LessEqual: value_ = builder_.CreateZExt(builder_.CreateICmpSLE(l, r, "le"), i32_); break;
        case BinaryOp::Greater: value_ = builder_.CreateZExt(builder_.CreateICmpSGT(l, r, "gt"), i32_); break;
        case BinaryOp::GreaterEqual: value_ = builder_.CreateZExt(builder_.CreateICmpSGE(l, r, "ge"), i32_); break;
        case BinaryOp::And:
        case BinaryOp::Or:
            break;
    }
}

void CodeGen::visit(UnaryExpr& n) {
    auto* op = emit(*n.operand);
    switch (n.op) {
        case UnaryOp::Neg: value_ = builder_.CreateNeg(op, "neg"); break;
        case UnaryOp::Not:
            value_ = builder_.CreateZExt(
                builder_.CreateICmpEQ(op, llvm::ConstantInt::get(i32_, 0), "not"),
                i32_);
            break;
    }
}
