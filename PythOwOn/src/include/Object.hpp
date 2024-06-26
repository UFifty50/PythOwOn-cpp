#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <stdint.h>

#include <cstddef>
#include <string>
#include <tuple>
#include <type_traits>
#include <xstring>


enum class ObjType {
    NONE,
    STRING,
};

struct ObjString;

struct Obj {
    ObjType type;

    static ObjType TypeOf(const Obj* obj) { return obj->type; }

    [[nodiscard]] const ObjString* asString() const {
        return reinterpret_cast<const ObjString*>(this);
    }

    bool operator==(const Obj& other) const;
    std::ostream& operator<<(std::ostream& os) const;
};

struct ObjString {
    Obj object;
    std::string str;

    static const ObjString* Create(const std::string& newStr);
    static const ObjString* Create(const std::string& newStr,
                                   std::tuple<int32_t, int32_t> slice);

    bool operator==(const ObjString& other) const;
};

template <>
struct std::hash<ObjString> {
    std::size_t operator()(const ObjString& string) const noexcept {
        return std::hash<std::string>()(string.str);
    }
};

#endif
