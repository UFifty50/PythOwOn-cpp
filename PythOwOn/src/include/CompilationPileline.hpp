#ifndef COMPILEPIPE_HPP
#define COMPILEPIPE_HPP

#include <string>

#include "Chunk.hpp"
#include "Common.hpp"
#include "Compiler.hpp"
#include "VirtualMachine.hpp"


class CompilationPileline {
private:
    VM* vm;
    Compiler* compiler;

public:
    CompilationPileline();
    ~CompilationPileline();

    InterpretResult interpret(std::string source);
    bool compile(std::string source, Chunk* chunk);
};

#endif
