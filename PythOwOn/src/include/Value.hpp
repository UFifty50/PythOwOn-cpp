#ifndef VALUE_HPP
#define VALUE_HPP

#include <vector>
#include <string>
#include <cmath>

#include "Common.hpp"


enum class ValueType {
    NONE,
    BOOL,
    DOUBLE
};

struct Value {
    ValueType type;
    union {
        bool boolean;
        double number;
    } as;

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

    inline static Value asBool(Value value) {
        return value.isFalsey() ? Value::boolVal(false) : Value::boolVal(true);
    }

    inline static Value asNumber(Value value) {
        if (value.isNumber()) return value;
        if (value.isBool()) return Value::numberVal(value.as.boolean ? 1 : 0);
        return Value::noneVal();
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

    inline bool isFalsey() {
        return isNone() || (isBool() && !as.boolean);
    }

    inline bool isEqualTo(Value other) {
        if (type != other.type) return false;
        switch (type) {
            case ValueType::BOOL: return as.boolean == other.as.boolean;
            case ValueType::DOUBLE: return as.number == other.as.number;
            case ValueType::NONE: return true;
            default: return false;
        }
    }

    std::string toString() {
        switch (type) {
            case ValueType::BOOL: return as.boolean ? "true" : "false";
            case ValueType::DOUBLE: return std::to_string(as.number);
            case ValueType::NONE: return "none";
        }
    }


    inline Value operator/(Value other) const {
        return Value::numberVal(as.number / other.as.number);
    }

    inline Value operator*(Value other) const {
        return Value::numberVal(as.number * other.as.number);
    }

    inline Value operator+(Value other) const {
        return Value::numberVal(as.number + other.as.number);
    }

    Value operator%(Value other) const {
        return Value::numberVal(fmod(as.number, other.as.number));
    }

    inline Value operator==(Value other) const {
        switch (type) {
            case ValueType::DOUBLE: return Value::boolVal(as.number == other.as.number);
            case ValueType::BOOL: return Value::boolVal(as.boolean == other.as.boolean);
            case ValueType::NONE: return Value::boolVal(other.type == ValueType::NONE ? true : false);
        }
    }

    inline Value operator!=(Value other) const {
        switch (type) {
            case ValueType::DOUBLE: return Value::boolVal(as.number != other.as.number);
            case ValueType::BOOL: return Value::boolVal(as.boolean != other.as.boolean);
            case ValueType::NONE: return Value::boolVal(other.type != ValueType::NONE ? true : false);
        }
    }

    inline bool operator>(Value other) const {
        return as.number > other.as.number;
    }
           
    inline bool operator<(Value other) const {
        return as.number < other.as.number;
    }
           
    inline bool operator>=(Value other) const {
        return as.number >= other.as.number;
    }
           
    inline bool operator<=(Value other) const {
        return as.number <= other.as.number;
    }

    inline Value operator<<(Value other) const {
        return Value::numberVal((size_t)as.number << (size_t)other.as.number);
    }

    inline Value operator>>(Value other) const {
        return Value::numberVal((size_t)as.number >> (size_t)other.as.number);
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

    inline size_t size() { return this->std::vector<T>::size(); }
    std::vector<T>::iterator begin() { return this->std::vector<T>::begin(); }
    std::vector<T>::iterator end() { return this->std::vector<T>::end(); }
};

#endif // !VALUE_HPP
