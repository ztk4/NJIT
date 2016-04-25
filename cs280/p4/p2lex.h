/*
 * p2lex.h
 *
 *  CS280 Spring 2016 Project 2
 */

#ifndef P2LEX_H_
#define P2LEX_H_

#include <iostream>
#include <string>
using namespace std;;

class Token {
public:
    // defines all of the known token types
    enum TokenType {
        VAR,        // an identifier

        SCONST,     // a string enclosed in ""
        ICONST,     // an integer constant

                    // four operators
        PLUSOP,     // the + operator
        MINUSOP,    // the - operator
        STAROP,     // the * operator
        EQOP,       // the assignment op

                    // three keywords
        PRINTKW,    // print
        INTKW,      // int
        STRKW,      // string

        LPAREN,     // left parenthesis
        RPAREN,     // right parenthesis

        SC,         // the semicolon

        ERR,        // some error condition was reached
        DONE,       // end of file
    };

private:
    TokenType    ttype;        // the token type for this instance
    string        tlexeme;    // the corresponding lexeme

public:
    // constructor has default values
    Token(TokenType t = ERR, string l = "") {
        ttype = t;
        tlexeme = l;
    }

    // copy constructor
    Token(const Token&);

    // getters
    const TokenType getType() const { return ttype; }
    const string getLexeme() const { return tlexeme; }

    // equality/inequality test against a token type;
    bool operator==(const Token::TokenType tt) const { return ttype == tt; }
    bool operator!=(const Token::TokenType tt) const { return ttype != tt; }

    // assignment operator
    const Token& operator=(const Token& rhs);

    // for printing out using <<
    friend ostream& operator<<(ostream& out, const Token& t);
};

extern Token getToken(istream* br);
extern bool pushbackToken(Token& t);

extern int linenum;

#endif /* P2LEX_H_ */
