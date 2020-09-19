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
* Function/Procedure: `(int, int) void`, `((int int) void, int, int) void`

## Statements
* Assignment: `a := 1`, `b := 3.0`
* Conditional: `if a>b then a else b`
* Compound: `prog a:=1; b:=3.0 end`
* Block (Compound with local variables): `prog vars a b c; ... end`
* Return (in Compound or Block): `return <result>`
* Goto: `prog label: a:=1; go label end`
* Function: `fun max a b; if a>b then a else b`
* Declaration: `decl <type> <var>,<var>...; <type> ... end`
* Union case: `case <var> in <sel>:<expr>; <sel>: ...; else <expr> end`

## Data Definition:
* Structure/Record with two members: `data rect = length * width`
* Structure/Record with a single member: `data circle = * radius`
* Union: `data shape = srect | scircle`
* Alias: `data window = rect`, `data string = [] char`

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
end max
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
   decl circle c; int areaofcircle end;
prog vars r;
   decl int r end;
   r := radius(c);
   return r * r * 3.14
end areaofcircle;

fun areaofshape s;
   decl shape s; int areaofshape end;
case s in
   srect: areaofrect(srect(s));
   else areaofcircle(scircle(s))
end areaofshape;

fun main;
   decl void main end;
prog vars shapes c;
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
(DATA RECT (CARTESIAN LENGTH WIDTH)
   (DECLARE (INT LENGTH WIDTH)))
(DATA CIRCLE (CARTESIAN RADIUS)
   (DECLARE (INT RADIUS)))
(DATA SHAPE (UNION SRECT SCIRCLE)
   (DECLARE (RECT SRECT) (CIRCLE SCIRCLE)))
   
(DEFINE AREAOFRECT (LAMBDA (R)
   (TIMES (LENGTH R) (WIDTH R))))

(DEFINE AREAOFCIRCLE (LAMBDA (C)
   (DECLARE (CIRCLE C) (INT LAMBDA))
(PROG (R)
   (DECLARE (INT R))
   (SETQ R (RADIUS C))
   (RETURN (TIMES R R 3.14)))))

(DEFINE AREAOFSHAPE (LAMBDA (S)
   (DECLARE (SHAPE S) (INT LAMBDA))
(CASE S
   (SRECT (AREAOFRECT (SRECT S)))
   (AREAOFCIRCLE (SCIRCLE S)))))

(DEFINE MAIN (LAMBDA ()
   (DECLARE (VOID LAMBDA))
(PROG (SHAPES C)
   (DECLARE ((ARRAY (REFERENCE SHAPE)) SHAPES) (CIRCLE C))
   (SETQ SHAPES (MAKEARRAY 1 (REFERENCE SHAPE)))
   (SETQ (SHAPES 0) (MAKE SHAPE))
   (SETQ (RADIUS C) 10)
   (SETQ (SCIRCLE (DEREFERENCE (SHAPES 0))) C)
   (PRINT (AREAOFSHAPE (DEREFERENCE (SHAPES 0)))))))
```


## License
LST is distributed under the GNU GPL version 3 or later.
