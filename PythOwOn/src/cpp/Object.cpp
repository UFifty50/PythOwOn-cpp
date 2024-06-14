#include "Object.hpp"

#include "VirtualMachine.hpp"
#include "Value.hpp"


bool Obj::operator==(const Obj& other) const {
    if (type != other.type) return false;

    switch (other.type) {
        case ObjType::STRING:
            return asString()->str == other.asString()->str;

        case ObjType::NONE:
        default:
            return false;
    }
}

bool ObjString::operator==(const ObjString& other) const { return str == other.str; }


std::ostream& Obj::operator<<(std::ostream& os) const {
    switch (type) {
        case ObjType::STRING:
            return os << asString()->str;

        case ObjType::NONE:
        default:
            return os << std::string("None");
    }
}


ObjString* ObjString::create(const std::string& str) {
    auto* string = VM::newObject<ObjString>(ObjType::STRING);
    string->object.type = ObjType::STRING;
    string->str = str;
    VM::addString(string, Value::noneVal());
    return string;
}

ObjString* ObjString::create(const std::string& str, std::tuple<int32_t, int32_t> slice) {
    if (std::get<0>(slice) < 0)
        std::get<0>(slice) = static_cast<signed>(str.size()) + std::get<0>(slice) - 1;
    if (std::get<1>(slice) < 0)
        std::get<1>(slice) = static_cast<signed>(str.size()) + std::get<1>(slice) - 1;

    return create(str.substr(std::get<0>(slice), std::get<1>(slice)));
}
