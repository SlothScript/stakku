# TODO

## In Progress

### Phase 1: Logic & Control (Turing-completeness)

- [x] Implement comparison operators (`<`, `>`, `==`, etc.)
- [ ] Implement control flow (`if/else/then`, `do/loop`)
- [x] Implement logical operations (`AND`, `OR`, `NOT`)

## To Do

### Phase 2: Extensibility

- [ ] Implement word definitions (`:`, `]`)
- [ ] Implement memory access (`@` fetch, `!` store)
- [ ] Implement advanced stack manipulation (`ROT`, `ROLL`)

### Phase 3: Performance & Architecture (The Compiler Path)

- [ ] Bytecode/VM implementation
- [ ] Compilation of words to memory
- [ ] Produce assembly for MacOS(?)

## Done

- [x] RPN parsing & interpretation
- [x] Arithmetic operations (`+`, `-`, `*`, `/`)
- [x] Stack manipulation (`dup`, `drop`, `swap`, `over`)
- [x] Output and utility commands (`.`, `cr`, `emit`, `clear`)
- [x] REPL with session support (`.save`, `.load`, `.stack`, `.clear`, `.q`)
- [x] Line-aware error reporting (assoc tokens with line numbers to report errors like 'Stack underflow at line X')
- [x] Line aware comments
