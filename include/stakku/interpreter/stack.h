#pragma once

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace stakku {
class StackUnderflow : public std::runtime_error {
  public:
    StackUnderflow() : std::runtime_error("stack underflow") {
    }
};

class Stack {
  public:
    // Stack ops
    void push(double value) {
        stack.push_back(value);
    }

    double pop() {
        if (stack.empty())
            throw StackUnderflow();
        double v = stack.back();
        stack.pop_back();
        return v;
    }

    double peek(size_t depthFromTop = 0) const {
        if (depthFromTop >= stack.size())
            throw StackUnderflow();
        return stack[stack.size() - 1 - depthFromTop];
    }

    bool empty() const {
        return stack.empty();
    }

    size_t size() const {
        return stack.size();
    }

    void clear() {
        stack.clear();
    }

    // Saving and loading
    std::string serialize() const;
    void deserialize(const std::string &save);

  private:
    std::vector<double> stack;
};
} // namespace stakku
