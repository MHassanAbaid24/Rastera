%code requires {
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

enum class IType { Int, Array };

struct IExpr {
    explicit IExpr(int source_line) : line(source_line) {}
    virtual ~IExpr() = default;
    int line;
    IType type = IType::Int;
};

struct IIntExpr final : IExpr {
    IIntExpr(int source_line, int32_t number) : IExpr(source_line), value(number) {}
    int32_t value;
};

struct IVarExpr final : IExpr {
    IVarExpr(int source_line, std::string variable) : IExpr(source_line), name(std::move(variable)) {}
    std::string name;
};

struct ICallExpr final : IExpr {
    ICallExpr(int source_line, std::string function, std::vector<std::unique_ptr<IExpr>> values)
        : IExpr(source_line), name(std::move(function)), args(std::move(values)) {}
    std::string name;
    std::vector<std::unique_ptr<IExpr>> args;
};

struct IArrayExpr final : IExpr {
    IArrayExpr(int source_line, std::vector<std::unique_ptr<IExpr>> values)
        : IExpr(source_line), elements(std::move(values)) {}
    std::vector<std::unique_ptr<IExpr>> elements;
};

struct IArrayAllocExpr final : IExpr {
    IArrayAllocExpr(int source_line, std::unique_ptr<IExpr> length)
        : IExpr(source_line), size(std::move(length)) {}
    std::unique_ptr<IExpr> size;
};

struct IIndexExpr final : IExpr {
    IIndexExpr(int source_line, std::unique_ptr<IExpr> base, std::unique_ptr<IExpr> offset)
        : IExpr(source_line), array(std::move(base)), index(std::move(offset)) {}
    std::unique_ptr<IExpr> array;
    std::unique_ptr<IExpr> index;
};

enum class IBinaryOp {
    Add, Subtract, Multiply, Divide, Modulo, Equal, NotEqual, Less, LessEqual, Greater, GreaterEqual, And, Or
};

struct IBinaryExpr final : IExpr {
    IBinaryExpr(int source_line, IBinaryOp operation, std::unique_ptr<IExpr> left,
                std::unique_ptr<IExpr> right)
        : IExpr(source_line), op(operation), lhs(std::move(left)), rhs(std::move(right)) {}
    IBinaryOp op;
    std::unique_ptr<IExpr> lhs;
    std::unique_ptr<IExpr> rhs;
};

enum class IUnaryOp { Negate, Not };

struct IUnaryExpr final : IExpr {
    IUnaryExpr(int source_line, IUnaryOp operation, std::unique_ptr<IExpr> value)
        : IExpr(source_line), op(operation), operand(std::move(value)) {}
    IUnaryOp op;
    std::unique_ptr<IExpr> operand;
};

struct IStmt {
    explicit IStmt(int source_line) : line(source_line) {}
    virtual ~IStmt() = default;
    int line;
};

struct IBlock final : IStmt {
    explicit IBlock(int source_line = 0) : IStmt(source_line) {}
    std::vector<std::unique_ptr<IStmt>> statements;
};

struct IReturnStmt final : IStmt {
    IReturnStmt(int source_line, std::unique_ptr<IExpr> result)
        : IStmt(source_line), value(std::move(result)) {}
    std::unique_ptr<IExpr> value;
};

struct IAssignStmt final : IStmt {
    IAssignStmt(int source_line, std::string variable, std::unique_ptr<IExpr> result)
        : IStmt(source_line), name(std::move(variable)), value(std::move(result)) {}
    std::string name;
    std::unique_ptr<IExpr> value;
};

struct IArrayStoreStmt final : IStmt {
    IArrayStoreStmt(int source_line, std::string variable, std::unique_ptr<IExpr> offset,
                    std::unique_ptr<IExpr> result)
        : IStmt(source_line), name(std::move(variable)), index(std::move(offset)), value(std::move(result)) {}
    std::string name;
    std::unique_ptr<IExpr> index;
    std::unique_ptr<IExpr> value;
};

struct IExprStmt final : IStmt {
    IExprStmt(int source_line, std::unique_ptr<IExpr> result)
        : IStmt(source_line), expr(std::move(result)) {}
    std::unique_ptr<IExpr> expr;
};

struct IIfStmt final : IStmt {
    IIfStmt(int source_line, std::unique_ptr<IExpr> condition, std::unique_ptr<IBlock> yes,
            std::unique_ptr<IStmt> no)
        : IStmt(source_line), cond(std::move(condition)), then_branch(std::move(yes)), else_branch(std::move(no)) {}
    std::unique_ptr<IExpr> cond;
    std::unique_ptr<IBlock> then_branch;
    std::unique_ptr<IStmt> else_branch;
};

struct IWhileStmt final : IStmt {
    IWhileStmt(int source_line, std::unique_ptr<IExpr> condition, std::unique_ptr<IBlock> loop_body)
        : IStmt(source_line), cond(std::move(condition)), body(std::move(loop_body)) {}
    std::unique_ptr<IExpr> cond;
    std::unique_ptr<IBlock> body;
};

struct IParam {
    std::string name;
    int line;
};

struct IFunc {
    IFunc(int source_line, std::string function, std::vector<IParam> parameters,
          std::unique_ptr<IBlock> implementation)
        : line(source_line), name(std::move(function)), params(std::move(parameters)), body(std::move(implementation)) {}
    int line;
    std::string name;
    std::vector<IParam> params;
    std::unique_ptr<IBlock> body;
};

struct IProgram {
    std::vector<std::unique_ptr<IFunc>> functions;
};

class Interpreter {
public:
    explicit Interpreter(std::string source_file) : file_(std::move(source_file)) {}

    void error(int line, const std::string& message);
    bool hasErrors() const { return errors_ != 0; }
    bool check();
    int execute();

    std::unique_ptr<IProgram> program;

private:
    struct Value {
        IType type = IType::Int;
        int32_t integer = 0;
        std::shared_ptr<std::vector<int32_t>> array;
    };

    struct ReturnState {
        bool returned = false;
        int32_t value = 0;
    };

    using TypeScope = std::unordered_map<std::string, IType>;
    using ValueScope = std::unordered_map<std::string, Value>;

    bool checkFunction(IFunc& function);
    bool checkBlock(IBlock& block);
    bool checkStmt(IStmt& statement);
    IType checkExpr(IExpr& expression);
    IType* findType(const std::string& name);
    Value* findValue(const std::string& name);
    const IFunc* findFunction(const std::string& name) const;
    int32_t call(const IFunc& function, const std::vector<Value>& args);
    ReturnState execBlock(IBlock& block);
    ReturnState execStmt(IStmt& statement);
    Value eval(IExpr& expression);
    [[noreturn]] void runtimeError(const char* message) const;

    std::string file_;
    int errors_ = 0;
    std::unordered_map<std::string, IFunc*> functions_;
    std::vector<TypeScope> types_;
    std::vector<ValueScope> values_;
};

extern Interpreter* ip_interpreter;
}

