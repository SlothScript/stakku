#include "catch2/catch_test_macros.hpp"
#include "exceptions.h"
#include "interpreter/stack.h"
#include <string>

TEST_CASE("Stack: push/pop", "[stack]") {
    stakku::Stack s;
    s.push(1.0);
    s.push(2.0);
    REQUIRE(s.size() == 2);
    REQUIRE(s.pop() == 2.0);
    REQUIRE(s.pop() == 1.0);
    REQUIRE(s.empty());
}

TEST_CASE("Stack: peek", "[stack]") {
    stakku::Stack s;
    s.push(10.0);
    s.push(20.0);
    s.push(30.0);
    REQUIRE(s.peek(0) == 30.0);
    REQUIRE(s.peek(1) == 20.0);
    REQUIRE(s.peek(2) == 10.0);
}

TEST_CASE("Stack: underflow", "[stack]") {
    stakku::Stack s;
    REQUIRE_THROWS_AS(s.pop(), stakku::StackUnderflow);
    REQUIRE_THROWS_AS(s.peek(0), stakku::StackUnderflow);
}

TEST_CASE("Stack: serialize/deserialize", "[stack]") {
    stakku::Stack s;
    s.push(1.1);
    s.push(2.2);
    s.push(3.3);

    std::string data = s.serialize();

    stakku::Stack s2;
    s2.deserialize(data);

    REQUIRE(s2.size() == 3);
    REQUIRE(s2.pop() == 3.3);
    REQUIRE(s2.pop() == 2.2);
    REQUIRE(s2.pop() == 1.1);
}
