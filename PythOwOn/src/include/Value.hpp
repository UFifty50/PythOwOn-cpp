#ifndef VALUE_HPP
#define VALUE_HPP


#include <cmath>
#include <ostream>
#include <string>
#include <utility>
#include <vector>
#include <string>

#include "Common.hpp"
#include "Object.hpp"
#include <cfloat>


enum class ValueType : uint8_t {
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

    static Value NoneVal() { return {ValueType::NONE, {.obj = nullptr}}; }

    static Value BoolVal(const bool value) { return {ValueType::BOOL, {.boolean = value}}; }

    static Value IntegerVal(const ssize_t value) { return {ValueType::INT, {.integer = value}}; }

    static Value DoubleVal(const double value) { return {ValueType::DOUBLE, {.decimal = value}}; }

    static Value NumberVal(const double value, const bool isDouble) {
        return isDouble ? DoubleVal(value) : IntegerVal(static_cast<ssize_t>(value));
    }

    static Value Infinity(const bool positive) {
        return {ValueType::INFINITY, {.boolean = positive}};
    }

    static Value Nan(const bool positive) { return {ValueType::NAN, {.boolean = positive}}; }

    template <typename T>
    static Value ObjectVal(const T value) {
        return {
            ValueType::OBJECT,
            {.obj = const_cast<Obj*>(reinterpret_cast<const Obj*>(value))}
        };
    }

    static Value ObjectVal(const Value value) {
        if (value.isObject()) return value;
        return {ValueType::OBJECT, {.obj = value.as.obj}};
    }


    static Value AsBool(const Value value) {
        return value.isFalsey() ? BoolVal(false) : BoolVal(true);
    }

    static Value AsInteger(const Value value) {
        if (value.isInteger()) return value;
        if (value.isDouble()) return IntegerVal(static_cast<ssize_t>(value.as.decimal));
        if (value.isBool()) return IntegerVal(value.as.boolean ? 1 : 0);
        return NoneVal();
    }

    static Value AsDouble(const Value value) {
        if (value.isDouble()) return value;
        if (value.isInteger()) return DoubleVal(static_cast<double>(value.as.integer));
        if (value.isBool()) return DoubleVal(value.as.boolean ? 1 : 0);
        return NoneVal();
    }

    static Value AsNumber(const Value value) {
        if (value.isDouble() || value.isInteger()) return value;
        if (value.isBool()) return IntegerVal(value.as.boolean ? 1 : 0);
        return NoneVal();
    }

    static Obj* AsObject(const Value value) { return value.as.obj; }

    static const ObjString* ToObjString(const Value value) {
        switch (value.type) {
            case ValueType::OBJECT: return Obj::TypeOf(value.as.obj) == ObjType::STRING
                                               ? value.as.obj->asString()
                                               : ObjString::Create("<Unprintable Object Type>");
            case ValueType::INT: return ObjString::Create(std::to_string(value.as.integer));
            case ValueType::DOUBLE: return ObjString::Create(std::to_string(value.as.decimal));
            case ValueType::BOOL: return ObjString::Create(value.as.boolean ? "true" : "false");
            case ValueType::INFINITY: return ObjString::Create(value.as.boolean ? "inf" : "-inf");
            case ValueType::NAN: return ObjString::Create(value.as.boolean ? "Nan" : "-Nan");
            case ValueType::NONE: return ObjString::Create("None");
        }

        // Unreachable
        return nullptr;
    }

    static Obj* ToObjStringObj(const Value value) {
        const Value objStringValue = ObjectVal(ToObjString(value));
        return AsObject(objStringValue);
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
        return isObject() && Obj::TypeOf(as.obj) == objectType;
    }

    [[nodiscard]] bool isFalsey() const {
        return isNone() || (isBool() && !as.boolean) ||
            (isObjectType(ObjType::STRING) && as.obj->asString()->str.empty());
    }

