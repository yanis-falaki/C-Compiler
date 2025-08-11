#include <cxxopts.hpp>
#include <iostream>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include "utils.h"
#include "lexer.hpp"
#include "ast/ast_c.hpp"
#include "parser.hpp"
#include "visitors/asmb_visitors/printing.hpp"
#include "visitors/c_visitors/utils.hpp"
#include "visitors/tacky_visitors/printing.hpp"
#include "visitors/c_visitors/semantic_analysis.hpp"
#include "visitors/c_to_tacky.hpp"
#include "visitors/tacky_to_asmb.hpp"
#include "visitors/asmb_visitors/asmb_to_file.hpp"

namespace fs = std::filesystem;

fs::path preprocess_file(fs::path source_path, fs::path output_path, const cxxopts::ParseResult& args);
fs::path compile(fs::path source_path, fs::path output_path, const cxxopts::ParseResult& args);
void assemble(fs::path source_path, fs::path output_path, const cxxopts::ParseResult& args);

int main(int argc, char* argv[]) {
    cxxopts::Options options("Compiler Driver", "Driver for my C Compiler");
    options.add_options()
        ("s,source", "Source file", cxxopts::value<fs::path>())
        ("o,output", "Output File", cxxopts::value<fs::path>())
        ("P,no-linemarkers", "No linemarkers")
        ("E,preprocess", "Stop at preprocessing")
        ("S,assembly", "Stop at assembly generation")
        ("c", "Build object file and don't invoke linker")
        ("lex", "Stop at lexing")
        ("parse", "Stop at parsing")
        ("validate", "Stop at C AST validation")
        ("tacky", "Stop at tacky AST generation")
        ("codegen", "Stop at assembly generation");

    options.parse_positional({"source"});

    auto args = options.parse(argc, argv);

    fs::path source_path;
    fs::path output_path;

    if (!args.count("source")) {
        std::cerr << "Source file must be specified\n";
        return 1;
    }
    source_path = args["source"].as<fs::path>();

    if (args.count("output")) {
        output_path = args["output"].as<fs::path>();
    } else {
        output_path = source_path;
        output_path.replace_extension();  // removes .c or whatever is there
    }

    // Preprocessing Stage
    fs::path preprocessed_path;
    try {
        preprocessed_path = preprocess_file(source_path, output_path, args);
    } catch (const std::exception& e) {
        std::cerr << "Preprocessing failed: " << e.what() << std::endl;
        return 1;
    }

    if (args.count("preprocess"))
        return 0;

    // Compilation Stage
    fs::path compiled_path;
    try {
        compiled_path = compile(preprocessed_path, output_path, args);
    } catch(const std::exception& e) {
        std::cerr << "Compilation failed: " << e.what() << std::endl;
        fs::remove(preprocessed_path);
        return 1;
    }

    // Cleanup preprocessed file as it's no longer needed
    fs::remove(preprocessed_path);

    // Stop if -S or --assembly flag is set or incomplete compilation
    if (compiled_path.empty() || args.count("assembly"))
        return 0;

    // Assembling Stage
    try {
        assemble(compiled_path, output_path, args);
    } catch (const std::exception& e) {
        std::cerr << "Assembly failed: " << e.what() << std::endl;
        return 1;
    }

    // Cleanup assembly file as it's no longer needed
    fs::remove(compiled_path);

    return 0;
}


fs::path preprocess_file(fs::path source_path, fs::path output_path, const cxxopts::ParseResult& args) {
    std::string dest_path = std::format("{}.i", output_path.string());

    std::string command;
    if (args.count("no-linemarkers")) {
        command = std::format("gcc -E -P {} -o {}", source_path.string(), dest_path);
    } else {
        command = std::format("gcc -E -P {} -o {}", source_path.string(), dest_path);
    }

    if(system(command.c_str())) {
        throw std::runtime_error("Sys command error");
    }

    return fs::path(dest_path);
}


fs::path compile(fs::path source_path, fs::path output_path, const cxxopts::ParseResult& args) {
    std::string sourceString = Utils::readFile(source_path);

    
    auto lexList = compiler::lexer::lexer(sourceString);
    if (args.count("lex")) {
        lexList.print();
        return fs::path();
    } 

    auto program = compiler::parser::parseProgram(lexList);
    if (args.count("parse")) {
        compiler::ast::c::PrintVisitor()(program);
        return fs::path();
    }

    compiler::ast::SymbolMapType symbolMap;

    // Validate C AST
    compiler::ast::c::IdentifierResolution()(program);
    (compiler::ast::c::TypeChecking(symbolMap))(program);
    compiler::ast::c::ControlFlowLabelling()(program);
    compiler::ast::c::LabelResolution()(program);
    if (args.count("validate")) {
        compiler::ast::c::PrintVisitor()(program);
        return fs::path();
    }

    // Convert C to TACKY
    auto tackyProgram = compiler::codegen::CToTacky()(program);
    if (args.count("tacky")) {
        compiler::ast::tacky::PrintVisitor()(tackyProgram);
        return fs::path();
    }

    // 0th pass, asmb tree creation
    compiler::ast::asmb::Program asmb = compiler::codegen::TackyToAsmb()(tackyProgram);
    // 1st pass, removing pseudo-registers
    compiler::codegen::ReplacePseudoRegisters()(asmb, symbolMap);
    // 2nd pass, allocating stack memory and fixing memory-to-memory mov instructions
    compiler::codegen::FixUpAsmbInstructions()(asmb, symbolMap);

    if (args.count("codegen")) {
        compiler::ast::asmb::PrintVisitor()(asmb);;
        return fs::path();
    }

    std::string dest_path = std::format("{}.s", output_path.string());

    // Write assembly to file
    std::ofstream out(dest_path);
    out << (compiler::codegen::EmitAsmbVisitor(symbolMap))(asmb);
    out.close();

    return fs::path(dest_path);
}


void assemble(fs::path source_path, fs::path output_path, const cxxopts::ParseResult& args) {
    std::string command;
    if (args.count("c"))
        command = std::format("gcc -c {} -o {}", source_path.string(), output_path.string() + ".o");
    else
        command = std::format("gcc {} -o {}", source_path.string(), output_path.string());
    if(system(command.c_str())) {
        throw std::runtime_error("Sys command error");
    }
}