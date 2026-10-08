// Turns an LLVM module into a native executable:
//   module --(LLVM target machine)--> object file --(system C compiler as linker)--> executable
// The executable also links the Rastera runtime library (runtime/), which
// provides the C `main` and all built-in functions.
#pragma once

#include <memory>
#include <string>

#include <llvm/IR/Module.h>
#include <llvm/Target/TargetMachine.h>

// Target machine for the host CPU/OS; nullptr and `err` set on failure.
std::unique_ptr<llvm::TargetMachine> createHostTargetMachine(std::string& err);

// Writes `module` as a native object file to `objPath`.
bool emitObject(llvm::TargetMachine& tm, llvm::Module& module, const std::string& objPath, std::string& err);

// Links `objPath` with the runtime library into `exePath`.
bool linkExecutable(const std::string& objPath, const std::string& exePath, std::string& err);
