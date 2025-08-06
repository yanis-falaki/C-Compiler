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
#include "visitors/c_visitors/utils.hpp"

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
        case lexer::LexType::Negation:              return ast::c::BinaryOperator::Subtract;
        case lexer::LexType::Plus:                  return ast::c::BinaryOperator::Add;
        case lexer::LexType::Asterisk:              return ast::c::BinaryOperator::Multiply;
        case lexer::LexType::Forward_Slash:         return ast::c::BinaryOperator::Divide;
        case lexer::LexType::Percent:               return ast::c::BinaryOperator::Modulo;
        case lexer::LexType::Left_Shift:            return ast::c::BinaryOperator::Left_Shift;
        case lexer::LexType::Right_Shift:           return ast::c::BinaryOperator::Right_Shift;
        case lexer::LexType::Bitwise_AND:           return ast::c::BinaryOperator::Bitwise_AND;
        case lexer::LexType::Bitwise_OR:            return ast::c::BinaryOperator::Bitwise_OR;
        case lexer::LexType::Bitwise_XOR:           return ast::c::BinaryOperator::Bitwise_XOR;
        case lexer::LexType::Logical_AND:           return ast::c::BinaryOperator::Logical_AND;
        case lexer::LexType::Logical_OR:            return ast::c::BinaryOperator::Logical_OR;
        case lexer::LexType::Is_Equal:              return ast::c::BinaryOperator::Is_Equal;
        case lexer::LexType::Not_Equal:             return ast::c::BinaryOperator::Not_Equal;
        case lexer::LexType::Less_Than:             return ast::c::BinaryOperator::Less_Than;
        case lexer::LexType::Greater_Than:          return ast::c::BinaryOperator::Greater_Than;
        case lexer::LexType::Less_Or_Equal:         return ast::c::BinaryOperator::Less_Or_Equal;
        case lexer::LexType::Greater_Or_Equal:      return ast::c::BinaryOperator::Greater_Or_Equal;

        // Compound Assignment
        case lexer::LexType::Plus_Equal:            return ast::c::BinaryOperator::Add;
        case lexer::LexType::Minus_Equal:           return ast::c::BinaryOperator::Subtract;
        case lexer::LexType::Multiply_Equal:        return ast::c::BinaryOperator::Multiply;
        case lexer::LexType::Divide_Equal:          return ast::c::BinaryOperator::Divide;
        case lexer::LexType::Modulo_Equal:          return ast::c::BinaryOperator::Modulo;
        case lexer::LexType::AND_Equal:             return ast::c::BinaryOperator::Bitwise_AND;
        case lexer::LexType::OR_Equal:              return ast::c::BinaryOperator::Bitwise_OR;
        case lexer::LexType::XOR_Equal:             return ast::c::BinaryOperator::Bitwise_XOR;
        case lexer::LexType::Left_Shift_Equal:      return ast::c::BinaryOperator::Left_Shift;
        case lexer::LexType::Right_Shift_Equal:     return ast::c::BinaryOperator::Right_Shift;
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

// forward declarations
static ast::c::Expression parseExpression(lexer::LexList& lexList, uint32_t minPrecedence=0);
static ast::c::Declaration parseDeclaration(lexer::LexList& lexList);

// ------------------------------> parseFactor <------------------------------

static ast::c::Expression parseFactor(lexer::LexList& lexList) {
    const lexer::LexItem& currentToken = lexList.consume();

    ast::c::Expression expression = ast::c::Constant(0);

    // Constant
    if (currentToken.mLexType == lexer::LexType::Constant)
        return ast::c::Constant(std::stoi(std::string(currentToken.mSV)));

    // Unary Op
    else if (lexer::is_lextype_unary_op(currentToken.mLexType)) {
        auto factor = std::make_unique<ast::c::Expression>(parseFactor(lexList));
        expression = ast::c::Unary(lextype_to_unary_op(currentToken.mLexType), std::move(factor));
    }

    // Open parenthesis
    else if (currentToken.mLexType == lexer::LexType::Open_Parenthesis) {
        auto innerExpression = parseExpression(lexList);
        expectAndAdvance(lexer::LexType::Close_Parenthesis, lexList);
        expression = std::move(innerExpression);
    }

    // Variable
    else if (currentToken.mLexType == lexer::LexType::Identifier) {
        expression = ast::c::Variable(std::string(currentToken.mSV));
    }

    // Crement
    else if ((currentToken.mLexType == lexer::LexType::Increment)
            || (currentToken.mLexType == lexer::LexType::Decrement)
    ) {
        expression = ast::c::Crement(
            std::make_unique<ast::c::Expression>(parseFactor(lexList)),
            (currentToken.mLexType == lexer::LexType::Increment),
            false // Post always false as the crement operator was found before the factor
        );
    }

    else {
        std::string errorString = std::format("Malformed factor, got: {}", currentToken.mSV);
        throw std::runtime_error(errorString);
    }

    // Check for post increment/decrement
    if (lexList.current().mLexType == lexer::LexType::Increment ||
        lexList.current().mLexType == lexer::LexType::Decrement
    ) {
        expression = ast::c::Crement(
            std::make_unique<ast::c::Expression>(std::move(expression)),
            (lexList.current().mLexType == lexer::LexType::Increment),
            true // Post always true as the crement operator was found after the factor
        );
        lexList.advance();
    }

    return expression;
}

