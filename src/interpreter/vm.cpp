#include "vm.h"
#include "exceptions.h"
#include "opcode.h"
#include <csignal>
#include <cstdint>
#include <iostream>

using namespace stakku;

inline void checkBounds(const std::vector<uint8_t> &bytecode, size_t pc, size_t needed) {
    if (pc + needed > bytecode.size()) {
        throw StakkuException("Malformed bytecode: unexpected end at pc=" + std::to_string(pc));
    }
}

void VM::execute(const std::vector<uint8_t> &ops, const std::string &initialStack) {
    pc = 0;
    hadOutput_ = false;
    halted_ = false;
    returnStack.clear();
    running = true;
    stack.clear();
    if (!initialStack.empty()) {
        stack.deserialize(initialStack);
    }
    while (pc < ops.size() && running) {
        OpCode op = static_cast<OpCode>(ops[pc]);
        switch (op) {
        case OpCode::OP_HALT:
            halt();
            break;
        case OpCode::OP_PUSH_NUM:
            push(ops);
            break;
        case OpCode::OP_ADD:
            add();
            pc++;
            break;
        case OpCode::OP_SUB:
            sub();
            pc++;
            break;
        case OpCode::OP_MUL:
            mul();
            pc++;
            break;
        case OpCode::OP_DIV:
            div_();
            pc++;
            break;
        case OpCode::OP_EQ:
            eq();
            pc++;
            break;
        case OpCode::OP_LT:
            lt();
            pc++;
            break;
        case OpCode::OP_GT:
            gt();
            pc++;
            break;
        case OpCode::OP_LE:
            le();
            pc++;
            break;
        case OpCode::OP_GE:
            ge();
            pc++;
            break;
        case OpCode::OP_NEQ:
            neq();
            pc++;
            break;
        case OpCode::OP_DUP:
            dup();
            pc++;
            break;
        case OpCode::OP_DROP:
            drop();
            pc++;
            break;
        case OpCode::OP_SWAP:
            swap();
            pc++;
            break;
        case OpCode::OP_OVER:
            over();
            pc++;
            break;
        case OpCode::OP_ROT:
            rot();
            pc++;
            break;
        case OpCode::OP_PRINT:
            print();
            pc++;
            break;
        case OpCode::OP_EMIT:
            emit();
            pc++;
            break;
        case OpCode::OP_CR:
            cr();
            pc++;
            break;
        case OpCode::OP_CLEAR:
            clear();
            pc++;
            break;
        case OpCode::OP_JMP:
            jmp(ops);
            break;
        case OpCode::OP_JMP_IF_Z:
            jmp_if_z(ops);
            break;
        case OpCode::OP_BYE:
            bye();
            break;
        case OpCode::OP_CALL:
            call(ops);
            break;
        case OpCode::OP_RETURN:
            call_return(ops);
            break;
        default:
            throw StakkuException("Unknown opcode: 0x" + toHex(static_cast<uint8_t>(op)) +
                                  " at pc=" + std::to_string(pc));
        }
    }
}

std::string VM::toHex(uint8_t v) {
    static const char *hex = "0123456789abcdef";
    std::string s;
    s.push_back(hex[v >> 4]);
    s.push_back(hex[v & 0xF]);
    return s;
}

void VM::halt() {
    running = false;
}

void VM::bye() {
    running = false;
    halted_ = true;
}

void VM::push(const std::vector<uint8_t> &bytecode) {
    checkBounds(bytecode, pc, 1 + sizeof(double));
    double value;
    std::memcpy(&value, &bytecode[pc + 1], sizeof(double));
    pc += 1 + sizeof(double);
    stack.push(value);
}

void VM::add() {
    if (stack.size() < 2) {
        throw StackUnderflow();
    }

    double b = stack.pop();
    double a = stack.pop();
    stack.push(a + b);
}

void VM::sub() {
    if (stack.size() < 2) {
        throw StackUnderflow();
    }

    double b = stack.pop();
    double a = stack.pop();
    stack.push(a - b);
}

void VM::mul() {
    if (stack.size() < 2) {
        throw StackUnderflow();
    }

    double b = stack.pop();
    double a = stack.pop();
    stack.push(a * b);
}

void VM::div_() {
    if (stack.size() < 2) {
        throw StackUnderflow();
    }

    double b = stack.peek();
    if (b == 0) {
        throw DivideByZero();
    }
    b = stack.pop();
    double a = stack.pop();
    stack.push(a / b);
}

void VM::eq() {
    if (stack.size() < 2) {
        throw StackUnderflow();
    }

    double b = stack.pop();
    double a = stack.pop();
    if (a == b) {
        stack.push(1.0);
    } else {
        stack.push(0.0);
    }
}

