#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "interpreter.h"
#include "repl.h"
#include "split.h"

using namespace stakku;
namespace fs = std::filesystem;

bool checkFile(const char *filename) {
    if (!fs::exists(filename) || !fs::is_regular_file(filename)) {
        return false;
    }
    std::ifstream file(filename);
    return file.is_open();
}

bool interpretFile(const char *filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "File could not be opened: " << filename << std::endl;
        return true;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();

    std::vector<std::string> words = split(content, {" ", "\n"});

    Interpreter interp;
    try {
        interp.setStack(std::make_shared<Stack>());
        interp.interpret(words);
    } catch (const StackUnderflow &) {
        std::cerr << "Stack underflow" << std::endl;
        return true;
    } catch (const std::runtime_error &e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return true;
    }

    if (!interp.hadError()) {
        std::cout << (interp.needsNewline() ? " ok" : "ok") << std::endl;
    }

    return false;
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        REPL repl;
        repl.open();
        return 0;
    }

    if (argc == 2) {
        std::string arg = argv[1];
        if (arg == "repl") {
            REPL repl;
            repl.open();
            return 0;
        }
    }

    if (checkFile(argv[1])) {
        if (interpretFile(argv[1])) {
            return 1;
        }
        return 0;
    }

    std::vector<std::string> words;
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
