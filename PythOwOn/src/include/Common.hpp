#ifndef COMMON_HPP
#define COMMON_HPP

#include "fmt/core.h"

#undef INFINITY
#undef NAN
#undef EOF

#define FMT_PRINT(formatStr, ...) \
    fmt::print(fmt::runtime(formatStr) __VA_OPT__(, ) __VA_ARGS__)

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
    constexpr InterpretResult(Result result) : result(result), cause(NONE) {}
    constexpr InterpretResult(Result result, Cause cause)
        : result(result), cause(cause) {}


    constexpr bool isError() const { return result != OK; }

    constexpr std::string toString() const {
        if (result == COMPILE_ERROR) {
            switch (cause) {
                case UNTERMINATED:
                    return "COMPILE_ERROR: UNTERMINATED";
                default:
                    return "COMPILE_ERROR: UNKNOWN CAUSE";
            }
        }

        switch (result) {
            case OK:
                return "OK";
            case RUNTIME_ERROR:
                return "RUNTIME_ERROR";
            default:
                return "UNKNOWN";
        }
    }

    constexpr operator Result() const { return result; }

    explicit constexpr operator bool() const = delete;

private:
    Result result;
    Cause cause;
};

enum class TokenType {
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

struct Token {
    TokenType type;
    std::string lexeme;
    size_t line;
};


template <class _Ty = void>
struct lshift {
    using _FIRST_ARGUMENT_TYPE_NAME _CXX17_DEPRECATE_ADAPTOR_TYPEDEFS = _Ty;
    using _SECOND_ARGUMENT_TYPE_NAME _CXX17_DEPRECATE_ADAPTOR_TYPEDEFS = _Ty;
    using _RESULT_TYPE_NAME _CXX17_DEPRECATE_ADAPTOR_TYPEDEFS = _Ty;

    _NODISCARD constexpr _Ty operator()(const _Ty& _Left, const _Ty& _Right) const {
        return _Left << _Right;
    }
};

template <class _Ty = void>
struct rshift {
    using _FIRST_ARGUMENT_TYPE_NAME _CXX17_DEPRECATE_ADAPTOR_TYPEDEFS = _Ty;
    using _SECOND_ARGUMENT_TYPE_NAME _CXX17_DEPRECATE_ADAPTOR_TYPEDEFS = _Ty;
    using _RESULT_TYPE_NAME _CXX17_DEPRECATE_ADAPTOR_TYPEDEFS = _Ty;

    _NODISCARD constexpr _Ty operator()(const _Ty& _Left, const _Ty& _Right) const {
        return _Left >> _Right;
    }
};

#endif
