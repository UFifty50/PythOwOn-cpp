#include "VirtualMachine.hpp"

#include <functional>
#include <string>

#include "Common.hpp"
#include "Value.hpp"
#include "Utils/Stack.hpp"


VM::State VM::VMstate;


void VM::InitVM() {
    VMstate.stack = Stack<Value>();
    VMstate.strings = std::unordered_set<ObjString>();
    VMstate.globals = std::unordered_map<ObjString, Value>();
    VMstate.objects = LinkedList::Single<Obj*>();
    VMstate.ip = nullptr;
}

void VM::ShutdownVM() {
    VMstate.stack.reset();
    VMstate.objects.clear();
    VMstate.strings.clear();
    VMstate.globals.clear();
    VMstate.ip = nullptr;
}

void VM::SetChunk(Chunk& chunk) {
    VMstate.chunk = chunk;
    VMstate.ip = chunk.code.data();
}

// Chunk* VM::getChunk() { return chunk; }

template <AllPrintable... Ts>
void VM::RuntimeError(const std::string& message, Ts... args) {
    FMT_PRINT(message + "\n", args...);

    const uint8_t instructionIdx =
        static_cast<uint8_t>(VMstate.ip - VMstate.chunk.code.data());
    size_t line = VMstate.chunk.lines[instructionIdx];
    FMT_PRINTLN("[line {}] in script", line);
    VMstate.stack.reset();
}

