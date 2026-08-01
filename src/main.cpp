#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "interpreter.h"
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

    std::string line;
    while (std::getline(file, line)) {
        // Strip empty lines and "done" lines (if it was created by a previous .stack command)
        // Potentially, you could also check for comments, but for now I'll just assume that a comment
        // means it is an "executable" file.
        if (line.empty() || line == "done") {
            continue;
        }

        try {
            std::stod(line);
        } catch (const std::invalid_argument &) {
            return false;
        } catch (const std::out_of_range &) {
            return false;
        }
    }

    return true;
}

// Read a file, run it through the interpreter into a fresh stack, and
// return that stack. Prints "ok" on success. Returns nullptr on any failure.
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

    Interpreter interp;
    try {
        interp.setStack(std::make_shared<Stack>());
        interp.interpret(words);
    } catch (const StackUnderflow &) {
        std::cerr << "Stack underflow" << std::endl;
        return nullptr;
    } catch (const std::runtime_error &e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return nullptr;
    }

    if (interp.hadError()) {
        return nullptr;
    }

    std::cout << (interp.needsNewline() ? " ok" : "ok") << std::endl;
    return interp.getStack();
}

// Open a REPL seeded with the contents of a file. The file is treated as
// saved stack state if it parses line-by-line as doubles; otherwise it's
// interpreted as a program and the resulting stack becomes the REPL's stack.
int runReplWithFile(const char *filename) {
    if (!fs::exists(filename)) {
        std::cerr << "File not found: " << filename << std::endl;
        return 1;
    }
    if (isStackFile(filename)) {
        REPL repl(filename);  // pre-loads stack via loadSession
        repl.open();
        return 0;
    }
    std::cout << "File is not a valid stack file, executing as a script and opening a REPL with the final output stack." << std::endl;
    auto stack = runFileIntoStack(filename);
    if (!stack) {
        return 1;
    }
    REPL repl;
    repl.setStack(std::move(stack));
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

    if (checkFile(argv[1])) {
        if (!runFileIntoStack(argv[1])) {
            return 1;
        }
        return 0;
    }

    std::vector<Word> words;
    for (int i = 1; i < argc; i++)
        words.emplace_back(argv[i]);

    Interpreter interp;
    try {
        interp.setStack(std::make_shared<Stack>());
        interp.interpret(words);
    } catch (const StackUnderflow &) {
        std::cerr << "Stack underflow" << std::endl;
        return 1;
    } catch (const std::runtime_error &e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return 1;
    }

    if (!interp.hadError()) {
        std::cout << (interp.needsNewline() ? " ok" : "ok") << std::endl;
    }
    return 0;
}
