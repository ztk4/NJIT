/*
 * parser.cpp
 *
 */

#include "parser.h"
#include "p2lex.h"

ParseTree *Program(istream *);      //::= StmtList
ParseTree *StmtList(istream *);     //::= Stmt [StmtList]
ParseTree *Stmt(istream *);         //::= PRINTKW Aop SC | INTKW VAR SC | STRKW VAR SC | Aop SC
//NOTE: Expr could evaluate to VAR... so if Expr is VAR, then check for EQOP to determine branch!
ParseTree *Aop(istream *);          //::= VAR EQOP Aop | Expr 
ParseTree *Expr(istream *);         //::= Term { (+|-) Expr }
ParseTree *Term(istream *);         //::= Primary { * Term }
ParseTree *Primary(istream *);      //::= SCONST | ICONST | VAR | LPAREN Aop RPAREN

ParseTree *getProgram(istream *isp) {
    return Program(isp);
}

static bool err = false;

//works kinda like assert, prints msg if !cond (returns cond)
//also sets err var
static inline bool check(bool cond, string msg) {
    if(!cond) {
        cerr << linenum << ": " << msg << endl;
        err = true;
    }
    return cond;
}

ParseTree *Program(istream *isp) {
	ParseTree *sl = StmtList(isp);
    check(sl, "empty program!");

	return err ? 0 : sl;
}

ParseTree *StmtList(istream *isp) {
	ParseTree *stmt = Stmt(isp);
	if(stmt) {
		return new StatementList(stmt, StmtList(isp));
	}

	return stmt;
}

ParseTree *Stmt(istream *isp) {
    Token t = getToken(isp);
    if(t == Token::PRINTKW) {

        ParseTree *aop = Aop(isp);
        if(     check(aop, "expected assignment or expression after print keyword") &&
                check(getToken(isp) == Token::SC, "Expecting a semicolon to end statement")) {
            
            return new PrintStatement(aop);
        }

    } else if (t == Token::INTKW || t == Token::STRKW) {
        Type dtype = (t == Token::INTKW ? INTEGER : STRING);

        t = getToken(isp);
        if(     check(t == Token::VAR, "expected variable name in variable declaration") &&
                check(getToken(isp) == Token::SC, "Expecting a semicolon to end statement")) {
            
            return new DeclStatement(t.getLexeme(), dtype);
        }

    } else {
        pushbackToken(t);
        ParseTree *aop = Aop(isp);
        
        if(aop && check(getToken(isp) == Token::SC, "Expecting a semicolon to end statement")) {
            return new AssignStatement(aop);
        }

    }

	return 0;
}

ParseTree *Aop(istream *isp) {
    ParseTree *expr = Expr(isp);

    if(expr && *expr == ParseTree::VAR) { //could be an assignment

        Token t = getToken(isp);
        if(t == Token::EQOP) {
            ParseTree *aop = Aop(isp);
            if(check(aop, "expected rvalue in assignment")) {
                return new AssignOp(expr, aop);
            }
        } else {
            pushbackToken(t);
        }

    }

    return expr;
}

ParseTree *Expr(istream *isp) {
    ParseTree *left = Term(isp);
    if(!left)
        return left;

    for(;;) {
        Token t = getToken(isp);
        if(t == Token::PLUSOP || t == Token::MINUSOP) {
            ParseTree *right = Term(isp);
            if(check(right, "expected term after operator")) {
                if(t == Token::PLUSOP) {
                    left = new AddOp(left, right);
                } else {
                    left = new SubtractOp(left, right);
                }
            } else {
                return right; //error
            }
        } else {
            pushbackToken(t);
            return left;
        }
    }

}

ParseTree *Term(istream *isp) {
    ParseTree *left = Primary(isp);
    if(!left)
        return left;

    for(;;) {
        Token t = getToken(isp);
        if(t == Token::STAROP) {
            ParseTree *right = Primary(isp);
            if(check(right, "expected primary after operator")) {
                left = new MultiplyOp(left, right);
            } else {
                return right; //error
            }
        } else {
            pushbackToken(t);
            return left;
        }
    }

}
ParseTree *Primary(istream *isp) {
    Token t = getToken(isp);

    if(t == Token::SCONST) {
        string s = t.getLexeme();
        return new SConstant(s.substr(1, s.size() - 2)); //trim quotes from sconst
    } else if (t == Token::ICONST) {
        return new IConstant(stoi(t.getLexeme(), NULL, 10));
    } else if (t == Token::VAR) {
        return new Variable(t.getLexeme());
    } else if (t == Token::LPAREN) {
        ParseTree *aop = Aop(isp);

        if(     check(aop, "expected expression after opening parenthesis") &&
                check(getToken(isp) == Token::RPAREN, "expected closing parenthesis")) {

            return aop;
        }
    }

    return 0;
}
