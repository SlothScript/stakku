#pragma once

#include <cstdint>

namespace stakku {

enum class OpCode : uint8_t {
    OP_HALT,
    OP_PUSH_NUM,

    OP_TO_R,
    OP_FROM_R,
    OP_FETCH_R,

    OP_STORE,
    OP_FETCH,

    OP_J,
    OP_LOOP,

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
    OP_EEMIT,
    OP_CR,

    OP_JMP,
    OP_JMP_IF_Z,
    OP_CALL,
    OP_RETURN,
    OP_CLEAR,
    OP_BYE,
    OP_ALLOC,
};

}

// Opcode Name        Description
// -------------------------------------------------------------------------
// 0x00   OP_HALT     Stop the program
// 0x01   OP_PUSH_NUM Pushes 8 bytes from the bytecode to the stack (double)
// 0x02   OP_TO_R     Pop data stack, push to return stack (>R)
// 0x03   OP_FROM_R   Pop return stack, push to data stack (R>)
// 0x04   OP_FETCH_R  Peek top of return stack, push copy (R@)
// 0x05   OP_STORE    Pop addr, pop value, write value into memory[addr]
// 0x06   OP_FETCH    Pop addr, read memory[addr], push value to data stack
// 0x07   OP_J        Peek outer loop index from return stack
// 0x08   OP_LOOP     Increment loop counter, jump back if < limit
// 0x09   OP_ADD      Pop 2, add, push
// 0x0A   OP_SUB      Pop 2, sub, push
// 0x0B   OP_MUL      Pop 2, mul, push
// 0x0C   OP_DIV      Pop 2, div, push
// 0x0D   OP_MOD      Pop 2, modulo, push
// 0x0E   OP_EQ       Pop 2, check if equal, push
// 0x0F   OP_LT       Pop 2, determining <, push
// 0x10   OP_GT       Pop 2, determine >, push
// 0x11   OP_LE       Pop 2, determine <=, push
// 0x12   OP_GE       Pop 2, determine >=, push
// 0x13   OP_NEQ      Pop 2, determining if not equal, push
// 0x14   OP_AND      Pop 2, bitwise AND, push
// 0x15   OP_OR       Pop 2, bitwise OR, push
// 0x16   OP_NOT      Pop 1, bitwise NOT, push
// 0x17   OP_XOR      Pop 2, bitwise XOR, push
// 0x18   OP_DUP      Duplicate top
// 0x19   OP_DROP     Remove top
// 0x1A   OP_SWAP     Swap top two
// 0x1B   OP_OVER     xy -> xyx
// 0x1C   OP_ROT      xyz -> yzx
// 0x1D   OP_PRINT    Pop 1, print
// 0x1E   OP_EMIT     Pop 1, print as ASCII
// 0x1F   OP_EEMIT    Pop 1, print as ASCII to stderr
// 0x20   OP_CR       Print a newline
// 0x21   OP_JMP      Unconditional jump to unsigned 16 bit addr
// 0x22   OP_JMP_IF_Z Jump to u16 addr if 0
// 0x23   OP_CALL     Push return addr to call stack, jump to word's addr
// 0x24   OP_RETURN   Return from word
// 0x25   OP_CLEAR    Clear the stack
// 0x26   OP_BYE      Stop the program (alias for HALT)
// 0x27   OP_ALLOC    Allocate N memory cells (u16 operand)
