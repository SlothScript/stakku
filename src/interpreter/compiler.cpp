#include "compiler.h"
#include "exceptions.h"
#include "opcode.h"
#include <cstdint>
#include <iostream>
#include <limits>
#include <unordered_set>
#include <vector>

using namespace stakku;

void Compiler::emit(OpCode op) {
    if (isDefining) {
        defBytecode.push_back(static_cast<uint8_t>(op));
        return;
    }
    bytecode.push_back(static_cast<uint8_t>(op));
}

bool Compiler::lookupWord(const std::string &name, size_t &offset) const {
    auto it = wordDict.find(name);
    if (it != wordDict.end()) {
        offset = it->second;
        return true;
    }
    size_t accumulated = allDefs.size();
    for (const auto &pdef : pendingDefs) {
        if (pdef.name == name) {
            offset = accumulated;
            return true;
        }
        accumulated += pdef.bytecode.size();
    }
    return false;
}

bool Compiler::isReservedName(const std::string &name) const {
    static const std::unordered_set<std::string> reserved = {":",    ";", "if", "else",
                                                             "then", "(", ")"};
    if (reserved.find(name) != reserved.end())
        return true;

    if (isNumber(name))
        return true;

    return simpleWords.find(name) != simpleWords.end();
}

bool Compiler::isNumber(const std::string &name) const {
    if (name.empty())
        return false;

    size_t i = 0;
    if (name[i] == '+' || name[i] == '-')
        ++i;
    if (i == name.size())
        return false;

    bool sawDigit = false, sawDot = false;
    for (; i < name.size(); ++i) {
        if (std::isdigit(static_cast<unsigned char>(name[i]))) {
            sawDigit = true;
        } else if (name[i] == '.' && !sawDot) {
            sawDot = true;
        } else {
            return false;
        }
    }
    return sawDigit;
}

void Compiler::emitPushNum(double value) {
    emit(OpCode::OP_PUSH_NUM);
    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&value);
    auto &buf = isDefining ? defBytecode : bytecode;
    buf.insert(buf.end(), bytes, bytes + sizeof(double));
}

size_t Compiler::emitJmp(OpCode op) {
    emit(op);

    if (isDefining) {
        size_t patchPos = defBytecode.size();
        defBytecode.push_back(0);
        defBytecode.push_back(0);
        return patchPos;
    }

    size_t patchPos = bytecode.size();
    bytecode.push_back(0); // placeholder bytes
    bytecode.push_back(0);
    return patchPos;
}

void Compiler::patchJmp(size_t patchPos, size_t target) {
    auto &buf = isDefining ? defBytecode : bytecode;
    if (target > std::numeric_limits<uint16_t>::max()) {
        throw StakkuException("Jump target exceeds 16-bit limit: " + std::to_string(target));
    }
    uint16_t target16 = static_cast<uint16_t>(target);
    if (isDefining) {
        currentDefJmpPatches.emplace_back(patchPos, target16);
    } else {
        std::memcpy(&buf[patchPos], &target16, sizeof(uint16_t));
    }
}

void Compiler::parseStakkuError(const StakkuException &e, const Word &word,
                                const std::vector<Word> &words) {
    std::cerr << std::endl << std::endl;

    // Print two lines above (if any)
    for (int ln = static_cast<int>(word.line) - 2; ln <= static_cast<int>(word.line) - 1; ++ln) {
        if (ln <= 0)
            continue;
        std::string lineText;
        for (const auto &w : words) {
            if (static_cast<int>(w.line) == ln) {
                if (!lineText.empty())
                    lineText += ' ';
                lineText += w.value;
            }
        }
        if (!lineText.empty()) {
            std::cerr << ln << " " << lineText << std::endl;
        }
    }

    // Build the full current line text and compute caret position for the offending token
    std::string fullLine;
    size_t caretOffset = 0;
    for (size_t k = 0; k < words.size(); ++k) {
        if (words[k].line != word.line)
            continue;
        if (!fullLine.empty())
            fullLine += ' ';
        if (&words[k] == &word) {
            caretOffset = fullLine.size();
        }
        fullLine += words[k].value;
    }

    std::cerr << word.line << " " << fullLine << std::endl;
    std::cerr << std::string(std::to_string(word.line).size() + 1 + caretOffset, ' ')
              << std::string(word.value.size(), '^') << " " << e.what() << std::endl;
}

