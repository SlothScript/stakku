#include "catch2/catch_test_macros.hpp"
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

double run_stakku(const std::string &code) {
    stakku::Compiler compiler;
    stakku::VM vm;
    vm.execute(compiler.compile(tokenize(code)));
    return vm.getStack().peek();
}

} // namespace

TEST_CASE("Variables: declare and fetch", "[variables]") {
    REQUIRE(run_stakku("variable x x @") == 0.0);
}

TEST_CASE("Variables: store and fetch", "[variables]") {
    REQUIRE(run_stakku("variable x 42 x ! x @") == 42.0);
}

TEST_CASE("Variables: multiple variables", "[variables]") {
    REQUIRE(run_stakku("variable a variable b 10 a ! 20 b ! a @ b @ +") == 30.0);
}

TEST_CASE("Variables: use in function", "[variables]") {
    stakku::Compiler compiler;
    stakku::VM vm;

    auto def = tokenize(": set5 variable x 5 x ! x @ ; set5");
    vm.execute(compiler.compile(def));
    REQUIRE(vm.getStack().peek() == 5.0);
}
