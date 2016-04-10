#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>

#include "ParseTree.h"
#include "Value.h"
#include "parser.h"
#include "p2lex.h"

using namespace std;

void post(ParseTree *pt, function<void(ParseTree *)> func) {
    if(pt) {
        post(pt->l(), func);
        post(pt->r(), func);

        func(pt);
    }
}

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
        enum Counters { VAR, STR, INT, ADDOP, SUBOP, MULOP, AOP };
        int count[AOP + 1] = {0}; //0 init
        set<string> vars;
        stringstream errs; //saves errors so they can be printed AFTER counts
        
        post(prgm, [&](ParseTree *pt) { //lambda that counts in count arr and prints errors (captures by reference)
                string s;

                switch(pt->pttype()) {
                case ParseTree::VAR:
                    s = dynamic_cast<Variable *>(pt)->getName();                        //get name of var
                    if(!vars.count(s)) {                                                //s not in var set
                        errs << "Variable " << s << " used but not defined at line " << pt->getLine() << endl;
                    }

                    ++count[VAR];
                    break;

                case ParseTree::SCONST:
                    dynamic_cast<SConstant *>(pt)->getVal().getvalue(s); //get value of SConst
                    if(!s.size()) {
                        errs << "Null string at line " << pt->getLine() << endl;
                    }

                    ++count[STR];
                    break;

                case ParseTree::ICONST:
                    ++count[INT];
                    break;

                case ParseTree::ADDOP:
                    ++count[ADDOP];
                    break;

                case ParseTree::SUBOP:
                    ++count[SUBOP];
                    break;

                case ParseTree::MULOP:
                    ++count[MULOP];
                    break;

                case ParseTree::AOP:
                    ++count[AOP];
                    break;

                case ParseTree::DECL:
                    s = dynamic_cast<DeclStatement *>(pt)->getIdent();  //get ident
                    vars.insert(s);                                     //insert ident into var set
                    break;
                }

                });

        cout << count[INT] << " ICONST, " << count[STR] << " SCONST, " << count[VAR] << " VARIABLES" << endl;
        
        cout << "Number of add operators: " << count[ADDOP] << endl;
        cout << "Number of subtract operators: " << count[SUBOP] << endl;
        cout << "Number of multiply operators: " << count[MULOP] << endl;
        cout << "Number of assignment operators: " << count[AOP] << endl;

        cerr << errs.str();
    }



    if(ifsp) {
        ifsp->close();
        delete ifsp;
    }

    return 0;
}
