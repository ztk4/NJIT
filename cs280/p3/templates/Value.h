/*
 * Value.h
 *
 * This class is useful for holding constants and the results of evaluating expressions
 */

#ifndef VALUE_H_
#define VALUE_H_

#include <string>

// create an enum for the type of the object
enum Type { INTEGER, STRING, NONE };

// a Value has a type and a value
class Value {
private:
	Type	type;
	int		iValue;
	std::string	sValue;

public:
	Value(int i) : type(INTEGER), iValue(i) {}
	Value(std::string s) : type(STRING), iValue(0), sValue(s) {}

	bool getvalue(int& reply) {
		if( type == INTEGER ) {
			reply = iValue;
			return true;
		}
		else {
			return false;
		}
	}

	bool getvalue(std::string& reply) {
		if( type == STRING ) {
			reply = sValue;
			return true;
		}
		else {
			return false;
		}
	}
};

#endif /* VALUE_H_ */
