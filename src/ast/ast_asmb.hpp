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
    Multiply,
    Left_Shift,
    Right_Shift,
    Bitwise_AND,
    Bitwise_OR,
    Bitwise_XOR
};

constexpr std::string_view binary_op_to_string(BinaryOperator op) {
    switch (op) {
        case BinaryOperator::Add:           return "Add";
        case BinaryOperator::Subtract:      return "Subtract";
        case BinaryOperator::Multiply:      return "Multiply";
        case BinaryOperator::Left_Shift:    return "Left Shift";
        case BinaryOperator::Right_Shift:   return "Right Shift";
        case BinaryOperator::Bitwise_AND:   return "Bitwise AND";
        case BinaryOperator::Bitwise_OR:    return "Bitwise OR";
        case BinaryOperator::Bitwise_XOR:   return "Bitwise XOR";
    }
    throw std::invalid_argument("Unhandled BinaryOperator in ast::asmb::binary_op_to_string");
}

constexpr std::string_view binary_op_to_instruction(BinaryOperator op) {
    switch (op) {
        case BinaryOperator::Add:           return "addl";
        case BinaryOperator::Subtract:      return "subl";
        case BinaryOperator::Multiply:      return "imull";
        case BinaryOperator::Left_Shift:    return "sall";
        case BinaryOperator::Right_Shift:   return "sarl";
        case BinaryOperator::Bitwise_AND:   return "andl";
        case BinaryOperator::Bitwise_OR:    return "orl";
        case BinaryOperator::Bitwise_XOR:   return "xorl";
    }
    throw std::invalid_argument("Unhandled BinaryOperator in ast::asmb::binary_op_to_instruction");
}

// ------------------------------> RegisterName <------------------------------

enum class RegisterName {
    AX,
    CX,
    DX,
    DI,
    SI,
    R8,
    R9,
    R10,
    R11
};

enum class RegisterSize {
    QWORD,
    DWORD,
    BYTE
};

constexpr std::string_view reg_name_to_string(RegisterName op, RegisterSize size = RegisterSize::DWORD) {
    switch (op) {
        case RegisterName::AX:
            switch (size) {
                case RegisterSize::QWORD: return "\%rax";
                case RegisterSize::DWORD: return "\%eax";
                case RegisterSize::BYTE: return "\%al";
            }
            break;
        case RegisterName::CX:
            switch (size) {
                case RegisterSize::QWORD: return "\%rcx";
                case RegisterSize::DWORD: return "\%ecx";
                case RegisterSize::BYTE: return "\%cl";
            }
            break;
        case RegisterName::DX:
            switch (size) {
                case RegisterSize::QWORD: return "\%rdx";
                case RegisterSize::DWORD: return "\%edx";
                case RegisterSize::BYTE: return "\%dl";
            }
            break;
        case RegisterName::DI:
            switch (size) {
                case RegisterSize::QWORD: return "\%rdi";
                case RegisterSize::DWORD: return "\%edi";
                case RegisterSize::BYTE: return "\%dil";
            }
            break;
        case RegisterName::SI:
            switch (size) {
                case RegisterSize::QWORD: return "\%rsi";
                case RegisterSize::DWORD: return "\%esi";
                case RegisterSize::BYTE: return "\%sil";
            }
            break;
        case RegisterName::R8:
            switch (size) {
                case RegisterSize::QWORD: return "\%r8";
                case RegisterSize::DWORD: return "\%r8d";
                case RegisterSize::BYTE: return "\%r8b";
            }
            break;
        case RegisterName::R9:
            switch (size) {
                case RegisterSize::QWORD: return "\%r9";
                case RegisterSize::DWORD: return "\%r9d";
                case RegisterSize::BYTE: return "\%r9b";
            }
            break;
        case RegisterName::R10:
            switch (size) {
                case RegisterSize::QWORD: return "\%r10";
                case RegisterSize::DWORD: return "\%r10d";
                case RegisterSize::BYTE: return "\%r10b";
            }
            break;
        case RegisterName::R11:
            switch (size) {
                case RegisterSize::QWORD: return "\%r11";
                case RegisterSize::DWORD: return "\%r11d";
                case RegisterSize::BYTE: return "\%r11b";
            }
            break;
    }
    throw std::invalid_argument("Unhandled RegisterName or RegisterSize in reg_name_to_string");
}

constexpr std::array<asmb::RegisterName, 6> ARG_REGISTERS = {
    asmb::RegisterName::DI,
    asmb::RegisterName::SI,
    asmb::RegisterName::DX,
    asmb::RegisterName::CX,
    asmb::RegisterName::R8,
    asmb::RegisterName::R9,
};

// ------------------------------> ConditionCode <------------------------------

enum class ConditionCode {
    E,
    NE,
    G,
    GE,
    L,
    LE
};

constexpr std::string_view condition_code_to_string(ConditionCode code) {
    switch (code) {
        case ConditionCode::E:  return "e";
        case ConditionCode::NE: return "ne";
        case ConditionCode::G:  return "g";
        case ConditionCode::GE: return "ge";
        case ConditionCode::L:  return "l";
        case ConditionCode::LE: return "le";
    }
    throw std::invalid_argument("Unhandled ConditionCode in condition_code_to_string");
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
    AllocateStack(uint32_t value) : mValue(value) {}
};

struct DeallocateStack {
    uint32_t mValue;
    DeallocateStack(uint32_t value) : mValue(value) {}
};

struct Cmp {
    Operand mOperand1;
    Operand mOperand2;
    Cmp(Operand operand1, Operand operand2) : mOperand1(std::move(operand1)), mOperand2(std::move(operand2)) {}
};

struct Jmp {
    std::string mIdentifier;
    Jmp(std::string identifier) : mIdentifier(std::move(identifier)) {}
};

struct JmpCC {
    ConditionCode mCondCode;
    std::string mIdentifier;
    JmpCC(ConditionCode condCode, std::string identifier)
    :   mCondCode(condCode),
        mIdentifier(std::move(identifier)) {}
};

struct SetCC {
    ConditionCode mCondCode;
    Operand mDst;
    SetCC(ConditionCode condCode, Operand dst)
    :   mCondCode(condCode),
        mDst(std::move(dst)) {}
};

struct Label {
    std::string mIdentifier;
    Label(std::string identifier) : mIdentifier(std::move(identifier)) {}
};

struct Push {
    Operand mOperand;
    Push(Operand operand) : mOperand(operand) {}
};

struct Call {
    std::string mFuncName;
    Call(std::string funcName) : mFuncName(std::move(funcName)) {}
};

struct Ret {
    // No members needed for ret instruction
};


using Instruction = std::variant<Mov, Unary, Binary, Idiv, Cdq, 
                                 AllocateStack, DeallocateStack, Cmp, Jmp,
                                 JmpCC, SetCC, Label, Push, Call, Ret >;

// ------------------------------> Function Definition <------------------------------

struct Function {
    std::string mIdentifier;
    std::vector<Instruction> mInstructions;
    
    Function(std::string identifier, std::vector<Instruction> instructions)
        : mIdentifier(std::move(identifier)), mInstructions(std::move(instructions)) {}
};

// ------------------------------> Program <------------------------------

struct Program {
    std::vector<Function> mFunctions;
    Program(std::vector<Function> functions) : mFunctions(std::move(functions)) {}
};

}