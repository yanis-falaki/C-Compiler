#include <filesystem>
#include <iostream>
#include <algorithm>
#include <ctre.hpp>
#include <stdexcept>
#include <format>
#include "lexer.hpp"
#include "ast/ast_c.hpp"
#include "parser.hpp"
#include "utils.h"

namespace fs = std::filesystem;

namespace compiler::parser {

// ------------------------------> Language Construct Parsing <------------------------------

// ------------------------------> Unary LexType to C AST Unary Op <------------------------------

inline constexpr ast::c::UnaryOperator lextype_to_unary_op(lexer::LexType unop) {
    switch (unop) {
        case lexer::LexType::Negation:               return ast::c::UnaryOperator::Negate;
        case lexer::LexType::BitwiseComplement:      return ast::c::UnaryOperator::Complement;
        case lexer::LexType::Logical_NOT:            return ast::c::UnaryOperator::Logical_NOT;
    }
    throw std::runtime_error("lextype_to_unary_op received an invalid lexer::LexType");
}

inline constexpr ast::c::BinaryOperator lextype_to_binary_op(lexer::LexType unop) {
    switch (unop) {
        case lexer::LexType::Negation:               return ast::c::BinaryOperator::Subtract;
        case lexer::LexType::Plus:                   return ast::c::BinaryOperator::Add;
        case lexer::LexType::Asterisk:               return ast::c::BinaryOperator::Multiply;
        case lexer::LexType::Forward_Slash:          return ast::c::BinaryOperator::Divide;
        case lexer::LexType::Percent:                return ast::c::BinaryOperator::Modulo;
        case lexer::LexType::Left_Shift:             return ast::c::BinaryOperator::Left_Shift;
        case lexer::LexType::Right_Shift:            return ast::c::BinaryOperator::Right_Shift;
        case lexer::LexType::Bitwise_AND:            return ast::c::BinaryOperator::Bitwise_AND;
        case lexer::LexType::Bitwise_OR:             return ast::c::BinaryOperator::Bitwise_OR;
        case lexer::LexType::Bitwise_XOR:            return ast::c::BinaryOperator::Bitwise_XOR;
        case lexer::LexType::Logical_AND:            return ast::c::BinaryOperator::Logical_AND;
        case lexer::LexType::Logical_OR:             return ast::c::BinaryOperator::Logical_OR;
        case lexer::LexType::Is_Equal:               return ast::c::BinaryOperator::Is_Equal;
        case lexer::LexType::Not_Equal:              return ast::c::BinaryOperator::Not_Equal;
        case lexer::LexType::Less_Than:              return ast::c::BinaryOperator::Less_Than;
        case lexer::LexType::Greater_Than:           return ast::c::BinaryOperator::Greater_Than;
        case lexer::LexType::Less_Or_Equal:          return ast::c::BinaryOperator::Less_Or_Equal;
        case lexer::LexType::Greater_Or_Equal:       return ast::c::BinaryOperator::Greater_Or_Equal;
    }
    throw std::runtime_error("lextype_to_binary_op received an invalid lexer::LexType");
}

// ------------------------------> expect <------------------------------

const lexer::LexItem& expectAndAdvance(lexer::LexType expectedLexType, lexer::LexList& lexList) {
    const lexer::LexItem& actual = lexList.consume();
    if (actual.mLexType != expectedLexType) {
        auto errorString = std::format("Expected: {}, got {}", lex_type_to_str(expectedLexType), actual.mSV);
        throw std::runtime_error(errorString);
    }
    return actual;
}

const lexer::LexItem& expectNoAdvance(lexer::LexType expectedLexType, lexer::LexList& lexList) {
    const lexer::LexItem& actual = lexList.current();
    if (actual.mLexType != expectedLexType) {
        auto errorString = std::format("Expected: {}, got {}", lex_type_to_str(expectedLexType), actual.mSV);
        throw std::runtime_error(errorString);
    }
    return actual;
}

// ------------------------------> parseConstant <------------------------------

static ast::c::Constant parseConstant(lexer::LexList& lexList) {
    const lexer::LexItem& lexConstant = expectAndAdvance(lexer::LexType::Constant, lexList);
    return ast::c::Constant(std::stoi(std::string(lexConstant.mSV)));
}

// forward declaration
static ast::c::Expression parseExpression(lexer::LexList& lexList, uint32_t minPrecedence=0);

// ------------------------------> parseFactor <------------------------------

static ast::c::Expression parseFactor(lexer::LexList& lexList) {
    const lexer::LexItem& currentToken = lexList.current();

    if (currentToken.mLexType == lexer::LexType::Constant)
        return parseConstant(lexList);

    else if (lexer::is_lextype_unary_op(currentToken.mLexType)) {
        lexList.advance();
        auto factor = std::make_unique<ast::c::Expression>(parseFactor(lexList));
        return ast::c::Unary(lextype_to_unary_op(currentToken.mLexType), std::move(factor));
    }

    else if (currentToken.mLexType == lexer::LexType::Open_Parenthesis) {
        lexList.advance();
        auto innerExpression = parseExpression(lexList);
        expectAndAdvance(lexer::LexType::Close_Parenthesis, lexList);
        return innerExpression;
    }

    else if (currentToken.mLexType == lexer::LexType::Identifier) {
        lexList.advance();
        return ast::c::Variable(std::string(currentToken.mSV));
    }

    std::string errorString = std::format("Malformed factor, got: {}", currentToken.mSV);
    throw std::runtime_error(errorString);
}

// ------------------------------> parseExpression <------------------------------

static ast::c::Expression parseExpression(lexer::LexList& lexList, uint32_t minPrecedence) {
    auto expression = parseFactor(lexList);
    const lexer::LexItem* currentToken = &lexList.current();

    // Check if operator is a binary op and is above minimum precendence level
    while (lexer::is_lextype_binary_op(currentToken->mLexType)
        && lexer::binary_op_precedence(currentToken->mLexType) >= minPrecedence) {
        
        if (currentToken->mLexType == lexer::LexType::Assignment) {
            lexList.advance();
            auto right = std::make_unique<ast::c::Expression>(
                parseExpression(lexList, lexer::binary_op_precedence(lexer::LexType::Assignment)));
            expression = ast::c::Assignment(std::make_unique<ast::c::Expression>(std::move(expression)), 
                                        std::move(right));
            break;
        }

        auto op = lextype_to_binary_op(currentToken->mLexType);
        lexList.advance();
        auto right = std::make_unique<ast::c::Expression>(parseExpression(lexList, lexer::binary_op_precedence(currentToken->mLexType)+1));
        expression = ast::c::Binary(op, std::make_unique<ast::c::Expression>(std::move(expression)), 
                                  std::move(right));
        currentToken = &lexList.current();
    }

    return expression;
}

// ------------------------------> parseStatement <------------------------------

static ast::c::Statement parseStatement(lexer::LexList& lexList) {
    // Return Statement
    if (lexList.current().mLexType == lexer::LexType::Return) {
        lexList.advance();
        auto returnObject = ast::c::Return(parseExpression(lexList));
        expectAndAdvance(lexer::LexType::Semicolon, lexList);
        return returnObject;
    }
    // Null Statement
    else if (lexList.current().mLexType == lexer::LexType::Semicolon) {
        lexList.advance();
        return ast::c::NullStatement();
    }
    // Expression Statement
    else {
        ast::c::ExpressionStatement es(parseExpression(lexList));
        expectAndAdvance(lexer::LexType::Semicolon, lexList);
        return es;
    }

    //std::string errorString = std::format("Expected a statement, instead got: {}", lexStatement.mSV);
    //throw std::runtime_error(errorString);
}

// ------------------------------> parseDeclaration <------------------------------

static ast::c::Declaration parseDeclaration(lexer::LexList& lexList) {
    expectAndAdvance(lexer::LexType::Int, lexList);
    std::string identifier(expectAndAdvance(lexer::LexType::Identifier, lexList).mSV);
    const auto& currentToken = lexList.current();
    lexList.advance();

    if (currentToken.mLexType == lexer::LexType::Assignment) {
        ast::c::Expression expression = parseExpression(lexList);
        expectAndAdvance(lexer::LexType::Semicolon, lexList);
        return ast::c::Declaration(std::move(identifier), std::move(expression));
    }
    else if (currentToken.mLexType == lexer::LexType::Semicolon) {
        return ast::c::Declaration(std::move(identifier));
    }
    else {
        throw(std::format("Invalid declaration, got {}", currentToken.mSV));
    }
}

// ------------------------------> parseBlockItem <------------------------------
static ast::c::BlockItem parseBlockItem(lexer::LexList& lexList) {
    const lexer::LexItem* currentToken = &lexList.current();

    // Declaration
    if (currentToken->mLexType == lexer::LexType::Int) {
        return parseDeclaration(lexList);
    }
    else return parseStatement(lexList);
}

// ------------------------------> parseFunction <------------------------------

static ast::c::Function parseFunction(lexer::LexList& lexList) {
    // Check for keyword void
    expectAndAdvance(lexer::LexType::Int, lexList);

    // Check for identifier (should be main but we'll pay no attention for now)
    auto lexIdentifier =  expectAndAdvance(lexer::LexType::Identifier, lexList);

    // Check for parameter list
    expectAndAdvance(lexer::LexType::Open_Parenthesis, lexList);
    expectAndAdvance(lexer::LexType::Void, lexList);
    expectAndAdvance(lexer::LexType::Close_Parenthesis, lexList);

    // Opening brace
    expectAndAdvance(lexer::LexType::Open_Brace, lexList);

    /*              END Function Preamble              */

    std::vector<ast::c::BlockItem> functionBody;
    while (lexList.current().mLexType != lexer::LexType::Close_Brace) {
        auto nextBlockItem = parseBlockItem(lexList);
        functionBody.push_back(std::move(nextBlockItem));
    }

    lexList.advance();
    return ast::c::Function(std::string(lexIdentifier.mSV), std::move(functionBody));
}

// ------------------------------> parseProgram <------------------------------

ast::c::Program parseProgram(lexer::LexList& lexList) {
    // For now the program can only take a single function.
    auto returnedProgram = ast::c::Program(parseFunction(lexList));

    if (lexList.hasCurrent())
        throw std::runtime_error("Program can only contain one top level function (for now)");

    return returnedProgram;
}

}
