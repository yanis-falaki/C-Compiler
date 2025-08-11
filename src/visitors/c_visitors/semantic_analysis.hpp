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
#include "../../ast/general.hpp"

namespace compiler::ast::c {

// ------------------------------> Helper function for making variable names unique <------------------------------

inline std::string makeUniqueVarName(const std::string& varName) {
    static uint32_t tmpRegisterNum = 0;
    // cv prefix for custom variable
    return std::format("{}.cv{}", varName, tmpRegisterNum++);
}

// ------------------------------> IdentifierResolution <------------------------------

struct IdentifierData {
    std::string mNewName;
    bool mFromCurrentScope;
    bool mHasExternalLinkage;

    IdentifierData() = default;
    IdentifierData(std::string newName, bool fromCurrentScope, bool hasExternalLinkage)
    :   mNewName(std::move(newName)), mFromCurrentScope(fromCurrentScope), mHasExternalLinkage(hasExternalLinkage) {}
};

struct IdentifierResolution {

private:
    std::vector<std::unordered_map<std::string, IdentifierData>> mIdentifierMaps;

    // helper methods
    auto& getCurrentScope() { return mIdentifierMaps.back(); }
    const auto& getCurrentScope() const { return mIdentifierMaps.back(); }

    void createNewScope() {
        auto newMap = getCurrentScope();
        for (auto& [key, identifierData] : newMap) {
            identifierData.mFromCurrentScope = false;
        }
        mIdentifierMaps.push_back(newMap);
    }

    void exitScope() {
        mIdentifierMaps.pop_back();
    };

    bool isGlobalScope() { return mIdentifierMaps.size() == 1; }

    // helper function for both variable declarations and declarations within function parameters
    void resolveVarDeclName(std::string& variableName) {
        auto& currentScope = getCurrentScope();

        if (currentScope.contains(variableName) && currentScope[variableName].mFromCurrentScope)
            throw std::runtime_error(std::format("Variable {} has already been declared!", variableName));

        std::string uniqueName = makeUniqueVarName(variableName);
        currentScope.insert_or_assign(variableName, IdentifierData(uniqueName, true, false));

        // Replace declaration identifier with new name.
        variableName = uniqueName;
    }

public:
    // Expression visitors
    void operator()(const Constant& constant) const {}