// ------------------------------> parseExpression <------------------------------

static ast::c::Expression parseExpression(lexer::LexList& lexList, uint32_t minPrecedence) {
    auto expression = parseFactor(lexList);
    const lexer::LexItem* currentToken = &lexList.current();

    // Check if operator is a binary op and is above minimum precendence level
    while (lexer::is_lextype_binary_op(currentToken->mLexType)
        && lexer::binary_op_precedence(currentToken->mLexType) >= minPrecedence) {

        lexList.advance();
        
        // Assignment type operator
        if (lexer::is_assignment(currentToken->mLexType)) {
            auto right = std::make_unique<ast::c::Expression>(
                parseExpression(lexList, lexer::binary_op_precedence(lexer::LexType::Assignment)));
            
            // Regular assignment
            if (currentToken->mLexType == lexer::LexType::Assignment)
                expression = ast::c::Assignment(
                    std::make_unique<ast::c::Expression>(std::move(expression)), 
                    std::move(right));
            // Compound assignment, right side is a binary expression
            else {
                auto op = lextype_to_binary_op(currentToken->mLexType);
                ast::c::Expression copiedExpr = std::visit(ast::c::CopyVisitor{}, expression);
                expression = ast::c::Assignment(
                    std::make_unique<ast::c::Expression>(std::move(copiedExpr)),
                    std::make_unique<ast::c::Expression>(
                        ast::c::Binary(
                            op,
                            std::make_unique<ast::c::Expression>(std::move(expression)),  
                            std::move(right))));
            }
        }
        // Conditional expression
        else if (currentToken->mLexType == lexer::LexType::Question_Mark) {
            auto middleExpr = std::make_unique<ast::c::Expression>(parseExpression(lexList));
            expectAndAdvance(lexer::LexType::Colon, lexList);
            auto rightExpr = std::make_unique<ast::c::Expression>(
                parseExpression(lexList, lexer::binary_op_precedence(currentToken->mLexType))
            );
            expression = ast::c::Conditional(
                std::make_unique<ast::c::Expression>(std::move(expression)),
                std::move(middleExpr),
                std::move(rightExpr)
            );
        }
        // Regular binary op
        else {
            auto op = lextype_to_binary_op(currentToken->mLexType);
            auto right = std::make_unique<ast::c::Expression>(parseExpression(lexList, lexer::binary_op_precedence(currentToken->mLexType)+1));
            expression = ast::c::Binary(op, std::make_unique<ast::c::Expression>(std::move(expression)), 
                                      std::move(right));
        }
        currentToken = &lexList.current();
    }

    return expression;
}

// ------------------------------> parseOptionalExpression <------------------------------

static std::optional<ast::c::Expression> parseOptionalExpression(lexer::LexType endingToken, lexer::LexList& lexList) {
    if (lexList.current().mLexType == endingToken) {
        lexList.advance();
        return std::nullopt;
    }

    auto expression = parseExpression(lexList);
    expectAndAdvance(endingToken, lexList);
    return expression;
}

// ------------------------------> parseStatement <------------------------------

// forward declaration
static ast::c::Block parseBlock(lexer::LexList& lexList);

