#include "Compiler.hpp"

#include <functional>

#include "fmt/core.h"

#include "Chunk.hpp"
#include "Common.hpp"
#include "Object.hpp"
#include "Scanner.hpp"
#include "Value.hpp"


Compiler::Compiler(std::shared_ptr<Chunk> chunkToCompile) : chunk(chunkToCompile) {
    parser.hadError = false;
    parser.panicMode = false;

    // clang-format off
    //       ParseTable Position        |         prefix        |     infix      |        precedence       |
    rules[(int)TokenType::LPAREN]     = { &Compiler::grouping,  nullptr,             Precedence::CALL       };
    rules[(int)TokenType::RPAREN]     = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::LBRACE]     = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::RBRACE]     = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::LBRACK]     = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::RBRACK]     = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::COMMA]      = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::DOT]        = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::MINUS]      = { &Compiler::unary,     &Compiler::binary,   Precedence::TERM       };
    rules[(int)TokenType::PLUS]       = { nullptr,              &Compiler::binary,   Precedence::TERM       };
    rules[(int)TokenType::PERCENT]    = { nullptr,              &Compiler::binary,   Precedence::FACTOR     };
    rules[(int)TokenType::SEMI]       = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::SLASH]      = { nullptr,              &Compiler::binary,   Precedence::FACTOR     };
    rules[(int)TokenType::STAR]       = { nullptr,              &Compiler::binary,   Precedence::FACTOR     };
    rules[(int)TokenType::COLON]      = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::BANG]       = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::BANG_EQ]    = { nullptr,              &Compiler::binary,   Precedence::EQUALITY   };
    rules[(int)TokenType::EQ]         = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::EQ_EQ]      = { nullptr,              &Compiler::binary,   Precedence::EQUALITY   };
    rules[(int)TokenType::GREATER]    = { nullptr,              &Compiler::binary,   Precedence::COMPARISON };
    rules[(int)TokenType::GREATER_EQ] = { nullptr,              &Compiler::binary,   Precedence::COMPARISON };
    rules[(int)TokenType::LESS]       = { nullptr,              &Compiler::binary,   Precedence::COMPARISON };
    rules[(int)TokenType::LESS_EQ]    = { nullptr,              &Compiler::binary,   Precedence::COMPARISON };
    rules[(int)TokenType::LSHIFT]     = { nullptr,              &Compiler::binary,   Precedence::SHIFT      };
    rules[(int)TokenType::RSHIFT]     = { nullptr,              &Compiler::binary,   Precedence::SHIFT      };
    rules[(int)TokenType::AMPERSAND]  = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::PIPE]       = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::CARET]      = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::IDENTIFIER] = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::STR]        = { &Compiler::string,    nullptr,             Precedence::NONE       };
    rules[(int)TokenType::NUM]        = { &Compiler::number,    nullptr,             Precedence::NONE       };
    rules[(int)TokenType::INF]        = { &Compiler::number,    nullptr,             Precedence::NONE       };
    rules[(int)TokenType::NAN]        = { &Compiler::number,    nullptr,             Precedence::NONE       };
    rules[(int)TokenType::AND]        = { nullptr,              nullptr,             Precedence::AND        };
    rules[(int)TokenType::OR]         = { nullptr,              nullptr,             Precedence::OR         };
    rules[(int)TokenType::NOT]        = { &Compiler::unary,     nullptr,             Precedence::NONE       };
    rules[(int)TokenType::CLASS]      = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::ELSE]       = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::FALSE]      = { &Compiler::literal,   nullptr,             Precedence::NONE       };
    rules[(int)TokenType::FOR]        = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::DEF]        = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::IF]         = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::NONE]       = { &Compiler::literal,   nullptr,             Precedence::NONE       };
    rules[(int)TokenType::PRINT]      = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::RETURN]     = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::SUPER]      = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::THIS]       = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::TRUE]       = { &Compiler::literal,   nullptr,             Precedence::NONE       };
    rules[(int)TokenType::VAR]        = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::WHILE]      = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::EXTENDS]    = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::SWITCH]     = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::CASE]       = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::DEFAULT]    = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::CONTINUE]   = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::BREAK]      = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::IN]         = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::ERROR]      = { nullptr,              nullptr,             Precedence::NONE       };
    rules[(int)TokenType::EOF]        = { nullptr,              nullptr,             Precedence::NONE       };
    // clang-format on
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
    } else if (token.type == TokenType::ERROR) {  // Nothing
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

inline ParseRule* Compiler::getRule(TokenType type) { return &rules[(int)type]; }


inline void Compiler::emitByte(uint8_t byte) { chunk->write(byte, parser.previous.line); }

inline void Compiler::emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

inline void Compiler::emitConstant(Value value) {
    chunk->writeConstant(value, parser.previous.line);
}

inline void Compiler::emitReturn() { emitByte(OpCode::RETURN); }

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
    auto& prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == nullptr) {
        errorAt(parser.previous, "Expected expression.");
        return;
    }

    prefixRule(this);

    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();
        auto& infixRule = getRule(parser.previous.type)->infix;
        infixRule(this);
    }
}


