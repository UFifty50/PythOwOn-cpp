#include "CompilationPileline.hpp"

#include "Chunk.hpp"
#include "Common.hpp"
#include "Compiler.hpp"
#include "VirtualMachine.hpp"


InterpretResult CompilationPileline::interpret(std::string source) {
    auto [result, chunk] = compile(source);
    if (result == InterpretResult::OK) {
        vm->setChunk(chunk);
        return vm->run();
    }

    return result;
}

std::pair<InterpretResult, std::shared_ptr<Chunk>> CompilationPileline::compile(
    std::string source) {
    std::shared_ptr<Chunk> chunk = std::make_shared<Chunk>();
    compiler = std::make_unique<Compiler>(chunk);

    if (!compiler->compile(source)) {
        return {InterpretResult::COMPILE_ERROR, nullptr};
    }

    return {InterpretResult::OK, chunk};
}

InterpretResult CompilationPileline::runCompiled(std::shared_ptr<Chunk> chunk) {
    vm->setChunk(chunk);
    return vm->run();
}
