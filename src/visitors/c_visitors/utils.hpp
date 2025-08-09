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
    void operator()(const Expression& expr) const {
        std::visit(*this, expr);
    }

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

    void operator()(const FunctionCall& functionCall) const {
        std::cout << indent() << "Function Call: " << functionCall.mIdentifier << std::endl;
        if (functionCall.mArgs.size() <= 0) return;
        std::cout << indent() << "  Arguments:\n";
        for(const auto& arg : functionCall.mArgs) {
            std::visit(PrintVisitor(depth+2), *arg);
        }
    }

    // Declaration
    void operator()(const Declaration& decl) const {
        std::visit(*this, decl);
    }

    void operator()(const VarDecl& varDecl) const {
        std::cout << indent() << "Declaration: " << varDecl.mIdentifier << std::endl;
        if (varDecl.mExpr.has_value()) {
            std::cout << indent() << "  Initialized Expression\n";
            std::visit(PrintVisitor(depth+2), varDecl.mExpr.value());
        }
    }

    void operator()(const FuncDecl& funcDecl) const {
        // identifier
        std::cout << indent() << "Function Declaration: " << funcDecl.mIdentifier << std::endl;

        // parameters
        if(funcDecl.mParams.size() <= 0) std::cout << indent() << "  No Parameters\n";
        else {
            std::cout << indent() << "  Parameters:\n";
            std::cout << indent() << "    ";
            for (const auto& param : funcDecl.mParams)
                std::cout << param << ",    ";
            std::cout << std::endl;
        }

        // definition
        if (!funcDecl.mBody) return;
        std::cout << indent() << "  Definition:\n";
        PrintVisitor(depth+2)(*funcDecl.mBody);
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

    void operator()(const Break& brk) const {
        std::cout << indent() << "Break (" << brk.mLabel << ")\n";
    }

    void operator()(const Continue& cont) const {
        std::cout << indent() << "Continue (" << cont.mLabel << ")\n";
    }

    void operator()(const While& whileStmt) const {
        std::cout << indent() << "While (" << whileStmt.mLabel << "):\n";
        std::cout << indent() << "  Condition:\n";
        std::visit(PrintVisitor(depth+2), whileStmt.mCondition);
        std::cout << indent() << "  Body:\n";
        std::visit(PrintVisitor(depth+2), *whileStmt.mBody);
    }

    void operator()(const DoWhile& doWhile) const {
        std::cout << indent() << "DoWhile (" << doWhile.mLabel << "):\n";
        std::cout << indent() << "  Body:\n";
        std::visit(PrintVisitor(depth+2), *doWhile.mBody);
        std::cout << indent() << "  Condition:\n";
        std::visit(PrintVisitor(depth+2), doWhile.mCondition);
    }

    void operator()(const For& forStmt) const {
        std::cout << indent() << "For (" << forStmt.mLabel << "):\n";

        if (std::holds_alternative<VarDecl>(forStmt.mForInit)) {
            std::cout << indent() << "  Initial Declaration:\n";
            PrintVisitor(depth+2)(std::get<VarDecl>(forStmt.mForInit));
        }
        else {
            const auto& initialExpression = std::get<std::optional<Expression>>(forStmt.mForInit);
            if (initialExpression.has_value()) {
                std::cout << indent() << "  Initial Expression:\n";
                PrintVisitor(depth+2)(initialExpression.value());
            }
        }

        if (forStmt.mCondition.has_value()) {
            std::cout << indent() << "  Condition:\n";
            std::visit(PrintVisitor(depth+2), forStmt.mCondition.value());
        }
        if (forStmt.mPost.has_value()) {
            std::cout << indent() << "  Iteration Expression:\n";
            std::visit(PrintVisitor(depth+2), forStmt.mPost.value());
        }

        std::cout << indent() << "  Loop Body:\n";
        std::visit(PrintVisitor(depth+2), *forStmt.mBody);
    }

    void operator()(const Switch& swtch) const {
        std::cout << indent() << "Switch: " << swtch.mLabel << std::endl;
        std::cout << indent() << "  Controlling Expression:\n";
        std::visit(PrintVisitor(depth+2), swtch.mSelector);
        std::cout << indent() << "  Switch Body:\n";
        std::visit(PrintVisitor(depth+2), *swtch.mBody);
    }

    void operator()(const Case& caseStmt) const {
        std::cout << indent() << "Case (" << caseStmt.mLabel << "):\n";
        std::cout << indent() << "  Condition:\n";
        std::visit(PrintVisitor(depth+2), caseStmt.mCondition);
        std::cout << indent() << "  Statement:\n";
        std::visit(PrintVisitor(depth+2), *caseStmt.mStmt);
    }

    void operator()(const Default& defaultStmt) const {
        std::cout << indent() <<"Default (" << defaultStmt.mLabel << "):\n";
        std::visit(PrintVisitor(depth+1), *defaultStmt.mStmt);
    }

    void operator()(const NullStatement& null) const {
        std::cout << indent() << "Null Statement\n";
    }

    // Block
    void operator()(const Block& block) const {
        for (const auto& blockItem : block.mItems){
            std::visit(PrintVisitor(depth+1), blockItem);
        }
    }

    // Program visitor
    void operator()(const Program& program) const {
        for (const auto& funcDecl : program.mDeclarations)
            (*this)(funcDecl);
    }
};


