#ifndef COMMON_HPP
#define COMMON_HPP

#include "fmt/core.h"

#undef INFINITY
#undef NAN
#undef EOF


#define FMT_PRINT(formatStr, ...) fmt::print(fmt::runtime(fmt::format(fmt::runtime(formatStr), __VA_ARGS__)))

enum class InterpretResult {
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

enum class TokenType {                  //TODO: add const token (maybe?)
    // single char tokens
    LPAREN, RPAREN,
    LBRACE, RBRACE,
    LBRACK, RBRACK,
    COMMA, DOT,
    MINUS, PLUS,
    PERCENT, SEMI,
    SLASH, STAR,
    COLON,

    // single/double char tokens
    BANG, BANG_EQ,
    EQ, EQ_EQ, GREATER,
    GREATER_EQ, LESS,
    LESS_EQ, LSHIFT, RSHIFT,

    // literals
    IDENTIFIER, STR, NUM,
    INFINITY, NAN,                  //TODO: implement infinity/NaN

    // keywords
    AND, CLASS, ELSE,
    FALSE, FOR, DEF, IF,
    NONE, OR, PRINT, RETURN,  //TODO: implement print in standard library instead of here
    SUPER, THIS, TRUE, VAR,
    WHILE, EXTENDS, SWITCH,
    CASE, DEFAULT, CONTINUE,
    BREAK, IN,

    ERROR, EOF
};

struct Token {
    TokenType type;
    std::string lexeme;
    size_t line;
};

#endif
