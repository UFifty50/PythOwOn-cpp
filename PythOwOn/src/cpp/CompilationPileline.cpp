#include "CompilationPileline.hpp"

#include "Chunk.hpp"
#include "Common.hpp"
#include "Compiler.hpp"
#include "VirtualMachine.hpp"


InterpretResult CompilationPileline::interpret(std::string source) {
    std::shared_ptr<Chunk> chunk = std::make_shared<Chunk>();
    compiler = std::make_unique<Compiler>(chunk);

    if (!compiler->compile(source)) {
        return InterpretResult::COMPILE_ERROR;
    }

    vm->setChunk(chunk);

    InterpretResult result = vm->run();
    return result;
}
