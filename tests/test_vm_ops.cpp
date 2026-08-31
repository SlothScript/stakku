#include "catch2/catch_test_macros.hpp"
#include "exceptions.h"
#include "interpreter/compiler.h"
#include "interpreter/stack.h"
#include "interpreter/vm.h"
#include <vector>

namespace {
stakku::Stack run(stakku::Compiler &compiler, stakku::VM &vm,
                  const std::vector<stakku::Word> &words) {
    auto bc = compiler.compile(words);
    vm.execute(bc);
    return vm.getStack();
}
} // namespace

TEST_CASE("VM ops: drop", "[vm_ops]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    auto s = run(compiler, vm, {stakku::Word("1"), stakku::Word("2"), stakku::Word("drop")});
    REQUIRE(s.size() == 1);
    REQUIRE(s.peek(0) == 1.0);

    // drop on empty stack underflows
    auto bc = compiler.compile({stakku::Word("drop")});
    REQUIRE_THROWS_AS(vm.execute(bc), stakku::StackUnderflow);
}

TEST_CASE("VM ops: clear", "[vm_ops]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    auto s = run(compiler, vm,
                 {stakku::Word("1"), stakku::Word("2"), stakku::Word("3"), stakku::Word("clear")});
    REQUIRE(s.empty());
}

TEST_CASE("VM ops: cr", "[vm_ops]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    vm.execute(compiler.compile({stakku::Word("cr")}));
    REQUIRE(vm.hadOutput() == true);
}

TEST_CASE("VM ops: emit valid", "[vm_ops]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    vm.execute(compiler.compile({stakku::Word("65"), stakku::Word("emit")}));
    REQUIRE(vm.hadOutput() == true);
}

TEST_CASE("VM ops: emit underflow", "[vm_ops]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    auto bc = compiler.compile({stakku::Word("emit")});
    REQUIRE_THROWS_AS(vm.execute(bc), stakku::StackUnderflow);
}

TEST_CASE("VM ops: bye halts and stops execution", "[vm_ops]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    // After bye, the remaining push must not be executed.
    vm.execute(compiler.compile({stakku::Word("bye"), stakku::Word("99")}));
    REQUIRE(vm.halted() == true);
    REQUIRE(vm.getStack().empty());
}

TEST_CASE("VM ops: swap underflow", "[vm_ops]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    auto bc = compiler.compile({stakku::Word("1"), stakku::Word("swap")});
    REQUIRE_THROWS_AS(vm.execute(bc), stakku::StackUnderflow);
}

TEST_CASE("VM ops: over underflow", "[vm_ops]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    auto bc = compiler.compile({stakku::Word("1"), stakku::Word("over")});
    REQUIRE_THROWS_AS(vm.execute(bc), stakku::StackUnderflow);
}

TEST_CASE("VM ops: rot underflow", "[vm_ops]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    auto bc = compiler.compile({stakku::Word("1"), stakku::Word("2"), stakku::Word("rot")});
    REQUIRE_THROWS_AS(vm.execute(bc), stakku::StackUnderflow);
}

TEST_CASE("VM ops: negatives and floats", "[vm_ops]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    auto s1 = run(compiler, vm, {stakku::Word("-3.5"), stakku::Word("2.5"), stakku::Word("+")});
    REQUIRE(s1.peek(0) == -1.0);

    auto s2 = run(compiler, vm, {stakku::Word("10"), stakku::Word("-3"), stakku::Word("*")});
    REQUIRE(s2.peek(0) == -30.0);
}

TEST_CASE("VM ops: bitwise AND", "[vm_ops]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    auto check = [&](double a, double b, double expected) {
        auto s = run(compiler, vm,
                     {stakku::Word(std::to_string(a)), stakku::Word(std::to_string(b)),
                      stakku::Word("AND")});
        REQUIRE(s.peek(0) == expected);
    };

    check(6, 3, 2.0);
    check(0, 5, 0.0);
    check(7, 7, 7.0);
}

