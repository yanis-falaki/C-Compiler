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
    ast::tacky::Val operator()(const ast::c::Expression& expr) {
        return std::visit(*this, expr);
    }

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

    ast::tacky::Val operator()(const ast::c::FunctionCall& functionCall) {
        return ast::tacky::Constant(0);
    }

    // optional expression
    std::optional<ast::tacky::Val> operator()(const std::optional<ast::c::Expression>& optionalExpression) {
        if (optionalExpression.has_value())
            return std::visit(*this, optionalExpression.value());
        return std::nullopt;
    }

    // Declaration
    void operator()(const ast::c::Declaration& decl) {
        std::visit(*this, decl);
    }

    void operator()(const ast::c::VarDecl& varDecl) {
        if (!varDecl.mExpr.has_value())
            return;

        auto result = std::visit(*this, varDecl.mExpr.value());
        ast::tacky::Var var(varDecl.mIdentifier);
        mInstructions.emplace_back(ast::tacky::Copy(result, var));
    }

    void operator()(const ast::c::FuncDecl& funcDecl) {
        if (funcDecl.mBody) {
            (*this)(*funcDecl.mBody);
            mInstructions.emplace_back(ast::tacky::Return(ast::tacky::Constant(0)));
        }

        //return ast::tacky::Function(funcDecl.mIdentifier, std::move(mInstructions));
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

    void operator()(const ast::c::Break& brk) {
        mInstructions.emplace_back(ast::tacky::Jump("break_" + brk.mLabel));
    }

    void operator()(const ast::c::Continue& cont) {
        mInstructions.emplace_back(ast::tacky::Jump("continue_" + cont.mLabel));
    }

    void operator()(const ast::c::While& whileStmt) {
        // Continue label (and start) to dilineate start of loop
        mInstructions.emplace_back(ast::tacky::Label("continue_" + whileStmt.mLabel));
        // Insert condition instructions and get result
        ast::tacky::Val conditionResult = std::visit(*this, whileStmt.mCondition);
        // Jump to break label past the loop if condition is zero
        mInstructions.emplace_back(ast::tacky::JumpIfZero(conditionResult, "break_" + whileStmt.mLabel));
        // Execute loop body
        std::visit(*this, *whileStmt.mBody);
        // Unconditionally jump back to start of loop where condition will be evaluated
        mInstructions.emplace_back(ast::tacky::Jump("continue_" + whileStmt.mLabel));
        // Break label after the loop
        mInstructions.emplace_back(ast::tacky::Label("break_" + whileStmt.mLabel));
    }

    void operator()(const ast::c::DoWhile& doWhile) {
        // start label jump back to
        mInstructions.emplace_back(ast::tacky::Label("start_" + doWhile.mLabel));
        // place loop body instructions in vector
        std::visit(*this, *doWhile.mBody);
        // continue label just before condition evaluation
        mInstructions.emplace_back(ast::tacky::Label("continue_" + doWhile.mLabel));
        ast::tacky::Val conditionResult = std::visit(*this, doWhile.mCondition);
        // return to start label if condition expression is not zero
        mInstructions.emplace_back(ast::tacky::JumpIfNotZero(conditionResult, "start_" + doWhile.mLabel));
        // break label for break statements to refer to outside the loop
        mInstructions.emplace_back(ast::tacky::Label("break_" + doWhile.mLabel));
    }

    void operator()(const ast::c::For& forStmt) {
        // instructions for init
        if (std::holds_alternative<ast::c::VarDecl>(forStmt.mForInit))
            (*this)(std::get<ast::c::VarDecl>(forStmt.mForInit));
        else
            (*this)(std::get<std::optional<ast::c::Expression>>(forStmt.mForInit));
        // start label
        mInstructions.emplace_back(ast::tacky::Label("start_" + forStmt.mLabel));
        // condition
        auto conditionResult = (*this)(forStmt.mCondition);
        if (conditionResult.has_value())
            mInstructions.emplace_back(ast::tacky::JumpIfZero(conditionResult.value(), "break_" + forStmt.mLabel));
        // Body instructions
        std::visit(*this, *forStmt.mBody);
        // continue label
        mInstructions.emplace_back(ast::tacky::Label("continue_" + forStmt.mLabel));
        // post expression
        (*this)(forStmt.mPost);
        // unconditionally jump to start
        mInstructions.emplace_back(ast::tacky::Jump("start_" + forStmt.mLabel));
        // break label outside loop (past unconditional jump)
        mInstructions.emplace_back(ast::tacky::Label("break_" + forStmt.mLabel));
    }

    void operator()(const ast::c::Switch& swtch) {
        // For now it'll just be a sequence of if statements
        ast::tacky::Val selector = std::visit(*this, swtch.mSelector);
        for (int cse : swtch.mCases) {
            mInstructions.emplace_back(ast::tacky::JumpIfEqual(
                selector,
                ast::tacky::Constant(cse),
                std::format("case_{}_{}", cse, swtch.mLabel)
            ));
        }
        if (swtch.hasDefault)
            mInstructions.emplace_back(ast::tacky::Jump("default_" + swtch.mLabel));
        else
            mInstructions.emplace_back(ast::tacky::Jump("break_" + swtch.mLabel));

        std::visit(*this, *swtch.mBody);

        mInstructions.emplace_back(ast::tacky::Label("break_" + swtch.mLabel));
    }

    void operator()(const ast::c::Case& caseStmt) {
        mInstructions.emplace_back(ast::tacky::Label(
            std::format("case_{}_{}", std::get<ast::c::Constant>(caseStmt.mCondition).mValue, caseStmt.mLabel)
        ));
        std::visit(*this, *caseStmt.mStmt);
    }

    void operator()(const ast::c::Default& defaultStmt) {
        mInstructions.emplace_back(ast::tacky::Label("default_" + defaultStmt.mLabel));
        std::visit(*this, *defaultStmt.mStmt);
    }

    void operator()(const ast::c::NullStatement& null) {}

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

    // Program visitor
    ast::tacky::Program operator()(const ast::c::Program& program) {
        return ast::tacky::Program(ast::tacky::Function("", std::vector<ast::tacky::Instruction>()));
        //return ast::tacky::Program((*this)(program.mFunction));
    }
};

}