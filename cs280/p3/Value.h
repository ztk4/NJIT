/*
 * Value.h
 *
 * This class is useful for holding constants and the results of evaluating expressions
 */

#ifndef VALUE_H_
#define VALUE_H_

#include <string>

using std::string;

// create an enum for the type of the object
enum Type { INTEGER, STRING, NONE };

// a Value has a type and a value
class Value {
private:
	Type	type;
	int		iValue;
	string	sValue;

public:
	Value(int i) : type(INTEGER), iValue(i) {}
	Value(string s) : type(STRING), iValue(0), sValue(s) {}
    Value(const Value &v) : type(v.type), iValue(v.iValue), sValue(v.sValue) {}

	bool getvalue(int& reply) const {
		if( type == INTEGER ) {
			reply = iValue;
			return true;
		}
		else {
			return false;
		}
	}

	bool getvalue(string& reply) const {
		if( type == STRING ) {
			reply = sValue;
			return true;
		}
		else {
			return false;
		}
	}
   
    //Allows copying of Values
    Value &operator=(const Value &v) {
        type = v.type;
        iValue = v.iValue;
        sValue = v.sValue;

        return *this;
    }

    //Arithmetic operations for Values
    //For ease of use in evaluation
    Value &operator*=(const Value &v) {
        if(type == INTEGER) {
            if(v.type == INTEGER) {
                iValue *= v.iValue;
            } else if(v.type == STRING) {
                sValue = mul(v.sValue, iValue);
                type = STRING;
            } else {
                type = NONE;
            }
        } else if(type == STRING) {
            if(v.type == INTEGER) {
                sValue = mul(sValue, v.iValue);
            } else {
                type = NONE;
            }
        }

        return *this;
    }

    Value &operator+=(const Value &v) {
        if(type == INTEGER && v.type == INTEGER)
            iValue += v.iValue;
        else if(type == STRING && v.type == STRING)
            sValue += v.sValue;
        else
            type = NONE;

        return *this;
    }

    Value &operator-=(const Value &v) {
        if(type == INTEGER && v.type == INTEGER)
            iValue -= v.iValue;
        else
            type = NONE;

        return *this;
    }

    Value &operator*(const Value &v) const {
        return Value(*this) *= v;
    }

    Value &operator+(const Value &v) const {
        return Value(*this) += v;
    }

    Value &operator-(const Value &v) const {
        return Value(*this) -= v;
    }

    //Evaluation to check if Value is valid
    operator bool() const {
        return type != NONE;
    }
    
    //Relational operators to types
    bool operator==(Type t) const {
        return type == t;
    }

    bool operator!=(Type t) const {
        return type != t;
    }

private:
    static string mul(string s, int m) {
        string result;

        for(int i = 0; i < m; ++i)
            result += s;

        return result;
    }

};

#endif /* VALUE_H_ */
