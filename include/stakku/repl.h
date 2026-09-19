#pragma once

#include <memory>
#include <string>
#include <utility>

#include "compiler/compiler.h"
#include "compiler/stack.h"
#include "compiler/vm.h"

namespace stakku {
class REPL {
  public:
    REPL() : stack_(std::make_shared<Stack>()), compiler(), vm() {
    }
    explicit REPL(const std::string &contextFile);

    void open();
    void saveSession(const std::string &path) const;
    void loadSession(const std::string &path);

    bool runProgram(const std::string &filename);

    void setStack(std::shared_ptr<Stack> stack) {
        stack_ = std::move(stack);
    }

    bool processSpecial(const std::string &line);

  private:
    bool running_ = true;
    std::shared_ptr<Stack> stack_;
    Compiler compiler;
    VM vm;
    void processLine(const std::string &line);
};
} // namespace stakku
