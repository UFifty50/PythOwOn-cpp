#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <string>
#include <variant>
#include <vector>

#include "Value.hpp"


class OpCode {
public:
    enum Code : uint8_t {
        CONSTANT,
        CONSTANT_LONG,
        NONE,
        TRUE,
        FALSE,
        POP,
        POPN,  // TODO: implement (pops n values from stack, useful for end of scope)
        GET_LOCAL,
        GET_LOCAL_LONG,
        SET_LOCAL,
        SET_LOCAL_LONG,
        GET_GLOBAL,
        GET_GLOBAL_LONG,
        DEF_GLOBAL,
        DEF_GLOBAL_LONG,
        SET_GLOBAL,
        SET_GLOBAL_LONG,
        EQUAL,
        GREATER,
        LESS,
        ADD,
        MULTIPLY,
        DIVIDE,
        LEFTSHIFT,
        RIGHTSHIFT,
        MODULO,
        NEGATE,
        AND,
        OR,
        NOT,
        PRINT,
        JUMP,
        JUMP_FALSE,
        JUMP_LONG,
        JUMP_FALSE_LONG,
        LOOP,
        LOOP_LONG,
        DUP,
        CALL,
        RETURN,
    };

    constexpr OpCode(const Code code) : code(code) {}
    constexpr OpCode(uint32_t code) : code(static_cast<Code>(code)) {}
    constexpr OpCode() : code(NONE) {}

    constexpr operator Code() const { return code; }
    constexpr operator uint8_t() const { return code; }

private:
    Code code;
};

class Chunk {
public:
    Chunk() = default;

    void write(uint8_t byte, size_t line);
    uint32_t addConstant(Value value);
    void writeConstant(Value value, size_t line);
    void writeVariable(OpCode::Code op, uint32_t var, size_t line);

    std::vector<size_t> lines;
    std::vector<uint8_t> code;
    std::vector<Value> constants;

#if defined(TRACE_EXECUTION)
    void disassemble(std::string name);
    size_t disassembleInstruction(size_t offset);
#endif
};

#endif
