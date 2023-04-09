#include "Chunk.hpp"


Chunk::Chunk() {
    count = 0;
    code = {};
    lines = {};
}

Chunk::~Chunk() = default;

void Chunk::write(OpCode byte) {
    code.emplace_back(byte);
    count++;
}

#if defined(_DEBUG)

#include <iostream>


void Chunk::disassemble(std::string name) {
    fmt::printt("== {} ==\n", name);
    for (int offset = 0; offset < count;) {
        offset = disassembleInstruction(&offset);
    }
}

#endif
