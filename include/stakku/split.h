#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "interpreter/compiler.h"

namespace stakku {

inline std::vector<Word> split(const std::string &str, const std::vector<std::string> &delimiters) {
    std::vector<Word> words;
    std::string current;
    int line = 1; // Initialize line number to 1
    bool skipLine = false;

    for (size_t i = 0; i < str.size();) {
        if (skipLine) {
            if (str[i] == '\r' && i + 1 < str.size() && str[i + 1] == '\n') {
                line++;
                i += 2;
                skipLine = false;
                continue;
            }
            if (str[i] == '\n') {
                line++;
                i++;
                skipLine = false;
                continue;
            }
            i++;
            continue;
        }

        // Check for Windows-style newline (\r\n)
        if (str[i] == '\r' && i + 1 < str.size() && str[i + 1] == '\n') {
            if (!current.empty()) {
                words.emplace_back(current, line);
                current.clear();
            }
            line++;
            i += 2; // Skip both '\r' and '\n'
            continue;
        }

        // Check for Unix-style newline (\n)
        if (str[i] == '\n') {
            if (!current.empty()) {
                words.emplace_back(current, line);
                current.clear();
            }
            line++;
            i++;
            continue;
        }

        // Treat '(' and ')' as standalone tokens regardless of surrounding whitespace
        if (str[i] == '(' || str[i] == ')') {
            if (!current.empty()) {
                words.emplace_back(current, line);
                current.clear();
            }
            words.emplace_back(std::string(1, str[i]), line);
            i++;
            continue;
        }

        // Ignore line-level comments with '\'
        if (str[i] == '\\') {
            skipLine = true;
            i++;
            continue;
        }

        // Treat tab as whitespace delimiter
        if (str[i] == '\t') {
            if (!current.empty()) {
                words.emplace_back(current, line);
                current.clear();
            }
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
                words.emplace_back(current, line);
            current.clear();
            // Ensure progress even if a zero-length delimiter sneaks in.
            i += (matchLen > 0 ? matchLen : 1);
        } else {
            current.push_back(str[i]);
            i++;
        }
    }
    if (!current.empty())
        words.emplace_back(current, line);
    return words;
}

} // namespace stakku
