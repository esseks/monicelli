%{
#define YYERROR_VERBOSE

extern void yyerror(const char *);
extern int yylex();

#include "Nodes.hpp"

using namespace monicelli;

extern Program *program;
%}

%code requires {
#include "Nodes.hpp"
}

%define api.prefix {monicelli_}

%token MAIN
%token RETURN
%token ARTICLE TYPENAME STAR
%token VARDECL ASSIGN
%token PRINT INPUT
%token ASSERT BANG
%token LOOP_BEGIN LOOP_CONDITION
%token BRANCH_CONDITION BRANCH_BEGIN BRANCH_ELSE BRANCH_END CASE_END
%token COLON COMMA
%token FUNDECL PARAMS FUNCALL FUN_END
%token ABORT
%token ID NUMBER FLOAT

%left OP_LT OP_GT OP_LTE OP_GTE
%left OP_PLUS OP_MINUS
%left OP_TIMES OP_DIV
%left OP_SHL OP_SHR

%nonassoc LOWER_THAN_ELSE
%nonassoc BRANCH_ELSE

%{
#include <stack>

static std::stack<StatementList*> stmtStack;
static std::stack<ExpressionList*> argsStack;
static std::stack<IdList*> paramsStack;
static std::stack<BranchCaseList*> branchCaseStack;
%}

%union {
    int intval;
    double floatval;
    char *strval;
    bool boolval;
    monicelli::Type typeval;
    monicelli::Statement* statementval;
    monicelli::StatementList* statlistval;
    monicelli::Assert* assertval;
    monicelli::FunctionCall* callval;
    monicelli::Print* printval;
    monicelli::Input* inputval;
    monicelli::Abort* abortval;
    monicelli::Branch* branchval;
    monicelli::VarDeclaration* declval;
    monicelli::Assignment* assignval;
    monicelli::Loop* loopval;
    monicelli::BranchCase *caseval;
    monicelli::Return* returnval;
    monicelli::Expression* expressionval;
    monicelli::SemiExpression *semiexpval;
    monicelli::Id* idval;
    monicelli::Number* numericval;
    monicelli::Function* funval;
    monicelli::Main* mainval;
}

%type<intval> NUMBER
%type<floatval> FLOAT
%type<strval> ID
%type<typeval> TYPENAME

%type<statementval> statement
// This is not nice :\ used to carry the else body
%type<statlistval> branch_body
%type<assertval> assert_stmt
%type<callval> fun_call
%type<funval> fun_decl
%type<printval> print_stmt
%type<inputval> input_stmt
%type<abortval> abort_stmt
%type<branchval> branch_stmt
%type<caseval> case_stmt
%type<declval> var_decl
%type<assignval> assign_stmt
%type<loopval> loop_stmt
%type<returnval> return_stmt
%type<expressionval> expression maybe_expression simple_expression var_init
%type<semiexpval> semi_expression
%type<idval> variable
%type<numericval> numeric
%type<mainval> main
%type<boolval> pointer

%start program

%%

program:
    /* epsilon */
    | fun_decls main fun_decls {
        program->setMain($2);
    }
;
fun_decls:
    /* epsilon */
    | fun_decl {
        program->addFunction($1);
    }
    fun_decls
;
fun_decl:
    FUNDECL ID {
        paramsStack.push(new IdList());
    } args FUN_END {
        stmtStack.push(new StatementList());
    }
    statements {
        $$ = new Function(new Id($2), paramsStack.top(), stmtStack.top());
        paramsStack.pop();
        stmtStack.pop();
    }
;
args:
    /* epsilon */ | PARAMS arglist
;
arglist:
    variable {
        paramsStack.top()->push_back($1); 
    }
    | variable {
        paramsStack.top()->push_back($1); 
    }
    arglist
;
main:
    MAIN {
        stmtStack.push(new StatementList());
    }
    statements {
        $$ = new Main(stmtStack.top());
        stmtStack.pop();
    }
;
statements:
    /* epsilon */ {
    }
    | statement {
        if ($1 != nullptr) {
            stmtStack.top()->push_back($1);
        }
    }
    statements
;
statement:
    assert_stmt   { $$ = $1; }
    | fun_call    { $$ = $1; }
    | print_stmt  { $$ = $1; }
    | input_stmt  { $$ = $1; }
    | abort_stmt  { $$ = $1; }
    | branch_stmt { $$ = $1; }
    | var_decl    { $$ = $1; }
    | assign_stmt { $$ = $1; }
    | loop_stmt   { $$ = $1; }
    | return_stmt { $$ = $1; }
    | COMMA       { $$ = nullptr; }
;
var_decl:
    VARDECL variable COMMA pointer TYPENAME var_init {
        $$ = new VarDeclaration($2, $5, $4, $6);
    }
