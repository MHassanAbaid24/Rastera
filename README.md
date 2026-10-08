# Rastera

A small compiled language for procedural graphics with symbolic control flow
(`@` functions, `?`/`:` branches, `*` loops, `^` return). Built with Flex, Bison and LLVM 18.
See [`docs/proposal.pdf`](docs/proposal.pdf) and the [language spec](docs/LANGUAGE_SPEC.md).

```
@main() [
    ^ 42;
]
```

## Build and test

Everything runs inside one Docker image, so every machine and CI use the same toolchain.

**Windows:** use WSL2 (Ubuntu) with Docker Desktop's WSL integration enabled, and clone the repo
inside the WSL home folder (`~/`), not under `/mnt/c/`. Run every command from the WSL terminal.
Git Bash / PowerShell are not supported.

```sh
./dev.sh build                   # build/rasterc
./dev.sh test                    # build + run tests/
./dev.sh                         # shell inside the container
```

## Using the compiler

```sh
./dev.sh build/rasterc prog.rst -o prog   # compile to a native executable
./dev.sh ./prog
./dev.sh build/rasterc prog.rst --tokens  # stop after lexing and print tokens
./dev.sh build/rasterc prog.rst --ast     # stop after parsing and print the AST
./dev.sh build/rasterc prog.rst --ir      # stop after codegen and print LLVM IR
```

## Pipeline

```
source.rst ─ lexer.l ─▶ tokens ─ parser.y ─▶ AST ─ sema ─▶ checked AST ─ codegen ─▶ LLVM IR ─ backend ─▶ object file ─ cc + runtime ─▶ executable
```

| Path | Stage |
|---|---|
| `src/parse/lexer.l` | Flex lexer: characters → tokens |
| `src/parse/parser.y` | Bison grammar: tokens → AST |
| `src/ast/` | AST node types, visitor, `--ast` printer |
| `src/sema/` | scopes, symbol table, semantic checks |
| `src/codegen/` | AST → LLVM IR |
| `src/backend/` | LLVM IR → object file → linked executable |
| `src/main.cpp` | `rasterc` driver |
| `runtime/` | C runtime linked into every program (entry point, built-ins, graphics) |
| `tests/` | `pass/` programs that must run, `fail/` programs that must be rejected; see `tests/run_tests.sh` for directives |

## Workflow

- `main` is protected: changes land only through pull requests with one approval from another member and green CI.
- One task per branch, named `T07-functions` (task id + short name).
- A PR changes at most **300 lines** in `src/` + `runtime/` (tests and docs not counted); CI enforces this.
- Every feature PR adds `tests/pass` and `tests/fail` cases and keeps `docs/LANGUAGE_SPEC.md` accurate.
- Squash merge; the PR title becomes the commit message (`T07: functions, calls and recursion`).
