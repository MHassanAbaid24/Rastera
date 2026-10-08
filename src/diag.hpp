// Compile-time error reporting shared by every stage.
// Errors are printed immediately as `file:line: error: message`; stages keep
// going where they can so one run reports several errors, and the driver
// stops before the next stage if any error was reported.
#pragma once

#include <cstdio>
#include <string>

#include "ast/ast.hpp"

class Diagnostics {
public:
    explicit Diagnostics(std::string file) : file_(std::move(file)) {}

    void error(Loc loc, const std::string& msg) {
        std::fprintf(stderr, "%s:%d: error: %s\n", file_.c_str(), loc.line, msg.c_str());
        ++errors_;
    }

    bool hasErrors() const { return errors_ > 0; }

private:
    std::string file_;
    int errors_ = 0;
};