void VM::lt() {
    if (stack.size() < 2) {
        throw StackUnderflow();
    }

    double b = stack.pop();
    double a = stack.pop();
    if (a < b) {
        stack.push(1.0);
    } else {
        stack.push(0.0);
    }
}

void VM::gt() {
    if (stack.size() < 2) {
        throw StackUnderflow();
    }

    double b = stack.pop();
    double a = stack.pop();
    if (a > b) {
        stack.push(1.0);
    } else {
        stack.push(0.0);
    }
}

void VM::le() {
    if (stack.size() < 2) {
        throw StackUnderflow();
    }

    double b = stack.pop();
    double a = stack.pop();
    if (a <= b) {
        stack.push(1.0);
    } else {
        stack.push(0.0);
    }
}

void VM::ge() {
    if (stack.size() < 2) {
        throw StackUnderflow();
    }

    double b = stack.pop();
    double a = stack.pop();
    if (a >= b) {
        stack.push(1.0);
    } else {
        stack.push(0.0);
    }
}

void VM::neq() {
    if (stack.size() < 2) {
        throw StackUnderflow();
    }

    double b = stack.pop();
    double a = stack.pop();
    if (a != b) {
        stack.push(1.0);
    } else {
        stack.push(0.0);
    }
}

void VM::dup() {
    if (stack.empty()) {
        throw StackUnderflow();
    }

    stack.push(stack.peek());
}

void VM::drop() {
    if (stack.empty()) {
        throw StackUnderflow();
    }

    stack.pop();
}

void VM::swap() {
    if (stack.size() < 2) {
        throw StackUnderflow();
    }

    // [1, 2]
    double b = stack.pop(); // [1], b=2
    double a = stack.pop(); // [],  b=2, a=1
    stack.push(b);          // [2]
    stack.push(a);          // [2, 1]
}

void VM::over() {
    if (stack.size() < 2) {
        throw StackUnderflow();
    }

    double b = stack.pop();
    double a = stack.pop();

    stack.push(a);
    stack.push(b);
    stack.push(a);
}

void VM::rot() {
    if (stack.size() < 3) {
        throw StackUnderflow();
    }

    // [1,2,3] -> [2,3,1]
    double c = stack.pop(); // [1,2], c=3
    double b = stack.pop(); // [1],   c=3, b=2
    double a = stack.pop(); // [],    c=3, b=2, a=1
    stack.push(b);          // [2]
    stack.push(c);          // [2,3]
    stack.push(a);          // [2,3,1]
}

void VM::print() {
    double val = stack.pop();
    hadOutput_ = true;
    std::cout << val;
}

void VM::emit() {
    double val = stack.pop();
    if (val < 0 || val > 255) {
        throw OutOfRange("emit value must be in range 0-255, got: " + std::to_string(val));
    }
    hadOutput_ = true;
    std::cout << static_cast<char>(val);
}

void VM::cr() {
    hadOutput_ = true;
    std::cout << '\n';
}

void VM::clear() {
    stack.clear();
}

void VM::jmp(const std::vector<uint8_t> &bytecode) {
    checkBounds(bytecode, pc, 1 + sizeof(uint16_t));
    uint16_t value;
    std::memcpy(&value, &bytecode[pc + 1], sizeof(uint16_t));
    if (value >= bytecode.size()) {
        throw StakkuException("Malformed bytecode: jump target out of range: " +
                              std::to_string(value));
    }
    pc = value;
}

void VM::jmp_if_z(const std::vector<uint8_t> &bytecode) {
    checkBounds(bytecode, pc, 1 + sizeof(uint16_t));
    uint16_t value;
    std::memcpy(&value, &bytecode[pc + 1], sizeof(uint16_t));
    if (value >= bytecode.size()) {
        throw StakkuException("Malformed bytecode: jump target out of range: " +
                              std::to_string(value));
    }
    if (!stack.pop()) {
        pc = value;
    } else {
        pc += 1 + sizeof(uint16_t);
    }
}

void VM::call(const std::vector<uint8_t> &bytecode) {
    checkBounds(bytecode, pc, 1 + sizeof(uint16_t));
    uint16_t target;
    std::memcpy(&target, &bytecode[pc + 1], sizeof(uint16_t));
    if (target >= bytecode.size()) {
        throw StakkuException("Malformed bytecode: call target out of range: " +
                              std::to_string(target));
    }
    pc += 1 + sizeof(uint16_t);
    returnStack.push_back(pc);
    pc = target;
}

void VM::call_return(const std::vector<uint8_t> &bytecode) {
    (void)bytecode;
    if (returnStack.empty())
        throw StackUnderflow();
    pc = returnStack.back();
    returnStack.pop_back();
}
