#include "Common.hpp"
#include "Value.hpp"


void printValue(Value value) {
    switch (value.type) {
        case ValueType::NONE: FMT_PRINT("None"); break;
        case ValueType::BOOL: FMT_PRINT("{}", value.as.boolean); break;
        case ValueType::DOUBLE: FMT_PRINT("{}", value.as.number); break;
    }
}
