#include "catch2/catch_test_macros.hpp"
#include "interpreter/compiler.h"
#include "interpreter/vm.h"
#include <vector>

TEST_CASE("Integration: compiler + VM basic ops", "[integration]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    // OP_PUSH_NUM expects a double; proper encoding requires the compiler, so
    // we exercise the VM through the compiler instead of hand-built bytecode.
    auto run = [&](const std::vector<stakku::Word> &words) {
        auto bc = compiler.compile(words);
        vm.execute(bc);
        return vm.getStack();
    };

    // Simple addition
    std::vector<stakku::Word> w1 = {stakku::Word("10"), stakku::Word("20"), stakku::Word("+")};
    auto s1 = run(w1);
    REQUIRE(s1.size() == 1);
    REQUIRE(s1.peek(0) == 30.0);
}

TEST_CASE("Integration: compiler + VM stack manipulation", "[integration]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    auto run = [&](const std::vector<stakku::Word> &words) {
        auto bc = compiler.compile(words);
        vm.execute(bc);
        return vm.getStack();
    };

    // Stack manipulation
    std::vector<stakku::Word> w2 = {stakku::Word("1"), stakku::Word("2"), stakku::Word("dup"),
                                    stakku::Word("swap")};
    auto s2 = run(w2);
    REQUIRE(s2.size() == 3);
    REQUIRE(s2.peek(0) == 2.0);
    REQUIRE(s2.peek(1) == 2.0);
    REQUIRE(s2.peek(2) == 1.0);

    // Subtraction and multiplication
    std::vector<stakku::Word> w3 = {stakku::Word("10"), stakku::Word("2"), stakku::Word("-"),
                                    stakku::Word("2"), stakku::Word("*")};
    auto s3 = run(w3);
    REQUIRE(s3.peek(0) == 16.0);
}

TEST_CASE("Integration: compiler definitions", "[integration]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    // Define a word: : square dup * ;
    std::vector<stakku::Word> w1 = {stakku::Word(":"), stakku::Word("square"), stakku::Word("dup"),
                                    stakku::Word("*"), stakku::Word(";")};
    auto bc1 = compiler.compile(w1);
    vm.execute(bc1);

    // Use the word: 5 square
    std::vector<stakku::Word> w2 = {stakku::Word("5"), stakku::Word("square")};
    auto bc2 = compiler.compile(w2);
    vm.execute(bc2);

    REQUIRE(vm.getStack().peek(0) == 25.0);
}
