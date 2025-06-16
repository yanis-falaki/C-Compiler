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
    throw std::invalid_argument("Unhandled UnaryOperator in ast::asmb::unary_op_to_string");
}

constexpr std::string_view unary_op_to_instruction(UnaryOperator op) {
    switch (op) {
        case UnaryOperator::Complement: return "notl";
        case UnaryOperator::Negate:     return "negl";
    }
    throw std::invalid_argument("Unhandled UnaryOperator in ast::asmb::unary_op_to_instruction");
}

// ------------------------------> Binary Operator <------------------------------

enum class BinaryOperator {
    Add,
    Subtract,
    Multiply
};

constexpr std::string_view binary_op_to_string(BinaryOperator op) {
    switch (op) {
        case BinaryOperator::Add:       return "Add";
        case BinaryOperator::Subtract:  return "Subtract";
        case BinaryOperator::Multiply:  return "Multiply";
    }
    throw std::invalid_argument("Unhandled BinaryOperator in ast::asmb::binary_op_to_string");
}

constexpr std::string_view binary_op_to_instruction(BinaryOperator op) {
    switch (op) {
        case BinaryOperator::Add:       return "addl";
        case BinaryOperator::Subtract:  return "subl";
        case BinaryOperator::Multiply:  return "imull";
    }
    throw std::invalid_argument("Unhandled BinaryOperator in ast::asmb::binary_op_to_instruction");
}

// ------------------------------> RegisterName <------------------------------

enum class RegisterName {
    AX,
    DX,
    R10,
    R11
};

constexpr std::string_view reg_name_to_string(RegisterName op) {
    switch (op) {
        case RegisterName::AX:           return "AX";
        case RegisterName::DX:           return "DX";
        case RegisterName::R10:          return "R10";
        case RegisterName::R11:          return "R11";
    }
    throw std::invalid_argument("Unhandled RegisterName in reg_name_to_string");
}

constexpr std::string_view reg_name_to_operand(RegisterName op) {
    switch (op) {
        case RegisterName::AX:           return "\%eax";
        case RegisterName::DX:           return "\%edx";
        case RegisterName::R10:          return "%r10d";
        case RegisterName::R11:          return "%r11d";
    }
    throw std::invalid_argument("Unhandled RegisterName in reg_name_to_operand");
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

struct Binary {
    BinaryOperator mOp;
    Operand mOperand1;
    Operand mOperand2;
    Binary(BinaryOperator op, Operand operand1, Operand operand2)
        :    mOp(op), 
             mOperand1(std::move(operand1)),
             mOperand2(std::move(operand2)) {}
};

struct Idiv {
    Operand mOperand;
    Idiv(Operand operand) : mOperand(std::move(operand)) {}
};

struct Cdq {};

struct AllocateStack {
    uint32_t mValue;
    AllocateStack(int32_t value) : mValue(value) {}
};

using Instruction = std::variant<Ret, Mov, Unary, Binary, Idiv, Cdq, AllocateStack>;

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