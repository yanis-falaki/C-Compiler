cmake_minimum_required(VERSION 3.10)
project(c_compiler)

set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_SOURCE_DIR}/include/)

add_executable(compiler src/compiler_driver.cpp src/lexer.cpp src/parser.cpp)
