#include <ctre.hpp>
#include <stdexcept>
#include <vector>
#include <cxxopts.hpp>
#include "./lexer.hpp"
#include "./utils.h"

namespace compiler::lexer {

// ------------------------------> checkForKeyword <------------------------------

static LexType checkForKeyword(std::string_view sv) {
    constexpr size_t len = std::size(KEYWORD_MAP);
    for (size_t i = 0; i < len; ++i) {
        auto [str, lexType] = KEYWORD_MAP[i];
        if (sv == str)
            return lexType;
    }
    // Fall back to identifier if not keyword
    return LexType::Identifier;
}

// ------------------------------> checkForType <------------------------------

static std::pair<LexType, std::string_view> checkForType(std::string_view sv) {
    static constexpr auto pattern = ctll::fixed_string{ R"((?<Identifier>[a-zA-Z_]\w*\b)|(?<Constant>[0-9]+\b))" };
    auto m = ctre::starts_with<pattern>(sv);
    if (m) {
        if (m.get<"Identifier">()) {
            return std::make_pair(LexType::Identifier, m);
        } else {
            return std::make_pair(LexType::Constant, m);
        }
    }

    // Check all other token types by iterating through them
    
    for (LexType type : LEX_TYPES_TO_CHECK) {
        std::string_view tokenStr = lex_type_to_str(type);
        if (sv.starts_with(tokenStr)) {
            return std::make_pair(type, tokenStr);
        }
    }
    
    return std::make_pair(LexType::Undefined, lex_type_to_str(LexType::Undefined));
}

// ------------------------------> lexer <------------------------------

/**
 * @brief Extracts a set of tokens from a string containing C source code.
 * 
 * @param preprocessed_input 
 */
LexList lexer(std::string_view preprocessed_input) {
    LexList lexList;

    // while not at end
    size_t pos = 0;
    size_t len = preprocessed_input.size();
    while (pos < len) {
        // remove leading whitespace
        if (std::isspace(preprocessed_input[pos])) {
            ++pos;
            continue;
        }

        // Check for lexType
        auto [lexType, token_sv] = checkForType(preprocessed_input.substr(pos, len-pos));

        // if no match is found, raise an error
        if (lexType == LexType::Undefined) {
            auto output_string = Utils::stringCenteredOnPos(preprocessed_input, pos, 30);
            throw std::runtime_error("Could not parse token at position " + std::to_string(pos)
                + "\nNearby text:\n" + output_string);
        }
        else if (lexType == LexType::Identifier) {
            LexType keyword = checkForKeyword(token_sv);
            lexList.append(keyword, token_sv);
        }
        else lexList.append(lexType, token_sv);

        pos += token_sv.size();
    }

    return lexList;
}

}