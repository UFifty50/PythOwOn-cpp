#include <functional>

#include "fmt/core.h"

#include "Compiler.hpp"
#include "Chunk.hpp"
#include "Scanner.hpp"
#include "Common.hpp"


Compiler::Compiler(Chunk* chunkToCompile) : scanner(nullptr) {
    chunk = chunkToCompile;
    parser.hadError = false;
    parser.panicMode = false;

    rules[(int)TokenType::LPAREN]     =  ParseRule { &Compiler::grouping,   nullptr,             Precedence::CALL };
    rules[(int)TokenType::RPAREN]     =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::LBRACE]     =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::RBRACE]     =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::LBRACK]     =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::RBRACK]     =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::COMMA]      =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::DOT]        =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::MINUS]      =  ParseRule { &Compiler::unary,      &Compiler::binary,   Precedence::TERM };
    rules[(int)TokenType::PLUS]       =  ParseRule { nullptr,               &Compiler::binary,   Precedence::TERM };
    rules[(int)TokenType::SEMI]       =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::SLASH]      =  ParseRule { nullptr,               &Compiler::binary,   Precedence::FACTOR };
    rules[(int)TokenType::STAR]       =  ParseRule { nullptr,               &Compiler::binary,   Precedence::FACTOR };
    rules[(int)TokenType::PERCENT]    =  ParseRule { nullptr,               &Compiler::binary,   Precedence::FACTOR };
    rules[(int)TokenType::BANG]       =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::BANG_EQ]    =  ParseRule { nullptr,               nullptr,             Precedence::EQUALITY };
    rules[(int)TokenType::EQ]         =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::EQ_EQ]      =  ParseRule { nullptr,               nullptr,             Precedence::COMPARISON };
    rules[(int)TokenType::GREATER]    =  ParseRule { nullptr,               nullptr,             Precedence::COMPARISON };
    rules[(int)TokenType::GREATER_EQ] =  ParseRule { nullptr,               nullptr,             Precedence::COMPARISON };
    rules[(int)TokenType::LESS]       =  ParseRule { nullptr,               nullptr,             Precedence::COMPARISON };
    rules[(int)TokenType::LESS_EQ]    =  ParseRule { nullptr,               nullptr,             Precedence::COMPARISON };
    rules[(int)TokenType::LSHIFT]     =  ParseRule { nullptr,               nullptr,             Precedence::SHIFT };
    rules[(int)TokenType::RSHIFT]     =  ParseRule { nullptr,               nullptr,             Precedence::SHIFT };
    rules[(int)TokenType::AMPERSAND]  =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::PIPE]       =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::CARET]      =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::IDENTIFIER] =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::STR]        =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::NUM]        =  ParseRule { &Compiler::number,     nullptr,             Precedence::NONE };
    rules[(int)TokenType::INFINITY]   =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::NAN]        =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::AND]        =  ParseRule { nullptr,               nullptr,             Precedence::AND };
    rules[(int)TokenType::CLASS]      =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::ELSE]       =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::FALSE]      =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::FOR]        =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::DEF]        =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::IF]         =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::NONE]       =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::OR]         =  ParseRule { nullptr,               nullptr,             Precedence::OR };
    rules[(int)TokenType::PRINT]      =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::RETURN]     =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType:: SUPER]     =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::THIS]       =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::TRUE]       =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::VAR]        =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::WHILE]      =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::EXTENDS]    =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::SWITCH]     =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::CASE]       =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::DEFAULT]    =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::CONTINUE]   =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::BREAK]      =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::IN]         =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::ERROR]      =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
    rules[(int)TokenType::EOF]        =  ParseRule { nullptr,               nullptr,             Precedence::NONE };
}

Compiler::~Compiler() {
    delete scanner;
}

void Compiler::advance() {
    parser.previous = parser.current;

    while (true) {
        parser.current = scanner->scanToken();

        if (parser.current.type != TokenType::ERROR) break;

        errorAt(parser.current, parser.current.lexeme);
    }
}

void Compiler::errorAt(Token token, std::string message) {
    if (parser.panicMode) return;
    parser.panicMode = true;
    FMT_PRINT("[line {}] Error", token.line);

    if (token.type == TokenType::EOF) {
        FMT_PRINT(" at end");
    } else if (token.type == TokenType::ERROR) { // Nothing
    } else {
        FMT_PRINT(" at '{}'", token.lexeme);
    }

    FMT_PRINT(": {}\n", message);
    parser.hadError = true;
}

void Compiler::consume(TokenType type, std::string message) {
    if (parser.current.type == type) {
        advance();
        return;
    }
    errorAt(parser.current, message);
}

inline ParseRule* Compiler::getRule(TokenType type) {
    return &rules[(int)type];
}



inline void Compiler::emitByte(uint8_t byte) {
    chunk->write(byte, parser.previous.line);
}

inline void Compiler::emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

inline void Compiler::emitConstant(Value value) {
    
    chunk->writeConstant(value, parser.previous.line);
}

inline void Compiler::emitReturn() {
    emitByte(OpCode::RETURN);
}

inline void Compiler::endCompiler() {
    emitReturn();

#if defined(TRACE_EXECUTION)
    if (!parser.hadError) {
        chunk->disassemble("code");
    }
#endif
}


// makeConstant (now not needed thanks to Chunk::writeConstant)
/*
uint8_t Compiler::makeConstant(Value value) {
    size_t constant = chunk->addConstant(value);
    if (constant > UINT8_MAX) {
        errorAt(parser.previous, "Too many constants in one chunk.");
        return 0;
    }
    return (uint8_t)constant;
} */

void Compiler::parsePrecedence(Precedence precedence) {
    advance();
    auto prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == nullptr) {
        errorAt(parser.previous, "Expected expression.");
        return;
    }

    prefixRule(this);

    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();
        auto infixRule = getRule(parser.previous.type)->infix;
        infixRule(this);
    }
}



void Compiler::expression() {
    parsePrecedence(Precedence::ASSIGNMENT);
}

void Compiler::unary() {
    TokenType operatorType = parser.previous.type;

    parsePrecedence(Precedence::UNARY);

    switch (operatorType) {
        case TokenType::MINUS: emitByte(OpCode::NEGATE); break;
        default: return; // Unreachable.
    }
}

inline void Compiler::number() {
    double value = std::stod(parser.previous.lexeme);
    emitConstant(Value::numberVal(value));
}

inline void Compiler::grouping() {
    expression();
    consume(TokenType::RPAREN, "Expected ')' after expression.");
}

void Compiler::binary() {
    TokenType operatorType = parser.previous.type;
    ParseRule* rule = getRule(operatorType);
    parsePrecedence((Precedence)((int)rule->precedence + 1));

    switch (operatorType) {
        case TokenType::PLUS:          emitByte(OpCode::ADD); break;
        case TokenType::STAR:          emitByte(OpCode::MULTIPLY); break;
        case TokenType::SLASH:         emitByte(OpCode::DIVIDE); break;
        default: break; // Unreachable
    }
}



bool Compiler::compile(std::string source) {
    scanner = new Scanner(source);

    advance();
    expression();
    consume(TokenType::EOF, "Expected end of expression.");

    endCompiler();
    return !parser.hadError;
}
