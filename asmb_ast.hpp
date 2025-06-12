#pragma once
#include <cstdint>
#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <optional>
#include <variant>

namespace compiler::ast::asmb {

// ------------------------------> Operands <------------------------------

struct Imm {
    uint32_t mValue;
    Imm(int32_t value) : mValue(value) {}
};

struct Register {
    std::string mName;
    Register(std::string name) : mName(std::move(name)) {}
};

using Operand = std::variant<Imm, Register>;

// ------------------------------> Instructions <------------------------------

struct Ret {
    // No members needed for ret instruction
};

struct Mov {
    Operand mSrc;
    Operand mDst;
    Mov(Operand src, Operand dst) : mSrc(std::move(src)), mDst(std::move(dst)) {}
};

using Instruction = std::variant<Ret, Mov>;

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

}