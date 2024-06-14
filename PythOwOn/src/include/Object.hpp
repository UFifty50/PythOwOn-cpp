#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <ostream>
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
    [[nodiscard]] const ObjString* asString() const {
        return reinterpret_cast<const ObjString*>(this);
    }

    bool operator==(const Obj& other) const;
    std::ostream& operator<<(std::ostream& os) const;
};

struct ObjString {
    Obj object;
    std::string str;

    static ObjString* create(const std::string& str);
    static ObjString* create(const std::string& str, std::tuple<int32_t, int32_t> slice);

    bool operator==(const ObjString& other) const;
};

template <>
struct std::hash<ObjString> {
    std::size_t operator()(const ObjString& string) const noexcept {
        return std::hash<std::string>()(string.str);
    }
};

#endif
