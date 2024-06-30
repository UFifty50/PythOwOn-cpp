#include "Compiler.hpp"

#include <functional>
#include <ranges>
#include <string>

#include "Chunk.hpp"
#include "Common.hpp"
#include "Object.hpp"
#include "Scanner.hpp"
#include "Value.hpp"
#include "Utils/Enumerate.hpp"


int32_t g_innermostLoopStart = -1;
uint16_t g_innermostLoopScopeDepth = 0;

template <typename T>
static T g_defaultRef = T();

Compiler::Compiler() : chunk(g_defaultRef<Chunk>), scanner(Scanner("")), parser(Parser()) {
    parser.hadError = false;
    parser.panicMode = false;

    // @formatter:off
    // clang-format off
    //       ParseTable Position        |         prefix        |     infix      |        precedence       |
    rules[TokenType::LPAREN]     = { &Compiler::grouping,  nullptr,             Precedence::CALL       };
    rules[TokenType::RPAREN]     = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::LBRACE]     = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::RBRACE]     = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::LBRACK]     = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::RBRACK]     = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::COMMA]      = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::DOT]        = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::MINUS]      = { &Compiler::unary,     &Compiler::binary,   Precedence::TERM       };
    rules[TokenType::PLUS]       = { nullptr,              &Compiler::binary,   Precedence::TERM       };
    rules[TokenType::PERCENT]    = { nullptr,              &Compiler::binary,   Precedence::FACTOR     };
    rules[TokenType::SEMI]       = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::SLASH]      = { nullptr,              &Compiler::binary,   Precedence::FACTOR     };
    rules[TokenType::STAR]       = { nullptr,              &Compiler::binary,   Precedence::FACTOR     };
    rules[TokenType::COLON]      = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::BANG]       = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::BANG_EQ]    = { nullptr,              &Compiler::binary,   Precedence::EQUALITY   };
    rules[TokenType::EQ]         = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::EQ_EQ]      = { nullptr,              &Compiler::binary,   Precedence::EQUALITY   };
    rules[TokenType::GREATER]    = { nullptr,              &Compiler::binary,   Precedence::COMPARISON };
    rules[TokenType::GREATER_EQ] = { nullptr,              &Compiler::binary,   Precedence::COMPARISON };
    rules[TokenType::LESS]       = { nullptr,              &Compiler::binary,   Precedence::COMPARISON };
    rules[TokenType::LESS_EQ]    = { nullptr,              &Compiler::binary,   Precedence::COMPARISON };
    rules[TokenType::LSHIFT]     = { nullptr,              &Compiler::binary,   Precedence::SHIFT      };
    rules[TokenType::RSHIFT]     = { nullptr,              &Compiler::binary,   Precedence::SHIFT      };
    rules[TokenType::AMPERSAND]  = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::PIPE]       = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::CARET]      = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::IDENTIFIER] = { &Compiler::variable,  nullptr,             Precedence::NONE       };
    rules[TokenType::STR]        = { &Compiler::string,    nullptr,             Precedence::NONE       };
    rules[TokenType::NUM]        = { &Compiler::number,    nullptr,             Precedence::NONE       };
    rules[TokenType::INF]        = { &Compiler::number,    nullptr,             Precedence::NONE       };
    rules[TokenType::NAN]        = { &Compiler::number,    nullptr,             Precedence::NONE       };
    rules[TokenType::AND]        = { nullptr,              &Compiler::and_,     Precedence::AND        };
    rules[TokenType::OR]         = { nullptr,              &Compiler::or_,      Precedence::OR         };
    rules[TokenType::NOT]        = { &Compiler::unary,     nullptr,             Precedence::NONE       };
    rules[TokenType::CLASS]      = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::ELSE]       = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::FALSE]      = { &Compiler::literal,   nullptr,             Precedence::NONE       };
    rules[TokenType::FOR]        = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::DEF]        = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::IF]         = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::NONE]       = { &Compiler::literal,   nullptr,             Precedence::NONE       };
    rules[TokenType::PRINT]      = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::RETURN]     = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::SUPER]      = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::THIS]       = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::TRUE]       = { &Compiler::literal,   nullptr,             Precedence::NONE       };
    rules[TokenType::LET]        = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::WHILE]      = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::EXTENDS]    = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::SWITCH]     = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::CASE]       = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::DEFAULT]    = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::CONTINUE]   = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::BREAK]      = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::IN]         = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::ERROR]      = { nullptr,              nullptr,             Precedence::NONE       };
    rules[TokenType::EOF]        = { nullptr,              nullptr,             Precedence::NONE       };
    // clang-format on
    // @formatter:on
}


