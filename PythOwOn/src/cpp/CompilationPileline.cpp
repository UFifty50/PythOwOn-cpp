#include "CompilationPileline.hpp"

#include "Chunk.hpp"
#include "Common.hpp"
#include "Compiler.hpp"
#include "VirtualMachine.hpp"


InterpretResult CompilationPipeline::interpret(const std::string& source) {
    auto [result, chunk] = compile(source);
    if (result == InterpretResult::OK) {
        VM::setChunk(chunk);
        return VM::run();
    }

    return result;
}

std::pair<InterpretResult, std::shared_ptr<Chunk>> CompilationPipeline::compile(
    const std::string& source) {
    auto chunk = std::make_shared<Chunk>();
    compiler = std::make_unique<Compiler>(chunk);

    if (!compiler->compile(source)) {
        return {InterpretResult::COMPILE_ERROR, nullptr};
    }

    return {InterpretResult::OK, chunk};
}

InterpretResult CompilationPipeline::runCompiled(const std::shared_ptr<Chunk>& chunk) {
    VM::setChunk(chunk);
    return VM::run();
}
