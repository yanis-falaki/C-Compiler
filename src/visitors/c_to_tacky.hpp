#pragma once
#include <memory>
#include <sstream>
#include <format>
#include <vector>
#include <stdexcept>
#include "../ast/ast_c.hpp"
#include "../ast/ast_tacky.hpp"

namespace compiler::codegen {

// ------------------------------> Helper function for temporary variables <------------------------------

inline ast::tacky::Var makeTemporaryRegister() {
    static uint32_t tmpRegisterNum = 0;
    return std::format("tmp.{}", tmpRegisterNum++);;
}

// ------------------------------> Helper functions for labels <------------------------------

inline std::pair<ast::tacky::Label, ast::tacky::Label> makeAndLabels() {
    static uint32_t andNum = 0;
    return {
        std::format("and_false.{}", andNum),
        std::format("and_end.{}", andNum++)
    };
}

inline std::pair<ast::tacky::Label, ast::tacky::Label> makeOrLabels() {
    static uint32_t orNum = 0;
    return {
        std::format("or_true.{}", orNum),
        std::format("or_end.{}", orNum++)
    };
}

inline std::pair<ast::tacky::Label, ast::tacky::Label> makeConditionalLabels() {
    static uint32_t conditionalNum = 0;
    return {
        std::format("cond_expr2.{}", conditionalNum),
        std::format("cond_end.{}", conditionalNum++)
    };
}

inline std::pair<ast::tacky::Label, ast::tacky::Label> makeIfLabels() {
    static uint32_t ifNum = 0;
    return {
        std::format("if_else.{}", ifNum),
        std::format("if_end.{}", ifNum++)
    };
}


// ------------------------------> Map between TACKY unops and C unops <------------------------------

inline constexpr ast::tacky::UnaryOperator c_to_tacky_unop(ast::c::UnaryOperator unop) {
    switch (unop) {
        case ast::c::UnaryOperator::Negate:          return ast::tacky::UnaryOperator::Negate;
        case ast::c::UnaryOperator::Complement:      return ast::tacky::UnaryOperator::Complement;
        case ast::c::UnaryOperator::Logical_NOT:     return ast::tacky::UnaryOperator::Logical_NOT;
        default:
            throw std::runtime_error("c_to_tacky_unop received an unknown ast::c::UnaryOperator");
            break;
    }
}

// ------------------------------> Map between TACKY unops and C binops <------------------------------

inline constexpr ast::tacky::BinaryOperator c_to_tacky_binops(ast::c::BinaryOperator unop) {
    switch (unop) {
        case ast::c::BinaryOperator::Add:               return ast::tacky::BinaryOperator::Add;
        case ast::c::BinaryOperator::Subtract:          return ast::tacky::BinaryOperator::Subtract;
        case ast::c::BinaryOperator::Multiply:          return ast::tacky::BinaryOperator::Multiply;
        case ast::c::BinaryOperator::Divide:            return ast::tacky::BinaryOperator::Divide;
        case ast::c::BinaryOperator::Modulo:            return ast::tacky::BinaryOperator::Modulo;
        case ast::c::BinaryOperator::Left_Shift:        return ast::tacky::BinaryOperator::Left_Shift;
        case ast::c::BinaryOperator::Right_Shift:       return ast::tacky::BinaryOperator::Right_Shift;
        case ast::c::BinaryOperator::Bitwise_AND:       return ast::tacky::BinaryOperator::Bitwise_AND;
        case ast::c::BinaryOperator::Bitwise_OR:        return ast::tacky::BinaryOperator::Bitwise_OR;
        case ast::c::BinaryOperator::Bitwise_XOR:       return ast::tacky::BinaryOperator::Bitwise_XOR;
        case ast::c::BinaryOperator::Is_Equal:          return ast::tacky::BinaryOperator::Is_Equal;
        case ast::c::BinaryOperator::Not_Equal:         return ast::tacky::BinaryOperator::Not_Equal;
        case ast::c::BinaryOperator::Less_Than:         return ast::tacky::BinaryOperator::Less_Than;
        case ast::c::BinaryOperator::Greater_Than:      return ast::tacky::BinaryOperator::Greater_Than;
        case ast::c::BinaryOperator::Less_Or_Equal:     return ast::tacky::BinaryOperator::Less_Or_Equal;
        case ast::c::BinaryOperator::Greater_Or_Equal:  return ast::tacky::BinaryOperator::Greater_Or_Equal;
    }
    throw std::runtime_error("c_to_tacky_binops received an unknown ast::c::UnaryOperator");
}

// ------------------------------> Conversion from C AST to TACKY AST <------------------------------

struct CToTacky {
    std::vector<ast::tacky::Instruction> mInstructions;

