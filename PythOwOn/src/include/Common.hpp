#ifndef COMMON_HPP
#define COMMON_HPP

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


class InterpretResult {
public:
    enum Result : uint8_t { OK, COMPILE_ERROR, RUNTIME_ERROR };
    enum Cause : uint8_t { NONE, UNTERMINATED };

    InterpretResult() = default;
    constexpr InterpretResult(const Result result) : result(result), cause(NONE) {}
    constexpr InterpretResult(const Result result, const Cause cause)
        : result(result), cause(cause) {}


    [[nodiscard]] constexpr bool isError() const { return result != OK; }

    [[nodiscard]] constexpr std::string toString() const {
        switch (result) {
            case OK:
                return "OK";
            case RUNTIME_ERROR:
                return "RUNTIME_ERROR";
            case COMPILE_ERROR:
                return "COMPILE_ERROR";
            default:
                return "UNKNOWN";
        }
    }

    constexpr operator Result() const { return result; }

    explicit constexpr operator bool() const = delete;

private:
    Result result;
    Cause cause;  // NOLINT(clang-diagnostic-unused-private-field)
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
        MINUS,
        PLUS,
        PERCENT,
        SEMI,
        SLASH,
        STAR,
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
        LSHIFT,
        RSHIFT,
        AMPERSAND,
        PIPE,
        CARET,

        // literals
        IDENTIFIER,
        STR,
        NUM,
        INF,
        NAN,  // TODO: implement inf/NaN

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
        PRINT,  // TODO: implement print in standard library instead of here
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

    _NODISCARD constexpr T operator()(const T& left, const T& right) const {
        return left << right;
    }
};

// ReSharper disable once CppInconsistentNaming
template <class T = void>
struct rshift {
    using _FIRST_ARGUMENT_TYPE_NAME _CXX17_DEPRECATE_ADAPTOR_TYPEDEFS = T;
    using _SECOND_ARGUMENT_TYPE_NAME _CXX17_DEPRECATE_ADAPTOR_TYPEDEFS = T;
    using _RESULT_TYPE_NAME _CXX17_DEPRECATE_ADAPTOR_TYPEDEFS = T;

    _NODISCARD constexpr T operator()(const T& left, const T& right) const {
        return left >> right;
    }
};

template <typename T = float>
[[nodiscard]] static constexpr bool ProxEqual(const T a, const T b,
                                              const T epsilon = 0.0001) {
    return std::abs(a - b) < epsilon;
}

#endif
