#ifndef COMPILER_HPP
#define COMPILER_HPP

#include <array>
#include <functional>
#include <memory>
#include <optional>
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
    std::function<void(Compiler*, bool)> prefix;
    std::function<void(Compiler*, bool)> infix;
    Precedence precedence;
};

struct Parser {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
};

struct Local {
    Token name;
    int depth;
};

struct CompilerState {
    std::vector<Local> locals;
    uint32_t scopeDepth;

    CompilerState() : scopeDepth(0) {}

    Local* addLocal(Token name) {
        locals.push_back(Local{name, -1});

        return &locals.back();
    }
};

class Compiler {
public:
    Compiler(std::shared_ptr<Chunk> chunkToCompile);
    ~Compiler() = default;

    bool compile(std::string source);

private:
    std::shared_ptr<Chunk> chunk;
    std::unique_ptr<Scanner> scanner;
    std::unique_ptr<CompilerState> state;
    std::array<ParseRule, (int)TokenType::TOKEN_COUNT> rules;
    Parser parser;

    void advance();
    void errorAt(Token token, std::string message);
    void consume(TokenType type, std::string message);
    bool match(TokenType type);
    inline ParseRule* getRule(TokenType type);

    inline void emitByte(uint8_t byte);
    inline void emitBytes(uint8_t byte1, uint8_t byte2);
    inline void emitConstant(Value value);
    inline void emitVariable(OpCode op, uint32_t var);
    inline void emitReturn();
    inline void endCompiler();

    //    uint8_t makeConstant(Value value);
    void parsePrecedence(Precedence precedence);
    uint32_t identifierConstant(Token* name);
    std::optional<uint32_t> resolveLocal(Token name);

    uint32_t parseVariable(std::string errorMessage);
    void markInitialized();
    void defineVariable(uint32_t global);
    void declareVariable();

    void namedVariable(Token name, bool canAssign);

    void expression();
    void expressionStatement();
    void printStatement();
    void statement();
    void declaration();
    void varDeclaration();
    void block();
    void beginScope();
    void endScope();
    void panicSync();
    void unary(bool);
    void binary(bool);
    void literal(bool);
    void string(bool);
    void variable(bool canAssign);
    void number(bool);
    void grouping(bool);
};

#endif
