## Zephyr Abstract Syntax Description Lanuguage description:
```
program = Program(function_definition)

function_definition = Function(identifier name, statement body)

statement = Return(exp value)
          | If(exp condition, statement then, statement? else)

exp = Constant(int)
    | Unary(unary_operator, exp)
    | Binary(binary_operator, exp, exp)

unary_operator = Complement | Negate | Logical_Not

binary_operator = Add | Subtract | Divide | Remainder
                | Left_Shift | Right_Shift 
                | Bitwise_AND | Bitwise_OR | Bitwise_XOR
                | Logical_AND | Logical_OR | Equal_to | Not_Equal
                | Less_Than | Greater_Than | Less_Or_Equal | Greater_Or_Equal
```

## Formal Grammar (Extended Backus-Naur Form)
```
<program> ::= <function>

<function> ::= "int" <identifier> "(" "void" ")" "{" <statement> "}"

<statement> ::= <return> | <if>

<return> ::= "return" <exp> ";"

<if> ::= "if" "(" <exp> ")" <statement> [ "else" <statement> ]

<exp> ::= <factor> | <exp> <binop> <exp>

<factor> ::= <int> | <unop> <factor> | "(" <exp> ")"

<unop> ::= "-" | "~" | "!"

<binop> ::= "-" | "+" | "*" | "/" | "%"
                | "<<" | ">>" 
                | "&" | "|" | "^"
                | "&&" | "||" | "==" | "!=" | "<" | ">| | "<=" | ">=

<identifier> ::= ? An identifier token ?

<constant> ::= ? A constant token ?
```

# Tacky IR ASDL
```
program = Program(function_definition)

function_definition = Function(identifier, instruction* body)

instruction = Return(val)
            | Unary(unary_operator, val src, val dst)
            | Binary(binary_operator, val src1, val src2, val dst)
            | Copy(val src, val dst)
            | Jump(identifier target)
            | JumpIfZero(val condition, identifier target)
            | JumpIfNotZero(val condition, identifier target)
            | Label(identifier)

val = Constant(int) | Var(identifier)

unary_operator = Complement | Negate | Not

binary_operator = Add | Subtract | Multiply | Divide | Remainder
                | Left_Shift | Right_Shift
                | Bitwise_AND | Bitwise_OR | Bitwise_XOR
                | Is_Equal | Not_Equal
                | Less_Than | Greater_Than | Less_Or_Equal | Greater_Or_Equal
```

# Assembly AST ASDL:
```
program = Program(function_definition)

function_definition = Function(identifier name, instruction* instructions)

instruction = Mov(operand src, operand dst) 
            | Ret
            | Unary(unary_operator, operand)
            | Binary(binary_operator, operand, operand)
            | Cmp(operand, operand)
            | Idiv(operand)
            | Cdq
            | Jmp(identifier)
            | JmpCC(cond_code, identifier)
            | SetCC(cond_code, operand)
            | Label(identifier)
            | AllocateStack(int)

unary_operator = Neg | Not

binary_operator = Add | Sub | Mult

operand = Imm(int) | Reg(reg) | Pseudo(identifier) | Stack(int)

cond_code = E | NE | G | GE | L | LE

reg = AX | DX | R10 | R11
```