;
pointer:
    /* epsilon */ { $$ = false; } | STAR { $$ = true; }
;
var_init:
    /* epsilon */ { $$ = nullptr; } | ASSIGN expression { $$ = $2; }
;
numeric:
    NUMBER { $$ = new Integer($1); } | FLOAT { $$ = new Float($1); }
;
variable:
    ID { $$ = new Id($1); } | ARTICLE ID { $$ = new Id($2); }
;
assign_stmt:
    variable ASSIGN expression {
        $$ = new Assignment($1, $3);
    }
;
print_stmt:
    expression PRINT {
        $$ = new Print($1);
    }
;
input_stmt:
    INPUT variable {
        $$ = new Input($2);
    }
;
return_stmt:
    RETURN maybe_expression BANG {
        $$ = new Return($2);
    }
;
maybe_expression:
    expression { $$ = $1; } | /* epsilon */ { $$ = nullptr; }
;
loop_stmt:
    LOOP_BEGIN {
        stmtStack.push(new StatementList());
    }
    statements LOOP_CONDITION expression {
        $$ = new Loop(stmtStack.top(), $5);
        stmtStack.pop();
    }
;
branch_stmt:
    BRANCH_CONDITION variable BRANCH_BEGIN {
        branchCaseStack.push(new BranchCaseList());
    }
    branch_body BRANCH_END {
        $$ = new Branch($2, branchCaseStack.top(), $5);
        branchCaseStack.pop();
    }
;
branch_body:
    cases %prec LOWER_THAN_ELSE {
        $$ = nullptr;
    }
    | cases BRANCH_ELSE COLON {
        stmtStack.push(new StatementList());
    }
    statements {
        $$ = stmtStack.top();
        stmtStack.pop();
    }
;
cases:
    case_stmt | case_stmt CASE_END cases
;
case_stmt:
    semi_expression COLON {
        stmtStack.push(new StatementList());
    } statements {
        branchCaseStack.top()->push_back(new BranchCase($1, stmtStack.top()));
        stmtStack.pop();
    }
;
fun_call:
    FUNCALL {
        argsStack.push(new ExpressionList());
    }
    ID call_args FUN_END {
        $$ = new FunctionCall(new Id($3), argsStack.top());
        argsStack.pop();
    }
;
call_args:
    /* epsilon */ | PARAMS call_arglist
;
call_arglist:
    expression { argsStack.top()->push_back($1); } 
    | expression { argsStack.top()->push_back($1); } COMMA call_arglist
;
abort_stmt:
    ABORT {
        $$ = new Abort();
    }
;
assert_stmt:
    ASSERT expression BANG {
        $$ = new Assert($2);
    }
;
expression:
    simple_expression {
        $$ = $1;
    }
    | expression OP_LT expression {
        $$ = new ExpLt($1, $3);
    }
    | expression OP_GT expression {
        $$ = new ExpGt($1, $3);
    }
    | expression OP_LTE expression {
        $$ = new ExpLte($1, $3);
    }
    | expression OP_GTE expression {
        $$ = new ExpGte($1, $3);
    }
    | expression OP_PLUS expression {
        $$ = new ExpPlus($1, $3);
    }
    | expression OP_MINUS expression {
        $$ = new ExpMinus($1, $3);
    }
    | expression OP_TIMES expression {
        $$ = new ExpTimes($1, $3);
    }
    | expression OP_DIV expression {
        $$ = new ExpDiv($1, $3);
    }
    | expression OP_SHL expression {
        $$ = new ExpShl($1, $3);
    }
    | expression OP_SHR expression {
        $$ = new ExpShr($1, $3);
    }
;
semi_expression:
    expression {
        $$ = new SemiExpEq($1);
    }
    | OP_LT expression {
        $$ = new SemiExpLt($2);
    }
    | OP_GT expression {
        $$ = new SemiExpGt($2);
    }
    | OP_LTE expression {
        $$ = new SemiExpLte($2);
    }
    | OP_GTE expression {
        $$ = new SemiExpGte($2);
    }
    | OP_PLUS expression {
        $$ = new SemiExpPlus($2);
    }
    | OP_MINUS expression {
        $$ = new SemiExpMinus($2);
    }
    | OP_TIMES expression {
        $$ = new SemiExpTimes($2);
    }
    | OP_DIV expression {
        $$ = new SemiExpDiv($2);
    }
    | OP_SHL expression {
        $$ = new SemiExpShl($2);
    }
    | OP_SHR expression {
        $$ = new SemiExpShr($2);
    }
;
simple_expression:
    fun_call { $$ = $1; }
    | numeric { $$ = $1; }
    | variable { $$ = $1; }
;
%%