    // Expression visitors
    ast::tacky::Val operator() (const ast::c::Constant& constant) {
        return ast::tacky::Constant(constant.mValue);
    }

    ast::tacky::Val operator()(const ast::c::Variable& var) {
        return ast::tacky::Var(var.mIdentifier);
    }

    ast::tacky::Val operator() (const ast::c::Unary& unary) {
        ast::tacky::Val src = std::visit(*this, *unary.mExpr);
        ast::tacky::Var dst = makeTemporaryRegister();
        auto tacky_op = c_to_tacky_unop(unary.mOp);
        mInstructions.emplace_back(ast::tacky::Unary(tacky_op, src, dst));
        return dst;
    }

    ast::tacky::Val operator() (const ast::c::Binary& binary) {
        // Logical operations need to short circuit
        if (binary.mOp == ast::c::BinaryOperator::Logical_AND) {
            auto [falseLabel, endLabel] = makeAndLabels();
            ast::tacky::Var result = makeTemporaryRegister();

            ast::tacky::Val expressionSrc1 = std::visit(*this, *binary.mLeft);
            mInstructions.emplace_back(ast::tacky::JumpIfZero(expressionSrc1, falseLabel.mIdentifier));

            ast::tacky::Val expressionSrc2 = std::visit(*this, *binary.mRight);
            mInstructions.emplace_back(ast::tacky::JumpIfZero(expressionSrc2, falseLabel.mIdentifier));
            
            mInstructions.emplace_back(ast::tacky::Copy(ast::tacky::Constant(1), result));
            mInstructions.emplace_back(ast::tacky::Jump(endLabel.mIdentifier));
            mInstructions.emplace_back(falseLabel);
            mInstructions.emplace_back(ast::tacky::Copy(ast::tacky::Constant(0), result));
            mInstructions.emplace_back(endLabel);

            return result;
        }
        else if (binary.mOp == ast::c::BinaryOperator::Logical_OR) {
            auto [trueLabel, endLabel] = makeOrLabels();
            ast::tacky::Var result = makeTemporaryRegister();

            ast::tacky::Val expressionSrc1 = std::visit(*this, *binary.mLeft);
            mInstructions.emplace_back(ast::tacky::JumpIfNotZero(expressionSrc1, trueLabel.mIdentifier));

            ast::tacky::Val expressionSrc2 = std::visit(*this, *binary.mRight);
            mInstructions.emplace_back(ast::tacky::JumpIfNotZero(expressionSrc2, trueLabel.mIdentifier));

            mInstructions.emplace_back(ast::tacky::Copy(ast::tacky::Constant(0), result));
            mInstructions.emplace_back(ast::tacky::Jump(endLabel.mIdentifier));
            mInstructions.emplace_back(trueLabel);
            mInstructions.emplace_back(ast::tacky::Copy(ast::tacky::Constant(1), result));
            mInstructions.emplace_back(endLabel);

            return result;
        }

        ast::tacky::Val src1 = std::visit(*this, *binary.mLeft);
        ast::tacky::Val src2 = std::visit(*this, *binary.mRight);
        ast::tacky::Var dst = makeTemporaryRegister();
        auto tacky_op = c_to_tacky_binops(binary.mOp);
        mInstructions.emplace_back(ast::tacky::Binary(tacky_op, src1, src2, dst));
        return dst;
    }

    ast::tacky::Val operator()(const ast::c::Assignment& assignment) {
        auto result = std::visit(*this, *assignment.mRight);
        ast::tacky::Var var(std::get<ast::c::Variable>(*assignment.mLeft).mIdentifier);
        mInstructions.emplace_back(ast::tacky::Copy(result, var));
        return var;
    }

    ast::tacky::Val operator()(const ast::c::Crement& crement) {
        // Get variable
        ast::tacky::Var var(std::get<ast::c::Variable>(*crement.mVar).mIdentifier);

        ast::tacky::BinaryOperator op;
        if (crement.mIncrement)
            op = ast::tacky::BinaryOperator::Add;
        else
            op = ast::tacky::BinaryOperator::Subtract;

        if (crement.mPost) {
            auto tmp = makeTemporaryRegister();
            // Copy value to tmp
            mInstructions.emplace_back(ast::tacky::Copy(var, tmp));
            // increment/decrement var.
            mInstructions.emplace_back(ast::tacky::Binary(
                op,
                var,
                ast::tacky::Constant(1),
                var
            ));
            return tmp;
        }
        else {
            // increment/decrement var.
            mInstructions.emplace_back(ast::tacky::Binary(
                op,
                var,
                ast::tacky::Constant(1),
                var
            ));
            return var;
        }
    }

