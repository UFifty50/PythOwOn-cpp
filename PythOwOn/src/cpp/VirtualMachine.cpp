#include <string>

#include "VirtualMachine.hpp"
#include "Value.hpp"
#include "Common.hpp"


VM::VM() {
    stack = Stack<Value>();
}

VM::~VM() = default;

void VM::setChunk(Chunk* chunk) {
    this->chunk = chunk;
    ip = chunk->code.data();
}

Chunk* VM::getChunk() {
    return chunk;
}

template<typename... T>
void VM::runtimeError(std::string message, T... args) {
//    FMT_PRINT(message+"\n", std::forward<T...>(args...));

    int instruction = ip - chunk->code.data();
    size_t line = chunk->lines[instruction];
    FMT_PRINT("[line {}] in script\n", line);
    stack.reset();
}

InterpretResult VM::run() {
    while (true) {
#if defined(TRACE_EXECUTION)
        FMT_PRINT("          ");
        for (auto& value : stack) {
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

            case OpCode::CONSTANT_LONG: {
                Value constant = readConstantLong();
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

            case OpCode::NEGATE:
                if (!stack.peek(0).isNumber()) {
                    runtimeError("Operand must be a number.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                stack.push(Value::numberVal(-stack.pop().asNumber()));
                break;

            case OpCode::RETURN: {
                printValue(stack.pop());
                FMT_PRINT("\n");
                return InterpretResult::OK;
            }
        }
    }
}