void Compiler::advance() {
    parser.previous = parser.current;

    while (true) {
        parser.current = scanner.scanToken();

        if (parser.current.type != TokenType::ERROR) break;

        errorAt(parser.current, parser.current.lexeme);
    }
}

void Compiler::errorAt(Token token, const std::string& message) {
    if (parser.panicMode) return;
    parser.panicMode = true;
    FMT_PRINT("[line {}] Error", token.line);

    if (token.type == TokenType::EOF) { FMT_PRINT(" at end"); }
    else if (token.type == TokenType::ERROR) { // Nothing
    }
    else { FMT_PRINT(" at '{}'", token.lexeme); }

    FMT_PRINT(": {}\n", message);
    parser.hadError = true;
}

ParseRule* Compiler::getRule(const TokenType::Type type) { return &rules[type]; }

void Compiler::consume(const TokenType::Type type, const std::string& message) {
    if (parser.current.type == type) {
        advance();
        return;
    }
    errorAt(parser.current, message);
}

bool Compiler::match(const TokenType::Type type) {
    if (parser.current.type != type) return false;
    advance();
    return true;
}

void Compiler::emitByte(const uint8_t byte) { chunk.write(byte, parser.previous.line); }

void Compiler::emitBytes(const uint8_t byte1, const uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

uint16_t Compiler::emitJump(const OpCode op) {
    emitByte(op);
    emitByte(0xff);
    emitByte(0xff);
    return static_cast<uint16_t>(chunk.code.size()) - 2;
}

uint32_t Compiler::emitJumpLong(const OpCode op) {
    emitByte(op);
    emitByte(0xff);
    emitByte(0xff);
    emitByte(0xff);
    emitByte(0xff);
    return static_cast<uint32_t>(chunk.code.size()) - 4;
}

void Compiler::emitConstant(const Value value) { chunk.writeConstant(value, parser.previous.line); }

void Compiler::patchJump(const int32_t offset) {
    // -2 to adjust for the bytecode for the jump offset itself.
    const int32_t jump = static_cast<int32_t>(chunk.code.size()) - offset - 2;

    if (jump > UINT16_MAX) {
        errorAt(parser.previous,
                "Too much code to jump over. Why wasn't patchJumpLong used?");
    }

    chunk.code[offset] = jump >> 8 & 0xff;
    chunk.code[offset + 1] = jump & 0xff;
}

void Compiler::patchJumpLong(const uint32_t offset) {
    const uint32_t jump = static_cast<uint32_t>(chunk.code.size()) - offset - 4;

    if (jump > UINT32_MAX) { errorAt(parser.previous, "Too much code to jump over."); }

    chunk.code[offset] = jump >> 24 & 0xff;
    chunk.code[offset + 1] = jump >> 16 & 0xff;
    chunk.code[offset + 2] = jump >> 8 & 0xff;
    chunk.code[offset + 3] = jump & 0xff;
}

void Compiler::emitVariable(const OpCode op, const uint32_t var) {
    chunk.writeVariable(op, var, parser.previous.line);
}

void Compiler::emitLoop(const uint16_t loopStart) {
    emitByte(OpCode::LOOP);

    const uint16_t offset = static_cast<uint16_t>(chunk.code.size()) - loopStart + 2;
    if (offset > UINT16_MAX) { errorAt(parser.previous, "Loop body too large."); }

    emitByte(offset >> 8 & 0xff);
    emitByte(offset & 0xff);
}

void Compiler::emitReturn() { emitByte(OpCode::RETURN); }

void Compiler::endCompiler() {
    emitReturn();

#if defined(TRACE_EXECUTION)
    if (!parser.hadError) { chunk.disassemble("code"); }
#endif
}


// makeConstant (now not needed thanks to Chunk::writeConstant)
/*
uint8_t Compiler::makeConstant(Value value) {
    size_t constant = chunk.addConstant(value);
    if (constant > UINT8_MAX) {
        errorAt(parser.previous, "Too many constants in one chunk.");
        return 0;
    }
    return (uint8_t)constant;
} */

void Compiler::parsePrecedence(const Precedence precedence) {
    advance();
    const auto& prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == nullptr) {
        errorAt(parser.previous, "Expected expression.");
        return;
    }

    const bool canAssign = precedence <= Precedence::ASSIGNMENT;
    prefixRule(this, canAssign);

    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();
        auto& infixRule = getRule(parser.previous.type)->infix;
        infixRule(this, canAssign);
    }

    if (canAssign && match(TokenType::EQ)) {
        errorAt(parser.previous, "Invalid assignment target.");
    }
}

