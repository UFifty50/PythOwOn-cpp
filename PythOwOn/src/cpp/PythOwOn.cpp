#include <iostream>
#include "VirtualMachine.hpp"
#include "Chunk.hpp"


int main(int argc, char** argv) {
    Chunk* chunk = new Chunk();
    chunk->write(OpCode::CALL);

#if defined(_DEBUG)
    chunk->disassemble("test chunk");
#endif

    delete chunk;
    return 0;
}
