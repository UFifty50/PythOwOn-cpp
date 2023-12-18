#include "Scanner.hpp"

#include <optional>
#include <string>

#include "Common.hpp"

#define AT_END (current >= source.length())
#define IS_DIGIT(c) (c >= '0' && c <= '9')
#define IS_ALPHA(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')


Scanner::Scanner(std::string source) : source(source), start(0), current(0), line(1) {}

Scanner::~Scanner() = default;


char Scanner::advance() {
    current++;
    return source[current - 1];
}

bool Scanner::match(char expected) {
    if (AT_END) return false;
    if (peek(0) != expected) return false;
    current++;
    return true;
}

char Scanner::peek(int distance) {
    if (current + distance >= source.length()) return '\0';
    return source[current + distance];
}

TokenType Scanner::checkKeyword(uint32_t begin, uint32_t length, std::string rest,
                                TokenType type) {
    if ((current - start) == (begin + length) &&
        source.substr(start + begin, length) == rest) {
        return type;
    }

    return TokenType::IDENTIFIER;
}

TokenType Scanner::identifierType() {
    switch (source[start]) {
        case 'a':
            return checkKeyword(1, 2, "nd", TokenType::AND);
        case 'b':
            return checkKeyword(1, 4, "reak", TokenType::BREAK);
        case 'c':
            if ((current - start) > 1) {
                switch (source[start + 1]) {
                    case 'l':
                        return checkKeyword(2, 3, "ass", TokenType::CLASS);
                    case 'a':
                        return checkKeyword(2, 2, "se", TokenType::CASE);
                    case 'o':
                        return checkKeyword(2, 6, "ntinue", TokenType::CONTINUE);
                    default:
                        break;
                }
            }
            break;

        case 'd':
            return checkKeyword(1, 6, "efault", TokenType::DEFAULT);
        case 'e':
            if ((current - start) > 1) {
                switch (source[start + 1]) {
                    case 'l':
                        return checkKeyword(2, 2, "se", TokenType::ELSE);
                    case 'x':
                        return checkKeyword(2, 5, "tends", TokenType::EXTENDS);
                    default:
                        break;
                }
            }
            break;

        case 'f':
            if ((current - start) > 1) {
                switch (source[start + 1]) {
                    case 'a':
                        return checkKeyword(2, 3, "lse", TokenType::FALSE);
                    case 'o':
                        return checkKeyword(2, 1, "r", TokenType::FOR);
                    case 'w':
                        return checkKeyword(2, 7, "unction", TokenType::DEF);
                    default:
                        break;
                }
            }
            break;

        case 'i':
            if ((current - start) > 2) {
                switch (source[start + 1]) {
                    case 'n':
                        return checkKeyword(2, 1, "f", TokenType::INF);
                    default:
                        break;
                }
            }
            return checkKeyword(1, 1, "f", TokenType::IF);

        case 'n':  // can be none, not or nan
            if ((current - start) > 2) {
                switch (source[start + 1]) {
                    case 'a':
                        return checkKeyword(2, 1, "n", TokenType::NAN);

                    case 'o':
                        switch (source[start + 2]) {
                            case 'n':
                                return checkKeyword(3, 1, "e", TokenType::NONE);
                            case 't':
                                return checkKeyword(3, 0, "", TokenType::NOT);
                            default:
                                break;
                        }
                    default:
                        break;
                }
            }
            break;


        case 'o':
            return checkKeyword(1, 1, "r", TokenType::OR);
        case 'p':
            return checkKeyword(1, 4, "rint", TokenType::PRINT);
        case 'r':
            return checkKeyword(1, 5, "eturn", TokenType::RETURN);
        case 's':
            if ((current - start) > 1) {
                switch (source[start + 1]) {
                    case 'u':
                        return checkKeyword(2, 3, "per", TokenType::SUPER);
                    case 'w':
                        return checkKeyword(2, 4, "itch", TokenType::SWITCH);
                    default:
                        break;
                }
            }
            break;

        case 't':
            if ((current - start) > 1) {
                switch (source[start + 1]) {
                    case 'h':
                        return checkKeyword(2, 2, "is", TokenType::THIS);
                    case 'r':
                        return checkKeyword(2, 2, "ue", TokenType::TRUE);
                    default:
                        break;
                }
            }
            break;

        case 'v':
            return checkKeyword(1, 2, "ar", TokenType::LET);
        case 'w':
            return checkKeyword(1, 4, "hile", TokenType::WHILE);

        default:
            return TokenType::IDENTIFIER;
    }

    return TokenType::IDENTIFIER;
}

// clang-format off
char escapeSequence(char identifier) {
    switch (identifier) {
        case '"':  return '"'; 
        case '\'': return '\'';
        case 'n':  return '\n';
        case 'r':  return '\r';
        case 't':  return '\t';
        case 'v':  return '\v';
        case 'f':  return '\f';
        case '\\': return '\\';
        case '0':  return '\0';
        case 'a':  return '\a';
        default:
            return (char)-1;
    }
}
// clang-format on

