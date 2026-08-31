#include "catch2/catch_test_macros.hpp"
#include "exceptions.h"
#include "interpreter/compiler.h"
#include "interpreter/vm.h"
#include <cctype>
#include <string>
#include <vector>

namespace {

std::vector<stakku::Word> tokenize(const std::string &code) {
    std::vector<stakku::Word> words;
    std::string word;
    for (char c : code) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!word.empty()) {
                words.push_back(stakku::Word(word));
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty())
        words.push_back(stakku::Word(word));
    return words;
}

// Helper to simplify running a string of Stakku code
double run_stakku(const std::string &code) {
    stakku::Compiler compiler;
    stakku::VM vm;

    auto bc = compiler.compile(tokenize(code));
    vm.execute(bc);
    return vm.getStack().peek();
}

} // namespace

TEST_CASE("Comprehensive: VM stack manipulation", "[comprehensive]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    auto run = [&](const std::string &code) {
        vm.execute(compiler.compile(tokenize(code)));
        return vm.getStack();
    };

    // OVER: xy -> xyx
    auto s1 = run("1 2 over");
    REQUIRE(s1.size() == 3);
    REQUIRE(s1.peek(0) == 1.0);
    REQUIRE(s1.peek(1) == 2.0);
    REQUIRE(s1.peek(2) == 1.0);

    // ROT: xyz -> yzx
    auto s2 = run("1 2 3 rot");
    REQUIRE(s2.size() == 3);
    REQUIRE(s2.peek(0) == 1.0);
    REQUIRE(s2.peek(1) == 3.0);
    REQUIRE(s2.peek(2) == 2.0);
}

TEST_CASE("Comprehensive: VM comparisons", "[comprehensive]") {
    REQUIRE(run_stakku("1 1 =") == -1.0);
    REQUIRE(run_stakku("1 2 =") == 0.0);
    REQUIRE(run_stakku("1 2 <") == -1.0);
    REQUIRE(run_stakku("2 1 <") == 0.0);
    REQUIRE(run_stakku("2 1 >") == -1.0);
    REQUIRE(run_stakku("1 2 >") == 0.0);
    REQUIRE(run_stakku("1 1 <=") == -1.0);
    REQUIRE(run_stakku("2 1 <=") == 0.0);
    REQUIRE(run_stakku("2 2 >=") == -1.0);
    REQUIRE(run_stakku("1 2 >=") == 0.0);
    REQUIRE(run_stakku("1 2 <>") == -1.0);
    REQUIRE(run_stakku("1 1 <>") == 0.0);
}

TEST_CASE("Comprehensive: VM IO behavior", "[comprehensive]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    // emit out of range
    std::vector<stakku::Word> emit_bad = {stakku::Word("300"), stakku::Word("emit")};
    REQUIRE_THROWS_AS(vm.execute(compiler.compile(emit_bad)), stakku::OutOfRange);

    // output flag
    std::vector<stakku::Word> print_ok = {stakku::Word("10"), stakku::Word(".")};
    vm.execute(compiler.compile(print_ok));
    REQUIRE(vm.hadOutput() == true);
}

TEST_CASE("Comprehensive: complex logic", "[comprehensive]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    // Define a word that returns whether its argument equals 1
    std::vector<stakku::Word> def = {stakku::Word(":"), stakku::Word("is_one"), stakku::Word("1"),
                                     stakku::Word("="), stakku::Word(";")};
    vm.execute(compiler.compile(def));

    vm.execute(compiler.compile({stakku::Word("1"), stakku::Word("is_one")}));
    REQUIRE(vm.getStack().peek(0) == -1.0);

    vm.execute(compiler.compile({stakku::Word("2"), stakku::Word("is_one")}));
    REQUIRE(vm.getStack().peek(0) == 0.0);
}
