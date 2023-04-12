#include <iostream>
#include <fstream>

#include "ArgumentParser.hpp"
#include "VirtualMachine.hpp"
#include "CompilationPileline.hpp"
#include "Common.hpp"
#include "Chunk.hpp"


void repl(std::string _);
void runFile(std::string path);

int main(int argc, char** argv) {
    ArgumentParser argParser(argc, argv);

    argParser.registerLongArg("version", {"Display the version of PythOwOn", [](std::string arg) {
                                            std::cout << "PythOwOn 0.0.1" << std::endl;
                                          }
                              });

    argParser.registerLongArg("file", {"Run a PythOwOn file", runFile, true});

    argParser.registerLongArg("interpret", {"Start PythOwOn in interactive mode", repl});

    argParser.registerLongArg("compile", {"Compile a PythOwOn file into bytecode.", [](std::string _){}, true});

    argParser.aliasLongArg("file", 'f');
    argParser.aliasLongArg("interpret", 'i');
    argParser.aliasLongArg("compile", 'c');

    argParser.setDefaultHelp();

    argParser.setDefaultBehaviour("i");

    argParser.parse();

    return 0;
}

void repl(std::string _) {
    std::string line;
    CompilationPileline* pipeline = new CompilationPileline();

    while (true) {
        FMT_PRINT("PythOwOn <<< ");

        // get line input
        if (!getline(std::cin, line)) {
            FMT_PRINT("\n");
            break;
        }

        pipeline->interpret(line);
    }

    delete pipeline;
}

static void runFile(std::string path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        FMT_PRINT("Could not open file \"{}\".\n", path);
        exit(74);
    }
    
    std::string source((std::istreambuf_iterator<char>(file)),
                  (std::istreambuf_iterator<char>()));
    file.close();

    CompilationPileline* pipeline = new CompilationPileline();
    InterpretResult result = pipeline->interpret(source);

    if (result == InterpretResult::COMPILE_ERROR) exit(65);
    if (result == InterpretResult::RUNTIME_ERROR) exit(70);

    delete pipeline;
}
