#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "help.h"
#include "interpreter/compiler.h"
#include "interpreter/vm.h"
#include "repl.h"
#include "split.h"

using namespace stakku;
namespace fs = std::filesystem;

namespace {

bool checkFile(const char *filename) {
    if (!fs::exists(filename) || !fs::is_regular_file(filename)) {
        return false;
    }
    std::ifstream file(filename);
    return file.is_open();
}

bool isStackFile(const char *filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        return false;
    }

    bool sawNumber = false;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty() || line == "done") {
            continue;
        }

        try {
            size_t idx = 0;
            std::stod(line, &idx);
            if (idx != line.size()) {
                return false;
            }
            sawNumber = true;
        } catch (const std::invalid_argument &) {
            return false;
        } catch (const std::out_of_range &) {
            return false;
        }
    }

    return sawNumber;
}

std::shared_ptr<Stack> runFileIntoStack(const std::string &filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "File could not be opened: " << filename << std::endl;
        return nullptr;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();

    std::vector<Word> words = split(content, {" ", "\n"});

    Compiler compiler;
    VM vm;
    try {
        auto bytecode = compiler.compile(words);
        if (bytecode.empty()) {
            return nullptr;
        }
        vm.execute(bytecode);
    } catch (const StackUnderflow &) {
        std::cerr << "Stack underflow" << std::endl;
        return nullptr;
    } catch (const std::runtime_error &e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return nullptr;
    }

    if (vm.hadOutput()) {
        std::cout << " ok" << std::endl;
    } else {
        std::cout << "ok" << std::endl;
    }

    auto stack = std::make_shared<Stack>();
    stack->deserialize(vm.saveStack());
    return stack;
}

std::vector<uint8_t> compileFile(const std::string &filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "File could not be opened: " << filename << std::endl;
        return {};
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();

    std::vector<Word> words = split(content, {" ", "\n"});

    Compiler compiler;
    try {
        auto bytecode = compiler.compile(words);
        if (bytecode.empty()) {
            throw std::runtime_error("File is empty");
        }
        return bytecode;
    } catch (const std::runtime_error &e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return std::vector<uint8_t>();
    }
}

int runReplWithFile(const char *filename) {
    if (!fs::exists(filename)) {
        std::cerr << "File not found: " << filename << std::endl;
        return 1;
    }
    if (isStackFile(filename)) {
        REPL repl(filename);
        repl.open();
        return 0;
    }
    std::cout << "File is not a valid stack file, executing as a script and opening a REPL with "
                 "the final output stack."
              << std::endl;
    REPL repl;
    if (!repl.runProgram(filename)) {
        return 1;
    }
    repl.open();
    return 0;
}

} // namespace

int main(int argc, char *argv[]) {
    if (argc == 1) {
        REPL repl;
        repl.open();
        return 0;
    }

    if (argc >= 2 && std::string(argv[1]) == "repl") {
        if (argc == 3) {
            return runReplWithFile(argv[2]);
        }
        REPL repl;
        repl.open();
        return 0;
    }

    if (argc >= 3 && std::string(argv[1]) == "compile") {
        std::vector<uint8_t> out = compileFile(argv[2]);
        if (out.empty()) {
            return 1;
        }
        std::string filename = (argc >= 4) ? argv[3] : "a.skbc";

        std::ofstream fout;
        fout.open(filename, std::fstream::binary);
        fout.write(reinterpret_cast<const char *>(out.data()), out.size());
        return 0;
    }

    if (argc >= 2) {
        std::string sub(argv[1]);
        if (sub == "help" || sub == "--help" || sub == "-h") {
            printHelp();
            return 0;
        }
    }

    if (checkFile(argv[1])) {
        if (!runFileIntoStack(argv[1])) {
            return 1;
        }
        return 0;
    }

    std::vector<Word> words;
    for (int i = 1; i < argc; i++)
        words.emplace_back(argv[i]);

    Compiler compiler;
    VM vm;
    try {
        auto bytecode = compiler.compile(words);
        if (bytecode.empty()) {
            return 1;
        }
        vm.execute(bytecode);
    } catch (const StackUnderflow &) {
        std::cerr << "Stack underflow" << std::endl;
        return 1;
    } catch (const std::runtime_error &e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return 1;
    }

    if (vm.hadOutput()) {
        std::cout << " ok" << std::endl;
    } else {
        std::cout << "ok" << std::endl;
    }
    return 0;
}