uint32_t Compiler::identifierConstant(const Token* name) {
    const ObjString* str = ObjString::Create(name->lexeme);
    const Value objVal = Value::ObjectVal(str);
    return chunk.addConstant(objVal);
}

std::optional<uint32_t> Compiler::resolveLocal(const Token& name) {
    for (auto&& [index, local] : enumerate(state.locals).reverse()) {
        if (name.lexeme == local.name.lexeme) {
            if (local.depth == -1) {
                errorAt(name, "Cannot read local variable in its own initializer.");
            }
            return index;
        }
    }

    return std::nullopt;
}

uint32_t Compiler::parseVariable(const std::string& errorMessage) {
    consume(TokenType::IDENTIFIER, errorMessage);

    declareVariable();
    if (state.scopeDepth > 0) return 0;

    return identifierConstant(&parser.previous);
}

void Compiler::markInitialized() {
    if (state.scopeDepth == 0) return;

    state.locals.back().depth = static_cast<int32_t>(state.scopeDepth);
}

void Compiler::defineVariable(const uint32_t global) {
    if (state.scopeDepth > 0) {
        markInitialized();
        return;
    }

    emitVariable(OpCode::DEF_GLOBAL, global);
}

void Compiler::declareVariable() {
    if (state.scopeDepth == 0) return;

    const Token* name = &parser.previous;

    for (const auto& [localName, localDepth] : state.locals | std::views::reverse) {
        if (localDepth != -1 && static_cast<unsigned>(localDepth) < state.scopeDepth) break;

        if (name->lexeme == localName.lexeme) {
            errorAt(*name, "Already variable with this name in this scope.");
        }
    }

    if (state.locals.size() >= UINT32_MAX) {
        errorAt(parser.previous, "Too many local variables in function.");
        return;
    }

    state.addLocal(*name);
}

void Compiler::namedVariable(const Token& name, const bool canAssign) {
    OpCode getOp, setOp;
    std::optional<uint32_t> arg = resolveLocal(name);

    if (arg) {
        getOp = OpCode::GET_LOCAL;
        setOp = OpCode::SET_LOCAL;
    }
    else {
        arg = identifierConstant(&name);
        getOp = OpCode::GET_GLOBAL;
        setOp = OpCode::SET_GLOBAL;
    }

    if (canAssign && match(TokenType::EQ)) {
        expression();
        emitVariable(setOp, *arg);
    }
    else { emitVariable(getOp, *arg); }
}

void Compiler::and_(bool) {
    const uint16_t endJump = emitJump(OpCode::JUMP_FALSE);

    emitByte(OpCode::POP);
    parsePrecedence(Precedence::AND);

    patchJump(endJump);
}

void Compiler::or_(bool) {
    const uint16_t elseJump = emitJump(OpCode::JUMP_FALSE);
    const uint16_t endJump = emitJump(OpCode::JUMP);

    patchJump(elseJump);
    emitByte(OpCode::POP);

    parsePrecedence(Precedence::OR);
    patchJump(endJump);
}

void Compiler::expression() { parsePrecedence(Precedence::ASSIGNMENT); }

void Compiler::expressionStatement() {
    expression();
    consume(TokenType::SEMI, "Expected ';' after expression.");
    emitByte(OpCode::POP);
}

