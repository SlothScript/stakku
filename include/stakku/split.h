#pragma once

#include <algorithm>
#include <string>
#include <vector>

namespace stakku {

inline std::vector<std::string> split(const std::string &str,
                                      const std::vector<std::string> &delimiters) {
    std::vector<std::string> words;
    std::string current;

    for (size_t i = 0; i < str.size();) {
        // Treat '(' and ')' as standalone tokens regardless of surrounding whitespace
        if (str[i] == '(' || str[i] == ')') {
            if (!current.empty()) {
                words.push_back(current);
                current.clear();
            }
            words.emplace_back(1, str[i]);
            i++;
            continue;
        }

        bool isDelimiter = false;
        size_t matchLen = 0;
        for (const auto &delim : delimiters) {
            if (i + delim.size() <= str.size() && str.compare(i, delim.size(), delim) == 0 &&
                delim.size() >= matchLen) {
                isDelimiter = true;
                matchLen = delim.size();
            }
        }
        if (isDelimiter) {
            if (!current.empty())
                words.push_back(current);
            current.clear();
            // Ensure progress even if a zero-length delimiter sneaks in.
            i += (matchLen > 0 ? matchLen : 1);
        } else {
            current.push_back(str[i]);
            i++;
        }
    }
    if (!current.empty())
        words.push_back(current);
    return words;
}

} // namespace stakku
