/*
 * getToken.cpp
 *
 *  Created on: Feb 12, 2016
 *      Author: gerardryan
 */

#include <cctype>
#include "p2lex.h"

Token
id_or_kw(const string& lexeme)
{
	Token::TokenType tt = Token::VAR;
	if( lexeme == "print" )
		tt = Token::PRINTKW;
	else if( lexeme == "int" )
		tt = Token::INTKW;
	else if( lexeme == "string" )
		tt = Token::STRKW;

	return Token(tt, lexeme);
}

static Token pushedBackToken;
bool isPushedBack = false;

bool
pushBackToken(Token& t)
{
	if( isPushedBack )
		return false;

	pushedBackToken = t;
	isPushedBack = true;
	return true;
}

static Token realGetToken(istream *);

Token
getToken(istream* br)
{
	if( isPushedBack ) {
		isPushedBack = false;
		return pushedBackToken;
	}

	return realGetToken(br);
}


static Token
realGetToken(istream* br)
{
	enum LexState { BEGIN, INID, INSTRING, ININT, ONESLASH, INCOMMENT } lexstate = BEGIN;
	string lexeme;

	for(;;) {
		int ch = br->get();
		if( br->bad() || br->eof() ) break;

		if( ch == '\n' )
			linenum++;

		switch( lexstate ) {
		case BEGIN:
			if( isspace(ch) )
				continue;

			lexeme = ch;

			if( isalpha(ch) ) {
				lexstate = INID;
			}
			else if( ch == '"' ) {
				lexstate = INSTRING;
			}
			else if( isdigit(ch) ) {
				lexstate = ININT;
			}
			else switch( ch ) {
			case '+':
				return Token::PLUSOP;
			case '-':
				return Token::MINUSOP;
			case '*':
				return Token::STAROP;
			case '/':
				lexstate = ONESLASH;
				break;
			case '=':
				return Token::EQOP;
			case '(':
				return Token::LPAREN;
			case ')':
				return Token::RPAREN;
			case ';':
				return Token::SC;
			default:
				return Token::ERR;
			}
			break;

		case INID:
			if( isalpha(ch) ) {
				lexeme += ch;
			}
			else if( isdigit(ch) ) {
				lexeme += ch;
				return Token(Token::ERR, lexeme);
			}
			else {
				br->putback(ch);
				return id_or_kw(lexeme);
			}
			break;

		case INSTRING:
			lexeme += ch;
			if( ch == '\n' ) {
				return Token(Token::ERR, lexeme );
			}
			if( ch == '"' ) {
				return Token(Token::SCONST, lexeme );
			}
			break;

		case ININT:
			if( isdigit(ch) ) {
				lexeme += ch;
			}
			else if( isalpha(ch) ) {
				lexeme += ch;
				return Token(Token::ERR, lexeme);
			}
			else {
				br->putback(ch);
				return Token(Token::ICONST, lexeme);
			}
			break;

		case ONESLASH:
			if( ch != '/' ) {
				lexeme += ch;
				return Token(Token::ERR, lexeme );
			}
			lexstate = INCOMMENT;
			break;

		case INCOMMENT:
			if( ch == '\n' ) {
				lexstate = BEGIN;
			}
			break;
		}

	}
	if( br->bad() ) return Token::ERR;
	if( br->eof() ) return Token::DONE;
	return Token();
}


