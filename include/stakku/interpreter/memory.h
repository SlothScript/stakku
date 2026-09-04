#pragma once
#include "exceptions.h"
#include <vector>

namespace stakku {
class Memory {
  public:
    double allocate(double initialValue = 0.0) {
        cells.push_back(initialValue);
        return static_cast<double>(cells.size() - 1);
    }
    double fetch(double addr) const {
        size_t i = checkedIndex(addr);
        return cells[i];
    }
    void store(double addr, double value) {
        size_t i = checkedIndex(addr);
        cells[i] = value;
    }
    void clear() {
        cells.clear();
    }

  private:
    size_t checkedIndex(double addr) const {
        if (addr < 0 || static_cast<size_t>(addr) >= cells.size())
            throw OutOfRange("memory address out of bounds");
        return static_cast<size_t>(addr);
    }
    std::vector<double> cells;
};
} // namespace stakku
