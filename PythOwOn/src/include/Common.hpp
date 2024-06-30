#ifndef COMMON_HPP
#define COMMON_HPP

#include <bit>
#include <ostream>
#include <vector>

#include "fmt/core.h"

#undef INFINITY
#undef NAN
#undef EOF

#define FMT_PRINT(formatStr, ...) \
    fmt::print(fmt::runtime(formatStr) __VA_OPT__(, ) __VA_ARGS__)

#define FMT_PRINTLN(formatStr, ...) \
    fmt::println(fmt::runtime(formatStr) __VA_OPT__(, ) __VA_ARGS__)

#define FMT_FORMAT(formatStr, ...) \
    fmt::format(fmt::runtime(formatStr) __VA_OPT__(, ) __VA_ARGS__)


template <typename T>
concept Printable = requires(std::ostream& os, const T& t) { os << t; };

template <typename... Ts>
concept AllPrintable = (Printable<Ts> && ...);

using ssize_t = intmax_t;


///
///@brief Function template to convert an LE value of type T to a BE byte array
///@tparam T The type of the value to convert
///@param value The value to convert
///@return A pointer to the byte array
///
template <typename T>
[[nodiscard]] const char* LEtoBEStr(const T& value) {
    static char byteArray[sizeof(T)];

    const auto* valueBytes = reinterpret_cast<const uint8_t*>(&value);

    for (size_t i = 0; i < sizeof(T); ++i) { byteArray[i] = valueBytes[sizeof(T) - 1 - i]; }

    return byteArray;
}

/// \brief Function template to convert a BE byte array to an LE value of type T
/// \tparam T The type of the value to convert to
/// \param byteArray The BE byte array to convert
/// \return The converted LE value
template <typename T>
[[nodiscard]] T BEStrToLE(const char* byteArray) {
    T value;

    // PTR to the raw bytes of `value`
    auto* valueBytes = reinterpret_cast<uint8_t*>(&value);

    for (size_t i = 0; i < sizeof(T); ++i) {
        valueBytes[sizeof(T) - 1 - i] = static_cast<uint8_t>(byteArray[i]);
    }

    return value;
}

template <typename T>
std::ostream& operator<<(std::ostream& os,
                         const std::vector<T>& vec) {
    for (auto& elem : vec) os.write(LEtoBEStr<T>(elem), sizeof(T));

    return os;
}

class InterpretResult {
public:
    enum Result : uint8_t { OK = 0, COMPILE_ERROR = 65, RUNTIME_ERROR = 70 };

    enum Cause : uint8_t { NONE, UNTERMINATED };

    InterpretResult() = default;
    constexpr InterpretResult(const Result result) : result(result), cause(NONE) {}

    constexpr InterpretResult(const Result result, const Cause cause)
        : result(result), cause(cause) {}


    [[nodiscard]] constexpr bool isError() const { return result != OK; }

    [[nodiscard]] constexpr std::string toString() const {
        switch (result) {
            case OK: return "OK";
            case RUNTIME_ERROR: return "RUNTIME_ERROR";
            case COMPILE_ERROR: return "COMPILE_ERROR";
            default: return "UNKNOWN";
        }
    }

    constexpr operator Result() const { return result; }

    explicit constexpr operator bool() const = delete;

private:
    Result result;
    Cause cause; // NOLINT(clang-diagnostic-unused-private-field)
};

class TokenType {
public:
    enum Type {
        // single char tokens
        LPAREN,
        RPAREN,
        LBRACE,
        RBRACE,
        LBRACK,
        RBRACK,
        COMMA,
        DOT,
        SEMI,
        COLON,

        // single/double char tokens
        BANG,
        BANG_EQ,
        EQ,
        EQ_EQ,
        GREATER,
        GREATER_EQ,
        LESS,
        LESS_EQ,
        PLUS,
        PLUSPLUS,
        PLUS_EQ,
        MINUS,
        MINUSMINUS,
        MINUS_EQ,
        STAR,
        STAR_EQ,
        SLASH,
        SLASH_EQ,
        LSHIFT,
        LSHIFT_EQ,
        RSHIFT,
        RSHIFT_EQ,
        PERCENT,
        PERCENT_EQ,
        AMPERSAND,
        PIPE,
        CARET,

        // literals
        IDENTIFIER,
        STR,
        NUM,
        INF,
        NAN, // TODO: implement inf/NaN

        // keywords
        AND,
        OR,
        NOT,
        CLASS,
        ELSE,
        FALSE,
        FOR,
        DEF,
        IF,
        NONE,
        PRINT, // TODO: implement print in standard library instead of here
        RETURN,
        SUPER,
        THIS,
        TRUE,
        LET,
        CONST,
        WHILE,
        EXTENDS,
        SWITCH,
        CASE,
        DEFAULT,
        CONTINUE,
        BREAK,
        IN,

        ERROR,
        EOF,

        TOKEN_COUNT
    };

    TokenType() = default;
    constexpr TokenType(const Type type) : type(type) {}

    [[nodiscard]] constexpr operator uint8_t() const { return type; }
    [[nodiscard]] constexpr operator Type() const { return type; }
    explicit constexpr operator bool() const = delete;

private:
    Type type;
};

struct Token {
    TokenType::Type type;
    std::string lexeme;
    size_t line = 0;
};


// ReSharper disable once CppInconsistentNaming
template <class T = void>
struct lshift {
    using _FIRST_ARGUMENT_TYPE_NAME _CXX17_DEPRECATE_ADAPTOR_TYPEDEFS = T;
    using _SECOND_ARGUMENT_TYPE_NAME _CXX17_DEPRECATE_ADAPTOR_TYPEDEFS = T;
    using _RESULT_TYPE_NAME _CXX17_DEPRECATE_ADAPTOR_TYPEDEFS = T;

    _NODISCARD constexpr T operator()(const T& left, const T& right) const { return left << right; }
};

// ReSharper disable once CppInconsistentNaming
template <class T = void>
struct rshift {
    using _FIRST_ARGUMENT_TYPE_NAME _CXX17_DEPRECATE_ADAPTOR_TYPEDEFS = T;
    using _SECOND_ARGUMENT_TYPE_NAME _CXX17_DEPRECATE_ADAPTOR_TYPEDEFS = T;
    using _RESULT_TYPE_NAME _CXX17_DEPRECATE_ADAPTOR_TYPEDEFS = T;

    _NODISCARD constexpr T operator()(const T& left, const T& right) const { return left >> right; }
};

template <typename T = float>
[[nodiscard]] static constexpr bool ProxEqual(const T a, const T b,
                                              const T epsilon = 0.0001) {
    return std::abs(a - b) < epsilon;
}

#endif
