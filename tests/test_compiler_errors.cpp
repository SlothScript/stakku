#include "catch2/catch_test_macros.hpp"
#include "exceptions.h"
#include "interpreter/compiler.h"
#include "interpreter/vm.h"
#include <vector>

TEST_CASE("Compiler errors: function inside function", "[compiler_errors]") {
    stakku::Compiler compiler;
    // ": a : b ; ;" - defining b while inside a is illegal.
    auto bc = compiler.compile({stakku::Word(":"), stakku::Word("a"), stakku::Word(":"),
                                stakku::Word("b"), stakku::Word(";"), stakku::Word(";")});
    REQUIRE(bc.empty());
}

TEST_CASE("Compiler errors: if without then", "[compiler_errors]") {
    stakku::Compiler compiler;
    // Unterminated if.
    auto bc1 =
        compiler.compile({stakku::Word("1"), stakku::Word("if"), stakku::Word("10")});
    REQUIRE(bc1.empty());

    // else without matching if.
    auto bc2 = compiler.compile({stakku::Word("else")});
    REQUIRE(bc2.empty());
}

TEST_CASE("Compiler errors: duplicate definition", "[compiler_errors]") {
    stakku::Compiler compiler;
    auto bc = compiler.compile({stakku::Word(":"), stakku::Word("x"), stakku::Word(";"),
                                stakku::Word(":"), stakku::Word("x"), stakku::Word(";")});
    REQUIRE(bc.empty());
}

TEST_CASE("Compiler: comment parsing", "[compiler_errors][comment]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    // Comment is ignored, only the number is compiled.
    vm.execute(compiler.compile({stakku::Word("("), stakku::Word("ignore"), stakku::Word("me"),
                                 stakku::Word(")"), stakku::Word("42")}));
    REQUIRE(vm.getStack().peek(0) == 42.0);

    // Comment around an instruction.
    vm.execute(compiler.compile({stakku::Word("1"), stakku::Word("("), stakku::Word("drop"),
                                 stakku::Word(")"), stakku::Word("2")}));
    REQUIRE(vm.getStack().size() == 2);
    REQUIRE(vm.getStack().peek(0) == 2.0);
    REQUIRE(vm.getStack().peek(1) == 1.0);
}

TEST_CASE("Compiler errors: unmatched comment", "[compiler_errors]") {
    stakku::Compiler compiler;
    // Opening comment with no closing delimiter.
    auto bc = compiler.compile({stakku::Word("1"), stakku::Word("(")});
    REQUIRE(bc.empty());
}
