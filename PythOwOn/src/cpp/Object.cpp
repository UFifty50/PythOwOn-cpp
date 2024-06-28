#include "Object.hpp"

#include "Value.hpp"
#include "VirtualMachine.hpp"


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


const ObjString* ObjString::create(const std::string& str) {
    if (VM::hasString(str)) return VM::getString(str);

    auto* string = VM::newObject<ObjString>(ObjType::STRING);
    string->object.type = ObjType::STRING;
    string->str = str;
    VM::addString(string);
    return string;
}

const ObjString* ObjString::create(const std::string& newStr,
                                   std::tuple<int32_t, int32_t> slice) {
    if (std::get<0>(slice) < 0)
        std::get<0>(slice) =
            static_cast<signed>(newStr.size()) + std::get<0>(slice) - 1;
    if (std::get<1>(slice) < 0)
        std::get<1>(slice) =
            static_cast<signed>(newStr.size()) + std::get<1>(slice) - 1;

    return create(newStr.substr(std::get<0>(slice), std::get<1>(slice)));
}