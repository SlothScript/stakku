#pragma once

#include "stack.h"
#include <cstdint>
#include <functional>
#include <vector>

namespace stakku {
using OpFunc = std::function<void()>;

class VM {
  public:
    void execute(const std::vector<uint8_t> &ops, const std::string &initialStack = "");
    const Stack &getStack() const {
        return stack;
    }
    bool hadOutput() const {
        return hadOutput_;
    }
    bool halted() const {
        return halted_;
    }
    void resetState() {
        hadOutput_ = false;
    }
    void abort() {
        stack.clear();
        returnStack.clear();
        hadOutput_ = false;
        running = false;
    }
    std::string saveStack() const {
        return stack.serialize();
    }

  private:
    Stack stack;
    std::vector<size_t> returnStack;
    size_t pc = 0;
    bool running = true;
    bool hadOutput_ = false;
    bool halted_ = false;

    void halt();
    void bye();
    void push(const std::vector<uint8_t> &bytecode);
    void add();
    void sub();
    void mul();
    void div_();
    void eq();
    void lt();
    void gt();
    void le();
    void ge();
    void neq();
    void dup();
    void drop();
    void swap();
    void over();
    void rot();
    void print();
    void emit();
    void cr();
    void clear();
    void jmp(const std::vector<uint8_t> &bytecode);
    void jmp_if_z(const std::vector<uint8_t> &bytecode);
    void call(const std::vector<uint8_t> &bytecode);
    void call_return(const std::vector<uint8_t> &bytecode);

    std::string toHex(uint8_t v);

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
};
} // namespace stakku
