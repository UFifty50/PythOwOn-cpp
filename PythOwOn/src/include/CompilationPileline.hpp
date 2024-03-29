#ifndef COMPILEPIPE_HPP
#define COMPILEPIPE_HPP

#include <memory>
#include <string>

#include "Chunk.hpp"
#include "Common.hpp"
#include "Compiler.hpp"
#include "VirtualMachine.hpp"


class CompilationPileline {
public:
    CompilationPileline() : vm(std::make_unique<VM>()) {}

    ~CompilationPileline() = default;

    InterpretResult interpret(std::string source);
    std::pair<InterpretResult, std::shared_ptr<Chunk>> compile(std::string source);
    InterpretResult runCompiled(std::shared_ptr<Chunk> chunk);

private:
    std::unique_ptr<Compiler> compiler;
    std::unique_ptr<VM> vm;
};

#endif
