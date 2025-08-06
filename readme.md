# A C Compiler Written in C++
This is a minimal C Compiler I'm writing to have a better understanding of what goes on under the hood after I compile my code. Once I'm done writing this compiler I'll write my own assembler and linker too, but for now I'll use GCC's.

## Build Steps
1. Open a terminal in the project root.
2. Run the following commands
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Using The Compiler
After building the compiler, you can use it from the command line as follows:

```bash
./compiler -s path/to/source.c -o path/to/output
```

### Command Line Options
| Flag                     | Description                                                                       |
| ------------------------ | --------------------------------------------------------------------------------- |
| `-s`, `--source`         | Path to the source C file to compile **(required)**                               |
| `-o`, `--output`         | Output file name/path. If omitted, defaults to source file name without extension |
| `-P`, `--no-linemarkers` | Disable linemarkers during preprocessing                                          |
| `-E`, `--preprocess`     | Stop after preprocessing stage (outputs `.i` file)                                |
| `-S`, `--assembly`       | Stop after assembly generation (outputs `.s` file)                                |
| `-c`                     | Build object file and don't invoke linker (outputs `.o` file)                     |
| `--lex`                  | Stop after lexing and print tokens                                                |
| `--parse`                | Stop after parsing and print the AST                                              |
| `--validate`             | Validate and print the C AST after semantic analysis                              |
| `--tacky`                | Stop after generating the intermediate TACKY AST                                  |
| `--codegen`              | Stop after generating assembly, print assembly code                               |

## TODO
There's still quite a lot to do, below is my todo list:

### The Fundamentals
- [x] Write compiler driver
- [x] Inital lexer
- [x] Initial parser
    - [x] Abstract Syntax Tree Representation
- [x] Assembly Backend IR
- [x] Code emission

### The Basics
- [x] New Middle-End Internal Representation (TACKY)
- [x] Unary Operators
- [x] Binary Operators
- [x] Logical and Relational Operators
- [x] Local Variables
- [x] If Statements and Conditional Expressions
- [x] Compound Statements
- [x] Loops
- [x] Switch statements
- [ ] Functions (Aside from main)
- [ ] File Scope Variable Declarations and Storage-Class Specifiers

### Types Beyond Int
- [ ] Long Integers
- [ ] Unsigned Integers
- [ ] Floating-Point Numbers
- [ ] Pointers
- [ ] Arrays and Pointer Arithmetic
- [ ] Characters and Strings
- [ ] Supporting Dynamic Memory Allocation
- [ ] Structures

### Optimizations
- [ ] Tacky optimization passes
    - [ ] Constant Folding
    - [ ] Unreachable Code Elimination
    - [ ] Copy Propagation
    - [ ] Dead Store Elimination
- [ ] Register Allocation