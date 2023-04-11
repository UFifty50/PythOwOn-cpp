#include <string>

#include "VirtualMachine.hpp"
#include "Value.hpp"
#include "Common.hpp"


VM::VM() {
    stack = Stack<Value>();
}

VM::~VM() = default;

InterpretResult VM::interpret(Chunk* chunk) {
    this->chunk = chunk;
    this->ip = chunk->code.data();

    return InterpretResult::OK;
}

InterpretResult VM::run() {
    while (true) {
#if defined(TRACE_EXECUTION)
        FMT_PRINT("          ");
        for (auto value : stack) {
            FMT_PRINT("[ ");
            printValue(value);
            FMT_PRINT(" ]");
        }
        FMT_PRINT("\n");

        chunk->disassembleInstruction((int)(ip - chunk->code.data()));
#endif
        uint8_t instruction = readByte();
        switch (instruction) {
            case OpCode::CONSTANT: {
                Value constant = readConstant();
                stack.push(constant);
                break;
            }
            
            case OpCode::ADD: {
                binaryOp(std::plus<Value>());
                break;
            }

            case OpCode::MULTIPLY: {
                binaryOp(std::multiplies<Value>());
                break;
            }
             
            case OpCode::DIVIDE: {
                binaryOp(std::divides<Value>());
                break;
            }

            case OpCode::NEGATE: stack.push(-stack.pop()); break;

            case OpCode::RETURN: {
                printValue(stack.pop());
                FMT_PRINT("\n");
                return InterpretResult::OK;
            }
        }
    }
}
