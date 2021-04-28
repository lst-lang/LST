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
* Basic Types: `INTEGER`, `REAL`, `STRING`, `LOGICAL`, `ATOM`, `SYMBOL`
* Array: `INTEGER ARRAY`, `ARRAY`, `STRING ARRAY`
* Reference: `INTEGER REFERENCE`, `REFERENCE`
* Functional: `INTEGER ARRAY[1:10] FUNCTIONAL(REAL)`
* Structure: `STRUCTURE (INTEGER l, w)`
* Union: `UNION (INTEGER n1, n2; REAL r1)`


## Statements
* Assignment: `a := 1`, `b := 3.0`
* Conditional: `IF a > b THEN a ELSE b`
* Compound Statements: `BEGIN a := 1; b := 3.0 END`
* Block: `BEGIN INTEGER a, b, c; ... END`
* Return: `RETURN` or `RETURN <result>`
* Goto: `BEGIN label: a := 1; GO TO label END` or `GO label`
* Procedure: `INTEGER PROCEDURE max(a, b) := IF a > b THEN a ELSE b`
* Type definition: `DEFINE TYPE COMPLEX = STRUCTURE (REAL a, b)`
* Cast: `INTEGER PROCEDURE() (refproc)`
* Comment: `COMMENT <any sequence not containing ;>;`


## Declaration
Complete specifications are required in procedure declarations if the
item is other than a simple variable of the same type as the procedure.
```
INTEGER PROCEDURE step(REAL u)
   := IF 0 <= u AND u <= 1 THEN 1 ELSE 0
```

Translate to s-expressions:
```
('INTEGER' 'PROCEDURE' STEP (('REAL' U))
   ('IF' (AND ('LESSEQUAL' 0 U) ('GREATEREQUAL' U 1))
      'THEN' 1 'ELSE' 0))
```


## Functional
A functional can be used to set a `FUNCTIONAL` variable, or as a `FUNCTIONAL` argument of a function. The syntax to create a functional is `<type> FUNCTION <parameters> <;|> (<expression>; <functional-arguments>)`, where the `<functional-arguments>` are variables used in the `<expression>` which must be evaluated at the time the functional argument is set up. 
```
SYMBOL a;

SYMBOL PROCEDURE mapcar(l, f);
   SYMBOL FUNCTIONAL f(SYMBOL);
   IF null(l) THEN nil
      ELSE cons(f(car(l)), mapcar(cdr(l), f));

SYMBOL PROCEDURE mapcons(l, a);
   mapcar(l, SYMBOL FUNCTION(k); (cons(k, a); a))

a := 'y;
mapcons('(1 2), 'x);
```

Here, the functional argument uses the value of `a` bound in `mapcons`, so the result is `'((1 . x) (2 . x))`. Translate to s-expressions:
```
('SYMBOL' A)

('SYMBOL' 'PROCEDURE' MAPCAR (L F)
   ('SYMBOL' 'FUNCTIONAL' F (SYMBOL))
   ('IF' ('NULL' L) 'THEN' NIL
      'ELSE' ('CONS' (F ('CAR' L)) (MAPCAR ('CDR' L) F))))
   
('SYMBOL' 'PROCEDURE' MAPCONS (L A)
   (MAPCAR L ('SYMBOL' 'FUNCTION' (K) (CONS K A) (A))))

('SETQ' A ('QUOTE' Y))
(MAPCONS ('QUOTE' (1 2)) ('QUOTE' X))
```


## Example
```
DEFINE TYPE 'GET AREA' = REAL FUNCTIONAL(ANY SHAPE REFERENCE);
DEFINE TYPE SHAPE = STRUCTURE ('GET AREA' get area)
DEFINE TYPE RECT = SHAPE STRUCTURE (INTEGER width, length);
DEFINE TYPE CIRCLE = SHAPE STRUCTURE (INTEGER radius);

REAL PROCEDURE area of rect(s);
   ANY SHAPE REFERENCE s;
BEGIN RECT REFERENCE r;
   r := s;
   RETURN length OF r * width OF r
END area of rect;

REAL PROCEDURE area of circle(s);
   ANY SHAPE REFERENCE s;
BEGIN INTEGER r;
   r := radius OF CIRCLE REFERENCE (s);
   RETURN r * r * 3.14
END area of circle;

REAL PROCEDURE area of shape(ANY SHAPE REFERENCE s)
   := (get area OF s)(s);

PROCEDURE;
BEGIN
   RECT r; CIRCLE c;
   r := <RECT <PROCEDURE area of rect>, 3, 3>;
   c := <CIRCLE <PROCEDURE area of circle>, 1.5>;
   out integer(max(area of shape(r), area of shape(c)))
END ()
```

Translate to s-expressions:
```
('DEFINE' 'TYPE' 'GETAREA'
   ('REAL' 'FUNCTIONAL' ('ANY' 'SHAPE' 'REFERENCE')))
('DEFINE' 'TYPE' 'SHAPE'
   ('STRUCTURE' ('GETAREA' GETAREA)))
('DEFINE' 'TYPE' 'RECT'
   ('SHAPE' 'STRUCTURE' (('INTEGER' WIDTH LENGTH))))
('DEFINE' 'TYPE' 'CIRCLE'
   ('SHAPE' 'STRUCTURE' (('INTEGER' RADIUS))))

('REAL' 'PROCEDURE' AREAOFRECT (S)
      ('ANY' 'SHAPE' 'REFERENCE' S)
   ('BEGIN' ('RECT' 'REFERENCE' R)
      ('SETQ' R S)
      ('RETURN' ('TIMES' ('OF' LENGTH R) ('OF' WIDTH R)))))

('REAL' 'PROCEDURE' AREAOFCIRCLE (S)
      ('ANY' 'SHAPE' 'REFERENCE' S)
   ('BEGIN' ('INTEGER' R)
      ('SETQ' R ('OF' RADIUS ('CAST' ('CIRCLE' 'REFERENCE') S)))
      ('RETURN' ('TIMES' R R 3.14))))

('REAL' 'PROCECURE' AREAOFSHAPE (('ANY' 'SHAPE' 'REFERENCE' S))
   (('OF' GETAREA S) S))

(('PROCEDURE'
   ('BEGIN' ('RECT' R) ('CIRCLE' C)
      ('SETQ' R <'RECT' <'PROCEDURE' AREAOFRECT> 3 3>)
      ('SETQ' C <'CIRCLE' <'PROCEDURE' AREAOFCIRCLE> 1.5>)
      (OUTINTEGER (MAX (AREAOFSHAPE R) (AREAOFSHAPE C))))))
```



# License
LST is distributed under the GNU GPL version 3 or later.
