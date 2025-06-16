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
    throw std::invalid_argument("Unhandled UnaryOperator in unary_op_to_string");
}

// ------------------------------> Binary Operator <------------------------------

enum class BinaryOperator {
    Add,
    Subtract,
    Multiply,
    Divide,
    Modulo,
    Left_Shift,
    Right_Shift,
    Bitwise_AND,
    Bitwise_OR,
    Bitwise_XOR
};

inline constexpr std::string_view binary_op_to_string(BinaryOperator op) {
    switch (op) {
        case BinaryOperator::Add:           return "Add";
        case BinaryOperator::Subtract:      return "Subtract";
        case BinaryOperator::Multiply:      return "Multiply";
        case BinaryOperator::Divide:        return "Divide";
        case BinaryOperator::Modulo:        return "Modulo";
        case BinaryOperator::Left_Shift:    return "Left Shift";
        case BinaryOperator::Right_Shift:   return "Right Shift";
        case BinaryOperator::Bitwise_AND:   return "Bitwise AND";
        case BinaryOperator::Bitwise_OR:    return "Bitwise OR";
        case BinaryOperator::Bitwise_XOR:   return "Bitwise XOR";
    }
    throw std::invalid_argument("Unhandled BinaryOperator in ast::tacky::binary_op_to_string");
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

struct Binary {
    BinaryOperator mOp;
    Val mSrc1;
    Val mSrc2;
    Val mDst;
    Binary(BinaryOperator op, Val src1, Val src2, Val dst)
        :   mOp(op),
            mSrc1(std::move(src1)),
            mSrc2(std::move(src2)),
            mDst(std::move(dst)) {}
};

using Instruction = std::variant<Return, Unary, Binary>;

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

// ------------------------------> Node  <------------------------------

using Node = std::variant<Program, Function,
                          Unary, Return,
                          Var, Constant>;

}