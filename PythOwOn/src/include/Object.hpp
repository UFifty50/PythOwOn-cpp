#ifndef OBJECT_HPP
#define OBJECT_HPP
#include <string>
#include <tuple>


enum class ObjType {
    NONE,
    STRING,
};

struct ObjString;
struct Obj {
    ObjType type;

    static ObjType typeOf(const Obj* obj) { return obj->type; }
    ObjString* asString() const { return (ObjString*)this; }

    bool operator==(const Obj& other) const;
};

struct ObjString {
    Obj object;
    std::string str;

    static ObjString* create(const std::string& str);
    static ObjString* create(const std::string& str, std::tuple<int, int> slice);
};

#endif