void Compiler::printStatement() {
    expression();
    consume(TokenType::SEMI, "Expected ';' after value.");
    emitByte(OpCode::PRINT);
}

void Compiler::ifStatement() {
    consume(TokenType::LPAREN, "Expected '(' after 'if'.");
    expression();
    consume(TokenType::RPAREN, "Expected ')' after condition.");

    const uint16_t ifJump = emitJump(OpCode::JUMP_FALSE);
    emitByte(OpCode::POP);
    statement();

    const uint16_t elseJump = emitJump(OpCode::JUMP);

    patchJump(ifJump);
    emitByte(OpCode::POP);

    if (match(TokenType::ELSE)) statement();
    patchJump(elseJump);
}

void Compiler::switchStatement() {
    enum class SwitchState { PRE_CASE, PRE_DEFAULT, DONE };

    consume(TokenType::LPAREN, "Expected '(' after 'switch'.");
    expression();
    consume(TokenType::RPAREN, "Expected ')' after condition.");
    consume(TokenType::LBRACE, "Expected '{' before switch cases.");

    auto switchState = SwitchState::PRE_CASE;
    std::array<uint16_t, 256> caseEnds{};
    uint16_t caseCount = 0;
    int16_t previousCaseSkip = -1;


    while (!match(TokenType::RBRACE) && parser.current.type != TokenType::EOF) {
        if (match(TokenType::CASE) || match(TokenType::DEFAULT)) {
            TokenType caseType = parser.previous.type;

            if (switchState == SwitchState::DONE) {
                errorAt(parser.previous,
                        "Can't have another case or default after the default case.");
            }

            if (switchState == SwitchState::PRE_DEFAULT) {
                caseEnds[caseCount++] = emitJump(OpCode::JUMP);

                patchJump(previousCaseSkip);
                emitByte(OpCode::POP);
            }

            if (caseType == TokenType::CASE) {
                switchState = SwitchState::PRE_DEFAULT;

                emitByte(OpCode::DUP);
                expression();

                consume(TokenType::COLON, "Expected ':' after case value.");

                emitByte(OpCode::EQUAL);
                previousCaseSkip = static_cast<int16_t>(emitJump(OpCode::JUMP_FALSE));

                emitByte(OpCode::POP);
            }
            else {
                switchState = SwitchState::DONE;
                consume(TokenType::COLON, "Expected ':' after 'default'.");
                previousCaseSkip = -1;
            }
        }
        else {
            if (switchState == SwitchState::PRE_CASE) {
                errorAt(parser.previous,
                        "Can't have a default case without a case before it.");
            }
            beginScope();
            while (parser.current.type != TokenType::CASE &&
                parser.current.type != TokenType::DEFAULT &&
                parser.current.type != TokenType::RBRACE &&
                parser.current.type != TokenType::EOF) { declaration(); }
            endScope();
            if (match(TokenType::RBRACE)) advance();
        }
    }

    if (caseCount < 1) { errorAt(parser.previous, "Switch statement must have more than 1 case."); }

    if (switchState == SwitchState::PRE_DEFAULT) {
        patchJump(previousCaseSkip);
        emitByte(OpCode::POP);
    }

    for (uint16_t i = 0; i < caseCount; i++) { patchJump(caseEnds[i]); }

    emitByte(OpCode::POP);
}

void Compiler::whileStatement() {
    const uint16_t loopStart = static_cast<uint16_t>(chunk.code.size());

    consume(TokenType::LPAREN, "Expected '(' after 'while'.");
    expression();
    consume(TokenType::RPAREN, "Expected ')' after condition.");

    const uint16_t exitJump = emitJump(OpCode::JUMP_FALSE);

    emitByte(OpCode::POP);
    statement();

    emitLoop(loopStart);

    patchJump(exitJump);
    emitByte(OpCode::POP);
}

