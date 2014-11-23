%{
#include "Type.h"
#include "Monicelli.tab.h"
#include <stdlib.h>

extern int lineNumber;
extern void yyerror(const char *);
%}

%option noyywrap

DIGIT [0-9]
HEXDIGIT [0-9a-zA-Z]
CHAR  [a-zA-Z_]

%%
("\r\n"|"\n") {
    lineNumber += 1;
    return NEWLINE;
}

"bituma"[^\n]* {}

"Lei ha clacsonato" {
    return MAIN;
}
"vaffanzum" {
    return RETURN;
}
"Necchi" {
    yylval.typeval = TYPENAME_INT;
    return TYPENAME;
}
"Mascetti" {
    yylval.typeval = TYPENAME_CHAR;
    return TYPENAME;
}
"Perozzi" {
    yylval.typeval = TYPENAME_FLOAT;
    return TYPENAME;
}
"Melandri" {
    yylval.typeval = TYPENAME_BOOL;
    return TYPENAME;
}
"Sassaroli" {
    yylval.typeval = TYPENAME_DOUBLE;
    return TYPENAME;
}
"conte" {
    return STAR;
}
"voglio" {
    return VARDECL;
}
"come fosse" {
    return ASSIGN;
}
("il"|"lo"|"la"|"i"|"gli"|"le"|"un"|"una"|"dei") {
    return ARTICLE;
}
"più" {
    return OP_PLUS;
}
"meno" {
    return OP_MINUS;
}
"per" {
    return OP_TIMES;
}
"diviso" {
    return OP_DIV;
}
"sinistra" {
    return OP_SHL;
}
"destra" {
    return OP_SHR;
}
"minore di" {
    return OP_LT;
}
"maggiore di" {
    return OP_GT;
}
"minore o uguale a" {
    return OP_LTE;
}
"maggiore o uguale a" {
    return OP_GTE;
}
"a posterdati" {
    return PRINT;
}
"mi porga" {
    return INPUT;
}
"ho visto la" {
    return ASSERT_BEGIN;
}
"!" {
    return ASSERT_END;
}
"stuzzica" {
    return LOOP_BEGIN;
}
"e brematura anche, se" {
    return LOOP_CONDITION;
}
"che cos'è" {
    return BRANCH_CONDITION;
}
"?" {
    return BRANCH_BEGIN;
}
"tarapia tapioco" {
    return BRANCH_ELSE;
}
"e velocità di esecuzione" {
    return BRANCH_END;
}
":" {
    return COLON;
}
"blinda la supercazzola" {
    return FUNDECL;
}
"con" {
    return PARAMS;
}
"," {
    return COMMA;
}
"{" {
    return LCURL;
}
"}" {
    return RCURL;
}
"brematurata la supercazzola" {
    return FUNCALL;
}
"avvertite don ulrico" {
    return ABORT;
}
"..." {
    return CASE_END;
}

[ \t\f\v] {}

{CHAR}({DIGIT}|{CHAR})* {
    yylval.strval = strdup(yytext);
    return ID;
} 

{DIGIT}+ {
    yylval.intval = strtol(yytext, NULL, 10);
    return NUMBER;
}

. {
    yyerror("Unexpected token\n");
    return -1;
};

%%

