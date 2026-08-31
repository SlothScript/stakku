#pragma once

#include <cstdint>

namespace stakku {

enum class OpCode : uint8_t {
    OP_HALT,
    OP_PUSH_NUM,

    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,

    OP_EQ,
    OP_LT,
    OP_GT,
    OP_LE,
    OP_GE,
    OP_NEQ,
    OP_AND,
    OP_OR,
    OP_NOT,
    OP_XOR,

    OP_DUP,
    OP_DROP,
    OP_SWAP,
    OP_OVER,
    OP_ROT,

    OP_PRINT,
    OP_EMIT,
    OP_CR,

    OP_JMP,
    OP_JMP_IF_Z,
    OP_CALL,
    OP_RETURN,
    OP_CLEAR,
    OP_BYE,
};

}

// Opcode Name        Description
// -------------------------------------------------------------------------
// 0x00   OP_HALT     Stop the program
// 0x01   OP_PUSH_NUM Pushes 8 bytes from the bytecode to the stack (double)
// 0x02   OP_ADD      Pop 2, add, push
// 0x03   OP_SUB      Pop 2, sub, push
// 0x04   OP_MUL      Pop 2, mul, push
// 0x05   OP_DIV      Pop 2, div, push
// 0x06   OP_MOD      Pop 2, modulo, push
// 0x07   OP_EQ       Pop 2, check if equal, push
// 0x08   OP_LT       Pop 2, determining <, push
// 0x09   OP_GT       Pop 2, determine >, push
// 0x0A   OP_LE       Pop 2, determine <=, push
// 0x0B   OP_GE       Pop 2, determine >=, push
// 0x0C   OP_NEQ      Pop 2, determining if not equal, push
// 0x0D   OP_AND      Pop 2, bitwise AND, push
// 0x0E   OP_OR       Pop 2, bitwise OR, push
// 0x0F   OP_NOT      Pop 1, bitwise NOT, push
// 0x10   OP_XOR      Pop 2, bitwise XOR, push
// 0x11   OP_DUP      Duplicate top
// 0x12   OP_DROP     Remove top
// 0x13   OP_SWAP     Swap top two
// 0x14   OP_OVER     xy -> xyx
// 0x15   OP_ROT      xyz -> yzx
// 0x16   OP_PRINT    Pop 1, print
// 0x17   OP_EMIT     Pop 1, print as ASCII
// 0x18   OP_CR       Print a newline
// 0x19   OP_CLEAR    Clear the stack
// 0x1A   OP_JMP      Unconditional jump to unsigned 16 bit addr
// 0x1B   OP_JMP_IF_Z Jump to u16 addr if 0
// 0x1C   OP_CALL     Push return addr to call stack, jump to word's addr
// 0x1D   OP_RETURN   Return from word
// 0x1E   OP_BYE      Stop the program (alias for HALT)
