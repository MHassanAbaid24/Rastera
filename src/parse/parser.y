/* Rastera grammar: turns the token stream into an AST (see ast/ast.hpp).
 * Semantic values are raw pointers inside the %union (a C union cannot hold
 * unique_ptr); each action immediately hands them to an owning unique_ptr.
 * %destructor frees values Bison discards while recovering from an error. */
%code requires {
#include <memory>
#include <string>
#include "ast/ast.hpp"
}

%{
#include "diag.hpp"
#include "parse/frontend.hpp"

void yyerror(std::unique_ptr<Program>& out, const char* msg);
%}

%locations
%define parse.error verbose
%parse-param { std::unique_ptr<Program>& out }

%union {
    int32_t num;
    std::string* str;
    Program* program;
    FuncDecl* func;
    Block* block;
    Stmt* stmt;
    Expr* expr;
}

%token <num> INT "integer"
%token <str> IDENT "identifier"

%type <program> program
%type <func> func
%type <block> block stmts
%type <stmt> stmt
%type <expr> expr

%destructor { delete $$; } <str> <program> <func> <block> <stmt> <expr>

%left '+' '-'
%left '*' '/' '%'
%precedence UMINUS

%%

start
    : program                   { out.reset($1); }
    ;

program
    : %empty                    { $$ = new Program(); $$->loc.line = 1; }
    | program func              { $1->funcs.emplace_back($2); $$ = $1; }
    ;

func
    : '@' IDENT '(' ')' block   {
                                    $$ = new FuncDecl(std::move(*$2), std::unique_ptr<Block>($5));
                                    delete $2;
                                    $$->loc.line = @1.first_line;
                                }
    ;

block
    : '[' stmts ']'             { $$ = $2; $$->loc.line = @1.first_line; }
    ;

stmts
    : %empty                    { $$ = new Block(); }
    | stmts stmt                { $1->stmts.emplace_back($2); $$ = $1; }
    ;

stmt
    : '^' expr ';'              { $$ = new ReturnStmt(std::unique_ptr<Expr>($2)); $$->loc.line = @1.first_line; }
    ;

expr
    : INT                       { $$ = new IntLit($1); $$->loc.line = @1.first_line; }
    | '(' expr ')'              { $$ = $2; }
    | expr '+' expr             { $$ = new BinaryExpr(BinaryOp::Add, std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); $$->loc.line = @2.first_line; }
    | expr '-' expr             { $$ = new BinaryExpr(BinaryOp::Sub, std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); $$->loc.line = @2.first_line; }
    | expr '*' expr             { $$ = new BinaryExpr(BinaryOp::Mul, std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); $$->loc.line = @2.first_line; }
    | expr '/' expr             { $$ = new BinaryExpr(BinaryOp::Div, std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); $$->loc.line = @2.first_line; }
    | expr '%' expr             { $$ = new BinaryExpr(BinaryOp::Mod, std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); $$->loc.line = @2.first_line; }
    | '-' expr %prec UMINUS     { $$ = new UnaryExpr(UnaryOp::Neg, std::unique_ptr<Expr>($2)); $$->loc.line = @1.first_line; }
    ;

%%

void yyerror(std::unique_ptr<Program>&, const char* msg) {
    g_diag->error({yylloc.first_line}, msg);
}

const char* token_name(int token) {
    return yysymbol_name(YYTRANSLATE(token));
}

std::unique_ptr<Program> parse(FILE* in, Diagnostics& diag) {
    lexer_begin(in, diag);
    std::unique_ptr<Program> out;
    if (yyparse(out) != 0) return nullptr;
    return out;
}
