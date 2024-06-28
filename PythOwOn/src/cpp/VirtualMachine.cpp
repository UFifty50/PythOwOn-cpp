#include "VirtualMachine.hpp"

#include <algorithm>
#include <functional>
#include <string>

#include "Common.hpp"
#include "Value.hpp"
#include "Utils/Stack.hpp"


VM::State VM::VMstate;


void VM::initVM() {
    VMstate.stack = Stack<Value>();
    VMstate.strings = std::unordered_map<ObjString, Value>();
    VMstate.globals = std::unordered_map<ObjString, Value>();
    VMstate.objects = LinkedList::Single<Obj*>();
    VMstate.ip = nullptr;
}

void VM::shutdownVM() {
    VMstate.chunk.reset();
    VMstate.stack.reset();
    VMstate.objects.clear();
    VMstate.strings.clear();
    VMstate.globals.clear();
    VMstate.ip = nullptr;
}

void VM::setChunk(const std::shared_ptr<Chunk>& chunk) {
    VMstate.chunk = chunk;
    VMstate.ip = chunk->code.data();
}

// Chunk* VM::getChunk() { return chunk; }

template <AllPrintable... Ts>
void VM::runtimeError(const std::string& message, Ts... args) {
    FMT_PRINT(message + "\n", args...);

    const uint8_t instructionIdx =
        static_cast<uint8_t>(VMstate.ip - VMstate.chunk->code.data());
    size_t line = VMstate.chunk->lines[instructionIdx];
    FMT_PRINTLN("[line {}] in script", line);
    VMstate.stack.reset();
}

InterpretResult VM::run() {
    while (true) {
#if defined(TRACE_EXECUTION)
        FMT_PRINT("          ");
        for (auto& value : VMstate.stack) {
            FMT_PRINT("[ ");
            Debug_printValue(value);
            FMT_PRINT(" ]");
        }
        FMT_PRINT("\n");

        VMstate.chunk->disassembleInstruction(
            static_cast<size_t>(VMstate.ip - VMstate.chunk->code.data()));
#endif

        switch (readByte()) {
            case OpCode::CONSTANT: {
                Value constant = readConstant();
                VMstate.stack.push(constant);
                break;
            }

            case OpCode::FALSE: VMstate.stack.push(Value::BoolVal(false));
                break;

            case OpCode::TRUE: VMstate.stack.push(Value::BoolVal(true));
                break;

            case OpCode::POP: VMstate.stack.pop();
                break;

            case OpCode::GET_LOCAL: {
                uint8_t slot = readByte();
                VMstate.stack.push(VMstate.stack[slot]);
                break;
            }

            case OpCode::GET_LOCAL_LONG: {
                uint32_t slot = readLong();
                VMstate.stack.push(VMstate.stack[slot]);
                break;
            }

            case OpCode::SET_LOCAL: {
                uint8_t slot = readByte();
                VMstate.stack[slot] = VMstate.stack.peek(0);
                break;
            }

            case OpCode::SET_LOCAL_LONG: {
                uint32_t slot = readLong();
                VMstate.stack[slot] = VMstate.stack.peek(0);
                break;
            }

            case OpCode::GET_GLOBAL: {
                const ObjString* name = Value::AsObject(readConstant())->asString();
                if (!VMstate.globals.contains(*name)) {
                    runtimeError("Undefined variable '{}'.", name->str);
                    return InterpretResult::RUNTIME_ERROR;
                }

                VMstate.stack.push(VMstate.globals[*name]);
                break;
            }

            case OpCode::GET_GLOBAL_LONG: {
                const ObjString* name = Value::AsObject(readConstantLong())->asString();
                if (!VMstate.globals.contains(*name)) {
                    runtimeError("Undefined variable '{}'.", name->str);
                    return InterpretResult::RUNTIME_ERROR;
                }

                VMstate.stack.push(VMstate.globals[*name]);
                break;
            }

            case OpCode::SET_GLOBAL: {
                const ObjString* name = Value::AsObject(readConstant())->asString();
                if (!VMstate.globals.contains(*name)) {
                    runtimeError("Undefined variable '{}'.", name->str);
                    return InterpretResult::RUNTIME_ERROR;
                }

                VMstate.globals[*name] = VMstate.stack.peek(0);
                break;
            }

            case OpCode::SET_GLOBAL_LONG: {
                const ObjString* name = Value::AsObject(readConstantLong())->asString();
                if (!VMstate.globals.contains(*name)) {
                    runtimeError("Undefined variable '{}'.", name->str);
                    return InterpretResult::RUNTIME_ERROR;
                }

                VMstate.globals[*name] = VMstate.stack.peek(0);
                break;
            }

            case OpCode::DEF_GLOBAL: {
                const ObjString* name = Value::AsObject(readConstant())->asString();
                defineGlobal(name, VMstate.stack.pop());
                break;
            }

            case OpCode::DEF_GLOBAL_LONG: {
                const ObjString* name = Value::AsObject(readConstantLong())->asString();
                defineGlobal(name, VMstate.stack.pop());
                break;
            }

            case OpCode::NONE: VMstate.stack.push(Value::NoneVal());
                break;

            case OpCode::CONSTANT_LONG: Value constant = readConstantLong();
                VMstate.stack.push(constant);
                break;

            case OpCode::DUP: VMstate.stack.push(VMstate.stack.peek(0));
                break;

            case OpCode::EQUAL: Value firstVal = VMstate.stack.pop();
                Value secondVal = VMstate.stack.pop();
                VMstate.stack.push(Value::BoolVal(secondVal.isEqualTo(firstVal)));
                break;

            case OpCode::GREATER: if (auto a = binaryOp<std::greater<
                    Value>>(&VM::digitChecker))
                    return a.value();
                break;

            case OpCode::LESS: if (auto a = binaryOp<std::less<Value>>(&VM::digitChecker))
                    return a.
                        value();
                break;

            case OpCode::ADD: if (auto a = binaryOp<std::plus<Value>>(&VM::addableChecker))
                    return a
                        .value();
                break;

            case OpCode::MULTIPLY: if (auto a = binaryOp<std::multiplies<Value>>(
                    &VM::multiplicableChecker))
                    return a.value();
                break;

            case OpCode::DIVIDE: if (auto a = binaryOp<std::divides<
                    Value>>(&VM::digitChecker))
                    return a.value();
                break;

            case OpCode::NOT: VMstate.stack.push(Value::BoolVal(VMstate.stack.pop().isFalsey()));
                break;

            case OpCode::NEGATE: {
                if (!VMstate.stack.peek(0).isNumber() &&
                    !VMstate.stack.peek(0).isSpecialNumber()) {
                    runtimeError("Operand must be a number.");
                    return InterpretResult::RUNTIME_ERROR;
                }

                auto a = VMstate.stack.pop();
                if (a.isSpecialNumber()) {
                    a.as.boolean = !a.as.boolean;
                    VMstate.stack.push(a);
                    break;
                }

                bool isDouble = a.isDouble();
                double val = -Value::AsDouble(a).as.decimal;
                VMstate.stack.push(Value::NumberVal(val, isDouble));
                break;
            }

            case OpCode::LEFTSHIFT: if (auto a = binaryOp<lshift<
                    Value>>(&VM::integerChecker))
                    return a.value();
                break;

            case OpCode::RIGHTSHIFT: if (auto a = binaryOp<rshift<
                    Value>>(&VM::integerChecker))
                    return a.value();
                break;

            case OpCode::MODULO: if (auto a = binaryOp<std::modulus<
                    Value>>(&VM::digitChecker))
                    return a.value();
                break;

            case OpCode::PRINT: {
                printValue(VMstate.stack.pop());
                FMT_PRINT("\n");
                break;
            }

            case OpCode::JUMP: {
                uint16_t offset = readShort();
                VMstate.ip += offset;
                break;
            }

            case OpCode::JUMP_FALSE: {
                uint16_t offset = readShort();
                if (VMstate.stack.peek(0).isFalsey()) VMstate.ip += offset;
                break;
            }

            case OpCode::LOOP: {
                uint32_t offset = readShort();
                VMstate.ip -= offset;
                break;
            }

            case OpCode::RETURN: {
                if (!VMstate.stack.empty()) printValue(VMstate.stack.pop());
                FMT_PRINT("\n");
                return InterpretResult::OK;
            }

            default: return InterpretResult::RUNTIME_ERROR;
        }
    }
}