TEST_CASE("VM ops: bitwise OR", "[vm_ops]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    auto check = [&](double a, double b, double expected) {
        auto s = run(
            compiler, vm,
            {stakku::Word(std::to_string(a)), stakku::Word(std::to_string(b)), stakku::Word("OR")});
        REQUIRE(s.peek(0) == expected);
    };

    check(6, 3, 7.0);
    check(0, 5, 5.0);
    check(4, 2, 6.0);
}

TEST_CASE("VM ops: bitwise XOR", "[vm_ops]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    auto check = [&](double a, double b, double expected) {
        auto s = run(compiler, vm,
                     {stakku::Word(std::to_string(a)), stakku::Word(std::to_string(b)),
                      stakku::Word("XOR")});
        REQUIRE(s.peek(0) == expected);
    };

    check(6, 3, 5.0);
    check(5, 5, 0.0);
    check(0, 7, 7.0);
}

TEST_CASE("VM ops: bitwise INVERT", "[vm_ops]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    // INVERT 0 = -1 (all bits set)
    auto s = run(compiler, vm, {stakku::Word("0"), stakku::Word("INVERT")});
    REQUIRE(s.size() == 1);
    REQUIRE(static_cast<int64_t>(s.peek(0)) == -1);

    // INVERT -1 = 0
    s = run(compiler, vm, {stakku::Word("-1"), stakku::Word("INVERT")});
    REQUIRE(static_cast<int64_t>(s.peek(0)) == 0);

    // INVERT 5 = bitwise NOT of 5
    s = run(compiler, vm, {stakku::Word("5"), stakku::Word("INVERT")});
    REQUIRE(static_cast<int64_t>(s.peek(0)) == ~5);
}

TEST_CASE("VM ops: bitwise edge cases", "[vm_ops]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    // NaN in AND -> 0
    auto s = run(compiler, vm, {stakku::Word("NaN"), stakku::Word("3"), stakku::Word("AND")});
    REQUIRE(s.peek(0) == 0.0);

    // Inf in OR -> INT64_MAX
    s = run(compiler, vm, {stakku::Word("inf"), stakku::Word("0"), stakku::Word("OR")});
    REQUIRE(static_cast<int64_t>(s.peek(0)) == INT64_MAX);

    // Large value in XOR
    // 9223372036854775807 XOR 0 = 9223372036854775807
    s = run(compiler, vm,
            {stakku::Word("9223372036854775807"), stakku::Word("0"), stakku::Word("XOR")});
    REQUIRE(static_cast<int64_t>(s.peek(0)) == INT64_MAX);
}

TEST_CASE("VM ops: mathematical modulo", "[vm_ops]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    // -5 mod 3 = 1
    auto s = run(compiler, vm, {stakku::Word("-5"), stakku::Word("3"), stakku::Word("MOD")});
    REQUIRE(s.peek(0) == 1.0);

    // 5 mod -3 = -1
    s = run(compiler, vm, {stakku::Word("5"), stakku::Word("-3"), stakku::Word("MOD")});
    REQUIRE(s.peek(0) == -1.0);

    // -5 mod -3 = -2
    s = run(compiler, vm, {stakku::Word("-5"), stakku::Word("-3"), stakku::Word("MOD")});
    REQUIRE(s.peek(0) == -2.0);

    // 0 mod 3 = 0
    s = run(compiler, vm, {stakku::Word("0"), stakku::Word("3"), stakku::Word("MOD")});
    REQUIRE(s.peek(0) == 0.0);
}

TEST_CASE("Stack: serialize/deserialize round-trip edge cases", "[vm_ops]") {
    stakku::Stack s;
    s.push(1e308);
    s.push(-2.5);
    s.push(123456789.0);
    s.push(0.0);

    std::string data = s.serialize();

    stakku::Stack s2;
    s2.deserialize(data);

    REQUIRE(s2.size() == 4);
    REQUIRE(s2.pop() == 0.0);
    REQUIRE(s2.pop() == 123456789.0);
    REQUIRE(s2.pop() == -2.5);
    REQUIRE(s2.pop() == 1e308);
}