%{
#include "interp.hpp"

#include <cstdlib>

int iplex(void);
void iprestart(FILE* input_file);
void iperror(Interpreter& interpreter, const char* message);
%}

%define api.prefix {ip}
%define parse.error verbose
%locations
%parse-param { Interpreter& interpreter }

%union {
    int32_t num;
    std::string* str;
    IProgram* program;
    IFunc* function;
    IBlock* block;
    IStmt* stmt;
    IExpr* expr;
    std::vector<std::unique_ptr<IExpr>>* expr_list;
    std::vector<IParam>* param_list;
}

%token <num> INT "integer"
%token <str> IDENT "identifier"
%token ASSIGN ":="
%token EQ "=="
%token NE "!="
%token LE "<="
%token GE ">="
%token ARR "array constructor"

%type <program> program
%type <function> function
%type <block> block statements
%type <stmt> statement if_statement while_statement
%type <expr> expression
%type <expr_list> args argument_list array_elements
%type <param_list> params parameter_list

%destructor { delete $$; } <str> <program> <function> <block> <stmt> <expr> <expr_list> <param_list>

%precedence IDENT
%left '|'
%left '&'
%nonassoc EQ NE '<' LE '>' GE
%precedence ARRAY_ELEMENT
%left '+' '-'
%left '*' '/' '%'
%right UMINUS
%precedence INDEX
%precedence '['