    void operator()(Variable& variable) const {
        auto& currentScope = getCurrentScope();
        if (!currentScope.contains(variable.mIdentifier)) {
            throw std::runtime_error(std::format("Variable {} is used before it is declared!", variable.mIdentifier));
        }
        
        variable.mIdentifier = currentScope.at(variable.mIdentifier).mNewName;
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

    void operator()(FunctionCall& functionCall) const {
        auto& currentScope = getCurrentScope();
        if (currentScope.contains(functionCall.mIdentifier)) {
            functionCall.mIdentifier = currentScope.at(functionCall.mIdentifier).mNewName;
            for (auto& arg : functionCall.mArgs)
                std::visit(*this, *arg);
        }
        else
            throw std::runtime_error("Undeclared function!");
    }

    // Declaration visitor
    void operator()(Declaration& decl) {
        std::visit(*this, decl);
    }

    void operator()(VarDecl& varDecl) {
        resolveVarDeclName(varDecl.mIdentifier);

        // Correct initializer with new var name if it exists
        if (varDecl.mExpr.has_value())
            std::visit(*this, varDecl.mExpr.value());
    }

    void operator()(FuncDecl& funcDecl) {
        auto& currentScope = getCurrentScope();
        bool declInGlobalScope = isGlobalScope();

        // Check that another identifier with internal linkage does not exist else throw an error
        if (currentScope.contains(funcDecl.mIdentifier)) {
            auto prevEntry = currentScope[funcDecl.mIdentifier];
            if (prevEntry.mFromCurrentScope && !prevEntry.mHasExternalLinkage)
                throw std::runtime_error("Function without external linkage declared more than once!");
        }

        // Add function declaration to current scope if not already
        currentScope.insert_or_assign(funcDecl.mIdentifier, IdentifierData(funcDecl.mIdentifier, true, true));

        // Enter function scope
        createNewScope();

        for (auto& varName : funcDecl.mParams)
            resolveVarDeclName(varName);

        // Body will always pop the stack back to zero after it's visited, so each new should have a clean stack
        if (funcDecl.mBody) {
            if (!declInGlobalScope)
                throw std::runtime_error("Nested function definitions are not allowed!");
            (*this)(*funcDecl.mBody, true);
        }

        // Exit function scope
        exitScope();
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
        createNewScope();

        // Resolve for init
        std::visit(*this, forStmt.mForInit);

        // Resolve optional expressions
        (*this)(forStmt.mCondition);
        (*this)(forStmt.mPost);

        // Resolve loop body
        std::visit(*this, *forStmt.mBody);

        // Destroy loop scope
        exitScope();
    }

    void operator()(Switch& swtch) {
        std::visit(*this, swtch.mSelector);
        std::visit(*this, *swtch.mBody);
    }

    void operator()(Case& caseStmt) {
        std::visit(*this, caseStmt.mCondition);
        std::visit(*this, *caseStmt.mStmt);
    }

    void operator()(Default& defaultStmt) {
        std::visit(*this, *defaultStmt.mStmt);
    }

    void operator()(const NullStatement& ns) const {}

    void operator()(Block& block, bool inheritScope = false) {

        // Create scope only if not inheriting
        if (!inheritScope)
            createNewScope();

        for (BlockItem& blockItem : block.mItems)
            std::visit(*this, blockItem);
        
        // Exit scope only if we created one
        if (!inheritScope)
            exitScope();
    }

    // Program visitor
    void operator()(Program& program) {
        // Create global scope
        mIdentifierMaps.push_back(std::unordered_map<std::string, IdentifierData>());
        for (FuncDecl& funcDecl : program.mDeclarations)
            (*this)(funcDecl);
    }
};

// ------------------------------> Type Checking <------------------------------

struct TypeChecking {
private:
    SymbolMapType& mSymbolMap; 

public:
    TypeChecking(SymbolMapType& symbolMap) : mSymbolMap(symbolMap) {}

    // Expression visitors
    void operator()(const Constant& constant) {}

    void operator()(const Variable& variable) {
        if (!std::holds_alternative<Int>(mSymbolMap.at(variable.mIdentifier).mType))
            throw std::runtime_error("Function " + variable.mIdentifier + " used as a variable!");
    }

    void operator()(const Unary& unary) {
        std::visit(*this, *unary.mExpr);
    }

    void operator()(const Binary& binary) {
        std::visit(*this, *binary.mLeft);
        std::visit(*this, *binary.mRight);
    }

    void operator()(const Assignment& assignment) {
        std::visit(*this, *assignment.mLeft);
        std::visit(*this, *assignment.mRight);
    }

    void operator()(const Crement& crement) {      
        std::visit(*this, *crement.mVar);
    }

    void operator()(const Conditional& conditional) {
        std::visit(*this, *conditional.mCondition);
        std::visit(*this, *conditional.mThen);
        std::visit(*this, *conditional.mElse);
    }

    void operator()(const FunctionCall& functionCall) {
        // guaranteed to be in symbol map as no errors were thrown during identifier resolution, i.e. a declaration is in scope
        auto symbolInfo = mSymbolMap.at(functionCall.mIdentifier);
        if (std::holds_alternative<Int>(symbolInfo.mType))
            throw std::runtime_error("Variable " + functionCall.mIdentifier + " used as a function name!");
        if (std::get<FuncType>(symbolInfo.mType).mParamCount != functionCall.mArgs.size())
            throw std::runtime_error("Function " + functionCall.mIdentifier + " with the wrong number of arguments!");
        for (auto& arg : functionCall.mArgs)
            std::visit(*this, *arg);
    }

