#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cstring>
#include <cstdio>

#include "p2lex.h"

using namespace std;

//Token::TokenType print as integers in this file unless I specify that this method is defined elsewhere
//can't add this to p2lex.h and a new header file just for this line is overkill
ostream &operator<<(ostream &os, Token::TokenType tt);

map<Token::TokenType, int> tok_count;
map<string, int> lex_count;

const char *usage = "Usage: %s [[-v] infile]\n";

int main(int argc, char **argv) {
    istream *isp = &cin;        //input stream pointer to be used for program
    ifstream *ifsp = NULL;      //file stream pointer that may be used. Will be freed
    bool verbose = false;       //verbose option
    bool printed;               //used in output for title lines
    
    char **arg = argv + 1; //first arg after prgrm name
    switch(argc) {
    //process first of TWO arguments
    case 3:
        if(!strcmp(*arg, "-v")) {
            verbose = true;
            ++arg; //advance to next arg
        } else {
            fprintf(stderr, "'%s' was not recognized as a verbose flag.\n", *arg);
            fprintf(stderr, usage, *argv);
            return 1;
        }
    //process second of TWO arguments or first of ONE argument
    case 2:
        ifsp = new ifstream(*arg);
        if(*ifsp) {
            isp = ifsp; //set in stream pointer to file's stream
        } else {
            fprintf(stderr, "Error openning file '%s'\n", *arg);
            return 2;
        }
    //do nothing
    case 1:
        break;

    default:
        cerr << "No more than two arguments may be supplied" << endl;
        fprintf(stderr, usage, *argv);
        return 3;
    }
    
    Token t;
    while( (t = getToken(isp)) != Token::ERR && t != Token::DONE) { //while not done and no error
        if(verbose)
            cout << t << endl;                                      //print token

        ++tok_count[t.getType()];                                   //incr count
        
        if(t.getType() == Token::SCONST || t.getType() == Token::ICONST || t.getType() == Token::VAR)
            ++lex_count[t.getLexeme()];                             //keep track of repetitions
    }

    if(t == Token::ERR) {
        cerr << t << endl;
        return 4;
    }

    printed = false;
    for(auto tcount : tok_count) {
        if(!printed) {
            cout << "Token counts:" << endl;
            printed = true;
        }
        cout << tcount.first << ':' << tcount.second << endl;
    }

    printed = false;
    for(auto lcount : lex_count) {
        if(lcount.second > 1) { 
            if(!printed) {
                cout << "Lexemes that appear more than once:" << endl;
                printed = true;
            }
            cout << lcount.first << " (" << lcount.second << " times)" << endl;
        }
    }

    if(ifsp) {
        ifsp->close();
        delete ifsp; //if ifsp was allocated, free it
    }

    return 0;
}