InterpretResult VM::digitChecker() {
    if (!VMstate.stack.peek(0).isNumber() || !VMstate.stack.peek(1).isNumber()) {
        runtimeError("Operands must be numbers.");
        return InterpretResult::RUNTIME_ERROR;
    }
    return InterpretResult::OK;
}

InterpretResult VM::integerChecker() {
    if (!VMstate.stack.peek(0).isInteger() || !VMstate.stack.peek(1).isInteger()) {
        runtimeError("Operands must be integers.");
        return InterpretResult::RUNTIME_ERROR;
    }
    return InterpretResult::OK;
}

InterpretResult VM::stringChecker() {
    if (!VMstate.stack.peek(0).isObjectType(ObjType::STRING) ||
        !VMstate.stack.peek(1).isObjectType(ObjType::STRING)) {
        runtimeError("Operands must be strings.");
        return InterpretResult::RUNTIME_ERROR;
    }
    return InterpretResult::OK;
}

InterpretResult VM::addableChecker() {
    if ((VMstate.stack.peek(0).isNumber() ||
            VMstate.stack.peek(0).isObjectType(ObjType::STRING)) &&
        (VMstate.stack.peek(1).isNumber() ||
            VMstate.stack.peek(1).isObjectType(ObjType::STRING))) { return InterpretResult::OK; }

    runtimeError("Can only add numbers or strings.");
    return InterpretResult::RUNTIME_ERROR;
}

InterpretResult VM::multiplicableChecker() {
    if ((VMstate.stack.peek(0).isNumber() && VMstate.stack.peek(1).isNumber()) ||
        (VMstate.stack.peek(0).isNumber() &&
            VMstate.stack.peek(1).isObjectType(ObjType::STRING)) ||
        (VMstate.stack.peek(0).isObjectType(ObjType::STRING) &&
            VMstate.stack.peek(1).isNumber())) { return InterpretResult::OK; }

    runtimeError("Can only multiply numbers.");
    return InterpretResult::RUNTIME_ERROR;
}