    void operator()(const std::optional<Expression>& optionalExpression) {
        if (optionalExpression.has_value())
            std::visit(*this, optionalExpression.value());
    }

    // Declaration visitor
    void operator()(const Declaration& decl) {
        std::visit(*this, decl);
    }

    void operator()(const VarDecl& varDecl) {
        mSymbolMap.insert_or_assign(varDecl.mIdentifier, SymbolInfo(Int(), true, false));
        (*this)(varDecl.mExpr);
    }

    void operator()(const FuncDecl& funcDecl) {
        FuncType funcType(funcDecl.mParams.size());
        bool hasBody = funcDecl.mBody != nullptr;
        bool alreadyDefined = false;

        if (mSymbolMap.contains(funcDecl.mIdentifier)) {
            auto symbolInfo = mSymbolMap[funcDecl.mIdentifier];
            if (!std::holds_alternative<FuncType>(symbolInfo.mType) || (std::get<FuncType>(symbolInfo.mType) != funcType))
                throw std::runtime_error("Incompatible function declarations!");
            alreadyDefined = symbolInfo.mDefined;
            if (alreadyDefined && hasBody)
                throw std::runtime_error("Function " + funcDecl.mIdentifier + " is defined more than once!");
        }

        mSymbolMap.insert_or_assign(funcDecl.mIdentifier, SymbolInfo(funcType, hasBody || alreadyDefined, true));
        
        if (funcDecl.mBody) {
            for (const auto& param : funcDecl.mParams)
                mSymbolMap.insert_or_assign(param, SymbolInfo(Int(), false, false));
            (*this)(*funcDecl.mBody);
        }
    }

    // Statement visitors
    void operator()(const Statement& statement) {
        std::visit(*this, statement);
    }

    void operator()(const Return& rs) {
        std::visit(*this, rs.mExpr);
    }

    void operator()(const ExpressionStatement& es) {
        std::visit(*this, es.mExpr);
    }

    void operator()(const If& ifStmt) {
        std::visit(*this, ifStmt.mCondition);
        std::visit(*this, *ifStmt.mThen);
        if (ifStmt.mElse.has_value())
            std::visit(*this, *ifStmt.mElse.value());
    }

    void operator()(const GoTo& gotoStmt) {}

    void operator()(const LabelledStatement& labelledStmt) {
        std::visit(*this, *labelledStmt.mStatement);
    }

    void operator()(const CompoundStatement& compoundStmt) {
        (*this)(*compoundStmt.mCompound);
    }

    void operator()(const Break& brk) {}

    void operator()(const Continue& cont) {}

    void operator()(const While& whileStmt) {
        std::visit(*this, whileStmt.mCondition);
        std::visit(*this, *whileStmt.mBody);
    }

    void operator()(const DoWhile& doWhile) {
        std::visit(*this, doWhile.mCondition);
        std::visit(*this, *doWhile.mBody);
    }

    void operator()(const For& forStmt) {
        // Resolve for init
        std::visit(*this, forStmt.mForInit);

        // Resolve optional expressions
        (*this)(forStmt.mCondition);
        (*this)(forStmt.mPost);

        // Resolve loop body
        std::visit(*this, *forStmt.mBody);
    }

    void operator()(const Switch& swtch) {
        std::visit(*this, swtch.mSelector);
        std::visit(*this, *swtch.mBody);
    }

    void operator()(const Case& caseStmt) {
        std::visit(*this, caseStmt.mCondition);
        std::visit(*this, *caseStmt.mStmt);
    }

    void operator()(const Default& defaultStmt) {
        std::visit(*this, *defaultStmt.mStmt);
    }

    void operator()(const NullStatement& ns) {}

    void operator()(const Block& block) {
        for (auto& blockItem : block.mItems)
            std::visit(*this, blockItem);
    }

