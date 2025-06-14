#pragma once
#include <cstdint>
#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <optional>
#include <variant>

namespace compiler::ast::asmb {

// ------------------------------> Unary Operator <------------------------------

enum class UnaryOperator {
    Complement,
    Negate
};

constexpr std::string_view unary_op_to_string(UnaryOperator op) {
    switch (op) {
        case UnaryOperator::Complement: return "Complement";
        case UnaryOperator::Negate:     return "Negate";
    }
    return "Unknown";
}

constexpr std::string_view unary_op_to_instruction(UnaryOperator op) {
    switch (op) {
        case UnaryOperator::Complement: return "notl";
        case UnaryOperator::Negate:     return "negl";
    }
    return "Unknown";
}

// ------------------------------> RegisterName <------------------------------

enum class RegisterName {
    AX,
    R10
};

constexpr std::string_view reg_name_to_string(RegisterName op) {
    switch (op) {
        case RegisterName::AX:           return "AX";
        case RegisterName::R10:          return "R10";
    }
    return "Unknown";
}

constexpr std::string_view reg_name_to_operand(RegisterName op) {
    switch (op) {
        case RegisterName::AX:           return "\%eax";
        case RegisterName::R10:          return "%r10d";
    }
    return "Unknown";
}

// ------------------------------> Operands <------------------------------

struct Imm {
    int32_t mValue;
    Imm(int32_t value) : mValue(value) {}
};

struct Reg {
    RegisterName mReg;
    Reg(RegisterName reg) : mReg(reg) {}
};

struct Pseudo {
    std::string mName;
    Pseudo(std::string name) : mName(std::move(name)) {}
};

struct Stack {
    int32_t mLocation;
    Stack(int32_t location) : mLocation(location) {}
};

using Operand = std::variant<Imm, Reg, Pseudo, Stack>;

// ------------------------------> Instructions <------------------------------

struct Ret {
    // No members needed for ret instruction
};

struct Mov {
    Operand mSrc;
    Operand mDst;
    Mov(Operand src, Operand dst) : mSrc(std::move(src)), mDst(std::move(dst)) {}
};

struct Unary {
    UnaryOperator mOp;
    Operand mOperand;
    Unary(UnaryOperator op, Operand operand) : mOp(op), mOperand(std::move(operand)) {}
};

struct AllocateStack {
    uint32_t mValue;
    AllocateStack(int32_t value) : mValue(value) {}
};

using Instruction = std::variant<Ret, Mov, Unary, AllocateStack>;

// ------------------------------> Function Definition <------------------------------

struct Function {
    std::optional<std::string> mIdentifier;
    std::vector<Instruction> mInstructions;
    
    Function(std::optional<std::string> identifier, std::vector<Instruction> instructions)
        : mIdentifier(std::move(identifier)), mInstructions(std::move(instructions)) {}
};

// ------------------------------> Program <------------------------------

struct Program {
    Function mFunction;
    Program(Function function) : mFunction(std::move(function)) {}
};

// ------------------------------> Node <------------------------------

using Node = std::variant<Program, Function,
                          AllocateStack, Mov, Ret,
                          Stack, Pseudo, Reg, Imm>;

}