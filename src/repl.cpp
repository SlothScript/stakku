#include "repl.h"
#include "help.h"
#include "split.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace stakku;
namespace fs = std::filesystem;

REPL::REPL(const std::string &contextFile) : stack_(std::make_shared<Stack>()), compiler(), vm() {
    loadSession(contextFile);
}

void REPL::saveSession(const std::string &path) const {
    std::string content = stack_->serialize();

    if (fs::exists(path)) {
        std::string overwrite;
        std::cout << "The file \"" << path << "\" already exists. Overwrite? [y/n] " << std::flush;
        std::cin >> overwrite;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::transform(overwrite.begin(), overwrite.end(), overwrite.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        if (overwrite != "y" && overwrite != "yes")
            return;
    }

    std::ofstream ofs(path);
    if (!ofs.is_open()) {
        std::cerr << "Could not open file \"" << path << "\"." << std::endl;
        return;
    }
    ofs << content;
}

void REPL::loadSession(const std::string &path) {
    if (!fs::exists(path)) {
        std::cerr << "The file \"" << path << "\" does not exist." << std::endl;
        return;
    }

    std::ifstream file(path);
    if (!file.is_open() || !fs::is_regular_file(path)) {
        std::cerr << "Could not open file \"" << path << "\"." << std::endl;
        return;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();

    try {
        stack_->deserialize(content);
    } catch (const std::exception &e) {
        std::cerr << "Failed to load session: " << e.what() << std::endl;
    }
}

void REPL::open() {
    std::string line;
    std::cout << ">>> " << std::flush;
    while (running_ && std::getline(std::cin, line)) {
        processLine(line);
        if (running_)
            std::cout << ">>> " << std::flush;
    }
}

static std::string takeArg(const std::string &line, const std::string &cmd) {
    if (line.size() > cmd.size() && line[cmd.size()] == ' ')
        return line.substr(cmd.size() + 1);
    return "";
}

bool REPL::processSpecial(const std::string &line) {
    if (line == ".help") {
        printHelp();
        return true;
    }
    if (line == ".q") {
        running_ = false;
        return true;
    }
    if (line == ".stack" || line.rfind(".stack ", 0) == 0) {
        if (stack_->empty()) {
            std::cout << "(empty)" << std::endl;
            return true;
        }
        std::cout << stack_->serialize();
        std::cout << "done" << std::endl;
        return true;
    }
    if (line == ".clear" || line.rfind(".clear ", 0) == 0) {
        stack_->clear();
        std::cout << "done" << std::endl;
        return true;
    }
    if (line == ".save" || line.rfind(".save ", 0) == 0) {
        std::string filename = takeArg(line, ".save");
        if (filename.empty()) {
            std::cout << "Enter filename: " << std::flush;
            std::cin >> filename;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        saveSession(filename);
        return true;
    }
    if (line == ".load" || line.rfind(".load ", 0) == 0) {
        std::string filename = takeArg(line, ".load");
        if (filename.empty()) {
            std::cout << "Enter filename: " << std::flush;
            std::cin >> filename;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        loadSession(filename);
        return true;
    }
    return false;
}

void REPL::processLine(const std::string &line) {
    if (processSpecial(line)) {
        return;
    }

    std::vector<Word> words = split(line, {" "});

    try {
        auto bytecode = compiler.compile(words);
        if (bytecode.empty()) {
            return;
        }
        vm.execute(bytecode, stack_->serialize());
        stack_->deserialize(vm.saveStack());
        if (vm.halted()) {
            running_ = false;
            return;
        }
    } catch (const StackUnderflow &) {
        std::cerr << "Stack underflow" << std::endl;
        stack_->clear();
        return;
    } catch (const std::runtime_error &e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return;
    }

    if (vm.hadOutput()) {
        std::cout << " ok" << std::endl;
    } else {
        std::cout << "ok" << std::endl;
    }
}