void Compiler::forStatement() {
    beginScope();

    consume(TokenType::LPAREN, "Expected '(' after 'for'.");
    if (match(TokenType::SEMI)) {
        // No initializer.
    }
    else if (match(TokenType::LET)) { varDeclaration(); }
    else { expressionStatement(); }

    const int32_t surroundingLoopStart = g_innermostLoopStart;
    const uint16_t surroundingLoopScopeDepth = g_innermostLoopScopeDepth;
    g_innermostLoopStart = static_cast<int32_t>(chunk.code.size());
    g_innermostLoopScopeDepth = static_cast<uint16_t>(state.scopeDepth);

    int32_t exitJump = -1;
    if (!match(TokenType::SEMI)) {
        expression();
        consume(TokenType::SEMI, "Expected ';' after loop condition.");

        exitJump = emitJump(OpCode::JUMP_FALSE);
        emitByte(OpCode::POP);
    }

    if (!match(TokenType::RPAREN)) {
        const uint16_t bodyJump = emitJump(OpCode::JUMP);
        const uint16_t incrementStart = static_cast<uint16_t>(chunk.code.size());
        expression();
        emitByte(OpCode::POP);
        consume(TokenType::RPAREN, "Expected ')' after for clauses.");

        emitLoop(static_cast<uint16_t>(g_innermostLoopStart));
        g_innermostLoopStart = incrementStart;
        patchJump(bodyJump);
    }

    statement();

    emitLoop(static_cast<uint16_t>(g_innermostLoopStart));

    if (exitJump != -1) {
        patchJump(exitJump);
        emitByte(OpCode::POP);
    }

    g_innermostLoopStart = surroundingLoopStart;
    g_innermostLoopScopeDepth = surroundingLoopScopeDepth;

    endScope();
}

void Compiler::continueStatement() {
    if (g_innermostLoopStart == -1) {
        errorAt(parser.previous, "Cannot continue outside of a loop.");
    }

    consume(TokenType::SEMI, "Expected ';' after continue.");

    for (auto& [_, localDepth] : state.locals | std::views::reverse) {
        if (localDepth <= g_innermostLoopScopeDepth) break;

        emitByte(OpCode::POP);
    }

    emitLoop(static_cast<uint16_t>(g_innermostLoopStart));
}

void Compiler::breakStatement() { consume(TokenType::SEMI, "Expected ';' after break."); }
// TODO: implement me

void Compiler::statement() {
    if (match(TokenType::PRINT)) {
        printStatement(); // TODO: move to native function
    }
    else if (match(TokenType::IF)) { ifStatement(); }
    else if (match(TokenType::LBRACE)) {
        beginScope();
        block();
        endScope();
    }
    else if (match(TokenType::SWITCH)) {
        switchStatement(); // TODO: make me work
    }
    else if (match(TokenType::WHILE)) { whileStatement(); }
    else if (match(TokenType::FOR)) { forStatement(); }
    else if (match(TokenType::CONTINUE)) { continueStatement(); }
    else if (match(TokenType::BREAK)) { breakStatement(); }
    else if (match(TokenType::RETURN)) {
        if (match(TokenType::SEMI)) { emitReturn(); }
        else {
            expression();
            consume(TokenType::SEMI, "Expected ';' after return value.");
            emitByte(OpCode::RETURN);
        }
    }
    else { expressionStatement(); }
}

void Compiler::declaration() {
    if (match(TokenType::LET)) { varDeclaration(); }
    else { statement(); }

    if (parser.panicMode) panicSync();
}

void Compiler::varDeclaration() {
    const uint32_t var = parseVariable("Expected variable name.");

    if (match(TokenType::EQ)) { expression(); }
    else { emitByte(OpCode::NONE); }

    consume(TokenType::SEMI, "Expected ';' after variable declaration.");

    defineVariable(var);
}

void Compiler::block() {
    while (parser.current.type != TokenType::RBRACE &&
        parser.current.type != TokenType::EOF) { declaration(); }

    consume(TokenType::RBRACE, "Expected '}' after block.");
}

void Compiler::beginScope() { state.scopeDepth++; }

void Compiler::endScope() {
    state.scopeDepth--;

    while (!state.locals.empty() &&
        static_cast<uint32_t>(state.locals.back().depth) > state.scopeDepth) {
        emitByte(OpCode::POP);
        state.locals.pop_back();
    }
}

