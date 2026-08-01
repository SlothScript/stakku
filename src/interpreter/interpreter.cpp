#include "interpreter.h"
#include <stdexcept>

using namespace stakku;

void Interpreter::ensureStackSet() const {
    if (!sharedStack_) {
        throw StackNotSet();
    }
}

void Interpreter::interpret(const std::vector<Word> &words) {
    bool isComment = false;
    for (const Word &word : words) {
        try {
            if (word.value == "(") {
                isComment = true;
                continue;
            } else if (word.value == ")") {
                isComment = false;
                continue;
            }

            if (word.value.empty() || isComment)
                continue;

            // Try parsing the token as a number first (accepts ".5", "+3", "-2", etc.)
            try {
                size_t idx = 0;
                double val = std::stod(word.value, &idx);
                if (idx == word.value.size()) {
                    push(val);
                    continue;
                }
            } catch (const std::invalid_argument &) {
                // not a number; fall through
            } catch (const std::out_of_range &) {
                throw InvalidNumber(word.value);
            }

            auto it = wordTable.find(word.value);
            if (it != wordTable.end()) {
                it->second();
            } else {
                throw UnknownWord(word.value);
            }
        } catch (const StakkuException &e) {
            // format error messages like this:
            // prev line
            // prev line
            // line rest of the line with tokens
            //          ^^^^^^^ error message
            std::cerr << std::endl << std::endl;

            // Print two lines above (if any)
            for (int ln = static_cast<int>(word.line) - 2; ln <= static_cast<int>(word.line) - 1;
                 ++ln) {
                if (ln <= 0)
                    continue;
                // gather all tokens on that line
                std::string lineText;
                for (const auto &w : words) {
                    if (static_cast<int>(w.line) == ln) {
                        if (!lineText.empty())
                            lineText += ' ';
                        lineText += w.value;
                    }
                }
                if (!lineText.empty()) {
                    std::cerr << ln << " " << lineText << std::endl;
                }
            }

            // Build the full current line text and compute caret position for this token
            std::string fullLine;
            size_t caretOffset = 0;
            for (size_t k = 0; k < words.size(); ++k) {
                if (words[k].line != word.line)
                    continue;
                if (!fullLine.empty())
                    fullLine += ' ';
                // If this is the word that caused the error, record offset before adding it
                if (&words[k] == &word) {
                    caretOffset = fullLine.size();
                }
                fullLine += words[k].value;
            }

            std::cerr << word.line << " " << fullLine << std::endl;
            // Print caret line aligned under the offending token
            std::cerr << std::string(std::to_string(word.line).size() + 1 + caretOffset, ' ')
                      << std::string(word.value.size(), '^') << " " << e.what() << std::endl;
            hadError_ = true;
            break; // Stop processing further words on error
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
        throw DivideByZero();
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

void Interpreter::comparison(char type) {
    ensureStackSet();
    if (sharedStack_->size() < 2) {
        throw StackUnderflow();
    }

    double b = sharedStack_->pop();
    double a = sharedStack_->pop();
    switch (type) {
    case '=':
        sharedStack_->push(a == b ? 1.0 : 0.0);
        break;
    case '<':
        sharedStack_->push(a < b ? 1.0 : 0.0);
        break;
    case '>':
        sharedStack_->push(a > b ? 1.0 : 0.0);
        break;
    }
}

void Interpreter::logical(char type) {
    ensureStackSet();
    if (sharedStack_->size() < 2) {
        throw StackUnderflow();
    }

    // Negation is a unary operator, so we handle it separately.
    if (type == '!') {
        double a = sharedStack_->pop();
        sharedStack_->push(a == 0.0 ? 1.0 : 0.0);
        return;
    }

    double b = sharedStack_->pop();
    double a = sharedStack_->pop();
    switch (type) {
    case '&':
        sharedStack_->push((a != 0.0 && b != 0.0) ? 1.0 : 0.0);
        break;
    case '|':
        sharedStack_->push((a != 0.0 || b != 0.0) ? 1.0 : 0.0);
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
        throw OutOfRange("emit value must be in range 0-255, got: " + std::to_string(val));
    }
    std::cout << static_cast<char>(val);
    needsNewline_ = true;
}

void Interpreter::clear() {
    ensureStackSet();
    sharedStack_->clear();
}
