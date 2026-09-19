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

void Compiler::resetCompilationState() {
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
}

bool Compiler::processComment(std::string word, Word _word) {
    if (word == "(") {
        commentDepth++;
        return true;
    }
    if (word == ")") {
        if (commentDepth == 0)
            throw UnmatchedComment(_word.value);
        commentDepth--;
        return true;
    }
    if (word.empty() || commentDepth > 0)
        return true;

    return false;
}

bool Compiler::processDefState(std::string word, Word _word) {
    if (word == ":") {
        if (isDefining)
            throw FunctionInFunction();
        isDefining = true;
        expectName = true;
        return true;
    }
    if (expectName) {
        if (isReservedName(word)) {
            throw StakkuException("Cannot use reserved word as a definition name: " + _word.value);
        }
        defName = word;
        expectName = false;
        return true;
    }

    if (word == ";") {
        if (!isDefining)
            throw UnmatchedFunction();
        if (defName.empty())
            throw UnnamedFunction();
        if (!patchStack.empty())
            throw UnmatchedControlWord(_word.value);

        defBytecode.push_back(static_cast<uint8_t>(OpCode::OP_RETURN));

        pendingDefs.push_back({defName, defBytecode, currentDefCallPatches, currentDefJmpPatches});
        currentDefCallPatches.clear();
        currentDefJmpPatches.clear();

        defBytecode.clear();
        defName.clear();
        isDefining = false;
        return true;
    }

    return false;
}

bool Compiler::processVarState(std::string word, Word _word) {
    if (word == "variable") {
        expectVariable = true;
        return true;
    }

    if (expectVariable) {
        if (isReservedName(word)) {
            throw StakkuException("Cannot use reserved word as a variable name: " + _word.value);
        }

        size_t addr = nextMemAddr++;
        variables[word] = addr;
        expectVariable = false;
        return true;
    }

    auto it = variables.find(word);
    if (it != variables.end()) {
        emitPushNum(it->second);
        return true;
    }

    return false;
}

bool Compiler::processValue(std::string word, Word _word) {
    try {
        size_t idx = 0;
        double val = std::stod(word, &idx);
        if (idx == word.size()) {
            emitPushNum(val);
            return true;
        }
    } catch (const std::invalid_argument &) {
    } catch (const std::out_of_range &) {
        throw InvalidNumber(_word.value);
    }

    return false;
}

bool Compiler::processKnownWord(std::string word, Word _word) {
    size_t offset = 0;
    if (lookupWord(word, offset)) {
        emit(OpCode::OP_CALL);
        auto &buf = isDefining ? defBytecode : bytecode;
        size_t opPos = buf.size();
        buf.insert(buf.end(), sizeof(uint16_t), 0);
        if (isDefining)
            currentDefCallPatches.push_back({opPos, offset});
        else
            topLevelCallPatches.push_back({opPos, offset});
        return true;
    }

    auto it2 = simpleWords.find(word);
    if (it2 != simpleWords.end()) {
        emit(it2->second);
        return true;
    } else {
        throw UnknownWord(_word.value);
    }
}

bool Compiler::compileConditional(std::string word, Word _word) {
    if (word == "if") {
        patchStack.push_back(emitJmp(OpCode::OP_JMP_IF_Z));
        return true;
    } else if (word == "else") {
        if (patchStack.empty())
            throw UnmatchedControlWord(_word.value);
        auto &buf = isDefining ? defBytecode : bytecode;
        size_t patchPos = emitJmp(OpCode::OP_JMP);
        patchJmp(patchStack.back(), buf.size());
        patchStack.pop_back();
        patchStack.push_back(patchPos);
        return true;
    } else if (word == "then") {
        if (patchStack.empty())
            throw UnmatchedControlWord(_word.value);
        auto &buf = isDefining ? defBytecode : bytecode;
        patchJmp(patchStack.back(), buf.size());
        patchStack.pop_back();
        return true;
    }

    return false;
}

bool Compiler::compileCountedLoop(std::string word, Word _word) {
    if (word == "do") {
        auto &buf = isDefining ? defBytecode : bytecode;
        emit(OpCode::OP_SWAP);
        emit(OpCode::OP_TO_R);
        emit(OpCode::OP_TO_R);
        size_t loopStart = buf.size();
        patchStack.push_back(loopStart);
        leavePatchStack.emplace_back();
        return true;
    } else if (word == "loop") {
        if (patchStack.empty())
            throw UnmatchedControlWord(_word.value);
        size_t loopStart = patchStack.back();
        patchStack.pop_back();
        size_t loopOpPos = emitJmp(OpCode::OP_LOOP);
        patchJmp(loopOpPos, loopStart);
        if (!leavePatchStack.empty()) {
            for (size_t pos : leavePatchStack.back())
                patchJmp(pos, loopOpPos + 2);
            leavePatchStack.pop_back();
        }
        return true;
    } else if (word == "i") {
        emit(OpCode::OP_FETCH_R);
        return true;
    } else if (word == "j") {
        emit(OpCode::OP_J);
        return true;
    } else if (word == "leave") {
        if (leavePatchStack.empty())
            throw StakkuException("'leave' used outside of do..loop");
        emit(OpCode::OP_FROM_R);
        emit(OpCode::OP_DROP);
        emit(OpCode::OP_FROM_R);
        emit(OpCode::OP_DROP);
        leavePatchStack.back().push_back(emitJmp(OpCode::OP_JMP));
        return true;
    }

    return false;
}

