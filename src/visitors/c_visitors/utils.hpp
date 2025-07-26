#pragma once
#include <string>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>
#include <optional>
#include <variant>
#include "../../ast/ast_c.hpp"

namespace compiler::ast::c {

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

    void operator()(const Variable& variable) const {
        std::cout << indent() << "Variable: " << variable.mIdentifier << std::endl;
    }

    void operator()(const Unary& unary) const {
        std::cout << indent() << "Unary: " << unary_op_to_string(unary.mOp) << std::endl;
        std::visit(PrintVisitor(depth+1), *unary.mExpr);
    }

    void operator()(const Binary& binary) const {
        std::cout << indent() << "Binary: " << binary_op_to_string(binary.mOp) << std::endl;
        std::cout << indent() + "  " << "Left Expression:\n";
        std::visit(PrintVisitor(depth+2), *binary.mLeft);
        std::cout << indent() + "  " << "Right Expression:\n";
        std::visit(PrintVisitor(depth+2), *binary.mRight);
    }

    void operator()(const Assignment& assignment) const {
        std::cout << indent() << "Assignment:\n";
        std::cout << indent() + "  " << "Left Expression:\n";
        std::visit(PrintVisitor(depth+2), *assignment.mLeft);
        std::cout << indent() + "  " << "Right Expression:\n";
        std::visit(PrintVisitor(depth+2), *assignment.mRight);
    }

    void operator()(const Crement& crement) const {
        if (crement.mPost)
            std::cout << indent() << "Post-";
        else
            std::cout << indent() << "Pre-";
        
        if (crement.mIncrement)
            std::cout << "Increment:\n";
        else
            std::cout << "Decrement:\n";

        std::visit(PrintVisitor(depth+1), *crement.mVar);
    }

    void operator()(const Conditional& conditional) const {
        std::cout << indent() << "Conditional Expression:\n";
        std::cout << indent() << "  If:\n";
        std::visit(PrintVisitor(depth+2), *conditional.mCondition);
        std::cout << indent() << "  Then:\n";
        std::visit(PrintVisitor(depth+2), *conditional.mThen);
        std::cout << indent() << "  Else:\n";
        std::visit(PrintVisitor(depth+2), *conditional.mElse);
    }
    
    // Statement visitors
    void operator()(const Statement& statement) const {
        std::visit(*this, statement);
    }

    void operator()(const Return& ret) const {
        std::cout << indent() << "Return:" << std::endl;
        std::visit(PrintVisitor(depth + 1), ret.mExpr);
    }

    void operator()(const ExpressionStatement& es) const {
        //std::cout << indent() << "Expression Statement:\n";
        //std::visit(PrintVisitor(depth+1), es.mExpr);
        std::visit(PrintVisitor(depth), es.mExpr);
    }

    void operator()(const If& ifStatement) const {
        std::cout << indent() << "If:\n";
        std::visit(PrintVisitor(depth+1), ifStatement.mCondition);
        std::cout << indent() << "Then:\n";
        std::visit(PrintVisitor(depth+1), *ifStatement.mThen);

        if (ifStatement.mElse.has_value()) {
            std::cout << indent() << "Else:\n";
            std::visit(PrintVisitor(depth+1), *ifStatement.mElse.value());
        }
    }

    void operator()(const GoTo& gotoStmt) const {
        std::cout << indent() << "Go to: " << gotoStmt.mTarget << std::endl;
    }

    void operator()(const LabelledStatement& labelledStmt) const {
        std::cout << indent() << "Labelled Statement: " << labelledStmt.mIdentifier << std::endl;
        std::visit(PrintVisitor(depth+1), *labelledStmt.mStatement);
    }

    void operator()(const CompoundStatement& compoundStmt) const {
        std::cout << indent() << "Compound Statement: " << std::endl;
        PrintVisitor(depth+1)(*(compoundStmt.mCompound));
    }

    void operator()(const NullStatement& null) const {
        std::cout << indent() << "Null Statement\n";
    }

    // Declaration
    void operator()(const Declaration& declaration) const {
        std::cout << indent() << "Declaration: " << declaration.mIdentifier << std::endl;
        if (declaration.mExpr.has_value()) {
            std::cout << indent() << "  Initialized Expression\n";
            std::visit(PrintVisitor(depth+2), declaration.mExpr.value());
        }
    }

    // Block
    void operator()(const Block& block) const {
        for (const auto& blockItem : block.mItems){
            std::visit(PrintVisitor(depth+1), blockItem);
        }
    }

