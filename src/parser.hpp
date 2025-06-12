#pragma once
#include <memory>
#include "ast/ast_c.hpp"
#include "lexer.hpp"

namespace compiler::parser {
    ast::c::Program parseProgram(lexer::LexList& lexList);
}