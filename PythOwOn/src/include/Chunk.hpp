#ifndef CHUNK_HPP
#define CHUNK_HPP

//#include "value.hpp"
#include <vector>
#include <string>

class Chunk {
private:
    int count;
    std::vector<OpCode> code;
    std::vector<int> lines;
    //ValueArray constants;

public:
    Chunk();
    ~Chunk();

    void write(OpCode byte);

#if defined(_DEBUG)
    void disassemble(std::string name);
    void disassembleInstruction(int* offset);
#endif
};

enum class OpCode {
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

#endif