    // Program visitor
    void operator()(const Program& program) {
        for (auto& funcDecl : program.mDeclarations)
            (*this)(funcDecl);
    }
};

// ------------------------------> ControlFlow Labelling <------------------------------

// ------------------------------> Helper functions <------------------------------

inline std::string makeUniqueLoopID() {
    static uint32_t currentLoopID = 0;
    return std::format("loop.{}", currentLoopID++);
}

inline std::string makeUniqueSwitchID() {
    static uint32_t currentSwitchID = 0;
    return std::format("switch.{}", currentSwitchID++);
}

struct ControlFlowLabelling {

    std::vector<std::string> loopIDs;
    std::vector<std::string> switchIDs;
    std::vector<Switch*> switchPtrs;
    std::vector<std::string> switchAndLoopIDs;

    void newLoop() {
        loopIDs.push_back(makeUniqueLoopID());
        switchAndLoopIDs.push_back(loopIDs.back());
    }

    void popLoop() {
        loopIDs.pop_back();
        switchAndLoopIDs.pop_back();
    }

    void newSwitch(Switch* swtchPtr) {
        switchIDs.push_back(makeUniqueSwitchID());
        switchAndLoopIDs.push_back(switchIDs.back());
        switchPtrs.push_back(swtchPtr);
    }

    void popSwitch() {
        switchIDs.pop_back();
        switchAndLoopIDs.pop_back();
        switchPtrs.pop_back();
    }

    // Expression visitors
    void operator()(const Constant& constant) const {}

    void operator()(const Variable& variable) const {}

    void operator()(const Unary& unary) const {}

    void operator()(const Binary& binary) const {}

    void operator()(const Assignment& assignment) const {}

    void operator()(const Crement& crement) const {}

    void operator()(const Conditional& conditional) const {}

    void operator()(const FunctionCall& functionCall) const {}

    // Declaration visitor
    void operator()(Declaration& decl) {
        std::visit(*this, decl);
    }

    void operator()(const VarDecl& varDecl) const {}

    void operator()(FuncDecl& funcDecl) {
        if (funcDecl.mBody) {
            (*this)(*funcDecl.mBody);
        }
    }

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
        if (switchAndLoopIDs.size() <= 0)
            throw std::runtime_error("Break statement found outside a loop or switch!");
        
        brk.mLabel = switchAndLoopIDs.back();
    }

    void operator()(Continue& cont) const {
        if (loopIDs.size() <= 0)
        throw std::runtime_error("Continue statement found outside a loop!");
    
        cont.mLabel = loopIDs.back();
    }

    void operator()(While& whileStmt) {
        newLoop();
        whileStmt.mLabel = loopIDs.back();
        std::visit(*this, *whileStmt.mBody);
        popLoop();
    }

    void operator()(DoWhile& doWhile) {
        newLoop();
        doWhile.mLabel = loopIDs.back();
        std::visit(*this, *doWhile.mBody);
        popLoop();
    }

    void operator()(For& forStmt) {
        newLoop();
        forStmt.mLabel = loopIDs.back();
        std::visit(*this, *forStmt.mBody);
        popLoop();
    }

    void operator()(Switch& swtch) {
        newSwitch(&swtch);
        swtch.mLabel = switchIDs.back();
        std::visit(*this, *swtch.mBody);
        popSwitch();
    }

    void operator()(Case& caseStmt) {
        if (switchIDs.size() <= 0)
            throw std::runtime_error("Case statement found outside a switch!");
        if (!std::holds_alternative<Constant>(caseStmt.mCondition))
            throw std::runtime_error("Only single integer literals are supported in case labels (constant expressions are not supported yet).");

        auto presentCases = switchPtrs.back()->mCases;
        auto currentCase = std::get<Constant>(caseStmt.mCondition).mValue;

        if (std::find(presentCases.begin(), presentCases.end(), currentCase) != presentCases.end())
            throw std::runtime_error("Duplicate cases found in switch statement!");
        
        caseStmt.mLabel = switchIDs.back();
        switchPtrs.back()->addCase(currentCase);
        std::visit(*this, *caseStmt.mStmt);
    }

