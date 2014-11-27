%{
#include "Monicelli.tab.h"
#include <stdlib.h>
#include <stdio.h>

extern int lineNumber;

void monicelli_error(const char *);
void monicelli_meta(const char *); // Extern void serve?
%}

%option noyywrap

DIGIT [0-9]
HEXDIGIT [0-9a-zA-Z]
CHAR  [a-zA-Z_]

%x shift

%%
<INITIAL,shift>("\r\n"|"\n") {
    lineNumber += 1;
}

"#"[^\n]* {
    monicelli_meta(yytext + 1);
}

"bituma"[^\n]* {}

"Lei ha clacsonato" {
    return MAIN;
}
"vaffanzum" {
    return RETURN;
}
"Necchi" {
    monicelli_lval.typeval = TYPENAME_INT;
    return TYPENAME;
}
"Mascetti" {
    monicelli_lval.typeval = TYPENAME_CHAR;
    return TYPENAME;
}
"Perozzi" {
    monicelli_lval.typeval = TYPENAME_FLOAT;
    return TYPENAME;
}
"Melandri" {
    monicelli_lval.typeval = TYPENAME_BOOL;
    return TYPENAME;
}
"Sassaroli" {
    monicelli_lval.typeval = TYPENAME_DOUBLE;
    return TYPENAME;
}
"conte" {
    return STAR;
}
"voglio" {
    return VARDECL;
}
"come "("se ")?"fosse" {
    return ASSIGN;
}
("il"|"lo"|"la"|"l'"|"i"|"gli"|"le"|"un"|"un'"|"una"|"dei") {
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
"con scappellamento a" {
    BEGIN(shift);
}
<shift>"per" {
    BEGIN(INITIAL);
}
<shift>"sinistra" {
    return OP_SHL;
}
<shift>"destra" {
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
"ho visto" {
    return ASSERT;
}
"!" {
    return BANG;
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
"o tarapia tapioco" {
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
"brematurata la supercazzola" {
    return FUNCALL;
}
"o scherziamo"("?")? {
    return FUN_END;
}
"avvertite don ulrico" {
    return ABORT;
}
"o magari" {
    return CASE_END;
}

<INITIAL,shift>[ \t\f\v] {}

{CHAR}({DIGIT}|{CHAR})* {
    monicelli_lval.strval = strdup(yytext);
    return ID;
} 

{DIGIT}+ {
    monicelli_lval.intval = strtol(yytext, NULL, 10);
    return NUMBER;
}

<INITIAL,shift>. {
    monicelli_error("Unexpected token");
    return -1;
};

%%

