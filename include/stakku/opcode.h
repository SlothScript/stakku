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

    OP_EQ,
    OP_LT,
    OP_GT,
    OP_LE,
    OP_GE,
    OP_NEQ,

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
// 0x06   OP_EQ       Pop 2, check if equal, push
// 0x07   OP_LT       Pop 2, determing <, push
// 0x08   OP_GT       Pop 2, determine >, push
// 0x09   OP_LE       Pop 2, determine <=, push
// 0x0A   OP_GE       Pop 2, determine >=, push
// 0x0B   OP_NEQ      Pop 2, determing if not equal, push
// 0x0C   OP_DUP      Duplicate top
// 0x0D   OP_DROP     Remove top
// 0x0E   OP_SWAP     Swap top two
// 0x0F   OP_OVER     xy -> xyx
// 0x10   OP_ROT      xyz -> yzx
// 0x11   OP_PRINT    Pop 1, print
// 0x12   OP_EMIT     Pop 1, print as ASCII
// 0x13   OP_CR       Print a newline
// 0x14   OP_JMP      Unconditional jump to unsigned 16 bit addr
// 0x15   OP_JMP_IF_Z Jump to u16 addr if 0
// 0x16   OP_CALL     Push return addr to call stack, jump to word's addr
// 0x17   OP_RETURN   Return from word