%%

start
    : program { interpreter.program.reset($1); }
    ;

program
    : %empty { $$ = new IProgram(); }
    | program function { $1->functions.emplace_back($2); $$ = $1; }
    ;

function
    : '@' IDENT '(' params ')' block {
        $$ = new IFunc(@1.first_line, std::move(*$2), std::move(*$4), std::unique_ptr<IBlock>($6));
        delete $2;
        delete $4;
    }
    ;

block
    : '[' statements ']' { $$ = $2; $$->line = @1.first_line; }
    ;

statements
    : %empty { $$ = new IBlock(); }
    | statements statement { $1->statements.emplace_back($2); $$ = $1; }
    ;

statement
    : '^' expression ';' { $$ = new IReturnStmt(@1.first_line, std::unique_ptr<IExpr>($2)); }
    | IDENT ASSIGN expression ';' {
        $$ = new IAssignStmt(@1.first_line, std::move(*$1), std::unique_ptr<IExpr>($3));
        delete $1;
    }
    | IDENT '[' expression ']' ASSIGN expression ';' {
        $$ = new IArrayStoreStmt(@1.first_line, std::move(*$1), std::unique_ptr<IExpr>($3),
                                 std::unique_ptr<IExpr>($6));
        delete $1;
    }
    | expression ';' { $$ = new IExprStmt($1->line, std::unique_ptr<IExpr>($1)); }
    | block { $$ = $1; }
    | if_statement { $$ = $1; }
    | while_statement { $$ = $1; }

if_statement
    : '?' '(' expression ')' block {
        $$ = new IIfStmt(@1.first_line, std::unique_ptr<IExpr>($3), std::unique_ptr<IBlock>($5), nullptr);
    }
    | '?' '(' expression ')' block ':' block {
        $$ = new IIfStmt(@1.first_line, std::unique_ptr<IExpr>($3), std::unique_ptr<IBlock>($5),
                         std::unique_ptr<IStmt>($7));
    }
    | '?' '(' expression ')' block ':' if_statement {
        $$ = new IIfStmt(@1.first_line, std::unique_ptr<IExpr>($3), std::unique_ptr<IBlock>($5),
                         std::unique_ptr<IStmt>($7));
    }
    ;

while_statement
    : '*' '(' expression ')' block {
        $$ = new IWhileStmt(@1.first_line, std::unique_ptr<IExpr>($3), std::unique_ptr<IBlock>($5));
    }
    ;

