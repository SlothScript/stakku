# stakku

A simple RPN calculator now, a potential Forth interpreter later

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
`stakku 4 6 + .`
`10 ok`

Run a file:
`stakku example.stku`

```txt
125
25
100 ok
```

Open a REPL:
`stakku` or `stakku repl`

```txt
> 4 6 + .
10 ok
> 10
ok
> 12
ok
> +
ok
> .
22 ok
```

## Current Status

Currently, `stakku` is a WIP, but aims to become a full Forth interpreter/compiler.
As of now, it supports: `+ - * / . dup drop swap over emit clear cr`
