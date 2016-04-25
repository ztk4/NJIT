#include <iostream>

#include "Value.h"

std::ostream &operator<<(std::ostream &o, const Value &v) {
    if(v.type == NONE) {
        return o << "None";
    } else if(v.type == STRING) {
        return o << v.sValue;
    } else {
        return o << v.iValue;
    }
}
