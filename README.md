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
* Basic Types: `int`, `real`, `char`, `bool`, `sexpr`, `atom`
* Array: `[] int`, `[][][] real`
* Reference/Pointer: `ref int`, `ref [] int`, `ref ref [] int`
* Function/Procedure: `lambda (int, int) void`
* Structure/Record: `cartes (int, int)`
* Structure/Record and selectors: `cartes ((length, int), (width, int))`
* Union: `union (int, real, char)`
* Union and selectors: `union ((i, int), (r, real), (c, char))`

## Statements
* Assignment: `a := 1`, `b := 3.0`
* Conditional: `if a>b then a else b`
* Compound: `prog a:=1; b:=3.0 end`
* Block (Compound with local variables): `prog vars a b c; ... end`
* Return (in Compound or Block): `return <result>`
* Goto: `prog label: a:=1; go label end`
* Function: `fun max a b; if a>b then a else b`
* Type definition: `type vector = [10] int`
* Union case: `case <var> in <sel>: <expr>; <sel>: ... end`

## Declaration
A declaration statement specifies the type of variables and functions.
Without explicit declaration, all variables and functions are assigned
a type of `sexpr`. All types of objects can be converted to s-expressions
and back without losing data.
```
fun max a b;
   int a,b; type int;
prog vars t;
   int t;
   if a>b then t:=a else t:=b;
   return t;
end max
```

## Example
```
type rect = cartes ((length, int), (width, int)),
     circle = cartes ((radius, int)),
     shape = union ((srect, rect), (scircle, circle));

fun areaofrect r;
   length(r) * width(r);
   
fun areaofcircle c;
   radius(c) * radius(c) * 3.14;

fun areaofshape s;
   shape s; type int;
prog
   case s in
   srect: return areaofrect(srect(s))
   scircle: return areaofcircle(scircle(s))
   else error("BAD SHAPE") end
end areaofshape;

lambda; type void;
prog vars c;
   ref circle c = loc circle;
   radius(c) := 10;
   print(areaofshape(c));
end lambda; ();
```

translate to s-expressions:
```
DEFINE ((
((TYPE RECT) (CARTESIAN ((LENGTH INTEGER) (WIDTH INTEGER))))
((TYPE CIRCLE) (CARTESIAN ((RADIUS INTEGER))))
((TYPE SHAPE) (UNION ((SRECT RECT) (SCIRCLE CIRCLE))))
))

DEFINE ((
(AREAOFRECT
   (LAMBDA (R)
      (TIMES (LENGTH R) (WIDTH R))))
(AREAOFCIRCLE
   (LAMBDA (C)
      (TIMES (RADIUS C) (RADIUS C) 3.14)))
(AREAOFSHAPE
   (LAMBDA (S) (SHAPE S) (TYPE INTEGER)
      (PROG ()
         (CASE S
	    (SRECT (RETURN (AREAOFRECT (SRECT S))))
            (SCIRCLE (RETURN (AREAOFCIRCLE (SCIRCLE S))))
            (T (ERROR "BAD SHAPE"))))))
))

(LAMBDA () (TYPE VOID)
   (PROG (C)
      (DEFINE ((REFERENCE CIRCLE) C) (LOCAL CIRCLE))
      (SETQ (RADIUS C) 10)
      (PRINT (AREAOFSHAPE C)))) ()
```


## License
LST is distributed under the GNU GPL version 3 or later.
