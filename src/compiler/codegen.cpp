#include "compiler/codegen.h"
#include "opcode.h"

#include <cstring>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace stakku {

namespace {

const char *opcodeName(OpCode opcode) {
    switch (opcode) {
    case OpCode::OP_HALT:
        return "OP_HALT";
    case OpCode::OP_PUSH_NUM:
        return "OP_PUSH_NUM";
    case OpCode::OP_TO_R:
        return "OP_TO_R";
    case OpCode::OP_FROM_R:
        return "OP_FROM_R";
    case OpCode::OP_FETCH_R:
        return "OP_FETCH_R";
    case OpCode::OP_STORE:
        return "OP_STORE";
    case OpCode::OP_FETCH:
        return "OP_FETCH";
    case OpCode::OP_J:
        return "OP_J";
    case OpCode::OP_LOOP:
        return "OP_LOOP";
    case OpCode::OP_ADD:
        return "OP_ADD";
    case OpCode::OP_SUB:
        return "OP_SUB";
    case OpCode::OP_MUL:
        return "OP_MUL";
    case OpCode::OP_DIV:
        return "OP_DIV";
    case OpCode::OP_MOD:
        return "OP_MOD";
    case OpCode::OP_EQ:
        return "OP_EQ";
    case OpCode::OP_LT:
        return "OP_LT";
    case OpCode::OP_GT:
        return "OP_GT";
    case OpCode::OP_LE:
        return "OP_LE";
    case OpCode::OP_GE:
        return "OP_GE";
    case OpCode::OP_NEQ:
        return "OP_NEQ";
    case OpCode::OP_AND:
        return "OP_AND";
    case OpCode::OP_OR:
        return "OP_OR";
    case OpCode::OP_NOT:
        return "OP_NOT";
    case OpCode::OP_XOR:
        return "OP_XOR";
    case OpCode::OP_DUP:
        return "OP_DUP";
    case OpCode::OP_DROP:
        return "OP_DROP";
    case OpCode::OP_SWAP:
        return "OP_SWAP";
    case OpCode::OP_OVER:
        return "OP_OVER";
    case OpCode::OP_ROT:
        return "OP_ROT";
    case OpCode::OP_PRINT:
        return "OP_PRINT";
    case OpCode::OP_EMIT:
        return "OP_EMIT";
    case OpCode::OP_EEMIT:
        return "OP_EEMIT";
    case OpCode::OP_CR:
        return "OP_CR";
    case OpCode::OP_JMP:
        return "OP_JMP";
    case OpCode::OP_JMP_IF_Z:
        return "OP_JMP_IF_Z";
    case OpCode::OP_CALL:
        return "OP_CALL";
    case OpCode::OP_RETURN:
        return "OP_RETURN";
    case OpCode::OP_CLEAR:
        return "OP_CLEAR";
    case OpCode::OP_BYE:
        return "OP_BYE";
    case OpCode::OP_ALLOC:
        return "OP_ALLOC";
    }

    return "UNKNOWN";
}

} // namespace

std::string CodeGen::toHex(uint64_t bits) {
    std::ostringstream stream;
    stream << std::setfill('0') << std::setw(sizeof(double) * 2) << std::hex << bits;
    return stream.str();
}

void CodeGen::emit(const std::string &text) {
    assembly += text;
    assembly.push_back('\n');
}

void CodeGen::emitPushValue(double value) {
    const std::string label = "Lconst" + std::to_string(nextConstantLabel++);

    uint64_t bits;
    std::memcpy(&bits, &value, sizeof(bits));

    data += label + ":\n";
    data += "    .quad 0x" + toHex(bits) + "\n";

    emit("    adrp x16, " + label + "@PAGE");
    emit("    add  x16, x16, " + label + "@PAGEOFF");
    emit("    ldr  d0, [x16]");
    emit("    bl   _stakku_push");
}

double CodeGen::readValue(const std::vector<uint8_t> &bytecode, size_t &pc) {
    if (pc + sizeof(double) > bytecode.size()) {
        throw std::runtime_error("Malformed bytecode: truncated numeric operand");
    }

    double value;
    std::memcpy(&value, bytecode.data() + pc, sizeof(value));
    pc += sizeof(value);
    return value;
}

std::string CodeGen::generate(const std::vector<uint8_t> &bytecode) {
    assembly.clear();
    data.clear();
    nextConstantLabel = 0;

    size_t pc = 0;
    while (pc < bytecode.size()) {
        const OpCode opcode = static_cast<OpCode>(bytecode[pc++]);

        switch (opcode) {
        case OpCode::OP_ADD:
            emit("    bl   _stakku_add");
            break;

        case OpCode::OP_PRINT:
            emit("    bl   _stakku_print");
            break;

        case OpCode::OP_PUSH_NUM:
            emitPushValue(readValue(bytecode, pc));
            break;

        case OpCode::OP_HALT:
        case OpCode::OP_BYE:
            pc = bytecode.size();
            break;

        default:
            throw std::runtime_error(
                "Unsupported opcode: " + std::to_string(static_cast<uint8_t>(opcode)) + " (" +
                opcodeName(opcode) + ")");
        }
    }

    return ".text\n"
           ".globl _stakku_generated\n"
           ".p2align 2\n"
           "_stakku_generated:\n"
           "    stp  x29, x30, [sp, #-16]!\n"
           "    mov  x29, sp\n" +
           assembly +
           "\n"
           "    ldp  x29, x30, [sp], #16\n"
           "    ret\n"
           "\n"
           ".section __TEXT,__const\n"
           ".p2align 3\n" +
           data;
}

} // namespace stakku
