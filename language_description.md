## Zephyr Abstract Syntax Description Lanuguage description:
```
program = Program(function_definition)

function_definition = Function(identifier name, statement body)

statement = Return(exp value)
          | If(exp condition, statement then, statement? else)

exp = Constant(int)
    | Unary(unary_operator, exp)
    | Binary(binary_operator, exp, exp)

unary_operator = Complement | Negate

binary_operator = Add | Subtract | Divide | Remainder
                | Left_Shift | Right_Shift 
                | Bitwise_AND | Bitwise_OR | Bitwise_XOR
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

<unop> ::= "-" | "~"

<binop> ::= "-" | "+" | "*" | "/" | "%"
                | "<<" | ">>" 
                | "&" | "|" | "^"

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

val = Constant(int) | Var(identifier)

unary_operator = Complement | Negate

binary_operator = Add | Subtract | Multiply | Divide | Remainder
                | Left_Shift | Right_Shift
                | Bitwise_AND | Bitwise_OR | Bitwise_XOR
```

# Assembly AST ASDL:
```
program = Program(function_definition)

function_definition = Function(identifier name, instruction* instructions)

instruction = Mov(operand src, operand dst) 
            | Ret
            | Unary(unary_operator, operand)
            | Binary(binary_operator, operand, operand)
            | Idiv(operand)
            | Cdq
            | AllocateStack(int)

unary_operator = Neg | Not

binary_operator = Add | Sub | Mult

operand = Imm(int) | Reg(reg) | Pseudo(identifier) | Stack(int)

reg = AX | DX | R10 | R11
```