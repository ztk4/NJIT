#include <iostream>
#include <utility>
#include <string>
#include <map>
#include <cstring>

#include "p2lex.h"

using namespace std;

ostream &operator<<(ostream &os, Token::TokenType tt);

map<Token::TokenType, int> tok_count;
map<string, int> lex_count;

int main(int argc, char **argv) {
    istream *isp = &cin;
    bool verbose = true;
    
    //TODO: command line args
    
    Token t;
    while( (t = getToken(isp)) != Token::ERR && t != Token::DONE) {
        if(verbose)
            cout << t << endl;

        ++tok_count[t.getType()];
        
        if(t.getType() == Token::SCONST || t.getType() == Token::ICONST || t.getType() == Token::VAR)
            ++lex_count[t.getLexeme()];
    }

    if(t == Token::ERR) {
        cerr << t << endl;
    }

    for(pair<Token::TokenType, int> tcount : tok_count)
        cout << (Token::TokenType)tcount.first << ": " << tcount.second << endl;

    for(pair<string, int> lcount : lex_count)
        if(lcount.second > 1)
            cout << lcount.first << ": " << lcount.second << endl;

    return 0;
}
