#ifndef VM_HPP
#define VM_HPP

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Chunk.hpp"
#include "Common.hpp"
#include "Object.hpp"
#include "Utils/LinkedList.hpp"
#include "Utils/Stack.hpp"
#include "Value.hpp"


class VM {
public:
    struct State {
        std::shared_ptr<Chunk> chunk;
        uint8_t* ip;
        Stack<Value> stack;
        std::unordered_map<ObjString, Value> strings;
        std::unordered_map<ObjString, Value> globals;
        LinkedList::Single<Obj*> objects;
    };

    static State VMstate;

    VM() { initVM(); }
    ~VM() { shutdownVM(); }

    VM(const VM&) = delete;
    VM& operator=(const VM&) = delete;
    VM(VM&&) = delete;
    VM& operator=(VM&&) = delete;

    static void initVM();
    static void shutdownVM();

    static uint8_t readByte() { return *VMstate.ip++; }

    static uint16_t readShort() {
        uint16_t value = static_cast<uint16_t>(readByte() << 8);
        value |= readByte();
        return value;
    }

    static uint32_t readLong() {
        uint32_t value = readByte() << 24;
        value |= readByte() << 16;
        value |= readByte() << 8;
        value |= readByte();
        return value;
    }

    static Value readConstant() { return VMstate.chunk->constants[readByte()]; }
    static Value readConstantLong() { return VMstate.chunk->constants[readLong()]; }

    template <typename O>
    static O* newObject(const ObjType type) {
        auto* object = reinterpret_cast<Obj*>(new O());
        object->type = type;

        VMstate.objects.push(object);

        return reinterpret_cast<O*>(object);
    }

    static void addString(const ObjString* string, const Value& value) {
        VMstate.strings[*string] = value;
    }

    static void defineGlobal(const ObjString* name, const Value& value) {
        VMstate.globals[*name] = value;
    }

    static void setChunk(const std::shared_ptr<Chunk>& chunk);
    // Chunk* getChunk();
    static InterpretResult run();

    template <AllPrintable... Ts>
    static void runtimeError(const std::string& message, Ts... args);

    /// <summary>
    /// Performs a binary operation on the stack.
    /// verifierFn is a function that checks if the top two values of the stack are
    /// valid for the operation.
    /// </summary>
    /// <typeparam name="Op">The binary operation to do.</typeparam>
    /// <param name="verifyFn">A function to verify that the stack contains values valid
    /// for the specified binary operation.</param>
    /// <returns>RUNTIME_ERROR if an error occurred, otherwise nullopt.</returns>
    template <typename Op>
    static std::optional<InterpretResult> binaryOp(
        const std::function<InterpretResult()>& verifyFn) {
        if (verifyFn() != InterpretResult::OK) return InterpretResult::RUNTIME_ERROR;

        Value b = VMstate.stack.pop();
        Value a = VMstate.stack.pop();

        if constexpr (auto val = Op{}(a, b); std::is_same_v<decltype(val), bool>) {
            VMstate.stack.push(Value::boolVal(val));
        } else if constexpr (std::is_same_v<decltype(val), std::string>) {
            VMstate.stack.push(Value::objectVal(ObjString::create(val)));
        } else if constexpr (std::is_same_v<decltype(val), Obj*>) {
            VMstate.stack.push(Value::objectVal(val));
        } else if constexpr (std::is_same_v<decltype(val), Value>) {
            VMstate.stack.push(val);
        } else {
            FMT_PRINT("Unknown type, type was {}.\n", typeid(val).name());
        }

        return std::nullopt;
    }

private:
    static InterpretResult digitChecker();
    static InterpretResult integerChecker();
    static InterpretResult stringChecker();
    static InterpretResult addableChecker();
    static InterpretResult multiplicableChecker();
};

#endif
