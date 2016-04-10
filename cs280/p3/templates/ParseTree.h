/*
 * ParseTree.h
 *
 *  This is a template for a starting point on the class hierarchy for the parse tree
 */

#ifndef PARSETREE_H_
#define PARSETREE_H_

#include <string>
using namespace std;

#include "p2lex.h"
#include "Value.h"

// the root class of everything in the tree is a ParseTree
class ParseTree {
private:
	ParseTree *left;
	ParseTree *right;

public:
	ParseTree(ParseTree *left = 0, ParseTree *right = 0) : left(left), right(right) {}
	virtual ~ParseTree() {}
};

/////// the language has Expressions and Statements
class Expression : public ParseTree {
public:
	Expression(ParseTree *lhs, ParseTree *rhs) : ParseTree(lhs,rhs) {}
};

class Statement : public ParseTree {
public:
	Statement(ParseTree *e = 0) : ParseTree(e) {}
};

///// a StatementList uses a binary tree: the left child is the first statement, and the right child is the rest of the statements
class StatementList : public ParseTree {
public:
	StatementList(ParseTree *statement, ParseTree *list = 0) : ParseTree(statement, list) {}
};

class PrintStatement : public Statement {
public:
	PrintStatement(Expression *expr) : Statement(expr) {}
};

class DeclStatement : public Statement {
private:
	Type		type;
	string		ident;

public:
	DeclStatement(string id, Type type) : Statement(), type(type), ident(id) {}
};

class SConstant : public ParseTree {

};

class IConstant : public ParseTree {

};

class Variable : public ParseTree {
private:
	string	varName;

public:
	Variable(string s) : ParseTree(), varName(s) {}
};

class MultiplyOp : public Expression {

};

class AddOp : public Expression {

};

class SubtractOp : public Expression {

};

class AssignOp : public Expression {
public:
	AssignOp(Expression *lhs, Expression *rhs) : Expression(lhs, rhs) {}
};

class AssignStatement : public Statement {
	AssignStatement(AssignOp *expr) : Statement(expr) {}
};

#endif /* PARSETREE_H_ */
