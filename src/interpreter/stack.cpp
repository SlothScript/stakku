#include "stack.h"

using namespace stakku;

std::string Stack::serialize() const {
    std::ostringstream oss;
    for (size_t i = 0; i < stack.size(); i++) {
        oss << stack[i] << '\n';
    }
    return oss.str();
}

void Stack::deserialize(const std::string &save) {
    std::istringstream iss(save);
    std::string line;
    stack.clear();
    while (std::getline(iss, line)) {
        if (!line.empty()) {
            try {
                stack.push_back(std::stod(line));
            } catch (const std::invalid_argument &e) {
                // Handle invalid number format
                throw std::runtime_error("Invalid number format in serialized data: " + line);
            } catch (const std::out_of_range &e) {
                // Handle number too large/small for double
                throw std::runtime_error("Number out of range in serialized data: " + line);
            }
        }
    }
}
