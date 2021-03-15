[![Actions Status](https://github.com/lst-lang/LST/workflows/GNU//Linux/badge.svg)](https://github.com/lst-lang/LST/actions?query=workflow%3AGNU%2FLinux)
[![Actions Status](https://github.com/lst-lang/LST/workflows/Windows/badge.svg)](https://github.com/lst-lang/LST/actions?query=workflow%3AWindows)
[![Actions Status](https://github.com/lst-lang/LST/workflows/macOS/badge.svg)](https://github.com/lst-lang/LST/actions?query=workflow%3AmacOS)



# LST
Lisp with a Simple Type system, written in FORTH.



# Implementation
The implementation would consist of four basic components:
* a FORTH virtual machine and a compact garbage collector;
* a compiler that compiles s-expressions into FORTH virtual machine bytecodes;
* an interpreter that accepts s-expressions as input and executes the code directly;
* a translater that translates Algol-like statements to s-expresssions.



# Language
## Data Types
* Basic Types: `INT`, `REAL`, `CHAR`, `BOOL`
* Array: `[0:10] INT`, `[10,10] INT`, `FLEX [] CHAR`
* Reference/Pointer: `REF INT`, `REF REF INT`, `REF ANY`
* Functional/Procedural: `FORMAL (OF INT, OF INT) VOID`
* Structure/Record: `STRUCT (length OF INT, width OF INT)`
* Union: `UNION (n1 OF INT, n2 OF INT, n3 OF INT)`


## Statements
* Assignment: `a := 1`, `b := 3.0`
* Conditional: `IF a > b THEN a ELSE b`
* Compound: `PROG; a := 1; b := 3.0 END`
* Block (Compound with local variables): `PROG a b c; ... END`
* Return (in Compound or Block): `RETURN <result>`
* Goto: `PROG; label: a := 1; GO label END`
* Function: `FUN max a b; IF a > b THEN a ELSE b`
* Type definition: `TYPE STRING = FLEX [] CHAR`
* Cast: `REF INT (refany)`


## Declaration
A declaration statement specifies the type of variables and functions.
Without explicit declaration, all variables and functions are assigned
a type of `SEXPR`. All types of objects can be converted to s-expressions
and back without losing data.
```
FUN max a b;
   INT a b; TYPE INT;
   IF a > b THEN a ELSE b
```

Translate to s-expressions:
```
'DEFINE' ((
(('FUNCTION' MAX)
   ('LAMBDA' (A B)
      ('INTEGER' (A B)) ('TYPE' 'INTEGER')
      ('IF' ('GREATERP' A B) 'THEN' A 'ELSE' B)))
))
```


## Functional
A functional can be used to set a `FORMAL` variable, or as a `FORMAL` argument of a function. The syntax to create a functional is `FUN <parameters>; (<expression>; <functional-arguments>)`, where the `<functional-arguments>` are variables used in the `<expression>` which must be evaluated at the time the functional argument is set up. 
```
SPECIAL a;

FUN mapcar l f;
   IF NULL l THEN nil
      ELSE f(CAR l) . mapcar(CDR l, f);

FUN mapcons l a;
   mapcar(l, FUN k; (k . a; a));

LAMBDA a; mapcons('(1 2), 'x); (y)
```

Here, the functional argument uses the value of `a` bound in `mapcons`, so the result is `'((1 . x) (2 . x))`. Translate to s-expressions:
```
'SPECIAL' ((A))

'DEFINE' ((
(('FUNCTION' MAPCAR)
   ('LAMBDA' (L F)
      ('IF' ('NULL' L) 'THEN' NIL
         'ELSE' ('CONS' (F ('CAR' L)) (MAPCAR ('CDR' L) F)))))
(('FUNCTION' MAPCONS)
   ('LAMBDA' (L A)
      (MAPCAR L ('FUNCTION' (K) ('CONS' K A) (A)))))
))

('LAMBDA' (A) (MAPCONS ('QUOTE' (1 2)) ('QUOTE' X))) (Y)
```


## Example
```
TYPE RECT = STRUCT (width OF INT, length OF INT);
TYPE CIRCLE = STRUCT (radius OF INT);
TYPE SHAPE = UNION (rectshape OF RECT, circleshape OF CIRCLE);

FUN areaofrect r;
   length OF r * width OF r;
   
FUN areaofcircle c;
   radius OF c * radius OF c * 3.14;

FUN areaofshape s;
   REF SHAPE s; TYPE INT;
PROG;
   IF NULL s THEN ERROR "BAD SHAPE";
   RETURN SELECT TYPE OF s CASE
      1: areaofrect(rectshape OF s)
      2: areaofcircle(s) ELSE ERROR "BAD SHAPE"
END areaofshape;

FUN largest ss;
   FLEX [0:] SHAPE ss; TYPE INT;
PROG i u m;
   INT i u m;
   u := UPB ss; m := 0;
   FOR i := 0 STEP 1 UNTIL u DO
      IF m < areaofshape(ss[i])
         THEN m := areaofshape(ss[i]);
   RETURN m
END largest;

LAMBDA;
   TYPE VOID;
PROG ss;
   REF [0:3] SHAPE ss;
   ss := HEAP [0:3] SHAPE; 
   ss[0] := RECT ('(9 9));
   ss[1] := RECT ('(10 8));
   ss[2] := CIRCLE ('(10));
   PRINT largest(ss)
END; ()
```

Translate to s-expressions:
```
'DEFINE' ((
(('TYPE' 'RECT') ('STRUCTURE' ((LENGTH ('INTEGER')) (WIDTH ('INTEGER')))))
(('TYPE' 'CIRCLE') ('STRUCTURE' ((RADIUS ('INTEGER')))))
(('TYPE' 'SHAPE') ('UNION' ((RECTSHAPE ('RECT')) (CIRCLESHAPE ('CIRCLE')))))
))

'DEFINE' ((
(('FUNCTION' AREAOFRECT)
   ('LAMBDA' (R)
      ('TIMES' ('OF' LENGTH R) ('OF' WIDTH R))))
(('FUNCTION' AREAOFCIRCLE)
   ('LAMBDA' (C)
      ('TIMES' ('OF' RADIUS C) ('OF' RADIUS C) 3.14)))
(('FUNCTION' AREAOFSHAPE)
   ('LAMBDA' (S)
         ('SHAPE' (S)) ('TYPE' 'INTEGER')
      ('PROG' ()
         ('IF' ('NULL' S) 'THEN' ('ERROR' "BAD SHAPE"))
         ('RETURN' ('SELECT' ('OF' 'TYPE' S)
            (1 (AREAOFRECT ('OF' RECTSHAPE S)))
            (2 (AREAOFCIRCLE S)) ('ERROR' "BAD SHAPE"))))))
(('FUNCTION' LARGEST)
   ('LAMBDA' (SS N)
         ('FLEXIBLE' ((0)) 'SHAPE' (SS)) ('TYPE' 'INTEGER')
      ('PROG' (I U M)
         ('INTEGER' (I U M))
	 ('SETQ' U ('UPPERBOUND' SS)) ('SETQ' M 0)
         ('FOR' I 0 1 U
	    ('IF' ('LESSP' M (AREAOFSHAPE (SS I)))
	        'THEN' ('SETQ' M (AREAOFSHAPE (SS I)))))
	 ('RETURN' M))))
))

('LAMBDA' ()
      ('TYPE' 'VOID')
   ('PROG' (SS)
      ('REFERENCE' 'ARRAY' ((0 3)) 'SHAPE' (SS))
      ('SETQ' SS ('HEAP' ('ARRAY' ((0 3)) 'SHAPE')))
      ('SETQ' (SS 0) ('CAST' ('RECT') (QUOTE (9 9))))
      ('SETQ' (SS 1) ('CAST' ('RECT') (QUOTE (10 8))))
      ('SETQ' (SS 2) ('CAST' ('CIRCLE') (QUOTE (10))))
      ('PRINT' (LARGEST SS)))) ()
```



# License
LST is distributed under the GNU GPL version 3 or later.
