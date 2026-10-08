/* Rastera lexer: turns source text into a stream of tokens for the parser.
 * Each token carries its line in yylloc and its value (number / name) in yylval. */
%{
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <string>

#include "diag.hpp"
#include "parse/frontend.hpp"
#include "parser.hpp"

Diagnostics* g_diag = nullptr;

/* Runs before every rule's action: record the line the token is on. */
#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;
%}

%option noyywrap nounput noinput yylineno

%%

[ \t\r\n]+                  { /* whitespace separates tokens, nothing more */ }
"#".*                       { /* comment to end of line */ }

[0-9]+                      {
                                errno = 0;
                                long long v = std::strtoll(yytext, nullptr, 10);
                                if (errno == ERANGE || v > INT32_MAX) {
                                    g_diag->error({yylineno}, std::string("integer literal out of range: ") + yytext);
                                    v = 0;
                                }
                                yylval.num = static_cast<int32_t>(v);
                                return INT;
                            }

[A-Za-z_][A-Za-z0-9_]*      { yylval.str = new std::string(yytext); return IDENT; }

[@()\[\]^;]                 { return yytext[0]; /* single-char tokens are their own code */ }

.                           { g_diag->error({yylineno}, std::string("unexpected character '") + yytext + "'"); }

%%

void lexer_begin(FILE* in, Diagnostics& diag) {
    g_diag = &diag;
    yylineno = 1;
    yyrestart(in);
}
