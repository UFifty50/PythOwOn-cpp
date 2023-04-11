#include <iostream>

#include "ArgumentParser.hpp"
#include "VirtualMachine.hpp"
#include "Chunk.hpp"


void repl(std::string _);

int main(int argc, char** argv) {
    ArgumentParser argParser(argc, argv);

    argParser.registerLongArg("version", {"Display the version of PythOwOn", [](std::string arg) {
                                            std::cout << "PythOwOn 0.0.1" << std::endl;
                                          }
                              });

    argParser.registerLongArg("file", {"Run a PythOwOn file", [](std::string arg) {
                                         std::cout << "not implemented yet" << std::endl;
                                       }
                              });

    argParser.registerLongArg("interactive", {"Start PythOwOn in interactive mode", repl});

    argParser.registerLongArg("compile", {"Compile a PythOwOn file into bytecode.", [](std::string _){}});

   // argParser.aliasLongArg("help", 'h');
    argParser.aliasLongArg("version", 'v');
    argParser.aliasLongArg("file", 'f');
    argParser.aliasLongArg("interactive", 'i');
    argParser.aliasLongArg("compile", 'c');

    argParser.setDefaultHelp();

    argParser.setDefaultBehaviour("interactive");


    argParser.parse();

    return 0;
}

void repl(std::string _) {
    std::string line;
    VM* vm = new VM();

    while (true) {
        FMT_PRINT("PythOwOn <<< ");

        // get line input
        if (!(std::cin >> line)) {
            FMT_PRINT("\n");
            break;
        }

        vm->interpret(line);
    }
    
    /*    VM* vm = new VM();
    Chunk* chunk = new Chunk();

    int constant = chunk->addConstant(1.2);
    chunk->write(OpCode::CONSTANT, 1);
    chunk->write(constant, 1);

    constant = chunk->addConstant(3.4);
    chunk->write(OpCode::CONSTANT, 2);
    chunk->write(constant, 2);

    chunk->write(OpCode::ADD, 2);

    constant = chunk->addConstant(5.6);
    chunk->write(OpCode::CONSTANT, 2);
    chunk->write(constant, 2);

    chunk->write(OpCode::DIVIDE, 2);
    chunk->write(OpCode::NEGATE, 2);
    chunk->write(OpCode::RETURN, 3);

#if defined(TRACE_EXECUTION)
    chunk->disassemble("test chunk");
#endif

    vm->interpret(chunk);
    vm->run();

    delete vm;
    delete chunk; */
}