    // Function visitor
    void operator()(const Function& func) const {
        std::string indent = std::string(depth * 2, ' ');
        if (func.mIdentifier.has_value()) {
            std::cout << indent << "Function " << func.mIdentifier.value() << ":" << std::endl;
        } else {
            std::cout << indent << "Function:" << std::endl;
        }
        (PrintVisitor(depth+1))(func.mBody);
    }

    // Program visitor
    void operator()(const Program& program) const {
        (*this)(program.mFunction);
    }
};


// ------------------------------> Copy Utils <------------------------------

// Print Visitor
struct CopyVisitor {
    Expression operator()(const Constant& constant) const {
        return Constant(constant.mValue);
    }

    Expression operator()(const Variable& variable) const {
        return Variable(variable.mIdentifier);
    }

    Expression operator()(const Unary& unary) const {
        std::visit(*this, *unary.mExpr);
        return Unary(
            unary.mOp,
            std::make_unique<Expression>(std::visit(*this, *unary.mExpr)));
    }

    Expression operator()(const Binary& binary) const {
        return Binary(
            binary.mOp,
            std::make_unique<Expression>(std::visit(*this, *binary.mLeft)),
            std::make_unique<Expression>(std::visit(*this, *binary.mRight))
        );
    }

    Expression operator()(const Assignment& assignment) const {
        return Assignment(
            std::make_unique<Expression>(std::visit(*this, *assignment.mLeft)),
            std::make_unique<Expression>(std::visit(*this, *assignment.mRight))
        );
    }

    Expression operator()(const Crement& crement) const {
        return Crement(
            std::make_unique<Expression>(std::visit(*this, *crement.mVar)),
            crement.mIncrement,
            crement.mPost
        );
    }

    Expression operator()(const Conditional& conditional) const {
        return Conditional(
            std::make_unique<Expression>(std::visit(*this, *conditional.mCondition)),
            std::make_unique<Expression>(std::visit(*this, *conditional.mThen)),
            std::make_unique<Expression>(std::visit(*this, *conditional.mElse))
        );
    }
    
    // Statement visitors
    BlockItem operator()(const Statement& statement) const {
        return std::visit(*this, statement);
    }

    Statement operator()(const Return& ret) const {
        return Return(std::visit(*this, ret.mExpr));
    }

    Statement operator()(const ExpressionStatement& es) const {
        return ExpressionStatement(std::visit(*this, es.mExpr));
    }

    Statement operator()(const If& ifStatement) const {
        return If(
            std::visit(*this, ifStatement.mCondition),
            std::make_unique<Statement>(std::visit(*this, *ifStatement.mThen)),
            ifStatement.mElse.has_value()
                ? std::make_unique<Statement>(std::visit(*this, *ifStatement.mElse.value()))
                : std::optional<std::unique_ptr<Statement>>(std::nullopt)
        );
    }

    Statement operator()(const GoTo& gotoStmt) const {
        return GoTo(gotoStmt.mTarget);
    }

    Statement operator()(const LabelledStatement& labelledStmt) const {
        return LabelledStatement(
            labelledStmt.mIdentifier,
            std::make_unique<Statement>(std::visit(*this, *labelledStmt.mStatement))
        );
    }

    Statement operator()(const CompoundStatement& compoundStmt) const {
        return CompoundStatement(
            std::make_unique<Block>((*this)(*compoundStmt.mCompound))
        );
    }

    Statement operator()(const NullStatement& null) const {
        return NullStatement();
    }

    // Declaration
    BlockItem operator()(const Declaration& declaration) const {
        if (declaration.mExpr.has_value())
            return Declaration(declaration.mIdentifier, std::visit(*this, declaration.mExpr.value()));
        else
            return Declaration(declaration.mIdentifier);
    }

    // Block
    Block operator()(const Block& block) const {
        std::vector<BlockItem> blockItems;
        blockItems.reserve(block.mItems.size()); // Reserve space instead of resize
        
        for (const auto& blockItem : block.mItems) {
            blockItems.emplace_back(std::visit(*this, blockItem));
        }

        return Block(std::move(blockItems));
    }

    // Function visitor
    Function operator()(const Function& func) const {
        Block body = (*this)(func.mBody);
        return Function(func.mIdentifier, std::move(body));
    }

    // Program visitor
    Program operator()(const Program& program) const {
        return Program((*this)(program.mFunction));
    }
};

}