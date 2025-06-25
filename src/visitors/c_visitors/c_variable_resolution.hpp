#pragma once
#include <string>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>
#include <optional>
#include <variant>
#include <unordered_map>
#include <format>
#include "../../ast/ast_c.hpp"

namespace compiler::ast::c {

// ------------------------------> Helper function for making variable names unique <------------------------------

inline std::string makeUniqueVarName(const std::string& varName) {
    static uint32_t tmpRegisterNum = 0;
    // cv prefix for custom variable
    return std::format("{}.cv{}", varName, tmpRegisterNum++);
}

struct VariableResolution {

    std::unordered_map<std::string, std::string> mMap;

    // Expression visitors
    void operator()(const Constant& constant) const {}

    void operator()(Variable& variable) const {
        if (!mMap.contains(variable.mIdentifier))
            throw std::runtime_error(std::format("Variable {} is used before it is declared!", variable.mIdentifier));
        
        variable.mIdentifier = mMap.at(variable.mIdentifier);
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

    // Statement visitors
    void operator()(Statement& statement) const {
        std::visit(*this, statement);
    }

    void operator()(Return& rs) const {
        std::visit(*this, rs.mExpr);
    }

    void operator()(ExpressionStatement& es) const {
        std::visit(*this, es.mExpr);
    }

    void operator()(const NullStatement& ns) const {}

    // Declaration visitor
    void operator()(Declaration& declaration) {
        if (mMap.contains(declaration.mIdentifier))
            throw std::runtime_error(std::format("Variable {} has already been declared!", declaration.mIdentifier));

        std::string uniqueName = makeUniqueVarName(declaration.mIdentifier);
        mMap.emplace(declaration.mIdentifier, uniqueName);

        // Replace declaration identifier with new name.
        declaration.mIdentifier = uniqueName;

        // Correct initializer with new var name if it exists
        if (declaration.mExpr.has_value())
            std::visit(*this, declaration.mExpr.value());
    }

    // Function visitor
    void operator()(Function& func) {
        for (BlockItem& blockItem : func.mBody) {
            std::visit(*this, blockItem);
        }
    }

    // Program visitor
    void operator()(Program& program) {
        (*this)(program.mFunction);
    }
};

}