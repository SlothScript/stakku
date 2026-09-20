#!/usr/bin/env bash

set -euo pipefail

VERBOSE=0
if [[ "${1:-}" == "-v" || "${1:-}" == "--verbose" ]]; then
    VERBOSE=1
    shift
fi

if [[ $# -lt 1 || $# -gt 2 ]]; then
    echo "usage: $0 [-v|--verbose] input.stku [output]" >&2
    exit 2
fi

if [[ "$VERBOSE" -eq 1 ]]; then
    PS4='+ '
    set -x
fi

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd -- "$SCRIPT_DIR/.." && pwd)"

INPUT="$1"
if [[ "$INPUT" != /* ]]; then
    INPUT="$PWD/$INPUT"
fi

if [[ ! -f "$INPUT" ]]; then
    echo "input file not found: $INPUT" >&2
    exit 1
fi

STAKKU_BIN="${STAKKU_BIN:-$PROJECT_ROOT/build/stakku}"
CXX="${CXX:-clang++}"

if [[ ! -x "$STAKKU_BIN" ]]; then
    echo "stakku executable not found: $STAKKU_BIN" >&2
    echo "build it first with: cmake --build build" >&2
    exit 1
fi

if [[ "$(uname -m)" != "arm64" ]]; then
    echo "this code generator currently targets Apple Silicon ARM64" >&2
    exit 1
fi

if [[ $# -eq 2 ]]; then
    OUTPUT="$2"
else
    INPUT_NAME="$(basename -- "$INPUT")"
    OUTPUT="${INPUT_NAME%.stku}.native"
fi

if [[ "$OUTPUT" != /* ]]; then
    OUTPUT="$PWD/$OUTPUT"
fi

TEMP_DIR="$(mktemp -d "${TMPDIR:-/tmp}/stakku-asm.XXXXXX")"
trap 'rm -rf "$TEMP_DIR"' EXIT

ASM_FILE="$TEMP_DIR/program.s"

"$STAKKU_BIN" asm "$INPUT" "$ASM_FILE"

if [[ "$VERBOSE" -eq 1 ]]; then
    echo "--- generated assembly: $ASM_FILE ---"
    cat "$ASM_FILE"
    echo "--- end generated assembly ---"
fi

"$CXX" \
    -std=c++17 \
    -I"$PROJECT_ROOT/include/stakku" \
    -I"$PROJECT_ROOT/include/stakku/compiler" \
    "$ASM_FILE" \
    "$SCRIPT_DIR/asm_runner.cpp" \
    "$PROJECT_ROOT/src/compiler/codegen_runtime.cpp" \
    "$PROJECT_ROOT/src/compiler/stack.cpp" \
    -o "$OUTPUT"

echo "generated native executable: $OUTPUT"