void Compiler::panicSync() {
    parser.panicMode = false;

    while (parser.current.type != TokenType::EOF) {
        if (parser.previous.type == TokenType::SEMI) return;

        switch (parser.current.type) {
            case TokenType::CLASS:
            case TokenType::DEF:
            case TokenType::LET:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::PRINT:
            case TokenType::RETURN: return;

            default: ; // Do nothing.
        }

        advance();
    }
}

void Compiler::unary(bool) {
    const TokenType::Type operatorType = parser.previous.type;

    parsePrecedence(Precedence::UNARY);

    // @formatter:off
    // clang-format off
    switch (operatorType) {
        case TokenType::NOT: emitByte(OpCode::NOT); break;
        case TokenType::MINUS: emitByte(OpCode::NEGATE); break;
        default: return; // Unreachable.
    }
    // clang-format on
    // @formatter:on
}

void Compiler::binary(bool) {
    const TokenType::Type operatorType = parser.previous.type;
    ParseRule* rule = getRule(operatorType);
    parsePrecedence(static_cast<Precedence>(static_cast<size_t>(rule->precedence) + 1));

    // @formatter:off
    // clang-format off
    switch (operatorType) {
        case TokenType::MINUS:      emitBytes(OpCode::NEGATE, OpCode::ADD);  break;
        case TokenType::PLUS:       emitByte(OpCode::ADD);                   break;
        case TokenType::SLASH:      emitByte(OpCode::DIVIDE);                break;
        case TokenType::STAR:       emitByte(OpCode::MULTIPLY);              break;
        case TokenType::BANG_EQ:    emitBytes(OpCode::EQUAL, OpCode::NOT);   break;
        case TokenType::EQ_EQ:      emitByte(OpCode::EQUAL);                 break;
        case TokenType::GREATER:    emitByte(OpCode::GREATER);               break;
        case TokenType::GREATER_EQ: emitBytes(OpCode::LESS, OpCode::NOT);    break;
        case TokenType::LESS:       emitByte(OpCode::LESS);                  break;
        case TokenType::LESS_EQ:    emitBytes(OpCode::GREATER, OpCode::NOT); break;
        case TokenType::LSHIFT:     emitByte(OpCode::LEFTSHIFT);             break;
        case TokenType::RSHIFT:     emitByte(OpCode::RIGHTSHIFT);            break;
        case TokenType::PERCENT:    emitByte(OpCode::MODULO);                break;
        default: return; // Unreachable
    }
    // clang-format on
    // @formatter:on
}

void Compiler::literal(bool) {
    // @formatter:off
    // clang-format off
    switch (parser.previous.type) {
        case TokenType::FALSE: emitByte(OpCode::FALSE); break;
        case TokenType::TRUE:  emitByte(OpCode::TRUE);  break;
        case TokenType::NONE:  emitByte(OpCode::NONE);  break;
        default: return; // Unreachable
    }
    // clang-format on
    // @formatter:on
}

void Compiler::string(bool) {
    const ObjString* str = ObjString::Create(parser.previous.lexeme, {1, -1});
    emitConstant(Value::ObjectVal(str));
}

void Compiler::variable(const bool canAssign) { namedVariable(parser.previous, canAssign); }

void Compiler::number(bool) {
    if (parser.previous.lexeme == "Nan") {
        emitConstant(Value::Nan(true));
        return;
    }

    if (parser.previous.lexeme == "inf") {
        emitConstant(Value::Infinity(true));
        return;
    }

    if (parser.previous.lexeme.find('.') != std::string::npos) {
        const double value = std::stod(parser.previous.lexeme);
        emitConstant(Value::DoubleVal(value));
    }
    else {
        const int value = std::stoi(parser.previous.lexeme);
        emitConstant(Value::IntegerVal(value));
    }
}

void Compiler::grouping(bool) {
    expression();
    consume(TokenType::RPAREN, "Expected ')' after expression.");
}


std::pair<InterpretResult, Chunk> Compiler::compile(const std::string& source) {
    chunk = Chunk();
    scanner = Scanner(source);

    advance();

    while (!match(TokenType::EOF)) { declaration(); }

    endCompiler();

    InterpretResult result = parser.hadError ? InterpretResult::COMPILE_ERROR : InterpretResult::OK;
    return {result, chunk};
}
