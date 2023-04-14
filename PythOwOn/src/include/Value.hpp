#ifndef VALUE_HPP
#define VALUE_HPP

#include <vector>
#include <string>

#include "Common.hpp"


enum class ValueType {
    NONE,
    BOOL,
    DOUBLE,
};

struct Value {
    ValueType type;
    union {
        bool boolean;
        double number;
    } as;

    Value operator/(Value other) const {
        return Value::numberVal(as.number / other.as.number);
    }

    Value operator*(Value other) const {
        return Value::numberVal(as.number * other.as.number);
    }

    Value operator+(Value other) const {
        return Value::numberVal(as.number + other.as.number);
    }

    inline static Value boolVal(bool value) {
        Value v {
            ValueType::BOOL,
            { .boolean = value }
        };

        return v;
    }

    inline static Value noneVal() {
        Value v {
            ValueType::NONE,
            { .number = 0 }
        };

        return v;
    }

    inline static Value numberVal(double value) {
        Value v {
            ValueType::DOUBLE,
            { .number = value }
        };

        return v;
    }



    inline bool isBool() {
        return type == ValueType::BOOL;
    }

    inline bool isNone() {
        return type == ValueType::NONE;
    }

    inline bool isNumber() {
        return type == ValueType::DOUBLE;
    }


    inline bool asBool() {
        return as.boolean;
    }

    inline double asNumber() {
        return as.number;
    }

    inline std::string toString() {
        switch (type) {
            case ValueType::BOOL: return as.boolean ? "true" : "false";
            case ValueType::DOUBLE: return std::to_string(as.number);
            case ValueType::NONE: return "none";
        }
    }
};


typedef std::vector<Value> ValueArray;

void printValue(Value value);

template <typename T>
class Stack : private std::vector<T> {
public:
    inline Value peek(int distance) { return this->at(this->size() - 1 - distance); }
    
    inline Value pop() {
        Value value = peek(0);
        this->pop_back();
        return value;
    }
    
    inline void push(T value) { this->emplace_back(value); }

    inline void reset() { this->clear(); }

    std::vector<T>::iterator begin() { return this->std::vector<T>::begin(); }
    std::vector<T>::iterator end() { return this->std::vector<T>::end(); }
};

#endif // !VALUE_HPP
