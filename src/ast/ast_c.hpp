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
struct Crement;
struct Conditional;
using Expression = std::variant<Constant, Unary, Binary, Variable, Assignment, Crement, Conditional>;

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

struct Crement {
    std::unique_ptr<Expression> mVar;
    bool mIncrement;
    bool mPost;
    Crement(std::unique_ptr<Expression> var, bool increment, bool post)
    :   mVar(std::move(var)),
        mIncrement(increment),
        mPost(post) {}
};

struct Conditional {
    std::unique_ptr<Expression> mCondition;
    std::unique_ptr<Expression> mThen;
    std::unique_ptr<Expression> mElse;
    Conditional(std::unique_ptr<Expression> conditionExpr,
                std::unique_ptr<Expression> thenExpr,
                std::unique_ptr<Expression> elseExpr)
    :   mCondition(std::move(conditionExpr)),
        mThen(std::move(thenExpr)),
        mElse(std::move(elseExpr)) {}
};

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

// ------------------------------> Statements <------------------------------

// forward declarations
struct Return;
struct ExpressionStatement;
struct If;
struct GoTo;
struct LabelledStatement;
struct CompoundStatement;
struct NullStatement;
struct Break;
struct Continue;
struct While;
struct DoWhile;
struct For;
using Statement = std::variant<Return, ExpressionStatement, If, GoTo, LabelledStatement, CompoundStatement,
                               Break, Continue, While, DoWhile, For, NullStatement>;

struct Return {
    Expression mExpr;
    Return(Expression expr) : mExpr(std::move(expr)) {}
};

struct ExpressionStatement {
    Expression mExpr;
    ExpressionStatement(Expression expr) : mExpr(std::move(expr)) {}
};

struct If {
    Expression mCondition;
    std::unique_ptr<Statement> mThen;
    std::optional<std::unique_ptr<Statement>> mElse;
    If(Expression condition, std::unique_ptr<Statement> then, std::optional<std::unique_ptr<Statement>> es)
    :   mCondition(std::move(condition)),
        mThen(std::move(then)),
        mElse(std::move(es)) {}
    
    If(Expression condition, std::unique_ptr<Statement> then)
    :   mCondition(std::move(condition)),
        mThen(std::move(then)),
        mElse(std::nullopt) {}
};

struct GoTo {
    std::string mTarget;
    GoTo(std::string target) : mTarget(std::move(target)) {}
};

struct LabelledStatement {
    std::string mIdentifier;
    std::unique_ptr<Statement> mStatement;
    LabelledStatement(std::string identifier, std::unique_ptr<Statement> statement)
    :   mIdentifier(std::move(identifier)),
        mStatement(std::move(statement)) {}
};

struct Block;
struct CompoundStatement {
    std::unique_ptr<Block> mCompound;
    CompoundStatement(std::unique_ptr<Block> compound) : mCompound(std::move(compound)) {}
};

struct Break {
    std::string mLabel;
    Break(std::string label) : mLabel(std::move(label)) {}
};

struct Continue {
    std::string mLabel;
    Continue(std::string label) : mLabel(std::move(label)) {}
};

struct While {
    Expression mCondition;
    std::unique_ptr<Statement> mBody;
    std::string mLabel;
    While(Expression condition, std::unique_ptr<Statement> body, std::string label)
    :   mCondition(std::move(condition)), mBody(std::move(body)), mLabel(std::move(label)) {}
};

struct DoWhile {
    std::unique_ptr<Statement> mBody;
    Expression mCondition;
    std::string mLabel;
    DoWhile(std::unique_ptr<Statement> body, Expression condition, std::string label)
    :   mBody(std::move(body)), mCondition(std::move(condition)), mLabel(std::move(label)) {}
};

using ForInit = std::variant<Declaration, std::optional<Expression>>;

struct For {
    ForInit mForInit;
    std::optional<Expression> mCondition;
    std::optional<Expression> mPost;
    std::unique_ptr<Statement> mBody;
    std::string mLabel;
    For(ForInit forInit, std::optional<Expression> condition, std::optional<Expression> post, std::unique_ptr<Statement> body, std::string label)
    :   mForInit(std::move(forInit)), mCondition(std::move(condition)), mPost(std::move(post)), mBody(std::move(body)), mLabel(std::move(label)) {}
};

struct NullStatement {};

// ------------------------------> Block Definition <------------------------------

using BlockItem = std::variant<Declaration, Statement>;

struct Block {
    std::vector<BlockItem> mItems;

    Block(std::vector<BlockItem> item) : mItems(std::move(item)) {}
};

// ------------------------------> Function Definition <------------------------------

struct Function {
    std::optional<std::string> mIdentifier;
    Block mBody;

    Function(std::optional<std::string> identifier, Block body)
        : mIdentifier(identifier), mBody(std::move(body)) {}

    Function(Block body)
        : mIdentifier(std::nullopt), mBody(std::move(body)) {}
};

// ------------------------------> Program Definition <------------------------------

struct Program {
    Function mFunction;

    Program(Function function)
        : mFunction(std::move(function)) {}
};

}