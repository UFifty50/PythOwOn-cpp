#ifndef COMPILER_HPP
#define COMPILER_HPP

#include <string>
#include <functional>

#include "Chunk.hpp"
#include "Value.hpp"
#include "Scanner.hpp"


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
private:
    Chunk* chunk;
    Scanner* scanner;
    Parser parser;
    ParseRule rules[(int)TokenType::TOKEN_COUNT];

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
    inline void number();
    inline void grouping();

public:
    Compiler(Chunk* chunkToCompile);
    ~Compiler();

    bool compile(std::string source);
};

#endif
