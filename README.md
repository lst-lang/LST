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

## Statements
* Assignment: `a := 1`, `b := 3.0`
* Conditional: `if a>b then a else b`
* Compound: `prog a:=1; b:=3.0 end`
* Block (Compound with local variables): `prog vars a b c; ... end`
* Return (in Compound or Block): `return <result>`
* Goto: `prog label: a:=1; go label end`
* Function: `fun max a b; if a>b then a else b`
* Declaration: `decl <type> <var>,<var>...; <type> ... end`

## Data Definition:
* Structure/Record with two members: `data rect = length * width`
* Structure/Record with a single member: `data circle = * radius`
* Union: `data shape = srect | scircle`
* Function/Procedure: `data filter = lambda x`

## Declaration
A declaration statement specifies the type of variables and functions.
Without explicit declaration, all variables and functions are assigned
a type of `sexpr`. All types of objects can be converted to s-expressions
and back without losing data.
```
fun max a b;
   decl int a,b,max end;
prog vars temp;
   decl int temp end;
   if a>b then temp:=a else temp:=b;
   return temp;
end
```

## Example
```
data rect = length * width;
   decl int length,width end;
data circle = * radius;
   decl int radius end;
data shape = srect | scircle;
   decl rect srect; circle scircle end;

fun areaofrect r;
   length(r) * width(r);
   
fun areaofcircle c;
   radius(c) * radius(c) * 3.14;

fun areaofshape s;
   decl shape s; int areaofshape end;
prog vars shapetype;
   decl int shapetype end;
   shapetype := type(s);
   if shapetype = shape(srect)
      then return areaofrect(srect(s))
      else return areaofcircle(scircle(s))
end;

fun main;
   decl void main end;
prog vars arrayofshapes c;
   decl [] ref shape arrayofshapes; circle c end;
   arrayofshapes := makearray(1, ref shape);
   arrayofshapes[0] := make(shape);
   radius(c) := 10;
   deref(arrayofshapes[0]) := c;
   print(areaofshape(deref(arrayofshapes[0])))
end
```

translate to s-expressions:
```
(DATA RECT (CARTESIAN LENGTH WIDTH)
   (DECLARE (INT LENGTH WIDTH)))
(DATA CIRCLE (CARTESIAN RADIUS)
   (DECLARE (INT RADIUS)))
(DATA SHAPE (UNION SRECT SCIRCLE)
   (DECLARE (RECT SRECT) (CIRCLE SCIRCLE)))
   
(DEFINE AREAOFRECT (LAMBDA (R)
   (TIMES (LENGTH R) (WIDTH R))))

(DEFINE AREAOFCIRCLE (LAMBDA (C)
   (TIMES (RADIUS C) (RADIUS C) 3.14)))

(DEFINE AREAOFSHAPE (LAMBDA (S)
   (DECLARE (SHAPE S) (INT LAMBDA))
(PROG (SHAPETYPE)
   (DECLARE (INT SHAPETYPE))
   (SETQ SHAPETYPE (TYPE S))
   (COND ((EQUAL SHAPETYPE (SHAPE RECT)) (RETURN (AREAOFRECT (SRECT S))))
       (T (RETURN (AREAOFCIRCLE (SCIRCLE S))))))))

(DEFINE MAIN (LAMBDA ()
   (DECLARE (VOID LAMBDA))
(PROG (ARRAYOFSHAPES C)
   (DECLARE ((ARRAY (REFERENCE SHAPE)) ARRAYOFSHAPES) (CIRCLE C))
   (SETQ ARRAYOFSHAPES (MAKEARRAY 1 (REFERENCE SHAPE)))
   (SETQ (ARRAYOFSHAPES 0) (MAKE SHAPE))
   (SETQ (RADIUS C) 10)
   (SETQ (DEREFERENCE (ARRAYOFSHAPES 0)) C)
   (PRINT (AREAOFSHAPE (DEREFERENCE (ARRAYOFSHAPES 0)))))))
```


## License
LST is distributed under the GNU GPL version 3 or later.
