# MagPhos Grammar Reference (Current)

```ebnf
program        = { declaration } EOF ;

declaration    = importDecl | useDecl | functionDecl | statement ;
importDecl     = "import" ident { "." ident } terminator ;
useDecl        = "use" string terminator ;

functionDecl   = "fn" ident "(" [ params ] ")" block ;
params         = ident { "," ident } ;

statement      = printStmt
               | returnStmt
               | assignmentOrExpr
               | block ;

printStmt      = "print" expression terminator ;
returnStmt     = "return" expression terminator ;
assignmentOrExpr = ident "=" expression terminator
                 | expression terminator ;

block          = "{" { declaration } "}" ;

expression     = addition ;
addition       = multiplication { ("+" | "-") multiplication } ;
multiplication = unary { ("*" | "/") unary } ;
unary          = ["-"] call ;
call           = primary { "(" [ arguments ] ")" } ;
arguments      = expression { "," expression } ;
primary        = number | string | ident | "(" expression ")" ;

terminator     = ";" | NEWLINE ;
ident          = IDENTIFIER ;
string         = STRING ;
number         = NUMBER ;
```

> Note: this grammar reflects currently implemented parser behavior and will evolve.
