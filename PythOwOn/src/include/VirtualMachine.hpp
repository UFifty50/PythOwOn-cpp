#ifndef VM_HPP
#define VM_HPP

#include <string>
#include <vector>
#include <optional>

#include "Common.hpp"
#include "Chunk.hpp"
#include "Value.hpp"




class VM {
private:
    Chunk* chunk;
    uint8_t* ip;
    Stack<Value> stack;

public:
    VM();    // initVM()
    ~VM();   // freeVM()

    
    inline uint8_t readByte() { return *ip++; }
    
    inline Value readConstant() { return chunk->constants[readByte()]; }
    inline Value readConstantLong() { 
        uint32_t index = (readByte() << 16) | (readByte() << 8) | readByte();
        return chunk->constants[index];
    }

    // template<typename T> inline InterpretResult binaryOp(std::function<Value(double)> as, T op)
    template<typename F, typename T>
    std::optional<InterpretResult> binaryOp(F op, std::function<Value(T)> type) {
        if (!stack.peek(0).isNumber() || !stack.peek(1).isNumber()) {
            runtimeError("Operands must be numbers.");
            return InterpretResult::RUNTIME_ERROR;
        }

        Value b = stack.pop();
        Value a = stack.pop();
        stack.push(type(op(a, b)));
        
        return std::nullopt;
    }

    void setChunk(Chunk* chunk);
    Chunk* getChunk();
    InterpretResult run();

    template <AllPrintable... Ts>
    void runtimeError(std::string message, Ts... args);
};

#endif
