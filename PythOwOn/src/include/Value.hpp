#ifndef VALUE_HPP
#define VALUE_HPP

#include <corecrt_math.h>
#include <float.h>

#include <cmath>
#include <ostream>
#include <string>
#include <utility>
#include <vector>
#include <xstring>

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
        ssize_t integer;
        double decimal;
        Obj* obj;
    } as;

    static Value noneVal() { return {ValueType::NONE, {.obj = nullptr}}; }

    static Value boolVal(const bool value) {
        return {ValueType::BOOL, {.boolean = value}};
    }

    static Value integerVal(const ssize_t value) {
        return {ValueType::INT, {.integer = value}};
    }

    static Value doubleVal(const double value) {
        return {ValueType::DOUBLE, {.decimal = value}};
    }

    static Value numberVal(const double value, const bool isDouble) {
        return isDouble ? doubleVal(value) : integerVal(static_cast<ssize_t>(value));
    }

    static Value infinity(const bool positive) {
        return {ValueType::INFINITY, {.boolean = positive}};
    }

    static Value nan(const bool positive) {
        return {ValueType::NAN, {.boolean = positive}};
    }

    template <typename T>
    static Value objectVal(const T value) {
        return {ValueType::OBJECT,
                {.obj = const_cast<Obj*>(reinterpret_cast<const Obj*>(value))}};
    }

    static Value objectVal(const Value value) {
        if (value.isObject()) return value;
        return {ValueType::OBJECT, {.obj = value.as.obj}};
    }


    static Value asBool(const Value value) {
        return value.isFalsey() ? boolVal(false) : boolVal(true);
    }

    static Value asInteger(const Value value) {
        if (value.isInteger()) return value;
        if (value.isDouble()) return integerVal(static_cast<ssize_t>(value.as.decimal));
        if (value.isBool()) return integerVal(value.as.boolean ? 1 : 0);
        return noneVal();
    }

    static Value asDouble(const Value value) {
        if (value.isDouble()) return value;
        if (value.isInteger()) return doubleVal(static_cast<double>(value.as.integer));
        if (value.isBool()) return doubleVal(value.as.boolean ? 1 : 0);
        return noneVal();
    }

    static Value asNumber(const Value value) {
        if (value.isDouble() || value.isInteger()) return value;
        if (value.isBool()) return integerVal(value.as.boolean ? 1 : 0);
        return noneVal();
    }

    static Obj* asObject(const Value value) { return value.as.obj; }

    static const ObjString* toObjString(const Value value) {
        switch (value.type) {
            case ValueType::OBJECT:
                return Obj::typeOf(value.as.obj) == ObjType::STRING
                           ? value.as.obj->asString()
                           : ObjString::create("<Unprintable Object Type>");
            case ValueType::INT:
                return ObjString::create(std::to_string(value.as.integer));
            case ValueType::DOUBLE:
                return ObjString::create(std::to_string(value.as.decimal));
            case ValueType::BOOL:
                return ObjString::create(value.as.boolean ? "true" : "false");
            case ValueType::INFINITY:
                return ObjString::create(value.as.boolean ? "inf" : "-inf");
            case ValueType::NAN:
                return ObjString::create(value.as.boolean ? "nan" : "-nan");
            case ValueType::NONE:
                return ObjString::create("None");
        }

        // Unreachable
        return nullptr;
    }

    static Obj* toObjStringObj(const Value value) {
        const Value objStringValue = objectVal(toObjString(value));
        return asObject(objStringValue);
    }

    [[nodiscard]] bool isNone() const { return type == ValueType::NONE; }
    [[nodiscard]] bool isBool() const { return type == ValueType::BOOL; }
    [[nodiscard]] bool isInteger() const { return type == ValueType::INT; }
    [[nodiscard]] bool isDouble() const { return type == ValueType::DOUBLE; }
    [[nodiscard]] bool isInf() const { return type == ValueType::INFINITY; }
    [[nodiscard]] bool isNaN() const { return type == ValueType::NAN; }
    [[nodiscard]] bool isNumber() const { return isInteger() || isDouble(); }
    [[nodiscard]] bool isSpecialNumber() const { return isInf() || isNaN(); }
    [[nodiscard]] bool isObject() const { return type == ValueType::OBJECT; }

    [[nodiscard]] bool isObjectType(const ObjType objectType) const {
        return isObject() && Obj::typeOf(as.obj) == objectType;
    }

    [[nodiscard]] bool isFalsey() const {
        return isNone() || (isBool() && !as.boolean) ||
               (isObjectType(ObjType::STRING) && as.obj->asString()->str.empty());
    }

    [[nodiscard]] bool isEqualTo(const Value other) const {
        if ((type == ValueType::DOUBLE && other.type == ValueType::INT) ||
            (type == ValueType::INT && other.type == ValueType::DOUBLE)) {
            return ProxEqual<double>(asDouble(*this).as.decimal, other.as.decimal,
                                     DBL_EPSILON);
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
                return ProxEqual<double>(as.decimal, other.as.decimal, DBL_EPSILON);
            case ValueType::OBJECT:
                return as.obj == other.as.obj;
            case ValueType::NAN:
                return false;
        }

        // Unreachable
        return false;
    }

    static Value addObjects(const Obj* a, const Obj* b) {
        if (Obj::typeOf(a) != Obj::typeOf(b)) return noneVal();

        switch (Obj::typeOf(a)) {
            case ObjType::STRING:
                return objectVal(
                    ObjString::create(a->asString()->str + b->asString()->str));

            case ObjType::NONE:
                return noneVal();
        }

        // Unreachable
        return noneVal();
    }


    template <typename A, typename B>
    static Value multiplyObjects(const A a, const B b) {
        if constexpr (std::is_same_v<A, Obj*> && std::is_same_v<B, Value>) {
            if (b.isNumber()) {
                std::string str;
                for (ssize_t i = 0; i < asInteger(b).as.integer; i++)
                    str += a->asString()->str;
                return objectVal(ObjString::create(str));
            }
        } else if constexpr (std::is_same_v<A, Value> && std::is_same_v<B, Obj*>) {
            if (a.isNumber()) {
                std::string str;
                for (ssize_t i = 0; i < asInteger(a).as.integer; i++)
                    str += b->asString()->str;
                return objectVal(ObjString::create(str));
            }
        }

        return noneVal();
    }

    Value operator/(const Value other) const {
        if (this->isNumber() && ProxEqual<double>(asDouble(*this).as.decimal, 0) &&
            (other.isNumber() && ProxEqual<double>(asDouble(other).as.decimal, 0)))
            return nan(false);

        if (other.isNumber() && ProxEqual<double>(asDouble(other).as.decimal, 0))
            return infinity(asDouble(*this).as.decimal > 0);

        return doubleVal(asDouble(*this).as.decimal / asDouble(other).as.decimal);
    }

    Value operator*(const Value other) const {
        if (this->isObject() && other.isNumber())
            return multiplyObjects(asObject(*this), asNumber(other));


        if (this->isNumber() && other.isObject())
            return multiplyObjects(asNumber(*this), asObject(other));


        if (this->type == ValueType::DOUBLE || other.type == ValueType::DOUBLE)
            return doubleVal(asDouble(*this).as.decimal * asDouble(other).as.decimal);


        return integerVal(this->as.integer * other.as.integer);
    }

    Value operator+(const Value other) const {
        switch (this->type) {
            case ValueType::OBJECT:
                switch (other.type) {
                    case ValueType::OBJECT:
                        return addObjects(asObject(*this), asObject(other));

                    case ValueType::INT:
                    case ValueType::DOUBLE:
                    case ValueType::BOOL:
                    case ValueType::INFINITY:
                    case ValueType::NAN:
                    case ValueType::NONE:
                        return addObjects(asObject(*this), toObjStringObj(other));
                }
                break;

            case ValueType::DOUBLE:
                switch (other.type) {
                    case ValueType::DOUBLE:
                        return doubleVal(this->as.decimal + other.as.decimal);
                    case ValueType::INT:
                        return doubleVal(this->as.decimal + asDouble(other).as.decimal);
                    case ValueType::OBJECT:
                        return addObjects(toObjStringObj(*this), asObject(other));
                    case ValueType::INFINITY:
                        return infinity(this->as.boolean);
                    case ValueType::NAN:
                        return nan(this->as.boolean);
                    case ValueType::BOOL:
                        return doubleVal(this->as.decimal + other.as.boolean);
                    case ValueType::NONE:
                        return noneVal();
                }
                break;

            case ValueType::INT:
                switch (other.type) {
                    case ValueType::INT:
                        return integerVal(this->as.integer + other.as.integer);
                    case ValueType::DOUBLE:
                        return doubleVal(asDouble(*this).as.decimal + other.as.decimal);
                    case ValueType::OBJECT:
                        return addObjects(toObjStringObj(*this), asObject(other));
                    case ValueType::INFINITY:
                        return infinity(this->as.boolean);
                    case ValueType::NAN:
                        return nan(this->as.boolean);
                    case ValueType::BOOL:
                        return integerVal(this->as.integer + other.as.boolean);
                    case ValueType::NONE:
                        return noneVal();
                }
                break;

            case ValueType::INFINITY:
                return infinity(this->as.boolean);

            case ValueType::NAN:
                return nan(this->as.boolean);

            case ValueType::BOOL:
                switch (other.type) {
                    case ValueType::BOOL:
                        return boolVal(this->as.boolean || other.as.boolean);
                    case ValueType::INT:
                        return integerVal(this->as.boolean + other.as.integer);
                    case ValueType::DOUBLE:
                        return doubleVal(this->as.boolean + other.as.decimal);
                    case ValueType::OBJECT:
                        return addObjects(toObjStringObj(*this), asObject(other));
                    case ValueType::INFINITY:
                        return infinity(this->as.boolean);
                    case ValueType::NAN:
                        return nan(this->as.boolean);
                    case ValueType::NONE:
                        return noneVal();
                }
                break;

            case ValueType::NONE:
                return noneVal();
        }

        // Unreachable
        return noneVal();
    }

    Value operator%(const Value other) const {
        return doubleVal(fmod(asDouble(*this).as.decimal, asDouble(other).as.decimal));
    }

    Value operator==(const Value other) const { return boolVal(isEqualTo(other)); }

    Value operator!=(const Value other) const { return boolVal(!isEqualTo(other)); }

    bool operator>(const Value other) const {
        if (this->type == ValueType::DOUBLE || other.type == ValueType::DOUBLE)
            return asDouble(*this).as.decimal > asDouble(other).as.decimal;
        else if (this->type == ValueType::INT && other.type == ValueType::INT)
            return asInteger(*this).as.integer > asInteger(other).as.integer;
        else
            return false;
    }

    bool operator<(const Value other) const {
        if (this->type == ValueType::DOUBLE || other.type == ValueType::DOUBLE)
            return asDouble(*this).as.decimal < asDouble(other).as.decimal;

        else if (this->type == ValueType::INT && other.type == ValueType::INT)
            return asInteger(*this).as.integer < asInteger(other).as.integer;
        else
            return false;
    }

    bool operator>=(const Value other) const {
        if (this->type == ValueType::DOUBLE || other.type == ValueType::DOUBLE)
            return asDouble(*this).as.decimal >= asDouble(other).as.decimal;
        else if (this->type == ValueType::INT && other.type == ValueType::INT)
            return asInteger(*this).as.integer >= asInteger(other).as.integer;
        else
            return false;
    }

    bool operator<=(const Value other) const {
        if (this->type == ValueType::DOUBLE || other.type == ValueType::DOUBLE)
            return asDouble(*this).as.decimal <= asDouble(other).as.decimal;
        else if (this->type == ValueType::INT && other.type == ValueType::INT)
            return asInteger(*this).as.integer <= asInteger(other).as.integer;
        else
            return false;
    }

    Value operator<<(const Value other) const {
        if (other.type == ValueType::INT && type == ValueType::INT)
            return integerVal(as.integer << other.as.integer);

        return noneVal();
    }

    Value operator>>(const Value other) const {
        if (other.type == ValueType::INT && type == ValueType::INT)
            return integerVal(as.integer >> other.as.integer);

        return noneVal();
    }

    std::ostream& operator<<(std::ostream& os) const {
        switch (type) {
            case ValueType::NONE:
                os << 0 << 0x0;
                break;
            case ValueType::BOOL:
                os << std::string(as.boolean ? "true" : "false");
                break;
            case ValueType::INT:
                os << std::to_string(as.integer);
                break;
            case ValueType::DOUBLE:
                os << std::to_string(as.decimal);
                break;
            case ValueType::INFINITY:
                os << std::string(as.boolean ? "inf" : "-inf");
                break;
            case ValueType::NAN:
                os << std::string(as.boolean ? "nan" : "-nan");
                break;
            case ValueType::OBJECT:
                as.obj->operator<<(os);
                break;
        }

        return os;
    }
};


#endif  // VALUE_HPP
