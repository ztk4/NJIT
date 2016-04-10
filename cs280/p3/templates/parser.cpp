/*
 * parser.cpp
 *
 */

#include "ParseTree.h"

ParseTree *Program(istream *);
ParseTree *StmtList(istream *);
ParseTree *Stmt(istream *);
ParseTree *Aop(istream *);
ParseTree *Expr(istream *);
ParseTree *Term(istream *);
ParseTree *Primary(istream *);

void
printError(string err)
{
	cerr << linenum << ": " << err;
}

ParseTree *
Program(istream *br)
{
	ParseTree *sl = StmtList(br);

	if( sl == 0 ) {
		printError("empty program!");
	}

	return sl;
}

ParseTree *
StmtList(istream *br)
{
	ParseTree *stmt = Stmt(br);
	if( stmt != 0 ) {
		return new StatementList(stmt,
				StmtList(br) );
	}

	return stmt;
}

ParseTree *
Stmt(istream *br)
{
	return 0;
}


