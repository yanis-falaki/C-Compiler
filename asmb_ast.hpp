#pragma once
#include <cstdint>
#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <optional>

namespace compiler::ast::asmb {

// ------------------------------> Operands <------------------------------

struct Operand {
    virtual ~Operand() = default;
    virtual void print(uint32_t depth=0) const = 0;
};

struct Imm : Operand {
    uint32_t mValue;
    Imm(int32_t value) : mValue(value) {}
    void print(uint32_t depth=0) const override{
        auto indent = std::string(depth*2, ' ');
        std::cout << indent << "Imm: " << mValue << std::endl;
    }
};

struct Register : Operand {
    std::string mName;
    Register(std::string name): mName(name) {}
    void print(uint32_t depth=0) const override {
        auto indent = std::string(depth*2, ' ');
        std::cout << indent << "Register: " << mName << std::endl;
    }
};

// ------------------------------> Instructions <------------------------------

struct Instruction {
    virtual ~Instruction() = default;
    virtual void print(uint32_t depth=0) const = 0;
};

struct Ret : Instruction{
    void print(uint32_t depth=0) const override {
        auto indent = std::string(depth*2, ' ');
        std::cout << indent << "Ret" << std::endl;
    }
};

struct Mov : Instruction{
    std::unique_ptr<Operand> mSrc;
    std::unique_ptr<Operand> mDst;
    Mov(std::unique_ptr<Operand> src, std::unique_ptr<Operand> dst) : mSrc(std::move(src)), mDst(std::move(dst)) {}
    void print(uint32_t depth=0) const override{
        auto indent = std::string(depth*2, ' ');
        std::cout << indent << "Mov:" << std::endl;
        std::cout << indent << "  " << "Source:" << std::endl;
        mSrc->print(depth+2);
        std::cout << indent << "  " << "Destination:" << std::endl;
        mDst->print(depth+2);
    }
};

// ------------------------------> Function Definition <------------------------------

struct Function {
    std::optional<std::string> mIdentifier;
    std::vector<std::unique_ptr<Instruction>> mInstructions;
    Function(std::optional<std::string> mIdentifier, std::vector<std::unique_ptr<Instruction>> instructions)
        : mIdentifier(mIdentifier), mInstructions(instructions) {}

    void print(uint32_t depth=0) const {
        std::string indent = std::string(depth*2, ' ');
        if (mIdentifier.has_value())
            std::cout << indent << "Function " << mIdentifier.value() << ":" << std::endl;
        else
            std::cout << indent << "Function " << ":" << std::endl;
        
        for (const std::unique_ptr<Instruction>& instruction : mInstructions) {
            instruction->print(depth+1);
        }
    }
};

// ------------------------------> Program <------------------------------

struct Program {
    std::unique_ptr<Function> mFunction;

    Program(std::unique_ptr<Function> function)
        : mFunction(std::move(function)) {}
    
    void print(uint32_t depth=0) const {
        mFunction->print();
    }
};

}