bool Compiler::compileBeginLoop(std::string word, Word _word) {
    if (word == "begin") {
        auto &buf = isDefining ? defBytecode : bytecode;
        patchStack.push_back(buf.size());
        return true;
    } else if (word == "while") {
        patchStack.push_back(emitJmp(OpCode::OP_JMP_IF_Z));
        return true;
    } else if (word == "repeat") {
        if (patchStack.size() < 2)
            throw UnmatchedControlWord(_word.value);
        auto &buf = isDefining ? defBytecode : bytecode;
        size_t whilePatch = patchStack.back();
        patchStack.pop_back();
        size_t beginPos = patchStack.back();
        patchStack.pop_back();
        emitJmp(OpCode::OP_JMP);
        patchJmp(buf.size() - 2, beginPos);
        patchJmp(whilePatch, buf.size());
        return true;
    }

    return false;
}

bool Compiler::compileControlWord(std::string word, Word _word) {
    if (word == "if" || word == "else" || word == "then") {
        return compileConditional(word, _word);
    }

    if (word == "do" || word == "loop" || word == "leave" || word == "i" || word == "j") {
        return compileCountedLoop(word, _word);
    }

    if (word == "begin" || word == "while" || word == "repeat") {
        return compileBeginLoop(word, _word);
    }

    return false;
}

bool Compiler::compileWord(std::string word, Word _word) {
    return processComment(word, _word) || processDefState(word, _word) ||
           processVarState(word, _word) || processValue(word, _word) ||
           compileControlWord(word, _word) || processKnownWord(word, _word);
}

void Compiler::finalizeDefs() {
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
}

bool Compiler::compileWords(const std::vector<Word> &words) {
    const Word *currentWord = nullptr;
    try {
        for (const Word &word : words) {
            currentWord = &word;

            std::string lower = toLower(word.value);

            if (!compileWord(lower, word))
                throw UnknownWord(word.value);
        }
        if (!patchStack.empty())
            throw UnmatchedControlWord(currentWord->value);
        if (isDefining)
            throw UnmatchedFunction();
        if (commentDepth > 0)
            throw UnmatchedComment("(");

        finalizeDefs();
    } catch (const StakkuException &e) {
        if (currentWord)
            parseStakkuError(e, *currentWord, words);
        return false;
    }

    return true;
}

void Compiler::prependMemoryAllocation() {
    if (nextMemAddr == 0)
        return;

    if (nextMemAddr > std::numeric_limits<uint16_t>::max()) {
        throw StakkuException("Too many variables");
    }

    uint16_t count = static_cast<uint16_t>(nextMemAddr);

    bytecode.insert(bytecode.begin(),
                    {static_cast<uint8_t>(OpCode::OP_ALLOC), static_cast<uint8_t>(count & 0xff),
                     static_cast<uint8_t>((count >> 8) & 0xff)});

    for (auto &[position, target] : topLevelCallPatches) {
        position += 3;
    }
}

void Compiler::patchAddress(size_t position, size_t targetOffset, size_t definitionsBase) {
    size_t address = definitionsBase + targetOffset;

    if (address > std::numeric_limits<uint16_t>::max()) {
        throw StakkuException(
            "Program too large: compiled bytecode exceeds the 16-bit address space (" +
            std::to_string(address) + " > 65535 bytes).");
    }

    uint16_t address16 = static_cast<uint16_t>(address);
    std::memcpy(&bytecode[position], &address16, sizeof(address16));
}

void Compiler::assembleBytecode() {
    bytecode.push_back(static_cast<uint8_t>(OpCode::OP_HALT));

    defsBaseAddress = bytecode.size();
    bytecode.insert(bytecode.end(), allDefs.begin(), allDefs.end());
}

void Compiler::patchAddresses() {
    for (auto &[position, target] : topLevelCallPatches) {
        patchAddress(position, target, defsBaseAddress);
    }

    for (auto &[position, target] : defsCallPatches) {
        patchAddress(defsBaseAddress + position, target, defsBaseAddress);
    }

    for (auto &[position, target] : defsJmpPatches) {
        patchAddress(defsBaseAddress + position, target, defsBaseAddress);
    }
}

std::vector<uint8_t> Compiler::compile(const std::vector<Word> &words) {
    resetCompilationState();

    if (!compileWords(words)) {
        return {};
    }

    prependMemoryAllocation();
    assembleBytecode();
    patchAddresses();

    return bytecode;
}
