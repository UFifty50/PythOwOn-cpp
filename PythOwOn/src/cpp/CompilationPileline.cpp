#include "Common.hpp"
#include "CompilationPileline.hpp"


CompilationPileline::CompilationPileline() = default;

CompilationPileline::~CompilationPileline() {
    delete scanner;
}

InterpretResult CompilationPileline::interpret(std::string source) {
    compile(source);
    return InterpretResult::OK;
}

void CompilationPileline::compile(std::string source) {
    scanner = new Scanner(source);

    int line = -1;
    while (true) {
        Token token = scanner->scanToken();
        if (token.line != line) {
            FMT_PRINT("{:4d} ", token.line);
            line = token.line;
        } else {
            FMT_PRINT("   | ");
        }
        FMT_PRINT("{:2d} '{}'\n", (int)token.type, token.lexeme);
        if (token.type == TokenType::EOF) break;
    }
}
