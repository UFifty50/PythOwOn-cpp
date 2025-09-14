#ifndef SCANNER_HPP
#define SCANNER_HPP

#include <stdint.h>

#include <optional>
#include <string>
#include <string>

#include "Common.hpp"


class Scanner {
public:
    Scanner(std::string source);

    Token scanToken();

private:
    std::string source;
    size_t start;
    size_t current;
    size_t line;


    char advance();
    bool match(char expected);
    [[nodiscard]] char peek(int distance) const;
    [[nodiscard]] TokenType checkKeyword(uint32_t begin, uint32_t length,
                                         const std::string& rest, TokenType type) const;
    [[nodiscard]] TokenType identifierType() const;
    std::optional<Token> skipWhitespace();

    [[nodiscard]] Token makeToken(TokenType type) const;
    [[nodiscard]] Token makeToken(TokenType type, const std::string& token) const;
    [[nodiscard]] Token errorToken(const std::string& message) const;

    Token string();
    Token multiString();
    Token number();
    Token identifier();

    Token scanSymbol(char c);

    Token handleMinus();
    Token handlePlus();
    Token handleSlash();
    Token handleStar();
    Token handlePercent();
    Token handleBang();
    Token handleEqual();
    Token handleGreater();
    Token handleLess();
    Token handleString();
};

#endif
