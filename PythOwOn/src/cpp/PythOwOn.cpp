#include <cxxopts.hpp>

#include <csignal>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>

#include "Common.hpp"
#include "CompilationPileline.hpp"
#include "VirtualMachine.hpp"

using namespace std::string_literals;


uint8_t printVersion();
uint8_t repl();
uint8_t runFile(std::string path);
uint8_t compileFile(std::string path, std::string outFile);
[[noreturn]] void signalHandler(int sigNum);

CompilationPipeline* g_compilationPipeline;


int main(const int argc, char** argv) {
    (void)std::signal(SIGABRT, signalHandler);
    (void)std::signal(SIGFPE, signalHandler);
    (void)std::signal(SIGILL, signalHandler);
    (void)std::signal(SIGINT, signalHandler);
    (void)std::signal(SIGSEGV, signalHandler);
    (void)std::signal(SIGTERM, signalHandler);

    cxxopts::Options options("PythOwOn", "A simple programming language.");

    options.add_option("", {"file", "", cxxopts::value<std::string>()});
    options.add_option("", {"r,run", "Runs a given PythOwOn file."});
    options.add_option("", {"c,compile", "Compile a PythOwOn file into bytecode."});
    options.add_option("", {"o,output",
                            "Output file for compiled bytecode.",
                            cxxopts::value<std::string>()});

    options.add_option("", {"i,interpret", "Start PythOwOn in interactive mode"});
    options.add_option("", {"h,help", "Print usage"});
    options.add_option("", {"v,version", "Display the version of PythOwOn"});

    options.parse_positional({"file"});

    const auto result = options.parse(argc, argv);

    if (result.count("help")) {
        FMT_PRINTLN(options.help());
        return 0;
    }

    if (result.count("version")) return printVersion();
    if (result.count("interpret")) return repl();

    if (result.count("run")) {
        if (result.count("file") == 0) {
            FMT_PRINTLN("You must provide a file to run.");
            return 1;
        }

        return runFile(result["file"].as<std::string>());
    }

    if (result.count("compile")) {
        if (result.count("file") == 0) {
            FMT_PRINTLN("You must provide a file to compile.");
            return 1;
        }

        if (result.count("output") == 0) {
            FMT_PRINTLN("You must provide a output file.");
            return 1;
        }

        return compileFile(result["file"].as<std::string>(),
                           result["output"].as<std::string>());
    }

    FMT_PRINTLN(options.help());
    return 0;
}

[[noreturn]] void signalHandler(int sigNum) {
    FMT_PRINTLN("Recieved signal number {0}, exiting...", sigNum);

    delete g_compilationPipeline;

    std::quick_exit(sigNum);
}


uint8_t printVersion() {
    FMT_PRINTLN("PythOwOn 0.0.1");
    return 0;
}