    [[nodiscard]] bool isEqualTo(const Value other) const {
        if ((type == ValueType::DOUBLE && other.type == ValueType::INT) ||
            (type == ValueType::INT && other.type == ValueType::DOUBLE)) {
            return ProxEqual<double>(AsDouble(*this).as.decimal, other.as.decimal,
                                     DBL_EPSILON);
        }

        if (type != other.type) return false;

        switch (type) {
            case ValueType::NONE: return true;
            case ValueType::INFINITY:
            case ValueType::BOOL: return as.boolean == other.as.boolean;
            case ValueType::INT: return as.integer == other.as.integer;
            case ValueType::DOUBLE: return ProxEqual<double>(
                    as.decimal, other.as.decimal, DBL_EPSILON);
            case ValueType::OBJECT: return as.obj == other.as.obj;
            case ValueType::NAN: return false;
        }

        // Unreachable
        return false;
    }

    static Value AddObjects(const Obj* a, const Obj* b) {
        if (Obj::TypeOf(a) != Obj::TypeOf(b)) return NoneVal();

        switch (Obj::TypeOf(a)) {
            case ObjType::STRING: return ObjectVal(
                    ObjString::Create(a->asString()->str + b->asString()->str));

            case ObjType::NONE: return NoneVal();
        }

        // Unreachable
        return NoneVal();
    }


    template <typename A, typename B>
    static Value MultiplyObjects(const A a, const B b) {
        if constexpr (std::is_same_v<A, Obj*> && std::is_same_v<B, Value>) {
            if (b.isNumber()) {
                std::string str;
                for (ssize_t i = 0; i < AsInteger(b).as.integer; i
                     ++)
                    str += a->asString()->str;
                return ObjectVal(ObjString::Create(str));
            }
        }
        else if constexpr (std::is_same_v<A, Value> && std::is_same_v<B, Obj*>) {
            if (a.isNumber()) {
                std::string str;
                for (ssize_t i = 0; i < AsInteger(a).as.integer; i
                     ++)
                    str += b->asString()->str;
                return ObjectVal(ObjString::Create(str));
            }
        }

        return NoneVal();
    }

    Value operator/(const Value other) const {
        if (this->isNumber() && ProxEqual<double>(AsDouble(*this).as.decimal, 0) &&
            (other.isNumber() && ProxEqual<double>(AsDouble(other).as.decimal, 0)))
            return Nan(false);

        if (other.isNumber() && ProxEqual<double>(AsDouble(other).as.decimal, 0))
            return
                Infinity(AsDouble(*this).as.decimal > 0);

        return DoubleVal(AsDouble(*this).as.decimal / AsDouble(other).as.decimal);
    }

    Value operator*(const Value other) const {
        if (this->isObject() && other.isNumber())
            return MultiplyObjects(
                AsObject(*this), AsNumber(other));


        if (this->isNumber() && other.isObject())
            return MultiplyObjects(
                AsNumber(*this), AsObject(other));


        if (this->type == ValueType::DOUBLE || other.type == ValueType::DOUBLE)
            return
                DoubleVal(AsDouble(*this).as.decimal * AsDouble(other).as.decimal);


        return IntegerVal(this->as.integer * other.as.integer);
    }

    Value operator+(const Value other) const {
        switch (this->type) {
            case ValueType::OBJECT: switch (other.type) {
                    case ValueType::OBJECT: return AddObjects(AsObject(*this), AsObject(other));

                    case ValueType::INT:
                    case ValueType::DOUBLE:
                    case ValueType::BOOL:
                    case ValueType::INFINITY:
                    case ValueType::NAN:
                    case ValueType::NONE: return AddObjects(AsObject(*this), ToObjStringObj(other));
                }
                break;

            case ValueType::DOUBLE: switch (other.type) {
                    case ValueType::DOUBLE: return DoubleVal(this->as.decimal + other.as.decimal);
                    case ValueType::INT: return DoubleVal(
                            this->as.decimal + AsDouble(other).as.decimal);
                    case ValueType::OBJECT: return AddObjects(
                            ToObjStringObj(*this), AsObject(other));
                    case ValueType::INFINITY: return Infinity(this->as.boolean);
                    case ValueType::NAN: return Nan(this->as.boolean);
                    case ValueType::BOOL: return DoubleVal(this->as.decimal + other.as.boolean);
                    case ValueType::NONE: return NoneVal();
                }
                break;

            case ValueType::INT: switch (other.type) {
                    case ValueType::INT: return IntegerVal(this->as.integer + other.as.integer);
                    case ValueType::DOUBLE: return DoubleVal(
                            AsDouble(*this).as.decimal + other.as.decimal);
                    case ValueType::OBJECT: return AddObjects(
                            ToObjStringObj(*this), AsObject(other));
                    case ValueType::INFINITY: return Infinity(this->as.boolean);
                    case ValueType::NAN: return Nan(this->as.boolean);
                    case ValueType::BOOL: return IntegerVal(this->as.integer + other.as.boolean);
                    case ValueType::NONE: return NoneVal();
                }
                break;

            case ValueType::INFINITY: return Infinity(this->as.boolean);

            case ValueType::NAN: return Nan(this->as.boolean);

            case ValueType::BOOL: switch (other.type) {
                    case ValueType::BOOL: return BoolVal(this->as.boolean || other.as.boolean);
                    case ValueType::INT: return IntegerVal(this->as.boolean + other.as.integer);
                    case ValueType::DOUBLE: return DoubleVal(this->as.boolean + other.as.decimal);
                    case ValueType::OBJECT: return AddObjects(
                            ToObjStringObj(*this), AsObject(other));
                    case ValueType::INFINITY: return Infinity(this->as.boolean);
                    case ValueType::NAN: return Nan(this->as.boolean);
                    case ValueType::NONE: return NoneVal();
                }
                break;

            case ValueType::NONE: return NoneVal();
        }

        // Unreachable
        return NoneVal();
    }