void Compiler::expression() { parsePrecedence(Precedence::ASSIGNMENT); }

void Compiler::unary() {
    TokenType operatorType = parser.previous.type;

    parsePrecedence(Precedence::UNARY);

    // clang-format off
    switch (operatorType) {
        case TokenType::NOT: emitByte(OpCode::NOT); break;
        case TokenType::MINUS: emitByte(OpCode::NEGATE); break;
        default: return; // Unreachable.
    }
    // clang-format on
}

void Compiler::binary() {
    TokenType operatorType = parser.previous.type;
    ParseRule* rule = getRule(operatorType);
    parsePrecedence((Precedence)((int)rule->precedence + 1));

    // clang-format off
    switch (operatorType) {
        case TokenType::MINUS:        emitBytes(OpCode::NEGATE, OpCode::ADD); break;
        case TokenType::PLUS:          emitByte(OpCode::ADD); break;
        case TokenType::SLASH:         emitByte(OpCode::DIVIDE); break;
        case TokenType::STAR:          emitByte(OpCode::MULTIPLY); break;
        case TokenType::BANG_EQ:      emitBytes(OpCode::EQUAL, OpCode::NOT); break;
        case TokenType::EQ_EQ:         emitByte(OpCode::EQUAL); break;
        case TokenType::GREATER:       emitByte(OpCode::GREATER); break;
        case TokenType::GREATER_EQ:   emitBytes(OpCode::LESS, OpCode::NOT); break;
        case TokenType::LESS:          emitByte(OpCode::LESS); break;
        case TokenType::LESS_EQ:      emitBytes(OpCode::GREATER, OpCode::NOT); break;
        case TokenType::LSHIFT:        emitByte(OpCode::LEFTSHIFT); break;
        case TokenType::RSHIFT:        emitByte(OpCode::RIGHTSHIFT); break;
        default: return; // Unreachable
    }
    // clang-format on
}

void Compiler::literal() {
    // clang-format off
    switch (parser.previous.type) {
        case TokenType::FALSE: emitByte(OpCode::FALSE); break;
        case TokenType::TRUE:  emitByte(OpCode::TRUE); break;
        case TokenType::NONE:  emitByte(OpCode::NONE); break;
        default: return; // Unreachable
    }
    // clang-format on
}

void Compiler::string() {
    ObjString* str = ObjString::create(parser.previous.lexeme, {1, -1});
    emitConstant(Value::objectVal(str));
}

void Compiler::number() {
    if (parser.previous.lexeme == "nan") {
        emitConstant(Value::nan(true));
        return;
    } else if (parser.previous.lexeme == "inf") {
        emitConstant(Value::infinity(true));
        return;
    }

    if (parser.previous.lexeme.find('.') != std::string::npos) {
        double value = std::stod(parser.previous.lexeme);
        emitConstant(Value::doubleVal(value));
    } else {
        int value = std::stoi(parser.previous.lexeme);
        emitConstant(Value::integerVal(value));
    }
}

void Compiler::grouping() {
    expression();
    consume(TokenType::RPAREN, "Expected ')' after expression.");
}


bool Compiler::compile(std::string source) {
    scanner = std::make_unique<Scanner>(source);

    advance();
    expression();
    consume(TokenType::EOF, "Expected end of expression.");

    endCompiler();
    return !parser.hadError;
}
