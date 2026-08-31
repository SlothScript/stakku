#include "help.h"
#include <ostream>

namespace stakku {

void printHelp(std::ostream &os) {
    os << "stakku - a less simple RPN calculator with Forth-like elements\n"
          "\n"
          "USAGE\n"
          "  stakku                         Start an interactive REPL\n"
          "  stakku help                    Show this help text\n"
          "  stakku <file>                  Run a .stku script\n"
          "  stakku repl                    Start the REPL explicitly\n"
          "  stakku repl <file>             Load a stack file, or run a script and open a\n"
          "                                 REPL with its final stack\n"
          "  stakku compile <file> [output] Compile a .stku script to bytecode\n"
          "  stakku <word> ...              Evaluate words directly, e.g. `stakku 4 6 + .`\n"
          "\n"
          "NUMBERS\n"
          "  Numbers are doubles. Use `\\` for line comments and ( ) for block comments.\n"
          "\n"
          "SUPPORTED WORDS\n"
          "  Arithmetic            + - * /\n"
          "  Comparison/Logical   = < > <= >= <>\n"
          "  Stack                dup drop swap over rot\n"
          "  Output               . emit cr\n"
          "  Control flow         if else then\n"
          "  Definitions          : name ... ;\n"
          "  Misc                 bye\n"
          "\n"
          "  Comparisons leave -1 for true and 0 for false; `if` treats a nonzero\n"
          "  value as true.\n"
          "\n"
          "REPL COMMANDS\n"
          "  .stack        Display the current stack\n"
          "  .clear        Clear the current stack\n"
          "  .save <file>  Save the current stack\n"
          "  .load <file>  Load a saved stack\n"
          "  .help         Show this help text\n"
          "  bye           Quit\n"
          "  .q            Quit\n";
}

} // namespace stakku