Token Scanner::string() {
    std::string str = "";

    while (true) {
        char c = advance();
        if (c == '"') break;
        if (AT_END)
            return errorToken(
                FMT_FORMAT("Unterminated string at line {}.", std::to_string(line)));

        if (c == '\n') line++;

        if (c == '\\') {
            c = escapeSequence(advance());
            if (c == -1) return errorToken("Unknown escape sequence");
        }
        str += c;
    }

    return makeToken(TokenType::STR, "\"" + str + "\"");
}

Token Scanner::multiString() {
    std::string str = "";
    advance();
    advance();

    while (true) {
        if (peek(0) == '"' && peek(1) == '"' && peek(2) == '"') break;
        if (AT_END)
            return errorToken(FMT_FORMAT("Unterminated multi-line string at line {}.",
                                         std::to_string(line)));

        char c = advance();
        if (c == '\n') line++;

        if (c == '\\') {
            c = escapeSequence(advance());
            if (c == -1) return errorToken("Unknown escape sequence");
        }
        str += c;
    }

    advance();
    advance();
    advance();

    return makeToken(TokenType::STR, '"' + str + '"');
}

// no dot after e, no dot after dot, BUT e after dot is ok
Token Scanner::number() {
    while (IS_DIGIT(peek(0))) advance();

    if (peek(0) == '.' && IS_DIGIT(peek(1))) {
        advance();
        while (IS_DIGIT(peek(0))) advance();
    }
    if (peek(0) == 'e' && IS_DIGIT(peek(1))) {
        advance();
        while (IS_DIGIT(peek(0))) advance();
    }

    return makeToken(TokenType::NUM);
}

Token Scanner::identifier() {
    while (IS_ALPHA(peek(0)) || IS_DIGIT(peek(0))) advance();
    return makeToken(identifierType());
}

std::optional<Token> Scanner::skipWhitespace() {
    while (true) {
        char c = peek(0);
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                line++;
                advance();
                break;

            case '#': {
                if (peek(1) == '|') {
                    while ((peek(0) != '|' || peek(1) != '#') && !AT_END) {
                        if (peek(0) == '\n') line++;
                        advance();
                    }
                    if (AT_END) return errorToken("Unterminated comment.");
                    advance();
                    advance();
                } else {
                    while (peek(0) != '\n' && !AT_END) {
                        advance();
                        FMT_PRINT("{}", peek(0));
                    }
                }
                break;
            }

            default:
                break;
        }

        return std::nullopt;
    }
}

Token Scanner::scanToken() {
    if (auto token = skipWhitespace()) return token.value();
    start = current;

    if (AT_END) return makeToken(TokenType::EOF, "");

    char c = advance();
    if (IS_ALPHA(c)) return identifier();
    if (IS_DIGIT(c)) return number();

    switch (c) {
        case '(':
            return makeToken(TokenType::LPAREN);
        case ')':
            return makeToken(TokenType::RPAREN);
        case '{':
            return makeToken(TokenType::LBRACE);
        case '}':
            return makeToken(TokenType::RBRACE);
        case '[':
            return makeToken(TokenType::LBRACK);
        case ']':
            return makeToken(TokenType::RBRACK);
        case ',':
            return makeToken(TokenType::COMMA);
        case '.':
            return makeToken(TokenType::DOT);
        case '-':
            return makeToken(TokenType::MINUS);
        case '+':
            return makeToken(TokenType::PLUS);
        case ';':
            return makeToken(TokenType::SEMI);
        case '/':
            return makeToken(TokenType::SLASH);
        case '*':
            return makeToken(TokenType::STAR);

        case '!':
            return makeToken(match('=') ? TokenType::BANG_EQ : TokenType::BANG);

        case '=':
            return makeToken(match('=') ? TokenType::EQ_EQ : TokenType::EQ);

        case '>':
            return makeToken(match('=')
                                 ? TokenType::GREATER_EQ
                                 : (match('>') ? TokenType::RSHIFT : TokenType::GREATER));
        case '<':
            return makeToken(match('=')
                                 ? TokenType::LESS_EQ
                                 : (match('<') ? TokenType::LSHIFT : TokenType::LESS));

        case '"':
            return (peek(0) == '"' && peek(1) == '"') ? multiString() : string();
    }

    return errorToken("Unexpected character.");
}

Token Scanner::makeToken(TokenType type) {
    return Token{type, source.substr(start, current - start), line};
}

Token Scanner::makeToken(TokenType type, std::string token) {
    return Token{type, token, line};
}

Token Scanner::errorToken(std::string message) {
    return Token{TokenType::ERROR, message, line};
}
