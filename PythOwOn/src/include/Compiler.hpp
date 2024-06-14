#ifndef COMPILER_HPP
#define COMPILER_HPP

#include <stdint.h>

#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <xstring>

#include "Chunk.hpp"
#include "Common.hpp"
#include "Scanner.hpp"
#include "Value.hpp"

struct Value;


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
    Precedence precedence = Precedence::NONE;
};

struct Parser {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
};

struct Local {
    Token name;
    int32_t depth;
};

struct CompilerState {
    std::vector<Local> locals;
    uint32_t scopeDepth;

    CompilerState() : scopeDepth(0) {}

    Local* addLocal(const Token& name) {
        locals.push_back(Local{name, -1});

        return &locals.back();
    }
};

class Compiler {
public:
    Compiler(const std::shared_ptr<Chunk>& chunkToCompile);

    bool compile(const std::string& source);

private:
    std::shared_ptr<Chunk> chunk;
    std::unique_ptr<Scanner> scanner;
    std::unique_ptr<CompilerState> state;
    std::array<ParseRule, TokenType::TOKEN_COUNT> rules;
    Parser parser;

    void advance();
    void errorAt(Token token, const std::string& message);
    void consume(TokenType::Type type, const std::string& message);
    bool match(TokenType::Type type);
    ParseRule* getRule(TokenType::Type type);

    void emitByte(uint8_t byte) const;
    void emitBytes(uint8_t byte1, uint8_t byte2) const;
    [[nodiscard]] uint16_t emitJump(OpCode op) const;
    [[nodiscard]] uint32_t emitJumpLong(OpCode op) const;
    void emitConstant(Value value) const;
    void patchJump(int32_t offset);
    void patchJumpLong(uint32_t offset);
    void emitVariable(OpCode op, uint32_t var) const;
    void emitLoop(uint16_t loopStart);
    void emitReturn() const;
    void endCompiler() const;

    //    uint8_t makeConstant(Value value);
    void parsePrecedence(Precedence precedence);
    uint32_t identifierConstant(const Token* name) const;
    std::optional<uint32_t> resolveLocal(const Token& name);

    uint32_t parseVariable(const std::string& errorMessage);
    void markInitialized() const;
    void defineVariable(uint32_t global) const;
    void declareVariable();

    void and_(bool canAssign);
    void or_(bool canAssign);

    void namedVariable(const Token& name, bool canAssign);

    void expression();
    void expressionStatement();
    void printStatement();
    void ifStatement();
    void switchStatement();
    void whileStatement();
    void forStatement();
    void continueStatement();
    void breakStatement();
    void statement();
    void declaration();
    void varDeclaration();
    void block();
    void beginScope() const;
    void endScope() const;
    void panicSync();
    void unary(bool);
    void binary(bool);
    void literal(bool) const;
    void string(bool) const;
    void variable(bool canAssign);
    void number(bool) const;
    void grouping(bool);
};

#endif