static ast::c::Statement parseStatement(lexer::LexList& lexList) {
    auto currentToken = lexList.current();

    // Return Statement
    if (currentToken.mLexType == lexer::LexType::Return) {
        lexList.advance();
        auto returnObject = ast::c::Return(parseExpression(lexList));
        expectAndAdvance(lexer::LexType::Semicolon, lexList);
        return returnObject;
    }
    // If Statement
    else if (currentToken.mLexType == lexer::LexType::If) {
        std::optional<std::unique_ptr<ast::c::Statement>> elseStmt = std::nullopt;

        lexList.advance();
        expectAndAdvance(lexer::LexType::Open_Parenthesis, lexList);
        auto condition = parseExpression(lexList);
        expectAndAdvance(lexer::LexType::Close_Parenthesis, lexList);
        auto then = std::make_unique<ast::c::Statement>(parseStatement(lexList));

        if (lexList.current().mLexType == lexer::LexType::Else) {
            lexList.advance();
            elseStmt = std::make_unique<ast::c::Statement>(parseStatement(lexList));
        };

        return ast::c::If(
            std::move(condition),
            std::move(then),
            std::move(elseStmt)
        );
    }
    // goto Statement
    else if (currentToken.mLexType == lexer::LexType::Go_To) {
        lexList.advance();
        auto target = expectAndAdvance(lexer::LexType::Identifier, lexList);
        expectAndAdvance(lexer::LexType::Semicolon, lexList);
        return ast::c::GoTo(std::string(target.mSV));
    }
    // Labelled Statement
    else if (currentToken.mLexType == lexer::LexType::Identifier &&
             lexList.next().mLexType == lexer::LexType::Colon
    ) {
        std::string label(currentToken.mSV);
        lexList.advance();
        lexList.advance();
        auto statement = std::make_unique<ast::c::Statement>(parseStatement(lexList));
        return ast::c::LabelledStatement(
            std::move(label),
            std::move(statement)
        );
    }
    // Compound Statment
    else if (currentToken.mLexType == lexer::LexType::Open_Brace) {
        return ast::c::CompoundStatement(std::make_unique<ast::c::Block>(parseBlock(lexList)));
    }
    // Break Statement
    else if (currentToken.mLexType == lexer::LexType::Break) {
        lexList.advance();
        expectAndAdvance(lexer::LexType::Semicolon, lexList);
        return ast::c::Break();
    }
    // Continue statement
    else if (currentToken.mLexType == lexer::LexType::Continue) {
        lexList.advance();
        expectAndAdvance(lexer::LexType::Semicolon, lexList);
        return ast::c::Continue();
    }
    // While Loop
    else if (currentToken.mLexType == lexer::LexType::While) {
        lexList.advance();
        // Get condition expression
        expectAndAdvance(lexer::LexType::Open_Parenthesis, lexList);
        ast::c::Expression condition = parseExpression(lexList);
        expectAndAdvance(lexer::LexType::Close_Parenthesis, lexList);
        // Get statement
        auto body = std::make_unique<ast::c::Statement>(parseStatement(lexList));

        return ast::c::While(std::move(condition), std::move(body));
    }
    // DoWhile loop
    else if (currentToken.mLexType == lexer::LexType::Do) {
        lexList.advance();
        auto body = std::make_unique<ast::c::Statement>(parseStatement(lexList));
        expectAndAdvance(lexer::LexType::While, lexList);
        expectAndAdvance(lexer::LexType::Open_Parenthesis, lexList);
        ast::c::Expression condition = parseExpression(lexList);
        expectAndAdvance(lexer::LexType::Close_Parenthesis, lexList);
        expectAndAdvance(lexer::LexType::Semicolon, lexList);

        return ast::c::DoWhile(std::move(body), std::move(condition));
    }
    // For loop
    else if (currentToken.mLexType == lexer::LexType::For) {
        lexList.advance();
        expectAndAdvance(lexer::LexType::Open_Parenthesis, lexList);

        // For init
        ast::c::ForInit forInit = std::nullopt;

        // check for declaration
        if (lexList.current().mLexType == lexer::LexType::Int)
            forInit = parseDeclaration(lexList);
        else
            forInit = parseOptionalExpression(lexer::LexType::Semicolon, lexList);
        
        auto condition = parseOptionalExpression(lexer::LexType::Semicolon, lexList);

        auto postExpression = parseOptionalExpression(lexer::LexType::Close_Parenthesis, lexList);

        auto statement = std::make_unique<ast::c::Statement>(parseStatement(lexList));

        return ast::c::For(std::move(forInit), std::move(condition), std::move(postExpression), std::move(statement));
    }
    // Switch statement
    else if (currentToken.mLexType == lexer::LexType::Switch) {
        lexList.advance();
        expectAndAdvance(lexer::LexType::Open_Parenthesis, lexList);
        ast::c::Expression selector = parseExpression(lexList);
        expectAndAdvance(lexer::LexType::Close_Parenthesis, lexList);
        auto body = std::make_unique<ast::c::Statement>(parseStatement(lexList));
        
        return ast::c::Switch(std::move(selector), std::move(body));
    }
    // Case statement
    else if (currentToken.mLexType == lexer::LexType::Case) {
        lexList.advance();
        auto condition = parseExpression(lexList);
        expectAndAdvance(lexer::LexType::Colon, lexList);
        auto stmt = std::make_unique<ast::c::Statement>(parseStatement(lexList));
        return ast::c::Case(std::move(condition), std::move(stmt));
    }
    // Default statement
    else if (currentToken.mLexType == lexer::LexType::Default) {
        lexList.advance();
        expectAndAdvance(lexer::LexType::Colon, lexList);
        auto stmt = std::make_unique<ast::c::Statement>(parseStatement(lexList));
        return ast::c::Default(std::move(stmt));
    }
    // Null Statement
    else if (currentToken.mLexType == lexer::LexType::Semicolon) {
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

// ------------------------------> parseBlock <------------------------------

static ast::c::Block parseBlock(lexer::LexList& lexList) {
    // Opening brace
    expectAndAdvance(lexer::LexType::Open_Brace, lexList);

    std::vector<ast::c::BlockItem> blockItems;
    while (lexList.current().mLexType != lexer::LexType::Close_Brace) {
        auto nextBlockItem = parseBlockItem(lexList);
        blockItems.push_back(std::move(nextBlockItem));
    }
    
    // We've seen the closing brace, move forward
    lexList.advance();
    return ast::c::Block(std::move(blockItems));
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

    // parse block
    auto block = parseBlock(lexList);

    return ast::c::Function(std::string(lexIdentifier.mSV), std::move(block));
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
