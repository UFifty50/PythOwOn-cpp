#include <string>
#include <functional>
#include <algorithm>

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

template<AllPrintable... Ts>
void VM::runtimeError(std::string message, Ts... args) {
    FMT_PRINT(message+"\n", args...);

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

            case OpCode::FALSE: stack.push(Value::boolVal(false)); break;

            case OpCode::TRUE: stack.push(Value::boolVal(true)); break;

            case OpCode::NONE: stack.push(Value::noneVal()); break;

            case OpCode::CONSTANT_LONG:
                Value constant = readConstantLong();
                stack.push(constant);
                break;

            case OpCode::EQUAL:
                Value b = stack.pop();
                Value a = stack.pop();
                stack.push(Value::boolVal(a.isEqualTo(b)));
                break;

            case OpCode::GREATER:
                if (auto a = binaryOp<std::greater<Value>, bool>(std::greater<Value>(), Value::numberVal)) return a.value();
                break;

            case OpCode::LESS:
                if (auto a = binaryOp<std::less<Value>, bool>(std::less<Value>(), Value::numberVal)) return a.value();
                break;
            
            case OpCode::ADD: 
                if (auto a = binaryOp<std::plus<Value>, Value>(std::plus<Value>(), Value::asNumber)) return a.value();
                break;

            case OpCode::MULTIPLY:
                if (auto a = binaryOp<std::multiplies<Value>, Value>(std::multiplies<Value>(), Value::asNumber)) return a.value();
                break;
             
            case OpCode::DIVIDE:
                if (auto a = binaryOp<std::divides<Value>, Value>(std::divides<Value>(), Value::asNumber)) return a.value();
                break;

            case OpCode::NOT:
                stack.push(Value::boolVal(stack.pop().isFalsey()));
                break;

            case OpCode::NEGATE: {
                if (!stack.peek(0).isNumber()) {
                    runtimeError("Operand must be a number.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                stack.push(Value::numberVal(-stack.pop().as.number));
                break;
            }

            case OpCode::LEFTSHIFT:
                if (auto a = binaryOp<left_shifts<Value>, Value>(left_shifts<Value>(), Value::asNumber)) return a.value();
                break;

            case OpCode::RIGHTSHIFT:
                if (auto a = binaryOp<right_shifts<Value>, Value>(right_shifts<Value>(), Value::asNumber)) return a.value();
                break;

            case OpCode::RETURN: {
                if (stack.size() > 0) printValue(stack.pop());
                FMT_PRINT("\n");
                return InterpretResult::OK;
            }
        }
    }
}
