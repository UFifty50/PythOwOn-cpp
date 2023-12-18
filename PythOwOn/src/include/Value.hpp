#ifndef VALUE_HPP
#define VALUE_HPP

#include <cmath>
#include <string>
#include <vector>

#include "Common.hpp"
#include "Object.hpp"


enum class ValueType {
    NONE,
    BOOL,
    INT,
    DOUBLE,
    INFINITY,
    NAN,
    OBJECT,
};

struct Value;
void printValue(Value value);

#if defined(TRACE_EXECUTION) || defined(_DEBUG)
void Debug_printValue(Value value);
#endif


struct Value {
    ValueType type;
    union {
        bool boolean;
        intmax_t integer;
        double decimal;
        Obj* obj;
    } as;

    static Value noneVal() { return {ValueType::NONE, {.obj = 0}}; }

    static Value boolVal(bool value) { return {ValueType::BOOL, {.boolean = value}}; }

    static Value integerVal(intmax_t value) {
        return {ValueType::INT, {.integer = value}};
    }

    static Value doubleVal(double value) {
        return {ValueType::DOUBLE, {.decimal = value}};
    }

    static Value numberVal(double value, bool isDouble) {
        return isDouble ? doubleVal(value) : integerVal((intmax_t)value);
    }

    static Value infinity(bool positive) {
        return {ValueType::INFINITY, {.boolean = positive}};
    }

    static Value nan(bool positive) { return {ValueType::NAN, {.boolean = positive}}; }

    template <typename T>
    static Value objectVal(T object) {
        return {ValueType::OBJECT, {.obj = (Obj*)object}};
    }

    template <>
    static Value objectVal(Value value) {
        if (value.isObject()) return value;
        return {ValueType::OBJECT, {.obj = value.as.obj}};
    }


    static Value asBool(Value value) {
        return value.isFalsey() ? Value::boolVal(false) : Value::boolVal(true);
    }

    static Value asInteger(Value value) {
        if (value.isInteger()) return value;
        if (value.isDouble()) return Value::integerVal((intmax_t)value.as.decimal);
        if (value.isBool()) return Value::integerVal(value.as.boolean ? 1 : 0);
        return Value::noneVal();
    }

    static Value asDouble(Value value) {
        if (value.isDouble()) return value;
        if (value.isInteger()) return Value::doubleVal((double)value.as.integer);
        if (value.isBool()) return Value::doubleVal(value.as.boolean ? 1 : 0);
        return Value::noneVal();
    }

    static Value asNumber(Value value) {
        if (value.isDouble() || value.isInteger()) return value;
        if (value.isBool()) return Value::integerVal(value.as.boolean ? 1 : 0);
        return Value::noneVal();
    }

    static Obj* asObject(Value value) { return value.as.obj; }


    bool isNone() const { return type == ValueType::NONE; }
    bool isBool() const { return type == ValueType::BOOL; }
    bool isInteger() const { return type == ValueType::INT; }
    bool isDouble() const { return type == ValueType::DOUBLE; }
    bool isInf() const { return type == ValueType::INFINITY; }
    bool isNaN() const { return type == ValueType::NAN; }
    bool isNumber() const { return isInteger() || isDouble(); }
    bool isSpecialNumber() const { return isInf() || isNaN(); }
    bool isObject() const { return type == ValueType::OBJECT; }

    bool isObjectType(ObjType type) { return isObject() && Obj::typeOf(as.obj) == type; }

    bool isFalsey() {
        return isNone() || (isBool() && !as.boolean) ||
               (isObjectType(ObjType::STRING) && as.obj->asString()->str == "");
    }

    bool isEqualTo(const Value other) const {
        if ((type == ValueType::DOUBLE && other.type == ValueType::INT) ||
            (type == ValueType::INT && other.type == ValueType::DOUBLE)) {
            return Value::asDouble(*this).as.decimal == Value::asDouble(other).as.decimal;
        }

        if (type != other.type) return false;

        switch (type) {
            case ValueType::NONE:
                return true;
            case ValueType::INFINITY:
            case ValueType::BOOL:
                return as.boolean == other.as.boolean;
            case ValueType::INT:
                return as.integer == other.as.integer;
            case ValueType::DOUBLE:
                return as.decimal == other.as.decimal;
            case ValueType::OBJECT: {
                ObjString* stringA = Value::asObject(*this)->asString();
                ObjString* stringB = Value::asObject(other)->asString();
                return stringA->str == stringB->str;
            }

            default:
                return false;
        }
    }

    static Value addObjects(const Obj* a, const Obj* b) {
        if (Obj::typeOf(a) != Obj::typeOf(b)) return Value::noneVal();

        switch (Obj::typeOf(a)) {
            case ObjType::STRING: {
                ObjString* stringA = a->asString();
                ObjString* stringB = b->asString();
                return Value::objectVal(ObjString::create(stringA->str + stringB->str));
            }

            default:
                return Value::noneVal();
        }
    }