bool isIncomplete(std::string& line) {
    int openBrackets = 0;
    int openParentheses = 0;
    int openBraces = 0;
    bool openTripleQuotes = false;

    for (ssize_t i = 0; i < static_cast<ssize_t>(line.size()) - 2; i++) {
        if (line[i] == '"' && line[i + 1] == '"' && line[i + 2] == '"') {
            openTripleQuotes = !openTripleQuotes;
            i += 2;
        }
    }
    if (openTripleQuotes) return true;

    for (auto it = line.begin(); it != line.end(); ++it) {
        if (*it == '#') break;
        if (*it == '"') {
            while (it + 1 != line.end() && *it++ != '"') ++it;
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

bool isEmpty(const std::string& line, const bool allowWhitespace = false) {
    if (allowWhitespace) return line.empty();

    return line.empty() || std::ranges::all_of(line, isspace);
}

uint8_t repl() {
    std::string line;
    std::string tmp;
    g_compilationPipeline = new CompilationPipeline();

    while (true) {
        FMT_PRINT("PythOwOn <<< ");

        // TODO: Handle Multiline input
        getline(std::cin, line);
        while (isIncomplete(line)) {
            FMT_PRINT("         ... ");
            getline(std::cin, tmp);
            line += tmp + '\n';
        }

        if (!isEmpty(line, false)) g_compilationPipeline->interpret(line);
        line = "";
    }
}

uint8_t runInterpretedFile(std::ifstream& file) {
    file.seekg(0, std::ifstream::beg);
    std::stringstream ss;
    ss << file.rdbuf();
    const std::string source = ss.str();

    g_compilationPipeline = new CompilationPipeline();
    const InterpretResult result = g_compilationPipeline->interpret(source);

    delete g_compilationPipeline;

    if (result == InterpretResult::COMPILE_ERROR) return 65;
    if (result == InterpretResult::RUNTIME_ERROR) return 70;

    return 0;
}

uint8_t runCompiledFile(std::ifstream& file, const size_t fileLen,
                        const std::string& fileName) {
    // read 4 bytes for number of constants, 4 bytes for number line indices
    auto* temp32 = new char[sizeof(uint32_t)];
    auto* temp64 = new char[sizeof(size_t)];
    auto* tempValue = new char[sizeof(Value)];


    file.read(temp32, 4);
    const uint32_t numLines = BEStrToLE<uint32_t>(temp32);
    file.read(temp32, 4);
    const uint32_t numConstants = BEStrToLE<uint32_t>(temp32);

    // read line indices
    std::vector<size_t> lines(numLines);
    for (uint32_t i = 0; i < numLines; i++) {
        file.read(temp64, sizeof(size_t));
        lines[i] = BEStrToLE<size_t>(temp64);
    }

    // read constants
    std::vector<Value> constants(numConstants);
    for (uint32_t i = 0; i < numConstants; ++i) {
        file.read(tempValue, sizeof(Value));
        constants[i] = BEStrToLE<Value>(tempValue);
    }

    // read code
    std::vector<uint8_t> code(fileLen - file.tellg());
    for (auto& i : code) file.read(reinterpret_cast<char*>(&i), 1);


    if (file.tellg() != fileLen) {
        FMT_PRINTLN("File \"{}\" is not a valid PythOwOn compiled file.", fileName);
        return 74;
    }

    const auto chunk = std::make_shared<Chunk>();
    chunk->lines = std::move(lines);
    chunk->code = std::move(code);
    chunk->constants = std::move(constants);

    VM::initVM();
    VM::setChunk(chunk);
    const InterpretResult result = VM::run();
    VM::shutdownVM();

    if (result == InterpretResult::RUNTIME_ERROR) return 70;

    return 0;
}

uint8_t runFile(std::string path) {
    std::ifstream file(path, std::ifstream::in | std::ifstream::binary);
    if (!file.is_open()) {
        FMT_PRINTLN("Could not open file \"{}\".", path);
        return 74;
    }

    file.seekg(0, std::ifstream::end);
    const size_t length = file.tellg();
    file.seekg(0, std::ifstream::beg);

    if (length < 20) {
        FMT_PRINTLN("File \"{}\" is not a valid PythOwOn compiled file.", path);
        return 74;
    }

    std::string magic(7, '\0');
    file.read(magic.data(), 7);

    return magic == "POWON\0\0"s
               ? runCompiledFile(file, length, path)
               : runInterpretedFile(file);
}


[[nodiscard]] static const char* ValueToBytes(const Value& value,
                                              std::vector<std::string>& strTable) {
    static uint8_t byteArray[sizeof(Value)];
    byteArray[0] = static_cast<uint8_t>(value.type);

    switch (value.type) {
        case ValueType::NONE:
            break;
        case ValueType::INFINITY:
        case ValueType::NAN:
        case ValueType::BOOL:
            byteArray[1] = value.as.boolean;
            break;
        case ValueType::INT: {
            const char* integerBytes =
                LEtoBEStr<decltype(value.as.integer)>(value.as.integer);
            std::copy_n(integerBytes, sizeof(value.as.integer), &byteArray[1]);
            break;
        }
        case ValueType::DOUBLE: {
            const char* decimalBytes =
                LEtoBEStr<decltype(value.as.decimal)>(value.as.decimal);
            std::copy_n(decimalBytes, sizeof(value.as.decimal), &byteArray[1]);
            break;
        }
        case ValueType::OBJECT: {
            const Obj* object = value.as.obj;
            switch (object->type) {
                case ObjType::STRING:
                    byteArray[1] = static_cast<uint8_t>(ObjType::STRING);
                    strTable.push_back(object->asString()->str);
                    byteArray[2] = static_cast<uint8_t>(strTable.size() - 1);
                    break;

                case ObjType::NONE:
                default:
                    break;
            }
        default:
            break;
        }
    }
}

std::ostream& operator<<(std::ostream& os, const std::vector<Value>& value) {
    std::vector<std::string> strTable;
    std::vector<const char*> valueBytes;

    // first pass
    for (auto& val : value) { valueBytes.push_back(ValueToBytes(val, strTable)); }

    os.write(LEtoBEStr<uint32_t>(static_cast<uint32_t>(strTable.size())),
             sizeof(uint32_t));

    for (const char* val : valueBytes) os << val;

    for (const auto& str : strTable) {
        os.write(LEtoBEStr<uint32_t>(static_cast<uint32_t>(str.size())),
                 sizeof(uint32_t));
        os.write(str.data(), str.size());
    }
}

uint8_t compileFile(std::string path, std::string outFile) {
    std::ifstream file(path);
    if (!file.is_open()) {
        FMT_PRINTLN("Could not open file \"{}\".", path);
        return 74;
    }

    std::string source((std::istreambuf_iterator(file)),
                       (std::istreambuf_iterator<char>()));
    file.close();

    g_compilationPipeline = new CompilationPipeline();
    auto [result, chunk] = g_compilationPipeline->compile(source);

    delete g_compilationPipeline;

    if (result == InterpretResult::COMPILE_ERROR) return 65;

    std::ofstream out(outFile, std::ios::binary);
    if (!out.is_open()) {
        FMT_PRINTLN("Could not open file \"{}\".", outFile);
        return 74;
    }

    uint32_t linesSize = static_cast<uint32_t>(chunk->lines.size());
    uint32_t constantsSize = static_cast<uint32_t>(chunk->constants.size());

    out << "POWON\0\0"s;
    out.write(LEtoBEStr<uint32_t>(linesSize), sizeof(uint32_t));
    out.write(LEtoBEStr<uint32_t>(constantsSize), sizeof(uint32_t));
    out << chunk->constants; // includes string table
    out << chunk->lines;
    out << chunk->code;
    out.flush();
    out.close();

    return 0;
}