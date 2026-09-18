#pragma once

#include "exceptions.h"
#include "opcode.h"

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace stakku {

struct Word {
    std::string value;
    int line = 1;

    Word() = default;
    Word(std::string value_, int line_ = 1) : value(std::move(value_)), line(line_) {
    }
    Word(const char *value_, int line_ = 1) : value(value_), line(line_) {
    }
};

class Compiler {
  public:
    Compiler()
        : nextMemAddr(0), expectVariable(false), commentDepth(0), isDefining(false),
          expectName(false) {
    }
    std::vector<uint8_t> compile(const std::vector<Word> &words);

  private:
    void emit(OpCode op);
    void emitPushNum(double value);
    size_t emitJmp(OpCode op);
    void patchJmp(size_t patchPos, size_t target);
    bool lookupWord(const std::string &name, size_t &offset) const;
    bool isReservedName(const std::string &name) const;
    bool isNumber(const std::string &name) const;

    void parseStakkuError(const StakkuException &e, const Word &word,
                          const std::vector<Word> &words);

    // Compilation helpers
    void resetCompilationState();

    bool compileWords(const std::vector<Word> &words);
    bool compileWord(std::string word, Word _words);
    bool processComment(std::string word, Word _word);
    bool processDefState(std::string word, Word _word);
    bool processVarState(std::string word, Word _word);
    bool processValue(std::string word, Word _word);
    bool processKnownWord(std::string word, Word _word);

    bool compileControlWord(std::string word, Word _word);
    bool compileConditional(std::string word, Word _word);
    bool compileCountedLoop(std::string word, Word _word);
    bool compileBeginLoop(std::string word, Word _word);

    void finalizeDefs();
    void prependMemoryAllocation();
    void assembleBytecode();

    void patchAddress(size_t position, size_t targetOffset, size_t definitionsBase);
    void patchAddresses();

    size_t defsBaseAddress = 0;

    std::vector<uint8_t> bytecode;
    std::vector<size_t> patchStack;
    std::unordered_map<std::string, size_t> wordDict;
    std::vector<uint8_t> allDefs;
    std::vector<std::pair<size_t, size_t>> topLevelCallPatches;
    std::vector<std::pair<size_t, size_t>> defsCallPatches;
    std::vector<std::pair<size_t, size_t>> defsJmpPatches;
    std::vector<std::pair<size_t, size_t>> currentDefCallPatches;

    std::unordered_map<std::string, size_t> variables;
    size_t nextMemAddr;
    bool expectVariable;

    int commentDepth;

    bool isDefining;
    bool expectName;
    std::string defName;
    std::vector<uint8_t> defBytecode;

    std::vector<std::pair<size_t, size_t>> currentDefJmpPatches;

    std::vector<std::vector<size_t>> leavePatchStack;

    struct PendingDef {
        std::string name;
        std::vector<uint8_t> bytecode;
        std::vector<std::pair<size_t, size_t>> callPatches;
        std::vector<std::pair<size_t, size_t>> jmpPatches;
    };
    std::vector<PendingDef> pendingDefs;

    const std::unordered_map<std::string, OpCode> simpleWords = {
        {"bye", OpCode::OP_BYE},

        {"!", OpCode::OP_STORE},    {"@", OpCode::OP_FETCH},

        {"+", OpCode::OP_ADD},      {"-", OpCode::OP_SUB},       {"*", OpCode::OP_MUL},
        {"/", OpCode::OP_DIV},      {"mod", OpCode::OP_MOD},

        {"=", OpCode::OP_EQ},       {"<", OpCode::OP_LT},        {">", OpCode::OP_GT},
        {"<=", OpCode::OP_LE},      {">=", OpCode::OP_GE},       {"<>", OpCode::OP_NEQ},

        {"and", OpCode::OP_AND},    {"or", OpCode::OP_OR},       {"xor", OpCode::OP_XOR},
        {"invert", OpCode::OP_NOT},

        {">r", OpCode::OP_TO_R},    {"r>", OpCode::OP_FROM_R},   {"r@", OpCode::OP_FETCH_R},

        {"dup", OpCode::OP_DUP},    {"drop", OpCode::OP_DROP},   {"swap", OpCode::OP_SWAP},
        {"over", OpCode::OP_OVER},  {"rot", OpCode::OP_ROT},

        {".", OpCode::OP_PRINT},    {"emit", OpCode::OP_EMIT},   {"eemit", OpCode::OP_EEMIT},
        {"cr", OpCode::OP_CR},      {"clear", OpCode::OP_CLEAR},
    };
};

} // namespace stakku
