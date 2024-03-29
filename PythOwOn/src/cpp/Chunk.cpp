#include "Chunk.hpp"

#include <string>

#include "Common.hpp"


void Chunk::write(uint8_t byte, size_t line) {
    code.emplace_back(byte);
    lines.emplace_back(line);
}

uint32_t Chunk::addConstant(Value value) {
    constants.emplace_back(value);

    if (constants.size() < UINT32_MAX) {
        return constants.size() - 1;
    }

    FMT_PRINT("Too many constants in one chunk\n");
    return UINT32_MAX;
}

void Chunk::writeConstant(Value value, size_t line) {
    size_t index = addConstant(value);

    if (index < UINT8_MAX) {
        write(OpCode::CONSTANT, line);
        write((uint8_t)index, line);
    } else if (index < UINT32_MAX) {
        write(OpCode::CONSTANT_LONG, line);
        write((uint8_t)(index >> 24) & 0xff, line);
        write((uint8_t)((index >> 16) & 0xff), line);
        write((uint8_t)((index >> 8) & 0xff), line);
        write((uint8_t)(index & 0xff), line);
    } else {
        FMT_PRINT("Too many constants in one chunk\n");
    }
}

void Chunk::writeVariable(OpCode op, uint32_t var, size_t line) {
    if (var < UINT8_MAX) {
        write(op, line);
        write((uint8_t)var, line);
    } else if (var < UINT32_MAX) {
        op = (OpCode)((int)op + 1);
        write(op, line);
        write((uint8_t)(var >> 24) & 0xff, line);
        write((uint8_t)((var >> 16) & 0xff), line);
        write((uint8_t)((var >> 8) & 0xff), line);
        write((uint8_t)(var & 0xff), line);
    } else {
        FMT_PRINT("Too many variables in one chunk\n");
    }
}


#if defined(_DEBUG)

#include <iostream>

static size_t simpleInstruction(std::string name, size_t offset) {
    FMT_PRINT("{}\n", name);
    return offset + 1;
}

static size_t constantInstruction(std::string name, const Chunk* chunk, size_t offset) {
    uint8_t constant = chunk->code[offset + 1];
    FMT_PRINT("{:10} {:04}  ", name, constant);
    Debug_printValue(chunk->constants[constant]);
    FMT_PRINT("\n");
    return (size_t)offset + 2;
}


static size_t constantLongInstruction(std::string name, const Chunk* chunk,
                                      size_t offset) {
    uint32_t constant = (chunk->code[offset + 1]) | (chunk->code[offset + 2] << 8) |
                        (chunk->code[offset + 3] << 16) | (chunk->code[offset + 4] << 24);
    FMT_PRINT("{:10} {:04}  ", name, constant);
    Debug_printValue(chunk->constants[constant]);
    FMT_PRINT("\n");
    return (size_t)offset + 5;
}

static size_t byteInstruction(std::string name, const Chunk* chunk, size_t offset) {
    uint8_t slot = chunk->code[offset + 1];
    FMT_PRINT("{:10} {:04}  ", name, slot);
    FMT_PRINT("\n");
    return (size_t)offset + 2;
}

static size_t longInstruction(std::string name, const Chunk* chunk, size_t offset) {
    uint32_t slot = (chunk->code[offset + 1]) | (chunk->code[offset + 2] << 8) |
                    (chunk->code[offset + 3] << 16) | (chunk->code[offset + 4] << 24);
    FMT_PRINT("{:10} {:04}  ", name, slot);
    FMT_PRINT("\n");
    return (size_t)offset + 5;
}

static size_t jumpInstruction(std::string name, const Chunk* chunk, uint8_t j,
                              size_t offset) {
    return 1;
}

#if defined(TRACE_EXECUTION)

void Chunk::disassemble(std::string name) {
    FMT_PRINT("== {} ==\n", name);
    for (size_t offset = 0; offset < code.size();) {
        offset = disassembleInstruction(offset);
    }
    FMT_PRINT("\n");
}

size_t Chunk::disassembleInstruction(size_t offset) {
    FMT_PRINT("{:04} ", offset);
    if (offset > 0 && lines[offset] == lines[offset - 1]) {
        FMT_PRINT("   | ");
    } else {
        FMT_PRINT("{:4} ", lines[offset]);
    }

    OpCode instruction = static_cast<OpCode>(code[offset]);

    switch (instruction) {
        case OpCode::RETURN:
            return simpleInstruction("RETURN", offset);

        case OpCode::CONSTANT:
            return constantInstruction("CONSTANT", this, offset);

        case OpCode::CONSTANT_LONG:
            return constantLongInstruction("CONSTANT_LONG", this, offset);

        case OpCode::NONE:
            return simpleInstruction("NONE", offset);

        case OpCode::TRUE:
            return simpleInstruction("TRUE", offset);

        case OpCode::FALSE:
            return simpleInstruction("FALSE", offset);

        case OpCode::POP:
            return simpleInstruction("POP", offset);

        case OpCode::GET_LOCAL:
            return byteInstruction("GET_LOCAL", this, offset);

        case OpCode::GET_LOCAL_LONG:  // TODO: output file with debug mapping of slot
                                      // number to name and use that here
            return longInstruction("GET_LOCAL_LONG", this, offset);

        case OpCode::SET_LOCAL:
            return byteInstruction("SET_LOCAL", this, offset);

        case OpCode::SET_LOCAL_LONG:
            return longInstruction("SET_LOCAL_LONG", this, offset);

        case OpCode::GET_GLOBAL:
            return constantInstruction("GET_GLOBAL", this, offset);

        case OpCode::GET_GLOBAL_LONG:
            return constantLongInstruction("GET_GLOBAL_LONG", this, offset);

        case OpCode::DEF_GLOBAL:
            return constantInstruction("DEF_GLOBAL", this, offset);

        case OpCode::DEF_GLOBAL_LONG:
            return constantLongInstruction("DEF_GLOBAL_LONG", this, offset);

        case OpCode::SET_GLOBAL:
            return constantInstruction("SET_GLOBAL", this, offset);

        case OpCode::SET_GLOBAL_LONG:
            return constantLongInstruction("SET_GLOBAL_LONG", this, offset);

        case OpCode::EQUAL:
            return simpleInstruction("EQUAL", offset);

        case OpCode::GREATER:
            return simpleInstruction("GREATER", offset);

        case OpCode::LESS:
            return simpleInstruction("LESS", offset);

        case OpCode::ADD:
            return simpleInstruction("ADD", offset);

        case OpCode::MULTIPLY:
            return simpleInstruction("MULTIPLY", offset);

        case OpCode::DIVIDE:
            return simpleInstruction("DIVIDE", offset);

        case OpCode::NOT:
            return simpleInstruction("NOT", offset);

        case OpCode::LEFTSHIFT:
            return simpleInstruction("LEFTSHIFT", offset);

        case OpCode::RIGHTSHIFT:
            return simpleInstruction("RIGHTSHIFT", offset);

        case OpCode::MODULO:
            return simpleInstruction("MODULO", offset);

        case OpCode::NEGATE:
            return simpleInstruction("NEGATE", offset);

        case OpCode::PRINT:
            return simpleInstruction("PRINT", offset);

        case OpCode::JUMP:
            return jumpInstruction("JUMP", this, 1, offset);

        case OpCode::JUMP_FALSE:
            return jumpInstruction("JUMP_FALSE", this, 1, offset);

        default:
            FMT_PRINT("Uknown opcode {}\n", (int)instruction);
            return offset + 1;
    }
}

#endif

#endif
