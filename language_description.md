## Zephyr Abstract Syntax Description Lanuguage description:
```
program = Program(function_definition)

function_definition = Function(identifier name, statement body)

statement = Return(exp value)
          | If(exp condition, statement then, statement? else)

exp = Constant(int)
```

## Formal Grammar (Extended Backus-Naur Form)
```
<program> ::= <function>

<function> ::= "int" <identifier> "(" "void" ")" "{" <statement> "}"

<statement> ::= <return> | <if>

<return> ::= "return" <exp> ";"

<if> ::= "if" "(" <exp> ")" <statement> [ "else" <statement> ]

<exp> ::= <constant>

<identifier> ::= ? An identifier token ?

<constant> ::= ? A constant token ?
```

# Assembly AST ASDL:
```
program = Program(function_definition)

function_definition = Function(identifier name, instruction* instructions)

instruction = Mov(operand src, operand dst) | Ret

operand = Imm(int) | Register
```