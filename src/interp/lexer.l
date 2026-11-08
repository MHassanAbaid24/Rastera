%{
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <string>

#include "interp.hpp"

#define YY_USER_ACTION iplloc.first_line = iplloc.last_line = yylineno;
%}

%option noyywrap nounput noinput yylineno prefix="ip"

%%

[ \t\r\n]+                  { }
"#".*                       { }

[0-9]+                      {
                                errno = 0;
                                long long value = std::strtoll(yytext, nullptr, 10);
                                if (errno == ERANGE || value > INT32_MAX) {
                                    ip_interpreter->error(yylineno,
                                                          std::string("integer literal out of range: ") + yytext);
                                    value = 0;
                                }
                                iplval.num = static_cast<int32_t>(value);
                                return INT;
                            }

"arr"                       { return ARR; }
[A-Za-z_][A-Za-z0-9_]*      { iplval.str = new std::string(yytext); return IDENT; }

":="                        { return ASSIGN; }
"=="                        { return EQ; }
"!="                        { return NE; }
"<="                        { return LE; }
">="                        { return GE; }

[@()\[\]^;+\-*/%,?:<>]      { return yytext[0]; }
.                           { ip_interpreter->error(yylineno, std::string("unexpected character '") + yytext + "'"); }

%%
