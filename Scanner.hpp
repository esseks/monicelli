#ifndef SCANNER_HPP
#define SCANNER_HPP

#ifndef yyFlexLexerOnce
#include <FlexLexer.h>
#endif

#include "Parser.hpp"

namespace monicelli {

class Scanner: public yyFlexLexer {
public:
    Scanner(std::istream &in): yyFlexLexer(&in) {}

    int yylex(Parser::semantic_type *lval, Parser::location_type *loc) {
        this->lval = lval;
        location = loc;
        return yylex();
    }

private:
    int yylex();
    Parser::semantic_type *lval;
    Parser::location_type *location;
};

} // monicelli

#endif
