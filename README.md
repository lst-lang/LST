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
* Static Array: `[3] int`, `[3][4][5] real`
* Dynamic Array: `[] char`
* Reference/Pointer: `ref int`, `ref [3] real`, `ref [] char`
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
* Declaration: `decl <type> <var>,<var>...; <type> ... end`
* Type definition: `type string = [] char`

## Declaration
A declaration statement specifies the type of variables and functions.
Without explicit declaration, all variables and functions are assigned
a type of `sexpr`. All types of objects can be converted to s-expressions
and back without losing data.
```
fun max a b;
   decl int a,b end; type int;
prog vars temp;
   decl int temp end;
   if a>b then temp:=a else temp:=b;
   return temp;
end max
```

## Example
```
type rect = cartes ((length, int), (width, int));
type circle = cartes ((radius, int));
type shape = union ((srect, rect), (scircle, circle));

fun areaofrect r;
   length(r) * width(r);
   
fun areaofcircle c;
   radius(c) * radius(c) * 3.14;

fun areaofshape s;
   decl shape s end; type int;
prog
   if type(s)=1 then
      return areaofrect(srect(s))
   else if type(s)=2 then
      return areaofcircle(scircle(s))
   else error("BAD SHAPE")
end areaofshape;

fun main;
   type void;
prog
   vars shapes c;
   decl [] ref shape shapes; circle c end;
   shapes := makearray(1, ref shape);
   shapes[0] := make(shape);
   radius(c) := 10;
   scricle(deref(shapes[0])) := c;
   print(areaofshape(deref(shapes[0])))
end main
```

translate to s-expressions:
```
DEFLIST ((
(RECT (CARTESIAN ((LENGTH INTEGER) (WIDTH INTEGER))))
(CIRCLE (CARTESIAN ((RADIUS INTEGER))))
(SHAPE (UNION ((SRECT RECT) (SCIRCLE CIRCLE))))
) TEXPR)

DEFINE ((
(AREAOFRECT
   (LAMBDA (R)
      (TIMES (LENGTH R) (WIDTH R))))
(AREAOFCIRCLE
   (LAMBDA (C)
      (TIMES (RADIUS C) (RADIUS C) 3.14)))
(AREAOFSHAPE
   (LAMBDA (S) (DECLARE (SHAPE S)) (TYPE INTEGER)
      (PROG ()
         (COND ((EQUAL (TYPE S) 1)
	        (RETURN (AREAOFRECT (SRECT S))))
               ((EQUAL (TYPE S) 2)
	        (RETURN (AREAOFCIRCLE (SCIRCLE S))))
               (T (ERROR "BAD SHAPE"))))))
(MAIN
   (LAMBDA () (TYPE VOID)
      (PROG (SHAPES C)
         (DECLARE ((ARRAY () (REFERENCE SHAPE)) SHAPES)
	           (CIRCLE C))
         (SETQ SHAPES (MAKEARRAY 1 (REFERENCE SHAPE)))
         (SETQ (SHAPES 0) (MAKE SHAPE))
         (SETQ (RADIUS C) 10)
         (SETQ (SCIRCLE (DEREFERENCE (SHAPES 0))) C)
         (PRINT (AREAOFSHAPE (DEREFERENCE (SHAPES 0)))))))
))
```


## License
LST is distributed under the GNU GPL version 3 or later.
