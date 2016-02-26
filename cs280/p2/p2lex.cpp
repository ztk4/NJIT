#include "p2lex.h"

using namespace std;

Token::Token(const Token &t) : ttype(t.ttype), tlexeme(t.tlexeme) {}

const Token &Token::operator=(const Token &t) {
    this->ttype = t.ttype;
    this->tlexeme = t.tlexeme;
    return *this;
}

// Order of enum:       VAR,   SCONST,   ICONST,   PLUSOP,   MINUSOP,   STAROP,   EQOP,   PRINTKW,   INTKW,   STRKW,   LPAREN,   RPAREN,   SC,   ERR,   DONE (VAR=0, SCONST=1, ICONST=2, ...)
const char *toStr[] = {"var", "sconst", "iconst", "plusop", "minusop", "starop", "eqop", "printkw", "intkw", "strkw", "lparen", "rparen", "sc", "err", "done"}; //toStr[enum] == string representation

ostream &operator<<(ostream &os, Token::TokenType tt) {
    return os << toStr[tt];
}

ostream &operator<<(ostream &os, const Token &t) {
    os << t.ttype; //print lexeme type
    return  (t.ttype == Token::VAR || t.ttype == Token::SCONST || t.ttype == Token::ICONST || t.ttype == Token::ERR) ? //if VAR, SCONST, ICONST, or ERR 
            (os << '(' << t.tlexeme << ')') : os; //then print str value in parens (and return stream), else just return stream
}