    ast::tacky::Val operator()(const ast::c::Conditional& conditional) {
        auto [expr2Label, endLabel] = makeConditionalLabels();
        auto result = makeTemporaryRegister();

        // Conditional
        auto conditionResult = std::visit(*this, *conditional.mCondition);
        mInstructions.emplace_back(ast::tacky::JumpIfZero(conditionResult, expr2Label.mIdentifier));

        // Expression 1
        auto v1 = std::visit(*this, *conditional.mThen);
        mInstructions.emplace_back(ast::tacky::Copy(v1, result));
        mInstructions.emplace_back(ast::tacky::Jump(endLabel.mIdentifier));

        // Expression 2
        mInstructions.emplace_back(expr2Label);
        auto v2 = std::visit(*this, *conditional.mElse);
        mInstructions.emplace_back(ast::tacky::Copy(v2, result));

        // End Label
        mInstructions.emplace_back(endLabel);

        // Result stores value of evaluated expression
        return result;
    }

    // Statement visitors
    void operator()(const ast::c::Statement& statement){
        std::visit(*this, statement);
    }

    void operator()(const ast::c::Return& returnNode) {
        ast::tacky::Val src = std::visit(*this, returnNode.mExpr);
        mInstructions.emplace_back(ast::tacky::Return(src));
    }

    void operator()(const ast::c::ExpressionStatement& es) {
        std::visit(*this, es.mExpr);
    }

    void operator()(const ast::c::If& ifStmt) {
        auto [elseLabel, endLabel] = makeIfLabels();

        ast::tacky::Val conditionResult = std::visit(*this, ifStmt.mCondition);
        if (!ifStmt.mElse.has_value()) {
            mInstructions.emplace_back(ast::tacky::JumpIfZero(conditionResult, endLabel.mIdentifier));
            std::visit(*this, *ifStmt.mThen);
        }
        else {
            mInstructions.emplace_back(ast::tacky::JumpIfZero(conditionResult, elseLabel.mIdentifier));
            std::visit(*this, *ifStmt.mThen);
            mInstructions.emplace_back(ast::tacky::Jump(endLabel.mIdentifier));
            mInstructions.emplace_back(elseLabel);
            std::visit(*this, *ifStmt.mElse.value());
        }

        mInstructions.emplace_back(endLabel);
    }

    void operator()(const ast::c::GoTo& gotoStmt) {
        mInstructions.emplace_back(ast::tacky::Jump(gotoStmt.mTarget));
    }

    void operator()(const ast::c::LabelledStatement& labelledStmt) {
        mInstructions.emplace_back(ast::tacky::Label(labelledStmt.mIdentifier));
        std::visit(*this, *labelledStmt.mStatement);
    }

    void operator()(const ast::c::CompoundStatement& compoundStmt) {
        (*this)(*compoundStmt.mCompound);
    }

    void operator()(const ast::c::NullStatement& null) {}

    // Declaration
    void operator()(const ast::c::Declaration& declaration) {
        if (!declaration.mExpr.has_value())
            return;

        auto result = std::visit(*this, declaration.mExpr.value());
        ast::tacky::Var var(declaration.mIdentifier);
        mInstructions.emplace_back(ast::tacky::Copy(result, var));
    }

    // BlockItem
    void operator()(const ast::c::BlockItem& blockItem) {
        std::visit(*this, blockItem);
    }

    // Block
    void operator()(const ast::c::Block& block) {
        for (const auto& blockItem : block.mItems) {
            std::visit(*this, blockItem);
        }
    }

    // Function visitor
    ast::tacky::Function operator()(const ast::c::Function& functionNode) {
        (*this)(functionNode.mBody);
        mInstructions.emplace_back(ast::tacky::Return(ast::tacky::Constant(0)));

        if (functionNode.mIdentifier.has_value())
            return ast::tacky::Function(functionNode.mIdentifier.value(), std::move(mInstructions));
        else
            return ast::tacky::Function(makeTemporaryRegister().mIdentifier, std::move(mInstructions));
    }

    // Program visitor
    ast::tacky::Program operator()(const ast::c::Program& program) {
        return ast::tacky::Program((*this)(program.mFunction));
    }
};

}