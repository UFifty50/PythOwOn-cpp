#ifndef COMPILATIONPIPELINE_HPP
#define COMPILATIONPIPELINE_HPP

#include <memory>
#include <string>
#include <utility>
#include <xstring>

#include "Chunk.hpp"
#include "Common.hpp"
#include "Compiler.hpp"

class Chunk;


class CompilationPipeline {
public:
    CompilationPipeline() = default;

    InterpretResult interpret(const std::string& source);
    std::pair<InterpretResult, std::shared_ptr<Chunk>> compile(const std::string& source);
    static [[nodiscard]] InterpretResult runCompiled(const std::shared_ptr<Chunk>& chunk);

private:
    std::unique_ptr<Compiler> compiler;
};

#endif
