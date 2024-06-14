#ifndef COMPILATIONPIPELINE_HPP
#define COMPILATIONPIPELINE_HPP

#include <memory>
#include <string>

#include "Chunk.hpp"
#include "Common.hpp"
#include "Compiler.hpp"
#include "VirtualMachine.hpp"


class CompilationPipeline {
public:
    CompilationPipeline() : vm(std::make_unique<VM>()) {}

    InterpretResult interpret(const std::string& source);
    std::pair<InterpretResult, std::shared_ptr<Chunk>> compile(const std::string& source);
    [[nodiscard]] InterpretResult runCompiled(const std::shared_ptr<Chunk>& chunk) const;

private:
    std::unique_ptr<Compiler> compiler;
    std::unique_ptr<VM> vm;
};

#endif