expression
    : INT { $$ = new IIntExpr(@1.first_line, $1); }
    | IDENT { $$ = new IVarExpr(@1.first_line, std::move(*$1)); delete $1; }
    | IDENT '(' args ')' {
        $$ = new ICallExpr(@1.first_line, std::move(*$1), std::move(*$3));
        delete $1;
        delete $3;
    }
    | '<' array_elements '>' {
        $$ = new IArrayExpr(@1.first_line, std::move(*$2));
        delete $2;
    }
    | ARR '(' expression ')' { $$ = new IArrayAllocExpr(@1.first_line, std::unique_ptr<IExpr>($3)); }
    | expression '[' expression ']' %prec INDEX {
        $$ = new IIndexExpr(@2.first_line, std::unique_ptr<IExpr>($1), std::unique_ptr<IExpr>($3));
    }
    | '(' expression ')' { $$ = $2; }
    | expression EQ expression { $$ = new IBinaryExpr(@2.first_line, IBinaryOp::Equal, std::unique_ptr<IExpr>($1), std::unique_ptr<IExpr>($3)); }
    | expression NE expression { $$ = new IBinaryExpr(@2.first_line, IBinaryOp::NotEqual, std::unique_ptr<IExpr>($1), std::unique_ptr<IExpr>($3)); }
    | expression '<' expression { $$ = new IBinaryExpr(@2.first_line, IBinaryOp::Less, std::unique_ptr<IExpr>($1), std::unique_ptr<IExpr>($3)); }
    | expression LE expression { $$ = new IBinaryExpr(@2.first_line, IBinaryOp::LessEqual, std::unique_ptr<IExpr>($1), std::unique_ptr<IExpr>($3)); }
    | expression '>' expression { $$ = new IBinaryExpr(@2.first_line, IBinaryOp::Greater, std::unique_ptr<IExpr>($1), std::unique_ptr<IExpr>($3)); }
    | expression GE expression { $$ = new IBinaryExpr(@2.first_line, IBinaryOp::GreaterEqual, std::unique_ptr<IExpr>($1), std::unique_ptr<IExpr>($3)); }
    | expression '+' expression { $$ = new IBinaryExpr(@2.first_line, IBinaryOp::Add, std::unique_ptr<IExpr>($1), std::unique_ptr<IExpr>($3)); }
    | expression '-' expression { $$ = new IBinaryExpr(@2.first_line, IBinaryOp::Subtract, std::unique_ptr<IExpr>($1), std::unique_ptr<IExpr>($3)); }
    | expression '*' expression { $$ = new IBinaryExpr(@2.first_line, IBinaryOp::Multiply, std::unique_ptr<IExpr>($1), std::unique_ptr<IExpr>($3)); }
    | expression '/' expression { $$ = new IBinaryExpr(@2.first_line, IBinaryOp::Divide, std::unique_ptr<IExpr>($1), std::unique_ptr<IExpr>($3)); }
    | expression '%' expression { $$ = new IBinaryExpr(@2.first_line, IBinaryOp::Modulo, std::unique_ptr<IExpr>($1), std::unique_ptr<IExpr>($3)); }
    | expression '&' expression { $$ = new IBinaryExpr(@2.first_line, IBinaryOp::And, std::unique_ptr<IExpr>($1), std::unique_ptr<IExpr>($3)); }
    | expression '|' expression { $$ = new IBinaryExpr(@2.first_line, IBinaryOp::Or, std::unique_ptr<IExpr>($1), std::unique_ptr<IExpr>($3)); }
    | '-' expression %prec UMINUS { $$ = new IUnaryExpr(@1.first_line, IUnaryOp::Negate, std::unique_ptr<IExpr>($2)); }
    | '!' expression %prec UMINUS { $$ = new IUnaryExpr(@1.first_line, IUnaryOp::Not, std::unique_ptr<IExpr>($2)); }
    ;

args
    : %empty { $$ = new std::vector<std::unique_ptr<IExpr>>(); }
    | argument_list { $$ = $1; }
    ;

argument_list
    : expression { $$ = new std::vector<std::unique_ptr<IExpr>>(); $$->emplace_back($1); }
    | argument_list ',' expression { $1->emplace_back($3); $$ = $1; }
    ;

array_elements
    : expression %prec ARRAY_ELEMENT { $$ = new std::vector<std::unique_ptr<IExpr>>(); $$->emplace_back($1); }
    | array_elements ',' expression %prec ARRAY_ELEMENT { $1->emplace_back($3); $$ = $1; }
    ;

params
    : %empty { $$ = new std::vector<IParam>(); }
    | parameter_list { $$ = $1; }
    ;

parameter_list
    : IDENT { $$ = new std::vector<IParam>(); $$->push_back(IParam{std::move(*$1), @1.first_line}); delete $1; }
    | parameter_list ',' IDENT { $1->push_back(IParam{std::move(*$3), @3.first_line}); delete $3; $$ = $1; }
    ;

%%

#include <cstring>
#include <limits>

Interpreter* ip_interpreter = nullptr;

