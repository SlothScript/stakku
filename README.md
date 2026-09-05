
# stakku

A small Forth-like stack-based programming language.

`stakku` is a semi-complete RPN language with a bytecode compiler, VM, command-line execution, and an interactive REPL.

## Installation

Clone the repo:

```bash
git clone https://github.com/slothscript/stakku.git
cd stakku
```

Build:

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

Install (Optional):

```bash
cmake --install .
```

## Development

Format the C/C++ sources in place (uses the repo-root `.clang-format`):

```bash
find include src -type f \( -name '*.h' -o -name '*.cpp' \) -print0 | xargs -0 clang-format -i
```

## Quick Start

Run a command:

```text
$ stakku 4 6 + .
10 ok
```

Run a file:

```text
$ stakku examples/example.stku
```

```txt
37
Hello, World!
1050
16
64
10
-4
25
Hello!
1
1
7
7
7
7
 ok
```

Scripts use whitespace-separated words.
A backslash starts a comment that runs to the end of the line, and parenthesized comments are also supported.

Open a REPL:

```text
$ stakku
```

```txt
>>> 4 6 + .
10 ok
>>> 10
ok
>>> 12
ok
>>> +
ok
>>> .
22 ok
```

`stakku repl` opens the same REPL explicitly.
Use `stakku repl filename` to load a serialized numeric stack file, or to execute a script and open a REPL with its final stack.

REPL commands:

```text
.stack          display the current stack
.clear          clear the current stack
.save filename  save the current stack
.load filename  load a saved stack
bye             quit
.q              quit
```

## Supported words

| Category       | Words                    |
| -------------- | ------------------------ |
| Arithmetic     | `+ - * / mod`            |
| Comparison     | `= < > <= >= <>`         |
| Bitwise        | `and or xor invert`      |
| Stack          | `dup drop swap over rot` |
| Return stack   | `>r r> r@`               |
| Output         | `. emit eemit cr clear`  |
| Memory         | `@ !`                    |
| Loop (counted) | `do loop i j leave`      |
| Loop (general) | `begin while repeat`     |
| Control flow   | `if else then`           |
| Definitions    | `: name ... ;`           |
| Variables      | `variable`               |

Numbers are represented as doubles. Comparisons leave `-1` for true and `0` for false; `if` treats a nonzero value as true.
