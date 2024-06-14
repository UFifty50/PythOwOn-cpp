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
        return static_cast<uint32_t>(constants.size()) - 1;
    }

    FMT_PRINTLN("Too many constants in one chunk");
    return UINT32_MAX;
}

void Chunk::writeConstant(const Value value, const size_t line) {
    if (const size_t index = addConstant(value); index < UINT8_MAX) {
        write(OpCode::CONSTANT, line);
        write(index & 0xff, line);
    } else if (index < UINT32_MAX) {
        write(OpCode::CONSTANT_LONG, line);
        write(index >> 24 & 0xff, line);
        write(index >> 16 & 0xff, line);
        write(index >> 8 & 0xff, line);
        write(index & 0xff, line);
    } else {
        FMT_PRINTLN("Too many constants in one chunk");
    }
}

void Chunk::writeVariable(OpCode::Code op, const uint32_t var, const size_t line) {
    if (var < UINT8_MAX) {
        write(op, line);
        write(var & 0xff, line);
    } else if (var < UINT32_MAX) {
        op = static_cast<OpCode::Code>(op + 1);
        write(op, line);
        write(var >> 24 & 0xff, line);
        write(var >> 16 & 0xff, line);
        write(var >> 8 & 0xff, line);
        write(var & 0xff, line);
    } else {
        FMT_PRINTLN("Too many variables in one chunk");
    }
}


#if defined(_DEBUG)

#include <iostream>

namespace {
    size_t SimpleInstruction(std::string name, const size_t offset) {
        FMT_PRINT("{}\n", name);
        return offset + 1;
    }

    size_t ConstantInstruction(std::string name, const Chunk* chunk,
                               const size_t offset) {
        uint8_t constant = chunk->code[offset + 1];
        FMT_PRINT("{:10} {:04}  ", name, constant);
        Debug_printValue(chunk->constants[constant]);
        FMT_PRINT("\n");
        return offset + 2;
    }


    size_t ConstantLongInstruction(std::string name, const Chunk* chunk,
                                   const size_t offset) {
        uint32_t constant = chunk->code[offset + 1] | chunk->code[offset + 2] << 8 |
                            chunk->code[offset + 3] << 16 | chunk->code[offset + 4] << 24;
        FMT_PRINT("{:10} {:04}  ", name, constant);
        Debug_printValue(chunk->constants[constant]);
        FMT_PRINT("\n");
        return offset + 5;
    }

    size_t ByteInstruction(std::string name, const Chunk* chunk, const size_t offset) {
        uint8_t slot = chunk->code[offset + 1];
        FMT_PRINT("{:10} {:04}  ", name, slot);
        FMT_PRINT("\n");
        return offset + 2;
    }

    size_t LongInstruction(std::string name, const Chunk* chunk, const size_t offset) {
        uint32_t slot = chunk->code[offset + 1] | chunk->code[offset + 2] << 8 |
                        chunk->code[offset + 3] << 16 | chunk->code[offset + 4] << 24;
        FMT_PRINT("{:10} {:04}  ", name, slot);
        FMT_PRINT("\n");
        return offset + 5;
    }

