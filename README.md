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
* Array: `[10] int`, `[10][10] real`
* Reference/Pointer: `ref int`, `ref [10] int`, `ref ref [10][10] int`
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
* Type definition: `type string = ref char`

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
   return t
end max
```

## Example
```
type rect = cartes ((length, int), (width, int));
type circle = cartes ((radius, int));
type shape = union ((rectshape, rect), (circleshape, circle));

fun areaofrect r;
   length(r) * width(r);
   
fun areaofcircle c;
   radius(c) * radius(c) * 3.14;

fun areaofshape s;
   ref shape s; type int;
prog
   select type(s) case
   1: return areaofrect(s)
   2: return areaofcircle(s)
   else error("BAD SHAPE")
end areaofshape;

fun largest ss n;
   ref shape ss; int n; type int;
prog vars i m;
   int i, m;
   m := 0;
   for i := 0 step 1 until n do
      if areaofshape(ss[i]) > m
         then m := areaofshape(ss[i]);
   return m
end largest;

lambda; type void;
prog vars ss r q c;
   [3] shape ss; rect r, q; circle c;
   length(r):=12; width(r):=8; 
   length(q):=width(q):=10; radius(c):=10;
   ss[0]:=r; ss[1]:=q; ss[2]:=c;
   print(largest(ss, 3))
end lambda; ()
```

translate to s-expressions:
```
DEFINE ((
((TYPE RECT) (CARTESIAN ((LENGTH) INTEGER) ((WIDTH) INTEGER)))
((TYPE CIRCLE) (CARTESIAN ((RADIUS) INTEGER)))
((TYPE SHAPE) (UNION ((RECTSHAPE) RECT) ((CIRCLESHAPE) CIRCLE)))
))

DEFINE ((
((FUNCTION AREAOFRECT)
   (LAMBDA (R)
      (TIMES (LENGTH R) (WIDTH R))))
((FUNCTION AREAOFCIRCLE)
   (LAMBDA (C)
      (TIMES (RADIUS C) (RADIUS C) 3.14)))
((FUNCTION AREAOFSHAPE)
   (LAMBDA (S) (SHAPE S) (TYPE INTEGER)
      (PROG ()
         (SELECT (TYPE S)
	    (1 (RETURN (AREAOFRECT S)))
            (2 (RETURN (AREAOFCIRCLE S)))
	    (ERROR "BAD SHAPE")))))
((FUNCTION LARGEST)
   (LAMBDA (SS N) (REFERENCE SHAPE SS) (INTEGER N)
         (TYPE INTEGER)
      (PROG (I M) (INTEGER I M)
	 (SETQ M 0)
         (FOR I 0 1 N
	    (IF (< M (AREAOFSHAPE (SS I)))
	        THEN (SETQ M (AREAOFSHAPE (SS I)))))
	 (RETURN M))))
))

(LAMBDA () (TYPE VOID)
   (PROG (SS R Q C)
      (ARRAY (3) SHAPE SS) (RECT R Q) (CIRCLE C)
      (SETQ (LENGTH R) 12) (SETQ (WIDTH R) 8)
      (SETQ (LENGTH Q) (SETQ (WIDTH Q) 10))
      (SETQ (RADIUS C) 10)
      (SETQ (SS 0) R) (SETQ (SS 1) R) (SETQ (SS 2) C)
      (PRINT (LARGEST SS 3)))) ()
```


## License
LST is distributed under the GNU GPL version 3 or later.