// Should probably be refactored into more functions, but it (maybe) works so I'm not going to touch
// it
std::vector<uint8_t> Compiler::compile(const std::vector<Word> &words) {
    bytecode.clear();
    patchStack.clear();
    topLevelCallPatches.clear();
    currentDefCallPatches.clear();
    currentDefJmpPatches.clear();
    commentDepth = 0;
    isDefining = false;
    expectName = false;
    defName.clear();
    defBytecode.clear();
    pendingDefs.clear();

    const Word *currentWord = nullptr;
    try {
        for (const Word &word : words) {
            currentWord = &word;

            if (word.value == "(") {
                commentDepth++;
                continue;
            }
            if (word.value == ")") {
                if (commentDepth == 0)
                    throw UnmatchedComment(word.value);
                commentDepth--;
                continue;
            }
            if (word.value.empty() || commentDepth > 0)
                continue;

            if (word.value == ":") {
                if (isDefining)
                    throw FunctionInFunction();
                isDefining = true;
                expectName = true;
                continue;
            }
            if (expectName) {
                if (isReservedName(word.value)) {
                    throw StakkuException("Cannot use reserved word as a definition name: " +
                                          word.value);
                }
                defName = word.value;
                expectName = false;
                continue;
            }

            if (word.value == ";") {
                if (!isDefining)
                    throw UnmatchedFunction();
                if (defName.empty())
                    throw UnnamedFunction();
                if (!patchStack.empty())
                    throw UnmatchedControlWord(currentWord->value);

                defBytecode.push_back(static_cast<uint8_t>(OpCode::OP_RETURN));

                pendingDefs.push_back(
                    {defName, defBytecode, currentDefCallPatches, currentDefJmpPatches});
                currentDefCallPatches.clear();
                currentDefJmpPatches.clear();

                defBytecode.clear();
                defName.clear();
                isDefining = false;
                continue;
            }

            try {
                size_t idx = 0;
                double val = std::stod(word.value, &idx);
                if (idx == word.value.size()) {
                    emitPushNum(val);
                    continue;
                }
            } catch (const std::invalid_argument &) {
            } catch (const std::out_of_range &) {
                throw InvalidNumber(word.value);
            }

            if (word.value == "if") {
                patchStack.push_back(emitJmp(OpCode::OP_JMP_IF_Z));
                continue;
            } else if (word.value == "else") {
                if (patchStack.empty())
                    throw UnmatchedControlWord(currentWord->value);
                auto &buf = isDefining ? defBytecode : bytecode;
                size_t patchPos = emitJmp(OpCode::OP_JMP);
                patchJmp(patchStack.back(), buf.size());
                patchStack.pop_back();
                patchStack.push_back(patchPos);
                continue;
            } else if (word.value == "then") {
                if (patchStack.empty())
                    throw UnmatchedControlWord(currentWord->value);
                auto &buf = isDefining ? defBytecode : bytecode;
                patchJmp(patchStack.back(), buf.size());
                patchStack.pop_back();
                continue;
            }

            size_t offset = 0;
            if (lookupWord(word.value, offset)) {
                emit(OpCode::OP_CALL);
                auto &buf = isDefining ? defBytecode : bytecode;
                size_t opPos = buf.size();
                buf.insert(buf.end(), sizeof(uint16_t), 0);
                if (isDefining)
                    currentDefCallPatches.push_back({opPos, offset});
                else
                    topLevelCallPatches.push_back({opPos, offset});
                continue;
            }

            auto it2 = simpleWords.find(word.value);
            if (it2 != simpleWords.end()) {
                emit(it2->second);
            } else {
                throw UnknownWord(word.value);
            }
        }
        if (!patchStack.empty())
            throw UnmatchedControlWord(currentWord->value);
        if (isDefining)
            throw UnmatchedFunction();
        if (commentDepth > 0)
            throw UnmatchedComment("(");

        for (const auto &pdef : pendingDefs) {
            if (wordDict.find(pdef.name) != wordDict.end()) {
                throw StakkuException("Duplicate definition: " + pdef.name);
            }
            for (const auto &other : pendingDefs) {
                if (&other != &pdef && other.name == pdef.name) {
                    throw StakkuException("Duplicate definition: " + pdef.name);
                }
            }
        }

        for (auto &pdef : pendingDefs) {
            size_t wordStart = allDefs.size();
            wordDict.insert({pdef.name, wordStart});
            for (auto &[localPos, target] : pdef.callPatches)
                defsCallPatches.push_back({wordStart + localPos, target});
            for (auto &[localPos, target] : pdef.jmpPatches)
                defsJmpPatches.emplace_back(wordStart + localPos, wordStart + target);
            allDefs.insert(allDefs.end(), pdef.bytecode.begin(), pdef.bytecode.end());
        }
        pendingDefs.clear();

    } catch (const StakkuException &e) {
        if (currentWord)
            parseStakkuError(e, *currentWord, words);
        return {};
    }

    bytecode.push_back(static_cast<uint8_t>(OpCode::OP_HALT));
    size_t defsBaseAddress = bytecode.size();
    bytecode.insert(bytecode.end(), allDefs.begin(), allDefs.end());

    auto patchCall = [&](size_t pos, size_t targetOffset) {
        size_t addr = defsBaseAddress + targetOffset;
        if (addr > std::numeric_limits<uint16_t>::max()) {
            throw StakkuException(
                "Program too large: compiled bytecode exceeds the 16-bit address space (" +
                std::to_string(addr) + " > 65535 bytes). Definitions and code must fit in 64 KB.");
        }
        uint16_t addr16 = static_cast<uint16_t>(addr);
        std::memcpy(&bytecode[pos], &addr16, sizeof(addr16));
    };
    for (auto &[pos, target] : topLevelCallPatches)
        patchCall(pos, target);
    for (auto &[relPos, target] : defsCallPatches)
        patchCall(defsBaseAddress + relPos, target);
    for (auto &[relPos, target] : defsJmpPatches)
        patchCall(defsBaseAddress + relPos, target);

    return bytecode;
}
