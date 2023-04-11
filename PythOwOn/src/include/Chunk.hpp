#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <vector>
#include <string>
#include <variant>

#include "Value.hpp"


enum OpCode {
    CONSTANT,
    CONSTANT_LONG,
    NONE,
    TRUE,
    FALSE,
    POP,
    GET_LOCAL,
    SET_LOCAL,
    GET_GLOBAL,
    DEF_GLOBAL,
    SET_GLOBAL,
    EQUAL,
    GREATER,
    LESS,
    ADD,
    MULTIPLY,
    DIVIDE,
    NOT,
    LEFTSHIFT,
    RIGHTSHIFT,
    MODULO,
    NEGATE,
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

class Chunk {
private:
    std::vector<int> lines;

public:
    Chunk();
    ~Chunk();

    void write(uint8_t byte, int line);
    int addConstant(Value value);
    void writeConstant(Value value, int line);

    std::vector<uint8_t> code;
    ValueArray constants;

#if defined(TRACE_EXECUTION)
    void disassemble(std::string name);
    int disassembleInstruction(int offset);
#endif
};

#endif
