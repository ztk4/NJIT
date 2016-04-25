/*
 * ParseTree.h
 *
 *  This is a template for a starting point on the class hierarchy for the parse tree
 */

#ifndef PARSETREE_H_
#define PARSETREE_H_

#include <string>
#include <map>

#include "Value.h"
#include "p2lex.h"

using namespace std;

typedef map<string, Type> TypeMap;
typedef map<string, Value> ValMap;

extern TypeMap types;
extern ValMap values;

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

    virtual bool isValid(Type left = NONE, Type right = NONE) { return true; } //checks for validity
    virtual Type evalType(Type left = NONE, Type right = NONE) { return NONE; } //evaluates Type
    virtual const Value eval(const Value &left = Value(), const Value &right = Value()) { return Value(); } //evaluates Value
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

    const Value eval(const Value &l, const Value &r = Value()) {
        cout << l << endl;
        return Value();
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
    bool isValid(Type l = NONE, Type r = NONE) {
        return !types.count(ident); //valid declaration if ident has not been declared before
    }

    Type evalType(Type l = NONE, Type r = NONE) {
        types[ident] = type; //register ident as type
        return type;
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
    const Value eval(const Value &l = Value(), const Value &r = Value()) {
        return getVal();
    }
};

class SConstant : public Constant {
public:
    SConstant(string s) : Constant(s) {
		this->ptt = SCONST;
	}

    Type evalType(Type l = NONE, Type r = NONE) {
        return STRING;
    }
};

class IConstant : public Constant {
public:
    IConstant(int i) : Constant(i) {
		this->ptt = ICONST;
	}
    
    Type evalType(Type l = NONE, Type r = NONE) {
        return INTEGER;
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
    bool isValid(Type l = NONE, Type r = NONE) {
        return types.count(varName); //valid if type has been established, i.e. defined already
    }

    Type evalType(Type l = NONE, Type r = NONE) {
        return types[varName];
    }
    const Value eval(const Value &l = Value(), const Value &r = Value()) {
        return values[varName];
    }
};

class MultiplyOp : public Expression {
public:
    MultiplyOp(ParseTree *lhs, ParseTree *rhs) : Expression(lhs, rhs) {
		this->ptt = MULOP;
	}
    bool isValid(Type l, Type r) {
        return l == INTEGER && (r == STRING || r == INTEGER);
    }

    //Assumes valid types
    Type evalType(Type l, Type r) {
        return r;
    }
    const Value eval(const Value &l, const Value &r) {
        return l * r;
    }
};

class AddOp : public Expression {
public:
    AddOp(ParseTree *lhs, ParseTree *rhs) : Expression(lhs, rhs) {
		this->ptt = ADDOP;
	}
    bool isValid(Type l, Type r) {
        return l == r && (l == INTEGER || l == STRING);
    }
    
    //Assumes valid types
    Type evalType(Type l, Type r) {
        return l;
    }
    const Value eval(const Value &l, const Value &r) {
        return l + r;
    }
};

class SubtractOp : public Expression {
public:
    SubtractOp(ParseTree *lhs, ParseTree *rhs) : Expression(lhs, rhs) {
		this->ptt = SUBOP;
	}
    bool isValid(Type l, Type r) {
        return l == r && l == INTEGER;
    }

    //Assumes valid types
    Type evalType(Type l, Type r) {
        return INTEGER;
    }
    const Value eval(const Value &l, const Value &r) {
        return l - r;
    }
};

class AssignOp : public Expression {
public:
	AssignOp(ParseTree *lhs, ParseTree *rhs) : Expression(lhs, rhs) {
		this->ptt = AOP;
	}
    bool isValid(Type l, Type r) {
        return l == r;
    }

    //Assumes valid types
    Type evalType(Type l, Type r) {
        return l;
    }
    const Value eval(const Value &l, const Value &r) {
        values[dynamic_cast<Variable *>(this->l())->getName()] = r; //get name from left child
        return r;
    }
};

class AssignStatement : public Statement {
public:
	AssignStatement(ParseTree *expr) : Statement(expr) {
		this->ptt = ASSIGN;
	}
    bool isValid(Type l, Type r = NONE) {
        return l != NONE;
    }

    //Assumes valid types
    Type evalType(Type l, Type r = NONE) {
        return l;
    }
};

#endif /* PARSETREE_H_ */
