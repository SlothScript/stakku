#include "stack.h"
#include <iomanip>
#include <limits>
#include <sstream>

using namespace stakku;

std::string Stack::serialize() const {
    std::ostringstream oss;
    oss << std::setprecision(std::numeric_limits<double>::max_digits10);

    for (size_t i = 0; i < stack.size(); i++) {
        oss << stack[i] << '\n';
    }

    return oss.str();
}

void Stack::deserialize(const std::string &save) {
    std::istringstream iss(save);
    std::string line;
    std::vector<double> temp;
    while (std::getline(iss, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty() || line == "done") {
            continue;
        }
        try {
            size_t idx = 0;
            double val = std::stod(line, &idx);
            if (idx != line.size()) {
                throw std::invalid_argument("Trailing characters after number");
            }
            temp.push_back(val);
        } catch (const std::invalid_argument &e) {
            throw std::runtime_error("Invalid number format in serialized data: " + line);
        } catch (const std::out_of_range &e) {
            throw std::runtime_error("Number out of range in serialized data: " + line);
        }
    }
    stack.clear();
    stack = std::move(temp);
}
