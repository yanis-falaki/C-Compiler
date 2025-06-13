#pragma once
#include <variant>
#include <cstdint>
#include <string>
#include <vector>

namespace compiler::ast::tacky {

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
    return "Unknown"; // Optional, unreachable with exhaustive handling
}

// ------------------------------> Val <------------------------------

struct Constant {
    uint32_t mValue;
    Constant(uint32_t value): mValue(value) {}
};

struct Var {
    std::string mIdentifier;
    Var(std::string identifier) : mIdentifier(std::move(identifier)) {}
};

using Val = std::variant<Constant, Var>;

// ------------------------------> Instruction <------------------------------

struct Return {
    Val mVal;
    Return(Val val) : mVal(std::move(val)) {}
};

struct Unary {
    UnaryOperator mOp;
    Val mSrc;
    Val mDst;
    Unary(UnaryOperator op, Val src, Val dst) : mOp(op), mSrc(std::move(src)), mDst(std::move(dst)) {}
};

using Instruction = std::variant<Return, Unary>;

// ------------------------------> Function Definition <------------------------------

struct Function {
    std::string mIdentifier;
    std::vector<Instruction> mBody;

    Function(std::string identifier, std::vector<Instruction> body) : 
        mIdentifier(std::move(identifier)),
        mBody(std::move(body)) {}
};

// ------------------------------> Program  <------------------------------

struct Program {
    Function mFunction;
    Program(Function function) : mFunction(std::move(function)) {}
};

}