    size_t JumpInstruction(std::string name, const Chunk* chunk, const uint8_t sign,
                           const size_t offset) {
        uint16_t jump = static_cast<uint16_t>(chunk->code[offset + 1] << 8);
        jump |= chunk->code[offset + 2];
        FMT_PRINT("{:10} {:04} -> {:04}\n", name, offset,
                  offset + 3 + static_cast<size_t>(sign) * jump);
        return offset + 3;
    }
}  // namespace

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


    switch (const auto instruction = static_cast<OpCode::Code>(code[offset])) {
        case OpCode::RETURN:
            return SimpleInstruction("RETURN", offset);

        case OpCode::CONSTANT:
            return ConstantInstruction("CONSTANT", this, offset);

        case OpCode::CONSTANT_LONG:
            return ConstantLongInstruction("CONSTANT_LONG", this, offset);

        case OpCode::DUP:
            return SimpleInstruction("DUP", offset);

        case OpCode::NONE:
            return SimpleInstruction("NONE", offset);

        case OpCode::TRUE:
            return SimpleInstruction("TRUE", offset);

        case OpCode::FALSE:
            return SimpleInstruction("FALSE", offset);

        case OpCode::POP:
            return SimpleInstruction("POP", offset);

        case OpCode::POPN:
            return ByteInstruction("POPN", this, offset);

        case OpCode::GET_LOCAL:
            return ByteInstruction("GET_LOCAL", this, offset);

        case OpCode::GET_LOCAL_LONG:  // TODO: output file with debug mapping of slot
                                      // number to name and use that here
            return LongInstruction("GET_LOCAL_LONG", this, offset);

        case OpCode::SET_LOCAL:
            return ByteInstruction("SET_LOCAL", this, offset);

        case OpCode::SET_LOCAL_LONG:
            return LongInstruction("SET_LOCAL_LONG", this, offset);

        case OpCode::GET_GLOBAL:
            return ConstantInstruction("GET_GLOBAL", this, offset);

        case OpCode::GET_GLOBAL_LONG:
            return ConstantLongInstruction("GET_GLOBAL_LONG", this, offset);

        case OpCode::DEF_GLOBAL:
            return ConstantInstruction("DEF_GLOBAL", this, offset);

        case OpCode::DEF_GLOBAL_LONG:
            return ConstantLongInstruction("DEF_GLOBAL_LONG", this, offset);

        case OpCode::SET_GLOBAL:
            return ConstantInstruction("SET_GLOBAL", this, offset);

        case OpCode::SET_GLOBAL_LONG:
            return ConstantLongInstruction("SET_GLOBAL_LONG", this, offset);

        case OpCode::EQUAL:
            return SimpleInstruction("EQUAL", offset);

        case OpCode::GREATER:
            return SimpleInstruction("GREATER", offset);

        case OpCode::LESS:
            return SimpleInstruction("LESS", offset);

        case OpCode::ADD:
            return SimpleInstruction("ADD", offset);

        case OpCode::MULTIPLY:
            return SimpleInstruction("MULTIPLY", offset);

        case OpCode::DIVIDE:
            return SimpleInstruction("DIVIDE", offset);

        case OpCode::AND:
            return SimpleInstruction("AND", offset);

        case OpCode::OR:
            return SimpleInstruction("OR", offset);

        case OpCode::NOT:
            return SimpleInstruction("NOT", offset);

        case OpCode::LEFTSHIFT:
            return SimpleInstruction("LEFT-SHIFT", offset);

        case OpCode::RIGHTSHIFT:
            return SimpleInstruction("RIGHT-SHIFT", offset);

        case OpCode::MODULO:
            return SimpleInstruction("MODULO", offset);

        case OpCode::NEGATE:
            return SimpleInstruction("NEGATE", offset);

        case OpCode::PRINT:
            return SimpleInstruction("PRINT", offset);

        case OpCode::JUMP:
            return JumpInstruction("JUMP", this, 1, offset);

        case OpCode::JUMP_LONG:
            return LongInstruction("JUMP_LONG", this, offset);

        case OpCode::JUMP_FALSE:
            return JumpInstruction("JUMP_FALSE", this, 1, offset);

        case OpCode::JUMP_FALSE_LONG:
            return LongInstruction("JUMP_FALSE_LONG", this, offset);

        case OpCode::LOOP:
            return JumpInstruction("LOOP", this, -1, offset);

        case OpCode::LOOP_LONG:
            return LongInstruction("LOOP_LONG", this, offset);

        case OpCode::CALL:
            return ByteInstruction("CALL", this, offset);

        default:
            FMT_PRINT("Uknown opcode {}\n", static_cast<size_t>(instruction));
            return offset + 1;
    }
}
#endif

#endif
