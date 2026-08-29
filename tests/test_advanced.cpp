#include "catch2/catch_test_macros.hpp"
#include "exceptions.h"
#include "interpreter/compiler.h"
#include "interpreter/vm.h"
#include <vector>

namespace {
stakku::Stack compileRun(stakku::Compiler &compiler, stakku::VM &vm,
                         const std::vector<stakku::Word> &words) {
    auto bc = compiler.compile(words);
    vm.execute(bc);
    return vm.getStack();
}
} // namespace

TEST_CASE("Advanced: control flow (if/else/then)", "[advanced]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    // 1 1 = -> true (1), take the true branch (10)
    auto s1 = compileRun(compiler, vm,
                         {stakku::Word("1"), stakku::Word("1"), stakku::Word("="),
                          stakku::Word("if"), stakku::Word("10"), stakku::Word("else"),
                          stakku::Word("20"), stakku::Word("then")});
    REQUIRE(s1.peek(0) == 10.0);
    REQUIRE(s1.size() == 1);

    // 1 2 = -> false (0), take the false branch (20)
    auto s2 = compileRun(compiler, vm,
                         {stakku::Word("1"), stakku::Word("2"), stakku::Word("="),
                          stakku::Word("if"), stakku::Word("10"), stakku::Word("else"),
                          stakku::Word("20"), stakku::Word("then")});
    REQUIRE(s2.peek(0) == 20.0);
    REQUIRE(s2.size() == 1);
}

TEST_CASE("Advanced: error conditions", "[advanced]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    // Division by Zero (runtime error, actually thrown)
    std::vector<stakku::Word> div_zero = {stakku::Word("10"), stakku::Word("0"), stakku::Word("/")};
    auto bc_div = compiler.compile(div_zero);
    REQUIRE_THROWS_AS(vm.execute(bc_div), stakku::DivideByZero);

    // Unknown Word - the compiler reports the error and returns empty bytecode
    std::vector<stakku::Word> unknown = {stakku::Word("not_a_word")};
    REQUIRE(compiler.compile(unknown).empty());

    // Invalid Number - not a valid number, so it is treated as an unknown word
    std::vector<stakku::Word> bad_num = {stakku::Word("12.34.56")};
    REQUIRE(compiler.compile(bad_num).empty());

    // Unmatched Function (";" without ":")
    std::vector<stakku::Word> unmatched_fn = {stakku::Word(";")};
    REQUIRE(compiler.compile(unmatched_fn).empty());
}

TEST_CASE("Advanced: stack edge cases", "[advanced]") {
    stakku::VM vm;
    stakku::Compiler compiler;

    // Empty stack operation
    std::vector<stakku::Word> empty_op = {stakku::Word("+")};
    auto bc_empty = compiler.compile(empty_op);
    REQUIRE_THROWS_AS(vm.execute(bc_empty), stakku::StackUnderflow);
}
