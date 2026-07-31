#pragma once

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "interpreter.h"
#include "split.h"
#include "stack.h"

namespace stakku {
class REPL {
  public:
    REPL() : stack_(std::make_shared<Stack>()), interp() {
        interp.setStack(stack_);
    }
    explicit REPL(const std::string &contextFile);

    void open();
    void saveSession(const std::string &path) const;
    void loadSession(const std::string &path);

    bool processSpecial(const std::string &line);

  private:
    bool running_ = true;
    std::shared_ptr<Stack> stack_;
    Interpreter interp;
    void processLine(const std::string &line);
};
} // namespace stakku
