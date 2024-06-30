#ifndef VM_HPP
#define VM_HPP

#include <algorithm>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Chunk.hpp"
#include "Common.hpp"
#include "Object.hpp"
#include "Value.hpp"
#include "Utils/LinkedList.hpp"
#include "Utils/Stack.hpp"


class VM {
public:
    struct State {
        Chunk chunk;
        uint8_t* ip;
        Stack<Value> stack;
        std::unordered_set<ObjString> strings;
        std::unordered_map<ObjString, Value> globals;
        LinkedList::Single<Obj*> objects;
    };

    static State VMstate;

    VM() { InitVM(); }
    ~VM() { ShutdownVM(); }

    VM(const VM&) = delete;
    VM& operator=(const VM&) = delete;
    VM(VM&&) = delete;
    VM& operator=(VM&&) = delete;

    static void InitVM();
    static void ShutdownVM();

    static uint8_t ReadByte() { return *VMstate.ip++; }

    static uint16_t ReadShort() {
        uint16_t value = static_cast<uint16_t>(ReadByte() << 8);
        value |= ReadByte();
        return value;
    }

    static uint32_t ReadLong() {
        uint32_t value = ReadByte() << 24;
        value |= ReadByte() << 16;
        value |= ReadByte() << 8;
        value |= ReadByte();
        return value;
    }

    static Value ReadConstant() { return VMstate.chunk.constants[ReadByte()]; }
    static Value ReadConstantLong() { return VMstate.chunk.constants[ReadLong()]; }

    template <typename O>
    static O* NewObject(const ObjType type) {
        auto* object = reinterpret_cast<Obj*>(new O());
        object->type = type;

        VMstate.objects.push(object);

        return reinterpret_cast<O*>(object);
    }

    static void AddString(const ObjString* string) { VMstate.strings.emplace(*string); }

    static bool HasString(const ObjString* string) { return VMstate.strings.contains(*string); }

    static bool HasString(const std::string& string) {
        return std::ranges::any_of(VMstate.strings, [&string](const auto& objStr) {
            return objStr.str == string;
        });
    }

    static const ObjString* GetString(const std::string& string) {
        for (const auto& objStr : VMstate.strings) { if (objStr.str == string) return &objStr; }
        return nullptr;
    }

    static void DefineGlobal(const ObjString* name, const Value& value) {
        VMstate.globals[*name] = value;
    }

    static void SetChunk(Chunk chunk);
    static InterpretResult Run();

    template <AllPrintable... Ts>
    static void RuntimeError(const std::string& message, Ts... args);

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
    static std::optional<InterpretResult> BinaryOp(
        const std::function<InterpretResult()>& verifyFn) {
        if (verifyFn() != InterpretResult::OK) return InterpretResult::RUNTIME_ERROR;

        Value b = VMstate.stack.pop();
        Value a = VMstate.stack.pop();

        if constexpr (auto val = Op{}(a, b); std::is_same_v<
            decltype(val), bool>) { VMstate.stack.push(Value::BoolVal(val)); }
        else if
        constexpr (std::is_same_v<decltype(val), std::string>) {
            VMstate.stack.push(Value::ObjectVal(ObjString::Create(val)));
        }
        else if constexpr (std::is_same_v<decltype(val), Obj*>) {
            VMstate.stack.push(Value::ObjectVal(val));
        }
        else if constexpr (std::is_same_v<decltype(val), Value>) { VMstate.stack.push(val); }
        else { FMT_PRINT("Unknown type, type was {}.\n", typeid(val).name()); }

        return std::nullopt;
    }

private:
    static InterpretResult DigitChecker();
    static InterpretResult IntegerChecker();
    static InterpretResult StringChecker();
    static InterpretResult AddableChecker();
    static InterpretResult MultiplicableChecker();
};

#endif
