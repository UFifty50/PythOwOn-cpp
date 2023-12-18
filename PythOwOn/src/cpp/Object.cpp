#include "Object.hpp"

#include "VirtualMachine.hpp"


bool Obj::operator==(const Obj& other) const {
    if (type != other.type) return false;

    switch (other.type) {
        case ObjType::STRING:
            return asString()->str == other.asString()->str;
        default:
            return false;
    }
}

bool ObjString::operator==(const ObjString& other) const { return str == other.str; }


ObjString* ObjString::create(const std::string& str) {
    ObjString* string = (ObjString*)VM::newObject<ObjString>(ObjType::STRING);
    string->object.type = ObjType::STRING;
    string->str = str;
    VM::addString(string, Value::noneVal());
    return string;
}

ObjString* ObjString::create(const std::string& str, std::tuple<int, int> slice) {
    if (std::get<0>(slice) < 0)
        std::get<0>(slice) = (int)str.size() + std::get<0>(slice) - 1;
    if (std::get<1>(slice) < 0)
        std::get<1>(slice) = (int)str.size() + std::get<1>(slice) - 1;

    return create(str.substr(std::get<0>(slice), std::get<1>(slice)));
}