InterpretResult VM::Run() {
    while (true) {
#if defined(TRACE_EXECUTION)
        FMT_PRINT("          ");
        for (auto& value : VMstate.stack) {
            FMT_PRINT("[ ");
            Debug_printValue(value);
            FMT_PRINT(" ]");
        }
        FMT_PRINT("\n");

        VMstate.chunk.disassembleInstruction(
            static_cast<size_t>(VMstate.ip - VMstate.chunk.code.data()));
#endif

        switch (ReadByte()) {
            case OpCode::CONSTANT: {
                Value constant = ReadConstant();
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
                uint8_t slot = ReadByte();
                VMstate.stack.push(VMstate.stack[slot]);
                break;
            }

            case OpCode::GET_LOCAL_LONG: {
                uint32_t slot = ReadLong();
                VMstate.stack.push(VMstate.stack[slot]);
                break;
            }

            case OpCode::SET_LOCAL: {
                uint8_t slot = ReadByte();
                VMstate.stack[slot] = VMstate.stack.peek(0);
                break;
            }

            case OpCode::SET_LOCAL_LONG: {
                uint32_t slot = ReadLong();
                VMstate.stack[slot] = VMstate.stack.peek(0);
                break;
            }

            case OpCode::GET_GLOBAL: {
                const ObjString* name = Value::AsObject(ReadConstant())->asString();
                if (!VMstate.globals.contains(*name)) {
                    RuntimeError("Undefined variable '{}'.", name->str);
                    return InterpretResult::RUNTIME_ERROR;
                }

                VMstate.stack.push(VMstate.globals[*name]);
                break;
            }

            case OpCode::GET_GLOBAL_LONG: {
                const ObjString* name = Value::AsObject(ReadConstantLong())->asString();
                if (!VMstate.globals.contains(*name)) {
                    RuntimeError("Undefined variable '{}'.", name->str);
                    return InterpretResult::RUNTIME_ERROR;
                }

                VMstate.stack.push(VMstate.globals[*name]);
                break;
            }

            case OpCode::SET_GLOBAL: {
                const ObjString* name = Value::AsObject(ReadConstant())->asString();
                if (!VMstate.globals.contains(*name)) {
                    RuntimeError("Undefined variable '{}'.", name->str);
                    return InterpretResult::RUNTIME_ERROR;
                }

                VMstate.globals[*name] = VMstate.stack.peek(0);
                break;
            }

            case OpCode::SET_GLOBAL_LONG: {
                const ObjString* name = Value::AsObject(ReadConstantLong())->asString();
                if (!VMstate.globals.contains(*name)) {
                    RuntimeError("Undefined variable '{}'.", name->str);
                    return InterpretResult::RUNTIME_ERROR;
                }

                VMstate.globals[*name] = VMstate.stack.peek(0);
                break;
            }

            case OpCode::DEF_GLOBAL: {
                const ObjString* name = Value::AsObject(ReadConstant())->asString();
                DefineGlobal(name, VMstate.stack.pop());
                break;
            }

            case OpCode::DEF_GLOBAL_LONG: {
                const ObjString* name = Value::AsObject(ReadConstantLong())->asString();
                DefineGlobal(name, VMstate.stack.pop());
                break;
            }

            case OpCode::NONE: VMstate.stack.push(Value::NoneVal());
                break;

            case OpCode::CONSTANT_LONG: Value constant = ReadConstantLong();
                VMstate.stack.push(constant);
                break;

            case OpCode::DUP: VMstate.stack.push(VMstate.stack.peek(0));
                break;

            case OpCode::EQUAL: Value firstVal = VMstate.stack.pop();
                Value secondVal = VMstate.stack.pop();
                VMstate.stack.push(Value::BoolVal(secondVal.isEqualTo(firstVal)));
                break;

            case OpCode::GREATER: if (auto a = BinaryOp<std::greater<
                    Value>>(&VM::DigitChecker))
                    return a.value();
                break;

            case OpCode::LESS: if (auto a = BinaryOp<std::less<Value>>(&VM::DigitChecker))
                    return a.
                        value();
                break;

            case OpCode::ADD: if (auto a = BinaryOp<std::plus<Value>>(&VM::AddableChecker))
                    return a
                        .value();
                break;

            case OpCode::MULTIPLY: if (auto a = BinaryOp<std::multiplies<Value>>(
                    &VM::MultiplicableChecker))
                    return a.value();
                break;

            case OpCode::DIVIDE: if (auto a = BinaryOp<std::divides<
                    Value>>(&VM::DigitChecker))
                    return a.value();
                break;

            case OpCode::NOT: VMstate.stack.push(Value::BoolVal(VMstate.stack.pop().isFalsey()));
                break;

            case OpCode::NEGATE: {
                if (!VMstate.stack.peek(0).isNumber() &&
                    !VMstate.stack.peek(0).isSpecialNumber()) {
                    RuntimeError("Operand must be a number.");
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

            case OpCode::LEFTSHIFT: if (auto a = BinaryOp<lshift<
                    Value>>(&VM::IntegerChecker))
                    return a.value();
                break;

            case OpCode::RIGHTSHIFT: if (auto a = BinaryOp<rshift<
                    Value>>(&VM::IntegerChecker))
                    return a.value();
                break;

            case OpCode::MODULO: if (auto a = BinaryOp<std::modulus<
                    Value>>(&VM::DigitChecker))
                    return a.value();
                break;

            case OpCode::PRINT: {
                printValue(VMstate.stack.pop());
                FMT_PRINT("\n");
                break;
            }

            case OpCode::JUMP: {
                uint16_t offset = ReadShort();
                VMstate.ip += offset;
                break;
            }

            case OpCode::JUMP_FALSE: {
                uint16_t offset = ReadShort();
                if (VMstate.stack.peek(0).isFalsey()) VMstate.ip += offset;
                break;
            }

            case OpCode::LOOP: {
                uint32_t offset = ReadShort();
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

InterpretResult VM::DigitChecker() {
    if (!VMstate.stack.peek(0).isNumber() || !VMstate.stack.peek(1).isNumber()) {
        RuntimeError("Operands must be numbers.");
        return InterpretResult::RUNTIME_ERROR;
    }
    return InterpretResult::OK;
}

InterpretResult VM::IntegerChecker() {
    if (!VMstate.stack.peek(0).isInteger() || !VMstate.stack.peek(1).isInteger()) {
        RuntimeError("Operands must be integers.");
        return InterpretResult::RUNTIME_ERROR;
    }
    return InterpretResult::OK;
}

InterpretResult VM::StringChecker() {
    if (!VMstate.stack.peek(0).isObjectType(ObjType::STRING) ||
        !VMstate.stack.peek(1).isObjectType(ObjType::STRING)) {
        RuntimeError("Operands must be strings.");
        return InterpretResult::RUNTIME_ERROR;
    }
    return InterpretResult::OK;
}

InterpretResult VM::AddableChecker() {
    if ((VMstate.stack.peek(0).isNumber() ||
            VMstate.stack.peek(0).isObjectType(ObjType::STRING)) &&
        (VMstate.stack.peek(1).isNumber() ||
            VMstate.stack.peek(1).isObjectType(ObjType::STRING))) { return InterpretResult::OK; }

    RuntimeError("Can only add numbers or strings.");
    return InterpretResult::RUNTIME_ERROR;
}

InterpretResult VM::MultiplicableChecker() {
    if ((VMstate.stack.peek(0).isNumber() && VMstate.stack.peek(1).isNumber()) ||
        (VMstate.stack.peek(0).isNumber() &&
            VMstate.stack.peek(1).isObjectType(ObjType::STRING)) ||
        (VMstate.stack.peek(0).isObjectType(ObjType::STRING) &&
            VMstate.stack.peek(1).isNumber())) { return InterpretResult::OK; }

    RuntimeError("Can only multiply numbers.");
    return InterpretResult::RUNTIME_ERROR;
}
