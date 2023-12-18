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


enum class InterpretResult { OK, COMPILE_ERROR, RUNTIME_ERROR };

enum class TokenType {  // TODO: add const token (maybe?)
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
    VAR,
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
