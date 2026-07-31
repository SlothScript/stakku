#include "interpreter.h"
#include <stdexcept>

using namespace stakku;

void Interpreter::ensureStackSet() const {
    if (!sharedStack_) {
        throw std::runtime_error("stack not set");
    }
}

void Interpreter::interpret(const std::vector<std::string> &words) {
    bool isComment = false;
    for (const std::string &word : words) {
        if (word == "(") {
            isComment = true;
            continue;
        } else if (word == ")") {
            isComment = false;
            continue;
        }

        if (word.empty() || isComment)
            continue;

        // Try parsing the token as a number first (accepts ".5", "+3", "-2", etc.)
        try {
            size_t idx = 0;
            double val = std::stod(word, &idx);
            if (idx == word.size()) {
                push(val);
                continue;
            }
        } catch (const std::invalid_argument &) {
            // not a number; fall through
        } catch (const std::out_of_range &) {
            std::cerr << "Invalid number (out of range) >>> " << word << " <<<" << std::endl;
            throw std::runtime_error(std::string("Invalid number: ") + word);
        }

        auto it = wordTable.find(word);
        if (it != wordTable.end()) {
            it->second();
        } else {
            throw std::runtime_error(std::string("Unknown word: ") + word);
        }
    }
}

void Interpreter::cr() {
    std::cout << '\n';
}

void Interpreter::push(double value) {
    ensureStackSet();
    sharedStack_->push(value);
}

void Interpreter::arithmetic(char type) {
    ensureStackSet();
    // Validate stack depth before mutating so an underflow throw
    // doesn't leave the stack in a partially-popped state.
    if (sharedStack_->size() < 2) {
        throw StackUnderflow();
    }

    // For division, check the divisor (top of stack) for zero before
    // popping either operand. Throw on error so callers consistently
    // receive exceptions rather than relying on hadError_ flag.
    if (type == '/' && sharedStack_->peek() == 0.0) {
        throw std::runtime_error("Division by zero");
    }

    double b = sharedStack_->pop();
    double a = sharedStack_->pop();
    switch (type) {
    case '+':
        sharedStack_->push(a + b);
        break;
    case '-':
        sharedStack_->push(a - b);
        break;
    case '*':
        sharedStack_->push(a * b);
        break;
    case '/':
        sharedStack_->push(a / b);
        break;
    }
}

void Interpreter::print() {
    ensureStackSet();
    std::cout << sharedStack_->pop();
    needsNewline_ = true;
}

void Interpreter::dup() {
    ensureStackSet();
    sharedStack_->push(sharedStack_->peek());
}

void Interpreter::drop() {
    ensureStackSet();
    sharedStack_->pop();
}

void Interpreter::swap() {
    ensureStackSet();
    // Check stack depth before any pops so a single-element stack
    // doesn't end up half-consumed before underflow throws.
    if (sharedStack_->size() < 2) {
        throw StackUnderflow();
    }
    double top = sharedStack_->pop();
    double second = sharedStack_->pop();
    sharedStack_->push(top);
    sharedStack_->push(second);
}

void Interpreter::over() {
    ensureStackSet();
    // Guard the peek so underflow throws before anything else happens.
    if (sharedStack_->size() < 2) {
        throw StackUnderflow();
    }
    sharedStack_->push(sharedStack_->peek(1));
}

void Interpreter::emit() {
    ensureStackSet();
    double val = sharedStack_->pop();
    if (val < 0 || val > 255) {
        throw std::runtime_error("emit: value out of ASCII range");
    }
    std::cout << static_cast<char>(val);
    needsNewline_ = true;
}

void Interpreter::clear() {
    ensureStackSet();
    sharedStack_->clear();
}