    void operator()(Default& defaultStmt) {
        if (switchIDs.size() <= 0)
            throw std::runtime_error("Case statement found outside a switch!");
        if (switchPtrs.back()->hasDefault)
            throw std::runtime_error("Default case already declared within switch statement!");
        
        defaultStmt.mLabel = switchIDs.back();
        switchPtrs.back()->hasDefault = true;
        std::visit(*this, *defaultStmt.mStmt);
    }

    void operator()(const NullStatement& ns) const {}

    // Block visitor
    void operator()(Block& block) {
        for (BlockItem& blockItem : block.mItems) {
            std::visit(*this, blockItem);
        }
    }

    // Program visitor
    void operator()(Program& program) {
        for (auto& funcDecl : program.mDeclarations)
            (*this)(funcDecl);
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

    void clearNeededAndPresentLabels() {
        mPresentLabels.clear();
        mNeededLabels.clear();
    }

    // Expression visitors
    void operator()(const Constant& constant) const {}

    void operator()(const Variable& variable) const {}

    void operator()(const Unary& unary) const {}

    void operator()(const Binary& binary) const {}

    void operator()(const Assignment& assignment) const {}

    void operator()(const Crement& crement) const {}

    void operator()(const Conditional& conditional) const {}

    void operator()(const FunctionCall& functionCall) const {};

    // Declaration visitor
    void operator()(Declaration& decl) {
        std::visit(*this, decl);
    }

    void operator()(const VarDecl& varDecl) const {}

    void operator()(FuncDecl& funcDecl) {
        if (funcDecl.mBody) {
            (*this)(*funcDecl.mBody);
            checkNeededLabelsInPresentLabels();
            clearNeededAndPresentLabels();
        }
    }

    // Statement visitors
    void operator()(Statement& statement) {
        std::visit(*this, statement);
    }

    void operator()(const Return& rs) const {}

    void operator()(const ExpressionStatement& es) const {}

    void operator()(const If& ifStmt) {
        std::visit(*this, *ifStmt.mThen);
        if (ifStmt.mElse.has_value())
            std::visit(*this, *ifStmt.mElse.value());
    }

    void operator()(const GoTo& gotoStmt) {
        if (!mNeededLabels.contains(gotoStmt.mTarget))
            mNeededLabels.insert(gotoStmt.mTarget);
    }

    void operator()(const LabelledStatement& labelledStmt) {
        if (mPresentLabels.contains(labelledStmt.mIdentifier))
            throw std::runtime_error(std::format("Label: {} already declared!", labelledStmt.mIdentifier));
        
        mPresentLabels.insert(labelledStmt.mIdentifier);
        std::visit(*this, *labelledStmt.mStatement);
    }

    void operator()(const CompoundStatement& compoundStmt) {
        (*this)(*compoundStmt.mCompound);
    }

    void operator()(const Break& brk) const {}

    void operator()(const Continue& cont) const {}

    void operator()(const While& whileStmt) {
        std::visit(*this, *whileStmt.mBody);
    }

    void operator()(const DoWhile& doWhile) {
        std::visit(*this, *doWhile.mBody);
    }

    void operator()(const For& forStmt) {
        std::visit(*this, *forStmt.mBody);
    }

    void operator()(const Switch& swtch) {
        std::visit(*this, *swtch.mBody);
    }

    void operator()(const Case& caseStmt) {
        std::visit(*this, *caseStmt.mStmt);
    }

    void operator()(const Default& defaultStmt) {
        std::visit(*this, *defaultStmt.mStmt);
    }

    void operator()(const NullStatement& ns) const {}

    // Block visitor
    void operator()(Block& block) {
        for (BlockItem& blockItem : block.mItems) {
            std::visit(*this, blockItem);
        }
    }

    // Program visitor
    void operator()(Program& program) {
        for (auto& funcDecl : program.mDeclarations)
            (*this)(funcDecl);
    }
};

}