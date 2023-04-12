#ifndef VM_HPP
#define VM_HPP

#include <string>
#include <vector>
#include <functional>

#include "Common.hpp"
#include "Chunk.hpp"
#include "Value.hpp"




class VM {
private:
    Chunk* chunk;
    size_t* ip;
    Stack<Value> stack;

public:
    VM();    // initVM()
    ~VM();   // freeVM()

    
    inline size_t readByte() { return *ip++; }
    
    inline Value readConstant() { return chunk->constants[readByte()]; }
    
    template<typename T> inline void binaryOp(T op) {
        Value b = stack.pop();
        Value a = stack.pop();
        stack.push(op(a, b));
    }

    void setChunk(Chunk* chunk);
    Chunk* getChunk();
    InterpretResult run();
};

#endif
