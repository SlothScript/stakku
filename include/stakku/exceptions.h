#pragma once

#include <stdexcept>
#include <string>

namespace stakku {
class StakkuException : public std::runtime_error {
  public:
    StakkuException(std::string message) : std::runtime_error(message) {
    }
};

class StackUnderflow : public StakkuException {
  public:
    StackUnderflow() : StakkuException("stack underflow") {
    }
};

class DivideByZero : public StakkuException {
  public:
    DivideByZero() : StakkuException("Divide by zero") {
    }
};

class UnknownWord : public StakkuException {
  public:
    UnknownWord(std::string word) : StakkuException("Unknown word: " + word) {
    }
};

class InvalidNumber : public StakkuException {
  public:
    InvalidNumber(std::string number) : StakkuException("Invalid number: " + number) {
    }
};

class StackNotSet : public StakkuException {
  public:
    StackNotSet() : StakkuException("stack not set") {
    }
};

class OutOfRange : public StakkuException {
  public:
    OutOfRange(std::string message) : StakkuException("Out of range: " + message) {
    }
};

class UnmatchedComment : public StakkuException {
  public:
    UnmatchedComment(std::string word) : StakkuException("Unmatched comment: " + word) {
    }
};

class FunctionInFunction : public StakkuException {
  public:
    FunctionInFunction() : StakkuException("Cannot define a function inside another function") {
    }
};

class UnmatchedFunction : public StakkuException {
  public:
    UnmatchedFunction() : StakkuException("Unmatched function definition (; without :)") {
    }
};

class UnnamedFunction : public StakkuException {
  public:
    UnnamedFunction() : StakkuException("Function definition missing name (: name;)") {
    }
};

class UnmatchedControlWord : public StakkuException {
  public:
    UnmatchedControlWord(std::string word) : StakkuException("Unmatched control word: " + word) {
    }
};

} // namespace stakku
