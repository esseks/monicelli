#ifndef SCANNER_HPP
#define SCANNER_HPP

#ifndef yyFlexLexerOnce
#include <FlexLexer.h>
#endif

#include "Parser.hpp"

namespace monicelli {

class Scanner: public yyFlexLexer {
public:
    Scanner(std::istream &in): yyFlexLexer(&in), lval(nullptr) {}

    int yylex(Parser::semantic_type *lval) {
        this->lval = lval;
        return yylex();
    }

private:
    int yylex();
    Parser::semantic_type *lval;
};

} // monicelli

#endif
