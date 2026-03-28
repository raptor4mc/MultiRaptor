# MagPhos Grammar Reference (Current)

```ebnf
program        = { declaration } EOF ;

declaration    = importDecl | useDecl | functionDecl | statement ;
importDecl     = "import" ident { "." ident } terminator ;
useDecl        = "use" string terminator ;

functionDecl   = "fn" ident "(" [ params ] ")" block ;
params         = ident { "," ident } ;

statement      = ifStmt
               | whenStmt
               | whileStmt
               | repeatWhileStmt
               | loopStmt
               | forStmt
               | askStmt
               | setStmt
               | printStmt
               | returnStmt
               | assignmentOrExpr
               | block ;

ifStmt         = "if" expression block [ "else" block ] ;
whenStmt       = "when" expression block ;
whileStmt      = "while" expression block ;
repeatWhileStmt = "repeat" "while" expression block ;
loopStmt       = "loop" expression block ;
forStmt        = "for" "(" [ forInitializer ] ";" [ expression ] ";" [ expression ] ")" block ;
forInitializer = varDeclNoTerminator | assignmentOrExprNoTerminator ;
varDeclNoTerminator = ("var" | "const") ident "=" expression ;
assignmentOrExprNoTerminator = ident "=" expression | expression ;

askStmt        = "ask" expression "->" ident terminator ;
setStmt        = "set" ident "=" expression terminator ;
printStmt      = "print" expression terminator ;
returnStmt     = "return" expression terminator ;
assignmentOrExpr = ident "=" expression terminator
                 | expression terminator ;

block          = "{" { declaration } "}" ;

expression     = logicalOr ;
logicalOr      = logicalAnd { "or" logicalAnd } ;
logicalAnd     = equality { "and" equality } ;
equality       = comparison { ("==" | "!=") comparison } ;
comparison     = addition { ("<" | "<=" | ">" | ">=") addition } ;
addition       = multiplication { ("+" | "-") multiplication } ;
multiplication = unary { ("*" | "/") unary } ;
unary          = ["-" | "!" | "not"] call ;
call           = primary { "(" [ arguments ] ")" } ;
arguments      = expression { "," expression } ;
primary        = number | string | "true" | "false" | "null" | ident | "(" expression ")" | arrayLiteral ;
arrayLiteral   = "[" [ expression { "," expression } ] "]" ;

terminator     = ";" | NEWLINE ;
ident          = IDENTIFIER ;
string         = STRING ;
number         = NUMBER ;
```

> Note: this grammar reflects currently implemented parser behavior and will evolve.
