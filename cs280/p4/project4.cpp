#define NDEBUG

#include <functional>
#include <iostream>
#include <fstream>
#include <string>

#include "ParseTree.h"
#include "Value.h"
#include "parser.h"
#include "p2lex.h"

using namespace std;

//extern defs for ParseTree.h
TypeMap types;
ValMap values;

template<class T>
T post(ParseTree *pt, function<T(ParseTree *, T &, T &)> func) {
    if(pt) {
        T l = post(pt->l(), func);
        T r = post(pt->r(), func);
        
        return func(pt, l, r);
    }

    return T();
}

bool err = false;

int main(int argc, char **argv) {
    istream *isp = &cin;
    ifstream *ifsp = NULL;

    if(argc > 2) {
        cerr << "Usage: " << *argv << " [infile]" << endl;
        return 1;
    }
    if(argc == 2) {
        ifsp = new ifstream(argv[1]);
        if(!*ifsp) {
            cerr << "Error opening file '" << argv[1] << '\'' << endl;
            return 2;
        }

        isp = ifsp;
    }

    ParseTree *prgm = getProgram(isp);
    
    if(prgm) {
        bool err = false;

        post<Type>(prgm, [&err](ParseTree *pt, Type l, Type r) { //lambda that enforces rules and checks types
            bool valid = pt->isValid(l, r);
            string s;

            switch(pt->pttype()) {
            case ParseTree::VAR:
                if(!valid) { //invalid variable means undeclared
                    cerr << "Variable " << dynamic_cast<Variable *>(pt)->getName() << " used but not defined at line " << pt->getLine() << endl;
                    err = true;
                }
                break;

            case ParseTree::SCONST:
                dynamic_cast<SConstant *>(pt)->getVal().getvalue(s); //get value of SConst
                if(!s.size()) {
                    cerr << "Null string at line " << pt->getLine() << endl;
                    err = true;
                }
                break;
            
            case ParseTree::ADDOP:
                if(!valid) {
                    cerr << "Type mismatch on operands for addition at line " << pt->getLine() << endl;
                    err = true;
                }
                break;

            case ParseTree::SUBOP:
                if(!valid) {
                    cerr << "Type mismatch on operands for subtraction at line " << pt->getLine() << endl;
                    err = true;
                }
                break;

            case ParseTree::MULOP:
                if(!valid) {
                    cerr << "Type mismatch on operands for multiplication at line " << pt->getLine() << endl;
                    err = true;
                }
                break;

            case ParseTree::AOP:
                if(!valid) {
                    cerr << "Type mismatch on operands for assignment at line " << pt->getLine() << endl;
                    err = true;
                }
                break;

            case ParseTree::DECL:
                if(!valid) { //invalid declaration means duplicate variable
                    cerr << "Variable declared twice at line " << pt->getLine() << endl;
                    err = true;
                }
                break;
            }
            
            return pt->evalType(l, r);
        });

        if(!err) {
            post<const Value>(prgm, [](ParseTree *pt, const Value &l, const Value &r) -> const Value { return pt->eval(l, r); });
        }
    }



    if(ifsp) {
        ifsp->close();
        delete ifsp;
    }

    return 0;
}
