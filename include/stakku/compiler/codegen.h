#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace stakku {
class CodeGen {
  public:
    // Generate Apple Silicon (ARM64 Darwin) assembly from VM bytecode.
    std::string generate(const std::vector<uint8_t> &bytecode);

  private:
    void emit(const std::string &text);

    void emitPushValue(double value);
    double readValue(const std::vector<uint8_t> &bytecode, size_t &pc);

    std::string toHex(uint64_t bits);

    std::string assembly;
    std::string data;
    size_t nextConstantLabel = 0;
};
} // namespace stakku
