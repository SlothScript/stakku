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
    void mod();
    void eq();
    void lt();
    void gt();
    void le();
    void ge();
    void neq();
    void and_();
    void or_();
    void xor_();
    void not_();
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
};
} // namespace stakku
