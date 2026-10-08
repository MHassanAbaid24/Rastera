// rasterc: the Rastera compiler driver. Runs the pipeline
//   source -> tokens -> AST -> sema -> LLVM IR -> object file -> executable
// and can stop after any stage to print its output.
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>

#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

#include "ast/ast_printer.hpp"
#include "backend/backend.hpp"
#include "codegen/codegen.hpp"
#include "diag.hpp"
#include "parse/frontend.hpp"
#include "parser.hpp"
#include "sema/sema.hpp"

namespace {

enum class Stage { Tokens, Ast, Ir, Executable };

int usage() {
    std::fprintf(stderr, "usage: rasterc <file.rst> [-o <output>] [--tokens | --ast | --ir]\n");
    return 1;
}

void dumpTokens() {
    while (int token = yylex()) {
        std::printf("%d: %s", yylloc.first_line, token_name(token));
        if (token == INT) std::printf(" %d", yylval.num);
        if (token == IDENT) {
            std::printf(" %s", yylval.str->c_str());
            delete yylval.str;
        }
        std::printf("\n");
    }
}

}  // namespace

int main(int argc, char** argv) {
    const char* input = nullptr;
    std::string output = "a.out";
    Stage stage = Stage::Executable;

    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "--tokens")) stage = Stage::Tokens;
        else if (!std::strcmp(argv[i], "--ast")) stage = Stage::Ast;
        else if (!std::strcmp(argv[i], "--ir")) stage = Stage::Ir;
        else if (!std::strcmp(argv[i], "-o") && i + 1 < argc) output = argv[++i];
        else if (argv[i][0] != '-' && !input) input = argv[i];
        else return usage();
    }
    if (!input) return usage();

    FILE* in = std::fopen(input, "r");
    if (!in) {
        std::perror(input);
        return 1;
    }
    Diagnostics diag(input);

    if (stage == Stage::Tokens) {
        lexer_begin(in, diag);
        dumpTokens();
        return diag.hasErrors() ? 1 : 0;
    }

    std::unique_ptr<Program> program = parse(in, diag);
    std::fclose(in);
    if (!program || diag.hasErrors()) return 1;

    if (stage == Stage::Ast) {
        AstPrinter printer(std::cout);
        program->accept(printer);
        return 0;
    }

    Sema sema(diag);  // must outlive codegen: the AST points into its symbol table
    if (!sema.check(*program)) return 1;

    std::string err;
    auto tm = createHostTargetMachine(err);
    if (!tm) {
        std::fprintf(stderr, "rasterc: %s\n", err.c_str());
        return 1;
    }

    llvm::LLVMContext ctx;
    CodeGen codegen(ctx, input, *tm);
    std::unique_ptr<llvm::Module> module = codegen.generate(*program);

    if (stage == Stage::Ir) {
        module->print(llvm::outs(), nullptr);
        return 0;
    }

    llvm::SmallString<128> objPath;
    if (auto ec = llvm::sys::fs::createTemporaryFile("rastera", "o", objPath)) {
        std::fprintf(stderr, "rasterc: cannot create temporary file: %s\n", ec.message().c_str());
        return 1;
    }
    bool ok = emitObject(*tm, *module, objPath.str().str(), err) &&
              linkExecutable(objPath.str().str(), output, err);
    llvm::sys::fs::remove(objPath);
    if (!ok) {
        std::fprintf(stderr, "rasterc: %s\n", err.c_str());
        return 1;
    }
    return 0;
}