namespace {
const char* typeName(IType type) {
    return type == IType::Array ? "array" : "integer";
}

int32_t fromBits(uint32_t bits) {
    int32_t value;
    static_assert(sizeof(value) == sizeof(bits), "Rastera requires 32-bit integers");
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

int32_t wrappedAdd(int32_t left, int32_t right) {
    return fromBits(static_cast<uint32_t>(left) + static_cast<uint32_t>(right));
}

int32_t wrappedSubtract(int32_t left, int32_t right) {
    return fromBits(static_cast<uint32_t>(left) - static_cast<uint32_t>(right));
}

int32_t wrappedMultiply(int32_t left, int32_t right) {
    return fromBits(static_cast<uint32_t>(left) * static_cast<uint32_t>(right));
}

int32_t wrappedNegate(int32_t value) {
    return fromBits(0U - static_cast<uint32_t>(value));
}
}  // namespace

void Interpreter::error(int line, const std::string& message) {
    std::fprintf(stderr, "%s:%d: error: %s\n", file_.c_str(), line, message.c_str());
    ++errors_;
}

bool Interpreter::check() {
    if (!program) return false;
    for (const auto& function : program->functions) {
        if (function->name == "print") {
            error(function->line, "using built-in name 'print' as a function name");
            continue;
        }
        if (function->name == "arr") error(function->line, "using reserved name 'arr' as a function name");
        if (!functions_.emplace(function->name, function.get()).second)
            error(function->line, "duplicate function '" + function->name + "'");
    }
    const IFunc* main = findFunction("main");
    if (main == nullptr) {
        error(1, "program has no @main() function");
    } else if (!main->params.empty()) {
        error(main->line, "@main() must not have parameters");
    }
    for (const auto& function : program->functions) checkFunction(*function);
    return !hasErrors();
}

bool Interpreter::checkFunction(IFunc& function) {
    types_.emplace_back();
    for (const IParam& param : function.params) {
        if (param.name == "print") {
            error(param.line, "using built-in name 'print' as a parameter name");
        } else if (!types_.back().emplace(param.name, IType::Int).second) {
            error(param.line, "duplicate parameter '" + param.name + "'");
        }
    }
    const bool result = checkBlock(*function.body);
    types_.pop_back();
    return result;
}

bool Interpreter::checkBlock(IBlock& block) {
    types_.emplace_back();
    bool result = true;
    for (const auto& statement : block.statements) result = checkStmt(*statement) && result;
    types_.pop_back();
    return result;
}

bool Interpreter::checkStmt(IStmt& statement) {
    if (auto* return_statement = dynamic_cast<IReturnStmt*>(&statement)) {
        if (checkExpr(*return_statement->value) != IType::Int)
            error(return_statement->line, "return value must be an integer, functions return i32 only");
        return true;
    }
    if (auto* assign = dynamic_cast<IAssignStmt*>(&statement)) {
        if (assign->name == "print") {
            error(assign->line, "using built-in name 'print' as a variable name");
            return false;
        }
        if (assign->name == "arr") {
            error(assign->line, "using reserved name 'arr' as a variable name");
            return false;
        }
        const IType value_type = checkExpr(*assign->value);
        IType* existing = findType(assign->name);
        if (existing != nullptr) {
            if (*existing != value_type)
                error(assign->line, std::string("type mismatch: cannot assign ") + typeName(value_type) +
                                        " to variable '" + assign->name + "' of type " + typeName(*existing));
        } else {
            types_.back().emplace(assign->name, value_type);
        }
        return true;
    }
    if (auto* store = dynamic_cast<IArrayStoreStmt*>(&statement)) {
        IType* array_type = findType(store->name);
        if (array_type == nullptr) {
            error(store->line, "undefined variable '" + store->name + "'");
        } else if (*array_type != IType::Array) {
            error(store->line, "cannot index a non-array value");
        }
        if (checkExpr(*store->index) != IType::Int) error(store->index->line, "array index must be an integer");
        if (checkExpr(*store->value) != IType::Int) error(store->value->line, "array element must be an integer");
        return true;
    }
    if (auto* expression = dynamic_cast<IExprStmt*>(&statement)) {
        checkExpr(*expression->expr);
        return true;
    }
    if (auto* block = dynamic_cast<IBlock*>(&statement)) return checkBlock(*block);
    if (auto* while_statement = dynamic_cast<IWhileStmt*>(&statement)) {
        if (checkExpr(*while_statement->cond) != IType::Int)
            error(while_statement->cond->line, "while condition must be an integer");
        checkBlock(*while_statement->body);
        return true;
    }
    auto* if_statement = dynamic_cast<IIfStmt*>(&statement);
    checkExpr(*if_statement->cond);
    checkBlock(*if_statement->then_branch);
    if (if_statement->else_branch != nullptr) checkStmt(*if_statement->else_branch);
    return true;
}

IType Interpreter::checkExpr(IExpr& expression) {
    if (dynamic_cast<IIntExpr*>(&expression) != nullptr) return expression.type = IType::Int;
    if (auto* variable = dynamic_cast<IVarExpr*>(&expression)) {
        IType* found = findType(variable->name);
        if (found == nullptr) {
            error(variable->line, "undefined variable '" + variable->name + "'");
            return expression.type = IType::Int;
        }
        return expression.type = *found;
    }
    if (auto* call_expression = dynamic_cast<ICallExpr*>(&expression)) {
        for (const auto& argument : call_expression->args) checkExpr(*argument);
        const bool builtin = call_expression->name == "print";
        const IFunc* function = builtin ? nullptr : findFunction(call_expression->name);
        if (!builtin && function == nullptr) {
            error(call_expression->line, "unknown function '" + call_expression->name + "'");
            return expression.type = IType::Int;
        }
        const std::size_t expected = builtin ? 1U : function->params.size();
        if (call_expression->args.size() != expected) {
            error(call_expression->line, "wrong argument count for '" + call_expression->name + "': expected " +
                                            std::to_string(expected) + ", got " +
                                            std::to_string(call_expression->args.size()));
            return expression.type = IType::Int;
        }
        for (const auto& argument : call_expression->args) {
            if (argument->type != IType::Int) {
                error(call_expression->line, std::string(builtin ? "builtin" : "function") + " '" +
                                                call_expression->name + "' expects an integer argument");
                break;
            }
        }
        return expression.type = IType::Int;
    }
    if (auto* array = dynamic_cast<IArrayExpr*>(&expression)) {
        for (const auto& element : array->elements) {
            if (checkExpr(*element) != IType::Int) error(element->line, "array literal elements must be integers");
        }
        return expression.type = IType::Array;
    }
    if (auto* allocation = dynamic_cast<IArrayAllocExpr*>(&expression)) {
        if (checkExpr(*allocation->size) != IType::Int) error(allocation->size->line, "array size must be an integer");
        return expression.type = IType::Array;
    }
    if (auto* index = dynamic_cast<IIndexExpr*>(&expression)) {
        if (checkExpr(*index->array) != IType::Array) error(index->array->line, "cannot index a non-array value");
        if (checkExpr(*index->index) != IType::Int) error(index->index->line, "array index must be an integer");
        return expression.type = IType::Int;
    }
    if (auto* binary = dynamic_cast<IBinaryExpr*>(&expression)) {
        const IType left_type = checkExpr(*binary->lhs);
        const IType right_type = checkExpr(*binary->rhs);
        if ((binary->op == IBinaryOp::And || binary->op == IBinaryOp::Or) &&
            (left_type != IType::Int || right_type != IType::Int))
            error(binary->line, "logical operands must be integers");
        return expression.type = IType::Int;
    }
    auto* unary = dynamic_cast<IUnaryExpr*>(&expression);
    const IType operand_type = checkExpr(*unary->operand);
    if (unary->op == IUnaryOp::Not && operand_type != IType::Int)
        error(unary->line, "logical operand must be an integer");
    return expression.type = IType::Int;
}

IType* Interpreter::findType(const std::string& name) {
    for (auto scope = types_.rbegin(); scope != types_.rend(); ++scope) {
        auto value = scope->find(name);
        if (value != scope->end()) return &value->second;
    }
    return nullptr;
}

Interpreter::Value* Interpreter::findValue(const std::string& name) {
    for (auto scope = values_.rbegin(); scope != values_.rend(); ++scope) {
        auto value = scope->find(name);
        if (value != scope->end()) return &value->second;
    }
    return nullptr;
}

const IFunc* Interpreter::findFunction(const std::string& name) const {
    auto function = functions_.find(name);
    return function == functions_.end() ? nullptr : function->second;
}

int Interpreter::execute() {
    const IFunc* main = findFunction("main");
    return call(*main, {});
}

int32_t Interpreter::call(const IFunc& function, const std::vector<Value>& args) {
    std::vector<ValueScope> caller_scopes = std::move(values_);
    values_.emplace_back();
    for (std::size_t index = 0; index < args.size(); ++index) values_.back().emplace(function.params[index].name, args[index]);
    const ReturnState result = execBlock(*function.body);
    values_ = std::move(caller_scopes);
    return result.returned ? result.value : 0;
}

Interpreter::ReturnState Interpreter::execBlock(IBlock& block) {
    values_.emplace_back();
    ReturnState result;
    for (const auto& statement : block.statements) {
        result = execStmt(*statement);
        if (result.returned) break;
    }
    values_.pop_back();
    return result;
}

Interpreter::ReturnState Interpreter::execStmt(IStmt& statement) {
    if (auto* return_statement = dynamic_cast<IReturnStmt*>(&statement)) return {true, eval(*return_statement->value).integer};
    if (auto* assign = dynamic_cast<IAssignStmt*>(&statement)) {
        Value value = eval(*assign->value);
        Value* existing = findValue(assign->name);
        if (existing == nullptr) values_.back().emplace(assign->name, std::move(value));
        else *existing = std::move(value);
        return {};
    }
    if (auto* store = dynamic_cast<IArrayStoreStmt*>(&statement)) {
        Value* array = findValue(store->name);
        const int32_t index = eval(*store->index).integer;
        const int32_t value = eval(*store->value).integer;
        if (index < 0 || static_cast<std::size_t>(index) >= array->array->size())
            runtimeError("array index out of range");
        (*array->array)[static_cast<std::size_t>(index)] = value;
        return {};
    }
    if (auto* expression = dynamic_cast<IExprStmt*>(&statement)) {
        eval(*expression->expr);
        return {};
    }
    if (auto* block = dynamic_cast<IBlock*>(&statement)) return execBlock(*block);
    if (auto* while_statement = dynamic_cast<IWhileStmt*>(&statement)) {
        while (eval(*while_statement->cond).integer != 0) {
            const ReturnState result = execBlock(*while_statement->body);
            if (result.returned) return result;
        }
        return {};
    }
    auto* if_statement = dynamic_cast<IIfStmt*>(&statement);
    if (eval(*if_statement->cond).integer != 0) return execBlock(*if_statement->then_branch);
    if (if_statement->else_branch != nullptr) return execStmt(*if_statement->else_branch);
    return {};
}

Interpreter::Value Interpreter::eval(IExpr& expression) {
    if (auto* integer = dynamic_cast<IIntExpr*>(&expression)) return {IType::Int, integer->value, nullptr};
    if (auto* variable = dynamic_cast<IVarExpr*>(&expression)) return *findValue(variable->name);
    if (auto* call_expression = dynamic_cast<ICallExpr*>(&expression)) {
        std::vector<Value> args;
        args.reserve(call_expression->args.size());
        for (const auto& argument : call_expression->args) args.push_back(eval(*argument));
        if (call_expression->name == "print") {
            std::printf("%d\n", args.front().integer);
            return {};
        }
        return {IType::Int, call(*findFunction(call_expression->name), args), nullptr};
    }
    if (auto* array = dynamic_cast<IArrayExpr*>(&expression)) {
        auto values = std::make_shared<std::vector<int32_t>>();
        values->reserve(array->elements.size());
        for (const auto& element : array->elements) values->push_back(eval(*element).integer);
        return {IType::Array, 0, std::move(values)};
    }
    if (auto* allocation = dynamic_cast<IArrayAllocExpr*>(&expression)) {
        const int32_t size = eval(*allocation->size).integer;
        if (size < 0) runtimeError("negative array size");
        return {IType::Array, 0, std::make_shared<std::vector<int32_t>>(static_cast<std::size_t>(size), 0)};
    }
    if (auto* index = dynamic_cast<IIndexExpr*>(&expression)) {
        Value array = eval(*index->array);
        const int32_t offset = eval(*index->index).integer;
        if (offset < 0 || static_cast<std::size_t>(offset) >= array.array->size())
            runtimeError("array index out of range");
        return {IType::Int, (*array.array)[static_cast<std::size_t>(offset)], nullptr};
    }
    if (auto* binary = dynamic_cast<IBinaryExpr*>(&expression)) {
        const int32_t left = eval(*binary->lhs).integer;
        if (binary->op == IBinaryOp::And) {
            if (left == 0) return {IType::Int, 0, nullptr};
            return {IType::Int, eval(*binary->rhs).integer != 0 ? 1 : 0, nullptr};
        }
        if (binary->op == IBinaryOp::Or) {
            if (left != 0) return {IType::Int, 1, nullptr};
            return {IType::Int, eval(*binary->rhs).integer != 0 ? 1 : 0, nullptr};
        }
        const int32_t right = eval(*binary->rhs).integer;
        switch (binary->op) {
        case IBinaryOp::Add: return {IType::Int, wrappedAdd(left, right), nullptr};
        case IBinaryOp::Subtract: return {IType::Int, wrappedSubtract(left, right), nullptr};
        case IBinaryOp::Multiply: return {IType::Int, wrappedMultiply(left, right), nullptr};
        case IBinaryOp::Divide:
            if (right == 0) runtimeError("division by zero");
            if (left == std::numeric_limits<int32_t>::min() && right == -1)
                runtimeError("integer division overflow");
            return {IType::Int, fromBits(static_cast<uint32_t>(
                static_cast<int64_t>(left) / static_cast<int64_t>(right))), nullptr};
        case IBinaryOp::Modulo:
            if (right == 0) runtimeError("modulo by zero");
            if (left == std::numeric_limits<int32_t>::min() && right == -1)
                runtimeError("integer modulo overflow");
            return {IType::Int, fromBits(static_cast<uint32_t>(
                static_cast<int64_t>(left) % static_cast<int64_t>(right))), nullptr};
        case IBinaryOp::Equal: return {IType::Int, left == right ? 1 : 0, nullptr};
        case IBinaryOp::NotEqual: return {IType::Int, left != right ? 1 : 0, nullptr};
        case IBinaryOp::Less: return {IType::Int, left < right ? 1 : 0, nullptr};
        case IBinaryOp::LessEqual: return {IType::Int, left <= right ? 1 : 0, nullptr};
        case IBinaryOp::Greater: return {IType::Int, left > right ? 1 : 0, nullptr};
        case IBinaryOp::GreaterEqual: return {IType::Int, left >= right ? 1 : 0, nullptr};
        case IBinaryOp::And:
        case IBinaryOp::Or:
            break;
        }
    }
    auto* unary = dynamic_cast<IUnaryExpr*>(&expression);
    const int32_t operand = eval(*unary->operand).integer;
    if (unary->op == IUnaryOp::Not) return {IType::Int, operand == 0 ? 1 : 0, nullptr};
    return {IType::Int, wrappedNegate(operand), nullptr};
}

[[noreturn]] void Interpreter::runtimeError(const char* message) const {
    std::fflush(stdout);
    std::fprintf(stderr, "rastera: runtime error: %s\n", message);
    std::exit(2);
}

void iperror(Interpreter& interpreter, const char* message) {
    interpreter.error(iplloc.first_line, message);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::fprintf(stderr, "usage: rasteri <file.rst>\n");
        return 1;
    }
    FILE* input = std::fopen(argv[1], "r");
    if (input == nullptr) {
        std::perror(argv[1]);
        return 1;
    }
    Interpreter interpreter(argv[1]);
    ip_interpreter = &interpreter;
    iprestart(input);
    const int parse_result = ipparse(interpreter);
    std::fclose(input);
    if (parse_result != 0 || interpreter.hasErrors() || !interpreter.check()) return 1;
    return interpreter.execute();
}
