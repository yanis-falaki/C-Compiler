cmake_minimum_required(VERSION 3.10)
project(c_compiler)

set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_SOURCE_DIR}/include/)

add_executable(compiler src/compiler_driver.cpp src/lexer.cpp src/parser.cpp)

# ----------------------------------------------------------------------
# tests

# Build lexer only
add_library(lexer_test OBJECT EXCLUDE_FROM_ALL src/lexer.cpp)
target_compile_options(lexer_test PRIVATE -g)

# Build parser only  
add_library(parser_test OBJECT EXCLUDE_FROM_ALL src/parser.cpp src/lexer.cpp)
target_compile_options(parser_test PRIVATE -g)