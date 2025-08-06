#pragma once
#include <string>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>
#include <optional>
#include <variant>
#include <unordered_map>
#include <unordered_set>
#include <format>
#include "../../ast/ast_c.hpp"

namespace compiler::ast::c {

// ------------------------------> Helper function for making variable names unique <------------------------------

inline std::string makeUniqueVarName(const std::string& varName) {
    static uint32_t tmpRegisterNum = 0;
    // cv prefix for custom variable
    return std::format("{}.cv{}", varName, tmpRegisterNum++);
}

// ------------------------------> VariableResolution <------------------------------

struct VariableResolution {

    std::vector<std::unordered_map<std::string, std::string>> mScopes;
    std::vector<std::unordered_set<std::string>> mVarsDeclaredInScope;

// helper methods
private:
    auto& getCurrentScope() { return mScopes.back(); }
    const auto& getCurrentScope() const { return mScopes.back(); }
    
    auto& getCurrentDeclared() { return mVarsDeclaredInScope.back(); }
    const auto& getCurrentDeclared() const { return mVarsDeclaredInScope.back(); }

public:
    // Expression visitors
    void operator()(const Constant& constant) const {}

    void operator()(Variable& variable) const {
        auto& currentScope = getCurrentScope();
        if (!currentScope.contains(variable.mIdentifier)) {
            throw std::runtime_error(std::format("Variable {} is used before it is declared!", variable.mIdentifier));
        }
        
        variable.mIdentifier = currentScope.at(variable.mIdentifier);
    }

    void operator()(Unary& unary) const {
        std::visit(*this, *unary.mExpr);
    }

    void operator()(Binary& binary) const {
        std::visit(*this, *binary.mLeft);
        std::visit(*this, *binary.mRight);
    }

    void operator()(Assignment& assignment) const {
        if (!std::holds_alternative<Variable>(*assignment.mLeft))
            throw std::runtime_error("Assignment contains invalid lvalue!");

        std::visit(*this, *assignment.mLeft);
        std::visit(*this, *assignment.mRight);
    }

    void operator()(Crement& crement) const {
        if (!std::holds_alternative<Variable>(*crement.mVar))
            throw std::runtime_error("Assignment contains invalid lvalue!");
        
        std::visit(*this, *crement.mVar);
    }

    void operator()(Conditional& conditional) const {
        std::visit(*this, *conditional.mCondition);
        std::visit(*this, *conditional.mThen);
        std::visit(*this, *conditional.mElse);
    }

    void operator()(std::optional<Expression>& optionalExpression) const {
        if (optionalExpression.has_value())
            std::visit(*this, optionalExpression.value());
    }

    // Statement visitors
    void operator()(Statement& statement) {
        std::visit(*this, statement);
    }

    void operator()(Return& rs) const {
        std::visit(*this, rs.mExpr);
    }

    void operator()(ExpressionStatement& es) const {
        std::visit(*this, es.mExpr);
    }

    void operator()(If& ifStmt) {
        std::visit(*this, ifStmt.mCondition);
        std::visit(*this, *ifStmt.mThen);
        if (ifStmt.mElse.has_value())
            std::visit(*this, *ifStmt.mElse.value());
    }

    void operator()(GoTo& gotoStmt) const {}

    void operator()(LabelledStatement& labelledStmt) {
        std::visit(*this, *labelledStmt.mStatement);
    }

    void operator()(CompoundStatement& compoundStmt) {
        (*this)(*compoundStmt.mCompound);
    }

    void operator()(const Break& brk) const {}

    void operator()(const Continue& cont) const {}

    void operator()(While& whileStmt) {
        std::visit(*this, whileStmt.mCondition);
        std::visit(*this, *whileStmt.mBody);
    }

    void operator()(DoWhile& doWhile) {
        std::visit(*this, doWhile.mCondition);
        std::visit(*this, *doWhile.mBody);
    }

    void operator()(For& forStmt) {
        // Create loop scope
        mScopes.push_back(getCurrentScope());
        mVarsDeclaredInScope.push_back(std::unordered_set<std::string>());

        // Resolve for init
        std::visit(*this, forStmt.mForInit);

        // Resolve optional expressions
        (*this)(forStmt.mCondition);
        (*this)(forStmt.mPost);

        // Resolve loop body
        std::visit(*this, *forStmt.mBody);

        // Destroy loop scope
        mScopes.pop_back();
        mVarsDeclaredInScope.pop_back();
    }

