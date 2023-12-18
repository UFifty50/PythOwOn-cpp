#include "Value.hpp"

#include "Common.hpp"


void printObject(Value value) {
    switch (Obj::typeOf(value.as.obj)) {
        case ObjType::STRING:
            ObjString* string = (ObjString*)value.as.obj;
            FMT_PRINT("\"{}\"", string->str);
            break;
    }
}

// clang-format off
void printValue(Value value) {
    switch (value.type) {
        case ValueType::NONE: FMT_PRINT("None"); break;
        case ValueType::BOOL: FMT_PRINT("{}", value.as.boolean); break;
        case ValueType::DOUBLE: FMT_PRINT("{}", value.as.decimal); break;
        case ValueType::INT: FMT_PRINT("{}", value.as.integer); break;
        case ValueType::INFINITY: FMT_PRINT("{}", value.as.boolean ? "inf" : "-inf"); break;
        case ValueType::NAN: FMT_PRINT("{}", value.as.boolean ? "nan" : "-nan"); break;
        case ValueType::OBJECT: printObject(value); break;
    }
}

#if defined(TRACE_EXECUTION) || defined(_DEBUG)
std::string unEscape(const std::string& str) {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '\"':
                result += '"';
                break;
            case '\'':
                result += "\\'";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            case '\v':
                result += "\\v";
                break;
            case '\f':
                result += "\\f";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '\0':
                result += "\\0";
                break;
            case '\a':
                result += "\\a";
                break;

            default:
                result += c;
                break;
        }
    }

    return result;
}

void Debug_printObject(Value value) {
    switch (Obj::typeOf(value.as.obj)) {
        case ObjType::STRING:
            ObjString* string = (ObjString*)value.as.obj;
            FMT_PRINT("\"{}\"", unEscape(string->str));
            break;
    }
}

void Debug_printValue(Value value) {
    switch (value.type) {
        case ValueType::NONE: FMT_PRINT("None"); break;
        case ValueType::BOOL: FMT_PRINT("{}", value.as.boolean); break;
        case ValueType::DOUBLE: FMT_PRINT("{}", value.as.decimal); break;
        case ValueType::INT: FMT_PRINT("{}", value.as.integer); break;
        case ValueType::INFINITY: FMT_PRINT("{}", value.as.boolean ? "inf" : "-inf"); break;
        case ValueType::NAN: FMT_PRINT("{}", value.as.boolean ? "nan" : "-nan"); break;
        case ValueType::OBJECT: Debug_printObject(value); break;
    }
}
#endif
// clang-format on
