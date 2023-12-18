#ifndef COMPILER_HPP
#define COMPILER_HPP

#include <array>
#include <functional>
#include <memory>
#include <string>

#include "Chunk.hpp"
#include "Scanner.hpp"
#include "Value.hpp"


enum class Precedence {
    NONE,
    ASSIGNMENT,  // =, +=, /=, <<=, ... You get the point
    //   CONDITIONAL, // ?:
    OR,          // or
    AND,         // and
    EQUALITY,    // ==, !=
    COMPARISON,  // <, >, <=, >=
    SHIFT,       // <<, >>
    TERM,        // +, -
    FACTOR,      // *, /, %
    UNARY,       // !, -
    CALL,        // ., ()
    PRIMARY
};

class Compiler;

struct ParseRule {
    std::function<void(Compiler*)> prefix;
    std::function<void(Compiler*)> infix;
    Precedence precedence;
};

struct Parser {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
};

class Compiler {
public:
    Compiler(std::shared_ptr<Chunk> chunkToCompile);
    ~Compiler() = default;

    bool compile(std::string source);

private:
    std::shared_ptr<Chunk> chunk;
    std::unique_ptr<Scanner> scanner;
    std::array<ParseRule, (int)TokenType::TOKEN_COUNT> rules;
    Parser parser;

    void advance();
    void errorAt(Token token, std::string message);
    void consume(TokenType type, std::string message);
    inline ParseRule* getRule(TokenType type);

    inline void emitByte(uint8_t byte);
    inline void emitBytes(uint8_t byte1, uint8_t byte2);
    inline void emitConstant(Value value);
    inline void emitReturn();
    inline void endCompiler();

    //    uint8_t makeConstant(Value value);
    void parsePrecedence(Precedence precedence);


    void expression();
    //  void statement();
    void unary();
    void binary();
    void literal();
    void string();
    void number();
    void grouping();
};

#endif
