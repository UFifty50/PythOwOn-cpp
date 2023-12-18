#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <string>
#include <variant>
#include <vector>

#include "Value.hpp"


enum OpCode;
class Chunk {
public:
    Chunk() = default;
    ~Chunk() = default;

    void write(uint8_t byte, size_t line);
    uint32_t addConstant(Value value);
    void writeConstant(Value value, size_t line);
    void writeGlobal(OpCode setOrGet, uint32_t global, size_t line);

    std::vector<size_t> lines;
    std::vector<uint8_t> code;
    std::vector<Value> constants;

#if defined(TRACE_EXECUTION)
    void disassemble(std::string name);
    size_t disassembleInstruction(size_t offset);
#endif
};

enum OpCode {
    CONSTANT,
    CONSTANT_LONG,
    NONE,
    TRUE,
    FALSE,
    POP,
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

#endif
