// Copyright 2017 the Monicelli project authors. All rights reserved.
// Use of this source code is governed by a GPLv3 license, see LICENSE.txt.

%%{

machine Lexer;

access state_.;

accent_a = "a`"|"à";
accent_e = "e`"|"è";
accent_u = "u`"|"ù";
accent_vowels = "à"|"è"|"é"|"ì"|"ò"|"ó"|"ù"| [aeiou] "`";
articles = "il"|"lo"|"la"|"l'"|"i"|"gli"|"le"|"un"|"un'"|"una"|"dei"|"delle";
di = "di"|"dei"|"del"|"della"|"dell'";
prematura = [bp] "rematura";
supercazzola = "supercazzo" [lr] "a";

comment = "#"|"bituma";
identifier = (alpha | accent_vowels) (alnum | accent_vowels)*;
integer = [+\-]? digit+;
float = [+\-]? (digit* "." digit+ | digit+ "."?) [eE] [+\-]? digit+
        | [+\-]? (digit* "." digit+ | digit+ ".");

shift := |*
  "sinistra" => { SET_TOKEN(OP_SHL); fbreak; };
  "destra" => { SET_TOKEN(OP_SHR); fbreak; };
  "per" => { advanceColumn(); fgoto initial; };
  (space - '\n')+ => { advanceColumn(); fbreak; };
*|;

initial := |*
  "!" => { SET_TOKEN(BANG); fbreak; };
  "?" => { SET_TOKEN(BRANCH_BEGIN); fbreak; };
  ":" => { SET_TOKEN(COLON); fbreak; };
  "," => { SET_TOKEN(COMMA); fbreak; };
  "conte" => { SET_TOKEN(STAR); fbreak; };
  articles => { SET_TOKEN(ARTICLE); fbreak; };

  "pi" accent_u => { SET_TOKEN(OP_PLUS); fbreak; };
  "meno" => { SET_TOKEN(OP_MINUS); fbreak; };
  "per" => { SET_TOKEN(OP_TIMES); fbreak; };
  "diviso" => { SET_TOKEN(OP_DIV); fbreak; };
  "con scappellamento a" => { advanceColumn(); fgoto shift; };
  "minore " di => { SET_TOKEN(OP_LT); fbreak; };
  "maggiore " di => { SET_TOKEN(OP_GT); fbreak; };
  "minore " "o "? "uguale " ("a"|di) => { SET_TOKEN(OP_LE); fbreak; };
  "maggiore " "o "? "uguale " ("a"|di) => { SET_TOKEN(OP_GE); fbreak; };

  "vaffanzum" => { SET_TOKEN(RETURN); fbreak; };
  "voglio" => { SET_TOKEN(VARDECL); fbreak; };
  "come " ("se "?) "fosse" => { SET_TOKEN(ASSIGN); fbreak; };
  "a posterdati" => { SET_TOKEN(PRINT); fbreak; };
  "mi porga" => { SET_TOKEN(INPUT); fbreak; };
  "ho visto" => { SET_TOKEN(ASSERT); fbreak; };
  "avvertite don ulrico" => { SET_TOKEN(ABORT); fbreak; };

  "stuzzica" => { SET_TOKEN(LOOP_BEGIN); fbreak; };
  "e " prematura " anche, se" => { SET_TOKEN(LOOP_CONDITION); fbreak; };

  "che cos'" accent_e => { SET_TOKEN(BRANCH_CONDITION); fbreak; };
  "o tarapia tapioco" => { SET_TOKEN(BRANCH_ELSE); fbreak; };
  "e velocit" accent_a " di esecuzione" => { SET_TOKEN(BRANCH_END); fbreak; };
  "o magari" => { SET_TOKEN(CASE_END); fbreak; };

  "Lei ha clacsonato" => { SET_TOKEN(ENTRY_POINT); fbreak; };
  "blinda la " supercazzola => { SET_TOKEN(FUN_DECL); fbreak; };
  "con" => { SET_TOKEN(FUN_PARAMS); fbreak; };
  prematura "ta la " supercazzola => { SET_TOKEN(FUN_CALL); fbreak; };
  "o scherziamo" ("?"?) => { SET_TOKEN(FUN_END); fbreak; };

  "Necchi" => {
    SET_TOKEN(TYPENAME);
    token->setBuiltinTypeValue(Token::BUILTIN_TYPE_INTEGER);
    fbreak;
  };

  "Mascetti" => {
    SET_TOKEN(TYPENAME);
    token->setBuiltinTypeValue(Token::BUILTIN_TYPE_CHAR);
    fbreak;
  };

  "Perozzi" => {
    SET_TOKEN(TYPENAME);
    token->setBuiltinTypeValue(Token::BUILTIN_TYPE_FLOAT);
    fbreak;
  };

  "Melandri" => {
    SET_TOKEN(TYPENAME);
    token->setBuiltinTypeValue(Token::BUILTIN_TYPE_BOOL);
    fbreak;
  };

  "Sassaroli" => {
    SET_TOKEN(TYPENAME);
    token->setBuiltinTypeValue(Token::BUILTIN_TYPE_DOUBLE);
    fbreak;
  };

  identifier => {
    SET_TOKEN(IDENTIFIER);
    token->setStringValue({state_.ts, state_.te});
    fbreak;
  };

  float => {
    SET_TOKEN(FLOAT);
    token->setFloatValue(std::stod(std::string{state_.ts, state_.te}));
    fbreak;
  };

  integer => {
    SET_TOKEN(INTEGER);
    token->setIntValue(std::stoll(std::string{state_.ts, state_.te}));
    fbreak;
  };

  comment (^"\n")* | (space - '\n')+ => {
    advanceColumn();
    starting_location = current_location_;
  };

  '\n' => {
    newLine();
    starting_location = current_location_;
  };
*|;

}%%

#include "lexer.h"

#include <string>

namespace monicelli {

#define SET_TOKEN(NAME) \
  do { \
    advanceColumn(); \
    auto end_location = current_location_; \
    token.reset(new Token{Token::TOKEN_##NAME,\
                          starting_location, end_location}); \
  } while (false)

%% write data nofinal;

void Lexer::resetState() {
  %% write init;
}

std::unique_ptr<Token> Lexer::getNextToken() {
  if (buffer_.isExhausted()) advanceBuffer();

  char* p = buffer_.getCursor();
  char* pe = buffer_.getDataEnd();
  char* eof = input_? nullptr : pe;

  Location starting_location = current_location_;
  std::unique_ptr<Token> token;

  while (p != pe && !token) {
    %% write exec noend;
    if (state_.cs == %%{ write error; }%%) {
      token.reset(new Token{Token::TOKEN_UNKNOWN, starting_location});
    }
  }

  if (p == eof) {
    token.reset(new Token{Token::TOKEN_END, starting_location});
  }

  state_.ts = nullptr;
  buffer_.setCursor(p);
  if (trace_enabled_) token->print(std::cout);
  return token;
}

#undef SET_TOKEN

} // namespace
