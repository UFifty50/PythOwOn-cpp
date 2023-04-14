#ifndef COMPILEPIPE_HPP
#define COMPILEPIPE_HPP

#include <string>

#include "Chunk.hpp"
#include "Common.hpp"
#include "Compiler.hpp"
#include "VirtualMachine.hpp"


class CompilationPileline {
private:
    Compiler* compiler;
    VM* vm;

public:
    CompilationPileline();
    ~CompilationPileline();

    InterpretResult interpret(std::string source);
};

#endif
