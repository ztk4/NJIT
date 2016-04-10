/*
 * ParseTree.h
 *
 *  This is a template for a starting point on the class hierarchy for the parse tree
 */

#ifndef PARSETREE_H_
#define PARSETREE_H_

#include <string>
using namespace std;

#include "Value.h"
#include "p2lex.h"

// the root class of everything in the tree is a ParseTree
class ParseTree {
private:
	ParseTree *left;
	ParseTree *right;
    int linenum;

public:
    enum PTType { PTREE, EXPR, STMTLIST, STMT, PRINT, DECL, ASSIGN, CONST, SCONST, ICONST, VAR, ADDOP, MULOP, SUBOP, AOP }; //for identifying a parsetree

protected: //children can assign
    PTType ptt;

public:
	ParseTree(ParseTree *left = 0, ParseTree *right = 0) : left(left), right(right), ptt(PTREE) {
        linenum = ::linenum; //captures the externed linenum from p2lex.h
    }
	virtual ~ParseTree() {
        if(left)
            delete left;
        if(right)
            delete right;
    }

    bool operator==(PTType ptt) const {
        return this->ptt == ptt;
    }
    bool operator!=(PTType ptt) const {
        return this->ptt != ptt;
    }

    ParseTree *l() const { return left; }
    ParseTree *r() const { return right; }
    PTType pttype() const { return ptt; }
    int getLine() const { return linenum; }
};

/////// the language has Expressions and Statements
class Expression : public ParseTree {
public:
	Expression(ParseTree *lhs, ParseTree *rhs) : ParseTree(lhs, rhs) {
		this->ptt = EXPR;
	}
};

class Statement : public ParseTree {
public:
	Statement(ParseTree *e = 0) : ParseTree(e) {
		this->ptt = STMT;
	}
};

///// a StatementList uses a binary tree: the left child is the first statement, and the right child is the rest of the statements
class StatementList : public ParseTree {
public:
	StatementList(ParseTree *statement, ParseTree *list = 0) : ParseTree(statement, list) {
		this->ptt = STMTLIST;
	}
};

class PrintStatement : public Statement {
public:
	PrintStatement(ParseTree *expr) : Statement(expr) {
		this->ptt = PRINT;
	}
};

class DeclStatement : public Statement {
private:
	Type		type;
	string		ident;

public:
	DeclStatement(string id, Type type) : Statement(), type(type), ident(id) {
		this->ptt = DECL;
	}
    const string &getIdent() const {
        return ident;
    }
};

class Constant : public ParseTree {
private:
    Value val;

public:
    Constant(Value val) : val(val) {
        this->ptt = CONST;
    }
    const Value &getVal() const {
        return val;
    }
};

class SConstant : public Constant {
public:
    SConstant(string s) : Constant(s) {
		this->ptt = SCONST;
	}
};

class IConstant : public Constant {
public:
    IConstant(int i) : Constant(i) {
		this->ptt = ICONST;
	}
};

class Variable : public ParseTree {
private:
	string	varName;

public:
	Variable(string s) : ParseTree(), varName(s) {
		this->ptt = VAR;
	}
    const string &getName() const {
        return varName;
    }
};

class MultiplyOp : public Expression {
public:
    MultiplyOp(ParseTree *lhs, ParseTree *rhs) : Expression(lhs, rhs) {
		this->ptt = MULOP;
	}
};

class AddOp : public Expression {
public:
    AddOp(ParseTree *lhs, ParseTree *rhs) : Expression(lhs, rhs) {
		this->ptt = ADDOP;
	}
};

class SubtractOp : public Expression {
public:
    SubtractOp(ParseTree *lhs, ParseTree *rhs) : Expression(lhs, rhs) {
		this->ptt = SUBOP;
	}
};

class AssignOp : public Expression {
public:
	AssignOp(ParseTree *lhs, ParseTree *rhs) : Expression(lhs, rhs) {
		this->ptt = AOP;
	}
};

class AssignStatement : public Statement {
public:
	AssignStatement(ParseTree *expr) : Statement(expr) {
		this->ptt = ASSIGN;
	}
};

#endif /* PARSETREE_H_ */