    // TODO implement variable resolution for switch constructs
    void operator()(const Switch& swtch) const {
    }

    void operator()(const Case& caseStmt) const {
    }

    void operator()(const Default& defaultStmt) const {
    }

    void operator()(const NullStatement& ns) const {}

    // Declaration visitor
    void operator()(Declaration& declaration) {
        std::string variableName = declaration.mIdentifier;
        auto& currentScope = getCurrentScope();
        auto& currentDeclared = getCurrentDeclared();

        if (currentScope.contains(variableName) && currentDeclared.contains(variableName)) {
            std::cout << currentScope.contains(variableName) << std::endl;
            std::cout << currentDeclared.contains(variableName) << std::endl;
            throw std::runtime_error(std::format("Variable {} has already been declared!", variableName));
        }

        currentDeclared.insert(variableName);

        std::string uniqueName = makeUniqueVarName(variableName);
        currentScope.insert_or_assign(variableName, uniqueName);

        // Replace declaration identifier with new name.
        declaration.mIdentifier = uniqueName;

        // Correct initializer with new var name if it exists
        if (declaration.mExpr.has_value())
            std::visit(*this, declaration.mExpr.value());
    }

    void operator()(Block& block) {

        // Create scope
        if (mScopes.size() <= 0)
            mScopes.push_back(std::unordered_map<std::string, std::string>());
        else
            mScopes.push_back(getCurrentScope());
        
        mVarsDeclaredInScope.push_back(std::unordered_set<std::string>());

        for (BlockItem& blockItem : block.mItems) {
            std::visit(*this, blockItem);
        }

        // Exit scope
        mScopes.pop_back();
        mVarsDeclaredInScope.pop_back();
    }

    // Function visitor
    void operator()(Function& func) {
        (*this)(func.mBody);
    }

    // Program visitor
    void operator()(Program& program) {
        (*this)(program.mFunction);
    }
};

// ------------------------------> Loop Labelling <------------------------------

// ------------------------------> Helper function for making unique loop ids <------------------------------

inline std::string makeUniqueLoopID() {
    static uint32_t uniqueID = 0;
    return std::format("loop.{}", uniqueID++);
}

struct LoopLabelling {

    std::vector<std::string> loopIDs;

    // Expression visitors
    void operator()(const Constant& constant) const {}

    void operator()(const Variable& variable) const {}

    void operator()(const Unary& unary) const {}

    void operator()(const Binary& binary) const {}

    void operator()(const Assignment& assignment) const {}

    void operator()(const Crement& crement) const {}

    void operator()(const Conditional& conditional) const {}

    // Declaration visitor
    void operator()(const Declaration& declaration) const {}

    // Statement visitors
    void operator()(Statement& statement) {
        std::visit(*this, statement);
    }

    void operator()(const Return& rs) const {}

    void operator()(const ExpressionStatement& es) const {}

    void operator()(If& ifStmt) {
        std::visit(*this, ifStmt.mCondition);
        std::visit(*this, *ifStmt.mThen);
        if (ifStmt.mElse.has_value())
            std::visit(*this, *ifStmt.mElse.value());
    }

    void operator()(const GoTo& gotoStmt) const {}

    void operator()(LabelledStatement& labelledStmt) {
        std::visit(*this, *labelledStmt.mStatement);
    }

    void operator()(CompoundStatement& compoundStmt) {
        (*this)(*compoundStmt.mCompound);
    }

    void operator()(Break& brk) const {
        if (loopIDs.size() <= 0)
            throw std::runtime_error("Break statement found outside a loop!");
        
        brk.mLabel = loopIDs.back();
    }

    void operator()(Continue& cont) const {
        if (loopIDs.size() <= 0)
        throw std::runtime_error("Continue statement found outside a loop!");
    
        cont.mLabel = loopIDs.back();
    }

    void operator()(While& whileStmt) {
        loopIDs.push_back(makeUniqueLoopID());
        whileStmt.mLabel = loopIDs.back();
        std::visit(*this, *whileStmt.mBody);
        loopIDs.pop_back();
    }

