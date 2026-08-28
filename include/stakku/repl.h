#pragma once

#include <memory>
#include <string>
#include <utility>

#include "interpreter/compiler.h"
#include "interpreter/stack.h"
#include "interpreter/vm.h"

namespace stakku {
class REPL {
  public:
    REPL() : stack_(std::make_shared<Stack>()), compiler(), vm() {
    }
    explicit REPL(const std::string &contextFile);

    void open();
    void saveSession(const std::string &path) const;
    void loadSession(const std::string &path);

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
