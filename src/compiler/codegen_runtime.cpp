#include "compiler/stack.h"
#include "codegen_runtime.h"

#include <iostream>

namespace {
stakku::Stack dataStack;
}

extern "C" void stakku_push(double value) {
    dataStack.push(value);
}

extern "C" void stakku_add() {
    const double b = dataStack.pop();
    const double a = dataStack.pop();
    dataStack.push(a + b);
}

extern "C" void stakku_print() {
    const double value = dataStack.pop();
    std::cout << value;
}

extern "C" void stakku_reset() {
    dataStack.clear();
}

