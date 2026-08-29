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
    vm.execute(compiler.compile(
        {stakku::Word("bye"), stakku::Word("99")}));
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
