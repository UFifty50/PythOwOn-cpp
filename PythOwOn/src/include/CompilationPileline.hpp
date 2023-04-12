#ifndef COMPILEPIPE_HPP
#define COMPILEPIPE_HPP

#include <string>
#include "Common.hpp"
#include "Scanner.hpp"


class CompilationPileline {
private:
    Scanner* scanner;

public:
    CompilationPileline();
    ~CompilationPileline();

    InterpretResult interpret(std::string source);
    void compile(std::string source);
};

#endif
