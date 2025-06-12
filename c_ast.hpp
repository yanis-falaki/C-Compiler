#pragma once
#include <string>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>
#include <optional>
#include <variant>

namespace compiler::ast::c {

// ------------------------------> Expressions <------------------------------

struct Constant {
    int mValue;
    Constant(int constant) : mValue(constant) {}
    int evaluate() const { return mValue; }
};

using Expression = std::variant<Constant>;

// ------------------------------> Statements <------------------------------

// forward declarations
struct Return;
struct If;
using Statement = std::variant<Return, If>;

struct Return {
    Expression mExpr;
    Return(Expression expr) : mExpr(expr) {}
};

struct If {
    Expression mCondition;
    std::unique_ptr<Statement> mThen;
    std::optional<std::unique_ptr<Statement>> mElse;

    If(Expression condition,
       std::unique_ptr<Statement> thenBranch,
       std::optional<std::unique_ptr<Statement>> elseBranch = std::nullopt)
        : mCondition(condition),
          mThen(std::move(thenBranch)),
          mElse(std::move(elseBranch)) {}
};

// ------------------------------> Function Definition <------------------------------

struct Function {
    std::optional<std::string> mIdentifier;
    Statement mBody;

    Function(std::optional<std::string> identifier, Statement body)
        : mIdentifier(identifier), mBody(std::move(body)) {}

    Function(Statement body)
        : mIdentifier(std::nullopt), mBody(std::move(body)) {}
};

// ------------------------------> Program Definition <------------------------------

struct Program {
    Function mFunction;

    Program(Function function)
        : mFunction(std::move(function)) {}
};

// ------------------------------> Printing Utils <------------------------------

// Print Visitor
struct PrintVisitor {
    uint32_t depth;
    
    explicit PrintVisitor(uint32_t d = 0) : depth(d) {}
    
    std::string indent() const {
        return std::string(depth * 2, ' ');
    }
    
    // Expression visitors
    void operator()(const Constant& constant) const {
        std::cout << indent() << "Constant: " << constant.mValue << std::endl;
    }
    
    // Statement visitors
    void operator()(const Return& ret) const {
        std::cout << indent() << "Return:" << std::endl;
        std::visit(PrintVisitor(depth + 1), ret.mExpr);
    }
    
    void operator()(const If& ifStmt) const {
        std::cout << indent() << "If:" << std::endl;
        std::cout << indent() << "  Condition:" << std::endl;
        std::visit(PrintVisitor(depth + 2), ifStmt.mCondition);
        std::cout << indent() << "  Then:" << std::endl;
        std::visit(PrintVisitor(depth + 2), *ifStmt.mThen);
        if (ifStmt.mElse.has_value()) {
            std::cout << indent() << "  Else:" << std::endl;
            std::visit(PrintVisitor(depth + 2), *ifStmt.mElse.value());
        }
    }
};

inline void printAST(const Expression& expr, uint32_t depth = 0) {
    std::visit(PrintVisitor(depth), expr);
}

inline void printAST(const Statement& stmt, uint32_t depth = 0) {
    std::visit(PrintVisitor(depth), stmt);
}

inline void printAST(const Function& func, uint32_t depth = 0) {
    std::string indent = std::string(depth * 2, ' ');
    if (func.mIdentifier.has_value()) {
        std::cout << indent << "Function " << func.mIdentifier.value() << ":" << std::endl;
    } else {
        std::cout << indent << "Function:" << std::endl;
    }
    printAST(func.mBody, depth + 1);
}

inline void printAST(const Program& program, uint32_t depth = 0) {
    printAST(program.mFunction, depth);
}

}