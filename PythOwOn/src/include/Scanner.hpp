#ifndef SCANNER_HPP
#define SCANNER_HPP

#include <optional>
#include <string>

#include "Common.hpp"


class Scanner {
private:
    std::string source;
    size_t start;
    size_t current;
    size_t line;


    char advance();
    bool match(char expected);
    bool isNext(char expected);
    char peek(int distance);
    TokenType checkKeyword(uint32_t begin, uint32_t length, std::string rest,
                           TokenType type);
    TokenType identifierType();
    std::optional<Token> skipWhitespace();

    Token makeToken(TokenType type);
    Token makeToken(TokenType type, std::string token);
    Token errorToken(std::string message);

    Token string();
    Token multiString();
    Token number();
    Token identifier();

public:
    Scanner(std::string source);
    ~Scanner();

    Token scanToken();
};

#endif
