#include "backend/backend.hpp"

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>

std::unique_ptr<llvm::TargetMachine> createHostTargetMachine(std::string& err) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    std::string triple = llvm::sys::getDefaultTargetTriple();
    const llvm::Target* target = llvm::TargetRegistry::lookupTarget(triple, err);
    if (!target) return nullptr;

    // PIC: Linux toolchains build position-independent executables by default.
    return std::unique_ptr<llvm::TargetMachine>(
        target->createTargetMachine(triple, "generic", "", llvm::TargetOptions(), llvm::Reloc::PIC_));
}

bool emitObject(llvm::TargetMachine& tm, llvm::Module& module, const std::string& objPath, std::string& err) {
    std::error_code ec;
    llvm::raw_fd_ostream out(objPath, ec, llvm::sys::fs::OF_None);
    if (ec) {
        err = "cannot open " + objPath + ": " + ec.message();
        return false;
    }

    llvm::legacy::PassManager passes;
    if (tm.addPassesToEmitFile(passes, out, nullptr, llvm::CodeGenFileType::ObjectFile)) {
        err = "target cannot emit object files";
        return false;
    }
    passes.run(module);
    return true;
}

bool linkExecutable(const std::string& objPath, const std::string& exePath, std::string& err) {
    auto cc = llvm::sys::findProgramByName("cc");
    if (!cc) {
        err = "no C compiler (cc) found to link with";
        return false;
    }
    // Run the linker directly (no shell), so paths with spaces need no quoting.
    llvm::SmallVector<llvm::StringRef, 6> args = {*cc, objPath, RASTERA_RUNTIME_LIB, "-o", exePath};
    if (llvm::sys::ExecuteAndWait(*cc, args, std::nullopt, {}, 0, 0, &err) != 0) {
        if (err.empty()) err = "linking failed";
        return false;
    }
    return true;
}
