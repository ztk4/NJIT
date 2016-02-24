#include <limits>
#include <map>
#include <cctype>

#include "p2lex.h"

using namespace std;

map<string, Token::TokenType> kw = { {"print", Token::PRINTKW}, {"int", Token::INTKW}, {"string", Token::STRKW} }; //keywords

int linenum = 1;

Token getToken(istream *isp) {
    string lexeme;
    char c;
    
space_scan:
    while(isspace(c = isp->get())) {  //keep reading characters until non-space
        if(c == '\n')
            ++linenum;
    }
    
    if(c == '/') {
        if( (c = isp->get())  == '/') { //process comment
            isp->ignore(numeric_limits<streamsize>::max(), '\n');
            ++linenum;
            goto space_scan; //go back to look for whitespace on line after comment
        } else
            return Token(Token::ERR, "/" + c); // '/' followed by anything else is an error
    }
    
    lexeme = c;
    switch(c) {
    case EOF:
        return Token(Token::DONE);
    case '+':
        return Token(Token::PLUSOP, lexeme);
    case '-':
        return Token(Token::MINUSOP, lexeme);
    case '*':
        return Token(Token::STAROP, lexeme);
    case '=':
        return Token(Token::EQOP, lexeme);
    case ';':
        return Token(Token::SC, lexeme);
    case '(':
        return Token(Token::LPAREN, lexeme);
    case ')':
        return Token(Token::RPAREN, lexeme); 
    
    case '"': //String Constants
        while( (c = isp->get()) != '"') {
            if(c == '\n' || c == EOF)
                return Token(Token::ERR, lexeme + c); //include newline in ERR token (EOF is a ctrl char so doesn't matter)

            lexeme += c;
        }

        return Token(Token::SCONST, lexeme + c); //include closing '"' in SCONST token
    
    default:
        if(isdigit(c)) { //Integer Constants

            while(isdigit(c = isp->get()))
                lexeme += c;

            if(isalpha(c))
                return Token(Token::ERR, lexeme + c); //include invalid letter in ERR token

            isp->putback(c); //could be putting back EOF (will deal with in next token)

            return Token(Token::ICONST, lexeme);

        } else if(isalpha(c)) { //Identifiers/Keyword
            
            while(isalpha(c = isp->get()))
                lexeme += c;

            if(isdigit(c))
                return Token(Token::ERR, lexeme + c); //inlcude invalid digit in ERR token

            isp->putback(c); //could be putting back EOF (will deal with in next token)

            return Token( (kw.count(lexeme) ? kw[lexeme] : Token::VAR), lexeme); //if lexeme in kw map, type is mapped value, else just a var

        } else { //unknown/invalid character
            return Token(Token::ERR, lexeme);
        }
    }
}