// ------------------------------> Copy Utils <------------------------------

// Print Visitor
struct CopyVisitor {
    void operator()(const Expression& expr) const {
        std::visit(*this, expr);
    }

    Expression operator()(const Constant& constant) const {
        return Constant(constant.mValue);
    }

    Expression operator()(const Variable& variable) const {
        return Variable(variable.mIdentifier);
    }

    Expression operator()(const Unary& unary) const {
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

    Expression operator()(const FunctionCall& functionCall) const {
        std::vector<std::unique_ptr<Expression>> args;
        for (const auto& arg : functionCall.mArgs)
            args.emplace_back(std::make_unique<Expression>(std::visit(*this, *arg)));
        return FunctionCall(functionCall.mIdentifier, std::move(args));
    }

    // Declaration
    BlockItem operator()(const Declaration& decl) const {
        return std::visit(*this, decl);
    }

    Declaration operator()(const VarDecl& varDecl) const {
        if (varDecl.mExpr.has_value())
            return VarDecl(varDecl.mIdentifier, std::visit(*this, varDecl.mExpr.value()));
        else
            return VarDecl(varDecl.mIdentifier);
    }

    Declaration operator()(const FuncDecl& funcDecl) const {
        std::unique_ptr<Block> body = nullptr;
        if (funcDecl.mBody) body = std::make_unique<Block>((*this)(*funcDecl.mBody));
        return FuncDecl(funcDecl.mIdentifier, funcDecl.mParams, std::move(body));
    }
    
    // Statement visitors
    BlockItem operator()(const Statement& stmt) const {
        return std::visit(*this, stmt);
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

    Statement operator()(const Break& brk) const {
        return Break(brk.mLabel);
    }

    Statement operator()(const Continue& cont) const {
        return Continue(cont.mLabel);
    }

    Statement operator()(const While& whileStmt) const {
        return While(std::visit(*this, whileStmt.mCondition), std::make_unique<Statement>(std::visit(*this, *whileStmt.mBody)), whileStmt.mLabel);
    }

    Statement operator()(const DoWhile& doWhile) const {
        return DoWhile(std::make_unique<Statement>(std::visit(*this, *doWhile.mBody)), std::visit(*this, doWhile.mCondition), doWhile.mLabel);
    }

    Statement operator()(const For& forStmt) const {

        ForInit forInit = std::nullopt;
        if (std::holds_alternative<VarDecl>(forStmt.mForInit)) {
            forInit = std::get<VarDecl>((*this)(std::get<VarDecl>(forStmt.mForInit)));
        }
        else {
            const auto& initialExpression = std::get<std::optional<Expression>>(forStmt.mForInit);
            if (initialExpression.has_value()) {
                forInit = std::visit(*this, initialExpression.value());
            }
        }

        std::optional<Expression> condition = forStmt.mCondition.has_value() 
            ? std::optional<Expression>(std::visit(*this, forStmt.mCondition.value()))
            : std::nullopt;

        std::optional<Expression> post = forStmt.mPost.has_value() 
            ? std::optional<Expression>(std::visit(*this, forStmt.mPost.value()))
            : std::nullopt;

        std::unique_ptr<Statement> body = std::make_unique<Statement>(std::visit(*this, *forStmt.mBody));

        return For(std::move(forInit), std::move(condition), std::move(post), std::move(body), forStmt.mLabel);
    }

    Statement operator()(const Switch& swtch) const {
        return Switch(
            std::visit(*this, swtch.mSelector),
            std::make_unique<Statement>(std::visit(*this, *swtch.mBody)),
            swtch.mLabel
        );
    }

    Statement operator()(const Case& caseStmt) const {
        return Case(
            std::visit(*this, caseStmt.mCondition),
            std::make_unique<Statement>(std::visit(*this, *caseStmt.mStmt)),
            caseStmt.mLabel
        );
    }

    Statement operator()(const Default& defaultStmt) const {
        return Default(std::make_unique<Statement>(std::visit(*this, *defaultStmt.mStmt)), defaultStmt.mLabel);
    }

    Statement operator()(const NullStatement& null) const {
        return NullStatement();
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

    // Program visitor
    Program operator()(const Program& program) const {
        std::vector<FuncDecl> funcDecls;
        for (const auto& funcDecl : program.mDeclarations) {
            funcDecls.push_back(std::get<FuncDecl>((*this)(funcDecl)));
        }
        return Program(std::move(funcDecls));
    }
};

}