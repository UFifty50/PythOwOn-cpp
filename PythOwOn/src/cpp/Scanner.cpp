#include <string>
#include <optional>

#include "Common.hpp"
#include "Scanner.hpp"

#define AT_END (current >= source.length())
#define IS_DIGIT(c) (c >= '0' && c <= '9')
#define IS_ALPHA(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')


Scanner::Scanner(std::string source) {
    this->source = source;
    this->start = 0;
    this->current = 0;
    this->line = 1;
}

Scanner::~Scanner() = default;


char Scanner::advance() {
    current++;
    return source[current - 1];
}

bool Scanner::isNext(char expected) {
    if (AT_END) return false;
    if (peekNext() != expected) return false;
    current++;
    return true;
}

bool Scanner::isNextNext(char expected) {
    if (current+2 >= source.length()) return false;
    if (peekNextNext() != expected) return false;
    current += 2;
    return true;
}

char Scanner::peekNext() {
    if (AT_END) return '\0';
    return source[current+1];
}

char Scanner::peekNextNext() {
    if (current+2 >= source.length()) return '\0';
    return source[current+2];
}

TokenType Scanner::checkKeyword(int begin, int length,
                                std::string rest, TokenType type) {
    if ((current - start) == (begin + length) && source.substr(start + begin, length) == rest) {
        return type;
    }

    return TokenType::IDENTIFIER;
}

TokenType Scanner::identifierType() {
    switch (source[start]) {
        case 'a': return checkKeyword(1, 2, "nd", TokenType::AND);
        case 'b': return checkKeyword(1, 4, "reak", TokenType::BREAK);
        case 'c':
            if ((current - start) > 1) {
                switch (source[start+1]) {
                    case 'l': return checkKeyword(2, 3, "ass", TokenType::CLASS);
                    case 'a': return checkKeyword(2, 2, "se", TokenType::CASE);
                    case 'o': return checkKeyword(2, 6, "ntinue", TokenType::CONTINUE);
                    default: break;
                }
            }
            break;

        case 'd': return checkKeyword(1, 6, "efault", TokenType::DEFAULT);
        case 'e':
            if ((current - start) > 1) {
                switch (source[start+1]) {
                    case 'l': return checkKeyword(2, 2, "se", TokenType::ELSE);
                    case 'x': return checkKeyword(2, 5, "tends", TokenType::EXTENDS);
                    default: break;
                }
            }
            break;

        case 'f':
            if ((current - start) > 1) {
                switch (source[start+1]) {
                    case 'a': return checkKeyword(2, 3, "lse", TokenType::FALSE);
                    case 'o': return checkKeyword(2, 1, "r", TokenType::FOR);
                    case 'w': return checkKeyword(2, 7, "unction", TokenType::DEF);
                    default: break;
                }
            }
            break;

        case 'i': return checkKeyword(1, 1, "f", TokenType::IF);
        case 'n': return checkKeyword(1, 3, "one", TokenType::NONE);
        case 'o': return checkKeyword(1, 1, "r", TokenType::OR);
        case 'p': return checkKeyword(1, 4, "rint", TokenType::PRINT);
        case 'r': return checkKeyword(1, 5, "eturn", TokenType::RETURN);
        case 's':
            if ((current - start) > 1) {
                switch (source[start+1]) {
                    case 'u': return checkKeyword(2, 3, "per", TokenType::SUPER);
                    case 'w': return checkKeyword(2, 4, "itch", TokenType::SWITCH);
                    default: break;
                }
            }
            break;

        case 't':
            if ((current - start) > 1) {
                switch (source[start+1]) {
                    case 'h': return checkKeyword(2, 2, "is", TokenType::THIS);
                    case 'r': return checkKeyword(2, 2, "ue", TokenType::TRUE);
                    default: break;
                }
            }
            break;

        case 'v': return checkKeyword(1, 2, "ar", TokenType::VAR);
        case 'w': return checkKeyword(1, 4, "hile", TokenType::WHILE);

        default: return TokenType::IDENTIFIER;
    }

    return TokenType::IDENTIFIER;
}

Token Scanner::string() {
    while (source[current] != '"' && !AT_END) {
        advance();
    }

    if (AT_END) return errorToken("Unterminated single-line string.");

    advance();
    return makeToken(TokenType::STR);
}

Token Scanner::multiString() {
    while ((source[current] != '"' || peekNext() != '"' || peekNextNext() != '"') && !AT_END) {
        if (source[current] == '\n') line++;
        advance();
    }
    if (current+2 >= source.length()) return errorToken("Unterminated multi-line string.");
    advance();
    advance();
    advance();
    return makeToken(TokenType::STR);
}

Token Scanner::number() {
    while (IS_DIGIT(source[current])) advance();
    if ((source[current] == '.' || source[current] == 'e') && IS_DIGIT(peekNext())) {
        advance();
        while (IS_DIGIT(source[current])) advance();
    }
    return makeToken(TokenType::NUM);
}

Token Scanner::identifier() {
    while (IS_ALPHA(source[current]) || IS_DIGIT(source[current])) advance();
    return makeToken(identifierType());
}

std::optional<Token> Scanner::skipWhitespace() {   // TODO: error if EOF reached before comment close
    while(true) {
        char c = source[current];
        switch (c) {
            case ' ':
            case '\r':
            case '\t': advance(); break;
            case '\n': line++; advance(); break;

            case '#': {
                if (peekNext() == '|') {
                    while ((source[current] != '|' || peekNext() != '#') && !AT_END) {
                        if (source[current] == '\n') line++;
                        advance();
                    }
                    if (AT_END) return errorToken("Unterminated comment.");
                    advance();
                    advance();
                } else {
                    while (source[current] != '\n' && !AT_END) advance();
                }
                break;
            }

            default:  break;
        }

        return std::nullopt;
    }
}

Token Scanner::scanToken() {
    if (auto token = skipWhitespace()) return token.value();
    start = current;

    if (AT_END) return makeToken(TokenType::EOF);

    char c = advance();
    if (IS_ALPHA(c)) return identifier();
    if (IS_DIGIT(c)) return number();

    switch (c) {
        case '(': return makeToken(TokenType::LPAREN);
        case ')': return makeToken(TokenType::RPAREN);
        case '{': return makeToken(TokenType::LBRACE);
        case '}': return makeToken(TokenType::RBRACE);
        case '[': return makeToken(TokenType::LBRACK);
        case ']': return makeToken(TokenType::RBRACK);
        case ',': return makeToken(TokenType::COMMA);
        case '.': return makeToken(TokenType::DOT);
        case '-': return makeToken(TokenType::MINUS);
        case '+': return makeToken(TokenType::PLUS);
        case '/': return makeToken(TokenType::SLASH);
        case '*': return makeToken(TokenType::STAR);

        case '!':
            return makeToken(
                isNext('=') ? TokenType::BANG_EQ : TokenType::BANG
            );

        case '=':
            return makeToken(
                isNext('=') ? TokenType::EQ_EQ : TokenType::EQ
            );

        case '<':
            return makeToken(
                isNext('=') ? TokenType::LESS_EQ : TokenType::LESS
            );

        case '>':
            return makeToken(
                isNext('=') ? TokenType::GREATER_EQ : TokenType::GREATER
            );

        case '"':
            return (source[current] == '"' && peekNext() == '"') ? multiString() : string();

    }

    return errorToken("Unexpected character.");
}

Token Scanner::makeToken(TokenType type) {
    return Token {
        type,
        source.substr(start, current - start),
        line
    };
}

Token Scanner::errorToken(std::string message) {
    return Token {
        TokenType::ERROR,
        message,
        line
    };
}
