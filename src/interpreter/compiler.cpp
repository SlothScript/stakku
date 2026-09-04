#include "compiler.h"
#include "exceptions.h"
#include "opcode.h"
#include <cctype>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <unordered_set>
#include <vector>

using namespace stakku;

namespace {
std::string toLower(const std::string &s) {
    std::string r = s;
    for (char &c : r)
        c = std::tolower(c);
    return r;
}
} // namespace

void Compiler::emit(OpCode op) {
    if (isDefining) {
        defBytecode.push_back(static_cast<uint8_t>(op));
        return;
    }
    bytecode.push_back(static_cast<uint8_t>(op));
}

bool Compiler::lookupWord(const std::string &name, size_t &offset) const {
    std::string lower = toLower(name);
    auto it = wordDict.find(lower);
    if (it != wordDict.end()) {
        offset = it->second;
        return true;
    }
    size_t accumulated = allDefs.size();
    for (const auto &pdef : pendingDefs) {
        if (pdef.name == lower) {
            offset = accumulated;
            return true;
        }
        accumulated += pdef.bytecode.size();
    }
    return false;
}

bool Compiler::isReservedName(const std::string &name) const {
    static const std::unordered_set<std::string> reserved = {
        ":",      ";",  "if",   "else", "then", "(",     ")", "begin", "while",
        "repeat", "do", "loop", "i",    "j",    "leave", "@", "!",     "variable"};
    std::string lower = toLower(name);
    if (reserved.find(lower) != reserved.end())
        return true;

    if (isNumber(name))
        return true;

    return simpleWords.find(lower) != simpleWords.end();
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
    leavePatchStack.clear();
    commentDepth = 0;
    isDefining = false;
    expectName = false;
    expectVariable = false;
    defName.clear();
    defBytecode.clear();
    pendingDefs.clear();
    variables.clear();
    nextMemAddr = 0;

    const Word *currentWord = nullptr;
    try {
        for (const Word &word : words) {
            currentWord = &word;

            std::string lower = toLower(word.value);

            if (lower == "(") {
                commentDepth++;
                continue;
            }
            if (lower == ")") {
                if (commentDepth == 0)
                    throw UnmatchedComment(word.value);
                commentDepth--;
                continue;
            }
            if (lower.empty() || commentDepth > 0)
                continue;

            if (lower == ":") {
                if (isDefining)
                    throw FunctionInFunction();
                isDefining = true;
                expectName = true;
                continue;
            }
            if (expectName) {
                if (isReservedName(lower)) {
                    throw StakkuException("Cannot use reserved word as a definition name: " +
                                          word.value);
                }
                defName = lower;
                expectName = false;
                continue;
            }
            if (expectVariable) {
                if (isReservedName(lower)) {
                    throw StakkuException("Cannot use reserved word as a variable name: " +
                                          word.value);
                }

                size_t addr = nextMemAddr++;
                variables[lower] = addr;
                expectVariable = false;
                continue;
            }

            auto it = variables.find(lower);
            if (it != variables.end()) {
                emitPushNum(it->second);
                continue;
            }

            if (lower == ";") {
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
                double val = std::stod(lower, &idx);
                if (idx == lower.size()) {
                    emitPushNum(val);
                    continue;
                }
            } catch (const std::invalid_argument &) {
            } catch (const std::out_of_range &) {
                throw InvalidNumber(word.value);
            }

            if (lower == "if") {
                patchStack.push_back(emitJmp(OpCode::OP_JMP_IF_Z));
                continue;
            } else if (lower == "else") {
                if (patchStack.empty())
                    throw UnmatchedControlWord(currentWord->value);
                auto &buf = isDefining ? defBytecode : bytecode;
                size_t patchPos = emitJmp(OpCode::OP_JMP);
                patchJmp(patchStack.back(), buf.size());
                patchStack.pop_back();
                patchStack.push_back(patchPos);
                continue;
            } else if (lower == "then") {
                if (patchStack.empty())
                    throw UnmatchedControlWord(currentWord->value);
                auto &buf = isDefining ? defBytecode : bytecode;
                patchJmp(patchStack.back(), buf.size());
                patchStack.pop_back();
                continue;
            } else if (lower == "do") {
                auto &buf = isDefining ? defBytecode : bytecode;
                emit(OpCode::OP_SWAP);
                emit(OpCode::OP_TO_R);
                emit(OpCode::OP_TO_R);
                size_t loopStart = buf.size();
                patchStack.push_back(loopStart);
                leavePatchStack.emplace_back();
                continue;
            } else if (lower == "loop") {
                if (patchStack.empty())
                    throw UnmatchedControlWord(currentWord->value);
                size_t loopStart = patchStack.back();
                patchStack.pop_back();
                size_t loopOpPos = emitJmp(OpCode::OP_LOOP);
                patchJmp(loopOpPos, loopStart);
                if (!leavePatchStack.empty()) {
                    for (size_t pos : leavePatchStack.back())
                        patchJmp(pos, loopOpPos + 2);
                    leavePatchStack.pop_back();
                }
                continue;
            } else if (lower == "i") {
                emit(OpCode::OP_FETCH_R);
                continue;
            } else if (lower == "j") {
                emit(OpCode::OP_J);
                continue;
            } else if (lower == "leave") {
                if (leavePatchStack.empty())
                    throw StakkuException("'leave' used outside of do..loop");
                emit(OpCode::OP_FROM_R);
                emit(OpCode::OP_DROP);
                emit(OpCode::OP_FROM_R);
                emit(OpCode::OP_DROP);
                leavePatchStack.back().push_back(emitJmp(OpCode::OP_JMP));
                continue;
            } else if (lower == "begin") {
                auto &buf = isDefining ? defBytecode : bytecode;
                patchStack.push_back(buf.size());
                continue;
            } else if (lower == "while") {
                patchStack.push_back(emitJmp(OpCode::OP_JMP_IF_Z));
                continue;
            } else if (lower == "repeat") {
                if (patchStack.size() < 2)
                    throw UnmatchedControlWord(currentWord->value);
                auto &buf = isDefining ? defBytecode : bytecode;
                size_t whilePatch = patchStack.back();
                patchStack.pop_back();
                size_t beginPos = patchStack.back();
                patchStack.pop_back();
                emitJmp(OpCode::OP_JMP);
                patchJmp(buf.size() - 2, beginPos);
                patchJmp(whilePatch, buf.size());
                continue;
            }
            if (lower == "variable") {
                expectVariable = true;
                continue;
            }

            size_t offset = 0;
            if (lookupWord(lower, offset)) {
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

            auto it2 = simpleWords.find(lower);
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
            std::string lowerName = toLower(pdef.name);
            if (wordDict.find(lowerName) != wordDict.end()) {
                throw StakkuException("Duplicate definition: " + pdef.name);
            }
            for (const auto &other : pendingDefs) {
                if (&other != &pdef && toLower(other.name) == lowerName) {
                    throw StakkuException("Duplicate definition: " + pdef.name);
                }
            }
        }

        for (auto &pdef : pendingDefs) {
            size_t wordStart = allDefs.size();
            wordDict.insert({toLower(pdef.name), wordStart});
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

    if (nextMemAddr > 0) {
        bytecode.insert(bytecode.begin(), static_cast<uint8_t>(OpCode::OP_ALLOC));
        uint16_t count = static_cast<uint16_t>(nextMemAddr);
        bytecode.insert(bytecode.begin() + 1, static_cast<uint8_t>(count & 0xFF));
        bytecode.insert(bytecode.begin() + 2, static_cast<uint8_t>((count >> 8) & 0xFF));
        for (auto &[pos, target] : topLevelCallPatches)
            pos += 3;
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
