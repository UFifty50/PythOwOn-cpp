#include <fstream>
#include <iostream>

#include "ArgumentParser.hpp"
#include "Chunk.hpp"
#include "Common.hpp"
#include "CompilationPileline.hpp"
#include "VirtualMachine.hpp"


uint8_t printVersion(std::string _);
uint8_t repl(std::string _);
uint8_t runFile(std::string path);
uint8_t compileFile(std::string path);

int main(int argc, char** argv) {
    ArgumentParser argParser(argc, argv);

    argParser.registerLongArg("version",
                              {"Display the version of PythOwOn", printVersion});

    argParser.registerLongArg("file", {"Run a PythOwOn file", runFile, true});

    argParser.registerLongArg("interpret", {"Start PythOwOn in interactive mode", repl});

    argParser.registerLongArg(
        "compile", {"Compile a PythOwOn file into bytecode.", compileFile, true});

    argParser.aliasLongArg("file", 'f');
    argParser.aliasLongArg("interpret", 'i');
    argParser.aliasLongArg("compile", 'c');

    argParser.setDefaultHelp();

    argParser.setDefaultBehaviour("i");

    return argParser.parse();
}

uint8_t printVersion(std::string _) {
    std::cout << "PythOwOn 0.0.1" << std::endl;
    return 0;
}

uint8_t repl(std::string _) {
    std::string line;
    CompilationPileline* pipeline = new CompilationPileline();

    while (true) {
        FMT_PRINT("PythOwOn <<< ");

        // TODO: Handle Multiline input
        getline(std::cin, line);
        if (line == "") {
            continue;
        }

        pipeline->interpret(line);
    }

    delete pipeline;
    return 0;
}

uint8_t runFile(std::string path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        FMT_PRINT("Could not open file \"{}\".\n", path);
        return 74;
    }

    std::string source((std::istreambuf_iterator<char>(file)),
                       (std::istreambuf_iterator<char>()));
    file.close();

    CompilationPileline* pipeline = new CompilationPileline();
    InterpretResult result = pipeline->interpret(source);

    delete pipeline;

    if (result == InterpretResult::COMPILE_ERROR) return 65;
    if (result == InterpretResult::RUNTIME_ERROR) return 70;
    return 0;
}

uint8_t compileFile(std::string path) { return 0; }
