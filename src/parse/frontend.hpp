// Interface between the driver and the Flex/Bison front end.
// Flex and Bison generate C-style code that communicates through globals
// (yyin, yylval, yylloc); this header is the only place the rest of the
// compiler touches them.
#pragma once

#include <cstdio>
#include <memory>

#include "ast/ast.hpp"

class Diagnostics;

// Defined in lexer.l. The lexer reports bad characters through g_diag.
extern Diagnostics* g_diag;
void lexer_begin(FILE* in, Diagnostics& diag);
int yylex();

// Defined in parser.y.
const char* token_name(int token);

// Lex + parse a whole file. Returns nullptr on a syntax error.
std::unique_ptr<Program> parse(FILE* in, Diagnostics& diag);