    void operator()(DoWhile& doWhile) {
        loopIDs.push_back(makeUniqueLoopID());
        doWhile.mLabel = loopIDs.back();
        std::visit(*this, *doWhile.mBody);
        loopIDs.pop_back(); 
    }

    void operator()(For& forStmt) {
        loopIDs.push_back(makeUniqueLoopID());
        forStmt.mLabel = loopIDs.back();
        std::visit(*this, *forStmt.mBody);
        loopIDs.pop_back();
    }

    // TODO implement loop labelling for switch constructs
    void operator()(const Switch& swtch) const {
    }

    void operator()(const Case& caseStmt) const {
    }

    void operator()(const Default& defaultStmt) const {
    }

    void operator()(const NullStatement& ns) const {}

    // Block visitor
    void operator()(Block& block) {
        for (BlockItem& blockItem : block.mItems) {
            std::visit(*this, blockItem);
        }
    }

    // Function visitor
    void operator()(Function& func) {
        (*this)(func.mBody);
    }

    // Program visitor
    void operator()(Program& program) {
        (*this)(program.mFunction);
    }
};

// ------------------------------> LabelResolution <------------------------------

struct LabelResolution {

    std::unordered_set<std::string> mPresentLabels;
    std::unordered_set<std::string> mNeededLabels;

    void checkNeededLabelsInPresentLabels() const {
        for (auto& label : mNeededLabels) {
            if (!mPresentLabels.contains(label))
                throw std::runtime_error(std::format("Label {} used but not defined", label));
        }
    }

    // Expression visitors
    void operator()(const Constant& constant) const {}

    void operator()(const Variable& variable) const {}

    void operator()(const Unary& unary) const {}

    void operator()(const Binary& binary) const {}

    void operator()(const Assignment& assignment) const {}

    void operator()(const Crement& crement) const {}

    void operator()(const Conditional& conditional) const {}

    // Statement visitors
    void operator()(Statement& statement) {
        std::visit(*this, statement);
    }

    void operator()(const Return& rs) const {}

    void operator()(const ExpressionStatement& es) const {}

    void operator()(If& ifStmt) {
        std::visit(*this, *ifStmt.mThen);
        if (ifStmt.mElse.has_value())
            std::visit(*this, *ifStmt.mElse.value());
    }

    void operator()(GoTo& gotoStmt) {
        if (!mNeededLabels.contains(gotoStmt.mTarget))
            mNeededLabels.insert(gotoStmt.mTarget);
    }

    void operator()(LabelledStatement& labelledStmt) {
        if (mPresentLabels.contains(labelledStmt.mIdentifier))
            throw std::runtime_error(std::format("Label: {} already declared!", labelledStmt.mIdentifier));
        
        mPresentLabels.insert(labelledStmt.mIdentifier);
        std::visit(*this, *labelledStmt.mStatement);
    }

    void operator()(CompoundStatement& compoundStmt) {
        (*this)(*compoundStmt.mCompound);
    }

    void operator()(const Break& brk) const {}

    void operator()(const Continue& cont) const {}

    void operator()(While& whileStmt) {
        std::visit(*this, *whileStmt.mBody);
    }

    void operator()(DoWhile& doWhile) {
        std::visit(*this, *doWhile.mBody);
    }

    void operator()(For& forStmt) {
        std::visit(*this, *forStmt.mBody);
    }

    // TODO implement label resolution for switch constructs
    void operator()(const Switch& swtch) const {
    }

    void operator()(const Case& caseStmt) const {
    }

    void operator()(const Default& defaultStmt) const {
    }

    void operator()(const NullStatement& ns) const {}

    // Declaration visitor
    void operator()(const Declaration& declaration) const {}

    // Block visitor
    void operator()(Block& block) {
        for (BlockItem& blockItem : block.mItems) {
            std::visit(*this, blockItem);
        }
    }

    // Function visitor
    void operator()(Function& func) {
        (*this)(func.mBody);
        checkNeededLabelsInPresentLabels();
    }

    // Program visitor
    void operator()(Program& program) {
        (*this)(program.mFunction);
    }
};

}