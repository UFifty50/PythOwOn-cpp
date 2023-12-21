#include <cxxopts.hpp>

#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>

#include "Chunk.hpp"
#include "Common.hpp"
#include "CompilationPileline.hpp"
#include "Value.hpp"
#include "VirtualMachine.hpp"


uint8_t printVersion();
uint8_t repl();
uint8_t runFile(std::string path);
uint8_t compileFile(std::string path, std::string outFile);


int main(int argc, char** argv) {
    cxxopts::Options options("PythOwOn", "A simple programming language.");

    options.add_option("", {"file", "", cxxopts::value<std::string>()});
    options.add_option("", {"r,run", "Runs a given PythOwOn file."});
    options.add_option("", {"c,compile", "Compile a PythOwOn file into bytecode."});
    options.add_option("", {"o,output", "Output file for compiled bytecode.",
                            cxxopts::value<std::string>()});

    options.add_option("", {"i,interpret", "Start PythOwOn in interactive mode"});
    options.add_option("", {"h,help", "Print usage"});
    options.add_option("", {"v,version", "Display the version of PythOwOn"});

    options.parse_positional({"file"});

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (result.count("version")) return printVersion();
    if (result.count("interpret")) return repl();

    if (result.count("run")) {
        if (result.count("file") == 0) {
            std::cout << "You must provide a file to run." << std::endl;
            return 1;
        }

        return runFile(result["file"].as<std::string>());
    }

    if (result.count("compile")) {
        if (result.count("file") == 0) {
            std::cout << "You must provide a file to compile." << std::endl;
            return 1;
        }

        if (result.count("output") == 0) {
            std::cout << "You must provide a output file." << std::endl;
            return 1;
        }

        return compileFile(result["file"].as<std::string>(),
                           result["output"].as<std::string>());
    }

    std::cout << options.help() << std::endl;
    return 0;
}

uint8_t printVersion() {
    std::cout << "PythOwOn 0.0.1" << std::endl;
    return 0;
}

bool isIncomplete(std::string& line) {
    int openBrackets = 0;
    int openParentheses = 0;
    int openBraces = 0;
    bool openTripleQuotes = false;

    for (size_t i = 0; i < line.size() - 2; i++) {
        if (line[i] == '"' && line[i + 1] == '"' && line[i + 2] == '"') {
            openTripleQuotes = !openTripleQuotes;
            i += 2;
        }
    }
    if (openTripleQuotes) return true;

    for (auto it = line.begin(); it != line.end(); it++) {
        if (*it == '#') break;
        if (*it == '"') {
            while ((it + 1) != line.end() && *(it++) != '"') it++;
            continue;
        }
        if (*it == '[') openBrackets++;
        if (*it == ']') openBrackets--;
        if (*it == '(') openParentheses++;
        if (*it == ')') openParentheses--;
        if (*it == '{') openBraces++;
        if (*it == '}') openBraces--;
    }

    return openBrackets > 0 || openParentheses > 0 || openBraces > 0 || openTripleQuotes;
}

uint8_t repl() {
    std::string line;
    std::string tmp;
    CompilationPileline* pipeline = new CompilationPileline();

    while (true) {
        FMT_PRINT("PythOwOn <<< ");

        // TODO: Handle Multiline input
        getline(std::cin, tmp);
        line += tmp + '\n';
        while (isIncomplete(line)) {
            FMT_PRINT("         ... ");
            getline(std::cin, tmp);
            line += tmp + '\n';
        }

        pipeline->interpret(line);
        line = "";
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

    /*std::string magic;
    file.read(magic.data(), 16);

    bool isCompiled = magic == "POWON\0\0\0\0\0\0\0\0\0\0\0";*/

    std::stringstream ss;
    ss << file.rdbuf();
    std::string source = ss.str();

    CompilationPileline* pipeline = new CompilationPileline();
    InterpretResult result = pipeline->interpret(source);

    delete pipeline;

    if (result == InterpretResult::COMPILE_ERROR) return 65;
    if (result == InterpretResult::RUNTIME_ERROR) return 70;

    return 0;

    // if (isCompiled) {
    //     // read 4 bytes for number of constants, 4 bytes for number of strings, 4 bytes
    //     // for number of globals
    //     uint32_t numConstants, numStrings, numGlobals;
    //     file.read(reinterpret_cast<char*>(&numConstants), 4);
    //     file.read(reinterpret_cast<char*>(&numStrings), 4);
    //     file.read(reinterpret_cast<char*>(&numGlobals), 4);

    //    // read constants
    //    std::vector<Value> constants;
    //    for (uint32_t i = 0; i < numConstants; i++) {
    //        //     constants.push_back(Value::read(file));
    //    }
    //}


    // CompilationPileline* pipeline = new CompilationPileline();
}

uint8_t compileFile(std::string path, std::string outFile) {
    std::ifstream file(path);
    if (!file.is_open()) {
        FMT_PRINT("Could not open file \"{}\".\n", path);
        return 74;
    }

    std::string source((std::istreambuf_iterator<char>(file)),
                       (std::istreambuf_iterator<char>()));
    file.close();

    CompilationPileline* pipeline = new CompilationPileline();
    auto [result, chunk] = pipeline->compile(source);

    delete pipeline;

    if (result == InterpretResult::COMPILE_ERROR) return 65;

    std::ofstream out(outFile);
    if (!out.is_open()) {
        FMT_PRINT("Could not open file \"{}\".\n", outFile);
        return 74;
    }

    /*out << chunk->lines;
    out << chunk->code;
    out << chunk->constants;*/
    out.close();

    return 0;
}
