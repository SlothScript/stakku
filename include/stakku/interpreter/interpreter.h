#pragma once

#include <cctype>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "stack.h"

namespace stakku {
using WordFunc = std::function<void()>;

struct Word {
    std::string value;
    int line = 1;

    Word() = default;
    Word(std::string value_, int line_ = 1) : value(std::move(value_)), line(line_) {
    }
    Word(const char *value_, int line_ = 1) : value(value_), line(line_) {
    }
};

class Interpreter {
  public:
    bool hadError() const {
        return hadError_;
    }
    bool needsNewline() const {
        return needsNewline_;
    }

    void interpret(const std::vector<Word> &words);
    void setStack(std::shared_ptr<Stack> stack) {
        sharedStack_ = std::move(stack);
    }
    std::shared_ptr<Stack> getStack() const {
        return sharedStack_;
    }
    void resetStates() {
        needsNewline_ = false;
        hadError_ = false;
    }

  private:
    bool needsNewline_ = false;
    bool hadError_ = false;

    // current supported ops ===========
    // number:  push to stack
    // + - * /: arithmetic
    // .:       print top (and pop)
    // dup:     duplicate top
    // drop:    pop top
    // swap:    swap top two
    // over:    copy second item to top
    // emit:    print an ASCII character
    // clear:   empty stack

    void push(double value);
    void arithmetic(char type);
    void print();

    void cr();
    void dup();
    void drop();
    void swap();
    void over();
    void emit();
    void clear();

    // ensure a stack has been provided; throws std::runtime_error if not set
    void ensureStackSet() const;

    std::unordered_map<std::string, WordFunc> wordTable = {
        {"+", [this] { arithmetic('+'); }}, {"-", [this] { arithmetic('-'); }},
        {"*", [this] { arithmetic('*'); }}, {"/", [this] { arithmetic('/'); }},
        {".", [this] { print(); }},         {"cr", [this] { cr(); }},
        {"dup", [this] { dup(); }},         {"drop", [this] { drop(); }},
        {"swap", [this] { swap(); }},       {"over", [this] { over(); }},
        {"emit", [this] { emit(); }},       {"clear", [this] { clear(); }},
    };

    std::shared_ptr<Stack> sharedStack_;
};
} // namespace stakku
