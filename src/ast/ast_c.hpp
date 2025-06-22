#pragma once
#include <string>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>
#include <optional>
#include <variant>
#include <cassert>

namespace compiler::ast::c {

// ------------------------------> Unary Operator <------------------------------

enum class UnaryOperator {
    Complement,
    Negate,
    Logical_NOT
};

inline constexpr std::string_view unary_op_to_string(UnaryOperator op) {
    switch (op) {
        case UnaryOperator::Complement:     return "Complement";
        case UnaryOperator::Negate:         return "Negate";
        case UnaryOperator::Logical_NOT:    return "Logical NOT";
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
    Bitwise_XOR,
    Logical_AND,
    Logical_OR,
    Is_Equal,
    Not_Equal,
    Less_Than,
    Greater_Than,
    Less_Or_Equal,
    Greater_Or_Equal
};

inline constexpr std::string_view binary_op_to_string(BinaryOperator op) {
    switch (op) {
        case BinaryOperator::Add:               return "Add";
        case BinaryOperator::Subtract:          return "Subtract";
        case BinaryOperator::Multiply:          return "Multiply";
        case BinaryOperator::Divide:            return "Divide";
        case BinaryOperator::Modulo:            return "Modulo";
        case BinaryOperator::Left_Shift:        return "Left Shift";
        case BinaryOperator::Right_Shift:       return "Right Shift";
        case BinaryOperator::Bitwise_AND:       return "Bitwise AND";
        case BinaryOperator::Bitwise_OR:        return "Bitwise OR";
        case BinaryOperator::Bitwise_XOR:       return "Bitwise XOR";
        case BinaryOperator::Logical_AND:       return "Logical AND";
        case BinaryOperator::Logical_OR:        return "Logical OR";
        case BinaryOperator::Is_Equal:          return "Is Equal";
        case BinaryOperator::Not_Equal:         return "Not Equal";
        case BinaryOperator::Less_Than:         return "Less Than";
        case BinaryOperator::Greater_Than:      return "Greater Than";
        case BinaryOperator::Less_Or_Equal:     return "Less or Equal";
        case BinaryOperator::Greater_Or_Equal:  return "Greater or Equal";
    }
    throw std::invalid_argument("Unhandled BinaryOperator in binary_op_to_string");
}

// ------------------------------> Expressions <------------------------------

struct Constant;
struct Unary;
struct Binary;
struct Assignment;
struct Variable;
using Expression = std::variant<Constant, Unary, Binary, Variable, Assignment>;

struct Constant {
    int mValue;
    Constant(int constant) : mValue(constant) {}
};

struct Unary {
    UnaryOperator mOp;
    std::unique_ptr<Expression> mExpr;
    Unary(UnaryOperator op, std::unique_ptr<Expression> expr) : mOp(op), mExpr(std::move(expr)) {}
};

struct Binary {
    BinaryOperator mOp;
    std::unique_ptr<Expression> mLeft;
    std::unique_ptr<Expression> mRight;
    Binary(BinaryOperator op, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
        : mOp(op), mLeft(std::move(left)), mRight(std::move(right)) {}
};

struct Variable {
    std::string mIdentifier;
    Variable(std::string identifier) : mIdentifier(std::move(identifier)) {}
};

struct Assignment {
    std::unique_ptr<Expression> mLeft;
    std::unique_ptr<Expression> mRight;
    Assignment(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
    :   mLeft(std::move(left)),
        mRight(std::move(right)) {}
};

// ------------------------------> Statements <------------------------------

// forward declarations
struct Return;
struct If;
struct ExpressionStatement;
struct NullStatement;
using Statement = std::variant<Return, ExpressionStatement, NullStatement>;

struct Return {
    Expression mExpr;
    Return(Expression expr) : mExpr(std::move(expr)) {}
};

struct ExpressionStatement {
    Expression mExpr;
    ExpressionStatement(Expression expr) : mExpr(std::move(expr)) {}
};

struct NullStatement {};

// ------------------------------> Declaration <------------------------------

struct Declaration {
    std::string mIdentifier;
    std::optional<Expression> mExpr;

    Declaration(std::string identifier, Expression expression)
    :   mIdentifier(std::move(identifier)),
        mExpr(std::move(expression)) {}

    Declaration(std::string identifier) 
    :   mIdentifier(std::move(identifier)), 
        mExpr(std::nullopt) {}
};


// ------------------------------> BlockItem Definition <------------------------------

using BlockItem = std::variant<Declaration, Statement>;

// ------------------------------> Function Definition <------------------------------

struct Function {
    std::optional<std::string> mIdentifier;
    std::vector<BlockItem> mBody;

    Function(std::optional<std::string> identifier, std::vector<BlockItem> body)
        : mIdentifier(identifier), mBody(std::move(body)) {}

    Function(std::vector<BlockItem> body)
        : mIdentifier(std::nullopt), mBody(std::move(body)) {}
};

// ------------------------------> Program Definition <------------------------------

struct Program {
    Function mFunction;

    Program(Function function)
        : mFunction(std::move(function)) {}
};

}