    template <typename A, typename B>
    static Value multiplyObjects(const A a, const B b) {
        if constexpr (std::is_same_v<A, Obj*> && std::is_same_v<B, Value>) {
            if (b.isNumber()) {
                std::string str = "";
                for (size_t i = 0; i < asInteger(b).as.integer; i++)
                    str += a->asString()->str;
                return Value::objectVal(ObjString::create(str));
            }
        } else if constexpr (std::is_same_v<A, Value> && std::is_same_v<B, Obj*>) {
            if (a.isNumber()) {
                std::string str = "";
                for (size_t i = 0; i < asInteger(a).as.integer; i++)
                    str += b->asString()->str;
                return Value::objectVal(ObjString::create(str));
            }
        }

        return Value::noneVal();
    }

    Value operator/(const Value other) const {
        if ((this->isNumber() && asDouble(*this).as.decimal == 0) &&
            (other.isNumber() && asDouble(other).as.decimal == 0))
            return Value::nan(false);

        if (other.isNumber() && asDouble(other).as.decimal == 0)
            return Value::infinity(asDouble(*this).as.decimal > 0);

        return Value::doubleVal(Value::asDouble(*this).as.decimal /
                                Value::asDouble(other).as.decimal);
    }

    Value operator*(const Value other) const {
        if (this->isObject() && other.isNumber()) {
            return multiplyObjects(Value::asObject(*this), Value::asNumber(other));
        } else if (this->isNumber() && other.isObject()) {
            return multiplyObjects(Value::asNumber(*this), Value::asObject(other));
        }

        if (this->type == ValueType::DOUBLE || other.type == ValueType::DOUBLE) {
            return Value::doubleVal(Value::asDouble(*this).as.decimal *
                                    Value::asDouble(other).as.decimal);
        } else {
            return Value::integerVal(this->as.integer * other.as.integer);
        }
    }

    Value operator+(const Value other) const {
        if (this->type == ValueType::OBJECT || other.type == ValueType::OBJECT) {
            return addObjects(Value::asObject(*this), Value::asObject(other));
        } else if (this->type == ValueType::DOUBLE || other.type == ValueType::DOUBLE) {
            return Value::doubleVal(Value::asDouble(*this).as.decimal + other.as.decimal);
        } else {
            return Value::integerVal(Value::asInteger(*this).as.integer +
                                     other.as.integer);
        }
    }

    Value operator%(const Value other) const {
        return Value::doubleVal(
            fmod(Value::asDouble(*this).as.decimal, other.as.decimal));
    }

    inline Value operator==(const Value other) const { Value::boolVal(isEqualTo(other)); }

    inline Value operator!=(const Value other) const {
        Value::boolVal(!isEqualTo(other));
    }

    bool operator>(Value other) const {
        if (this->type == ValueType::DOUBLE || other.type == ValueType::DOUBLE)
            return Value::asDouble(*this).as.decimal > Value::asDouble(other).as.decimal;
        else if (this->type == ValueType::INT && other.type == ValueType::INT)
            return Value::asInteger(*this).as.integer >
                   Value::asInteger(other).as.integer;
        else
            return false;
    }

    bool operator<(Value other) const {
        if (this->type == ValueType::DOUBLE || other.type == ValueType::DOUBLE)
            return Value::asDouble(*this).as.decimal < Value::asDouble(other).as.decimal;

        else if (this->type == ValueType::INT && other.type == ValueType::INT)
            return Value::asInteger(*this).as.integer <
                   Value::asInteger(other).as.integer;
        else
            return false;
    }

    bool operator>=(Value other) const {
        if (this->type == ValueType::DOUBLE || other.type == ValueType::DOUBLE)
            return Value::asDouble(*this).as.decimal >= Value::asDouble(other).as.decimal;
        else if (this->type == ValueType::INT && other.type == ValueType::INT)
            return Value::asInteger(*this).as.integer >=
                   Value::asInteger(other).as.integer;
        else
            return false;
    }

    bool operator<=(Value other) const {
        if (this->type == ValueType::DOUBLE || other.type == ValueType::DOUBLE)
            return Value::asDouble(*this).as.decimal <= Value::asDouble(other).as.decimal;
        else if (this->type == ValueType::INT && other.type == ValueType::INT)
            return Value::asInteger(*this).as.integer <=
                   Value::asInteger(other).as.integer;
        else
            return false;
    }

    Value operator<<(Value other) const {
        if (other.type == ValueType::INT && type == ValueType::INT)
            return Value::integerVal(as.integer << other.as.integer);

        return Value::noneVal();
    }

    Value operator>>(Value other) const {
        if (other.type == ValueType::INT && type == ValueType::INT)
            return Value::integerVal(as.integer >> other.as.integer);

        return Value::noneVal();
    }
};


#endif  // VALUE_HPP
