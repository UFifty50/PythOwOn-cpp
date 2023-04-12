#include "CompilationPileline.hpp"
#include "VirtualMachine.hpp"
#include "Common.hpp"
#include "Chunk.hpp"


CompilationPileline::CompilationPileline() {
    compiler = new Compiler();
    vm = new VM();
}

CompilationPileline::~CompilationPileline() {
    delete compiler;
    delete vm;
}

InterpretResult CompilationPileline::interpret(std::string source) {
    Chunk* chunk = new Chunk();

    if (!compile(source, chunk)) {
        delete chunk;
        return InterpretResult::COMPILE_ERROR;
    }

    vm->setChunk(chunk);

    InterpretResult result = vm->run();

    delete chunk;
    return result;
}

bool CompilationPileline::compile(std::string source, Chunk* chunk) {
    return compiler->compile(source, chunk);
}
