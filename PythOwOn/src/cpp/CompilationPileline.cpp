#include "CompilationPileline.hpp"
#include "VirtualMachine.hpp"
#include "Common.hpp"
#include "Compiler.hpp"
#include "Chunk.hpp"


CompilationPileline::CompilationPileline() {
    compiler = nullptr;
    vm = new VM();
}

CompilationPileline::~CompilationPileline() {
    delete vm;
}

InterpretResult CompilationPileline::interpret(std::string source) {
    Chunk* chunk = new Chunk();
    compiler = new Compiler(chunk);

    if (!compiler->compile(source)) {
        delete chunk;
        delete compiler;
        return InterpretResult::COMPILE_ERROR;
    }

    vm->setChunk(chunk);

    InterpretResult result = vm->run();

    delete chunk;
    delete compiler;
    return result;
}