    Value operator%(const Value other) const {
        return DoubleVal(fmod(AsDouble(*this).as.decimal, AsDouble(other).as.decimal));
    }

    Value operator==(const Value other) const { return BoolVal(isEqualTo(other)); }

    Value operator!=(const Value other) const { return BoolVal(!isEqualTo(other)); }

    bool operator>(const Value other) const {
        if (this->type == ValueType::DOUBLE || other.type == ValueType::DOUBLE)
            return
                AsDouble(*this).as.decimal > AsDouble(other).as.decimal;

        if (this->type == ValueType::INT && other.type == ValueType::INT)
            return
                AsInteger(*this).as.integer > AsInteger(other).as.integer;

        return false;
    }

    bool operator<(const Value other) const {
        if (this->type == ValueType::DOUBLE || other.type == ValueType::DOUBLE)
            return
                AsDouble(*this).as.decimal < AsDouble(other).as.decimal;

        if (this->type == ValueType::INT && other.type == ValueType::INT)
            return
                AsInteger(*this).as.integer < AsInteger(other).as.integer;

        return false;
    }

    bool operator>=(const Value other) const {
        if (this->type == ValueType::DOUBLE || other.type == ValueType::DOUBLE)
            return
                AsDouble(*this).as.decimal >= AsDouble(other).as.decimal;

        if (this->type == ValueType::INT && other.type == ValueType::INT)
            return
                AsInteger(*this).as.integer >= AsInteger(other).as.integer;

        return false;
    }

    bool operator<=(const Value other) const {
        if (this->type == ValueType::DOUBLE || other.type == ValueType::DOUBLE)
            return
                AsDouble(*this).as.decimal <= AsDouble(other).as.decimal;

        if (this->type == ValueType::INT && other.type == ValueType::INT)
            return
                AsInteger(*this).as.integer <= AsInteger(other).as.integer;

        return false;
    }

    Value operator<<(const Value other) const {
        if (other.type == ValueType::INT && type == ValueType::INT)
            return IntegerVal(
                as.integer << other.as.integer);

        return NoneVal();
    }

    Value operator>>(const Value other) const {
        if (other.type == ValueType::INT && type == ValueType::INT)
            return IntegerVal(
                as.integer >> other.as.integer);

        return NoneVal();
    }

    std::ostream& operator<<(std::ostream& os) const {
        switch (type) {
            case ValueType::NONE: os << 0 << 0x0;
                break;
            case ValueType::BOOL: os << std::string(as.boolean ? "true" : "false");
                break;
            case ValueType::INT: os << std::to_string(as.integer);
                break;
            case ValueType::DOUBLE: os << std::to_string(as.decimal);
                break;
            case ValueType::INFINITY: os << std::string(as.boolean ? "inf" : "-inf");
                break;
            case ValueType::NAN: os << std::string(as.boolean ? "Nan" : "-Nan");
                break;
            case ValueType::OBJECT: as.obj->operator<<(os);
                break;
        }

        return os;
    }
};


#endif  // VALUE_HPP
