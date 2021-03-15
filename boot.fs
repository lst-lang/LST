\ This file is part of LST.

\ LST is free software; you can redistribute it and/or modify
\ it under the terms of the GNU General Public License as
\ published by the Free Software Foundation, either version 3
\ of the License, or (at your option) any later version.

\ This program is distributed in the hope that it will be
\ useful, but WITHOUT ANY WARRANTY; without even the implied
\ warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
\ PURPOSE. See the GNU General Public License for more details.

\ You should have received a copy of the GNU General Public
\ License along with this program.
\ If not, see http://www.gnu.org/licenses/.

\ boot                primitives                           forth
declare terminal declare /buffer declare 'input
declare '#input declare >in declare state declare 'here
declare 'resume declare flag declare code declare parameter
declare msb declare /cell declare /char
0 >> bye >> nop >> dup >> swap >> drop >> over >> rot >> -rot
>> nip >> tuck >> rot-drop >> rot-drop-swap >> + >> - >> * >> /
>> u+ >> u- >> u* >> u/ >> 1+ >> 1- >> invert >> and >> or
>> xor >> 2* >> u2/ >> 2/ >> rshift >> lshift >> drop-true
>> drop-false >> 0= >> 0< >> u> >> u< >> = >> u>= >> u<= >> <>
>> > >> < >> >= >> <= >> >r >> r> >> r@ >> r>drop >> @ >> !
>> c@ >> c! | lit | jmp | jz | drjne | call | ret >> ex >> a
>> a! >> @a >> !a >> @+ >> !+ >> @r >> !r >> c@a >> c!a >> c@+
>> c!+ >> c@r >> c!r >> save >> restore >> pick >> rpick
>> depth >> mod >> umod >> negate | macro >> clear-parameters
>> clear-returns drop

\ boot                core word set                        forth
: align-number dup 1- invert >r + 1- r> and ;
: cells /cell * ;
: aligned 1 cells align-number ;
: here 'here @ ;
: align 'here a! @a aligned !a ;
: variable open ret ,slot align here swap ! 1 cells allot ;
   immediate
: execute code + @ >r ex ;
: throw 'resume @ execute ;
: in >in @ ;
: input 'input @ ;
: @in in input + c@ ;
: +in >in a! @a 1+ !a ;
: #input '#input @ ;
: ?eoi in #input = ;
: ?eos ?eoi if ; then drop over @in = ;
: ?+in ?eoi if drop ; then drop +in ;
: length ?eos if drop swap drop >r r@ input + in r> - ?+in ;
   then drop +in length ;
: 2drop drop drop ;
: parse ?eoi if 2drop 0 0 ; then drop +in in length ;
: 2dup over over ;
: constant open ! ret ,slot ; immediate
[ 32 ] constant bl
: ' bl parse 2dup find-word dup 0= if drop -13 throw ; then
   drop >r 2drop r> ;
: compile ' code + @ ,call ; immediate
: postpone ' dup flag + @ if drop code + @ ,call ; then drop
   code + @ compile literal [ ' ,call code + @ ] literal
   compile literal compile >r compile ex ; immediate
variable base [ 10 base ! ]
: /mod 2dup mod >r / r> ;
: u/mod 2dup umod >r u/ r> ;
: ud/ swap over u/mod swap >r 2* over 0 msb or swap u/mod >r
   over >r * r> r> * swap >r >r swap over u/mod r> + swap >r
   swap u/mod swap r> + r> + swap r> swap ;
: d>c dup 9 > if drop 10 - 'A' + ; then drop '0' + ;
[ 128 ] constant /pictured
: chars /char * ;
variable #pictured [ /pictured chars allot ]
: cell+ 1 cells + ;
[ #pictured cell+ ] constant pictured
: hold #pictured a! @a 0= if drop -17 throw ; then drop @a 1-
   dup !+ chars a + c! ;
: # base @ ud/ d>c hold ;
: #> 2drop #pictured a! @+ chars a + /pictured #pictured @ - ;
: begin ,fill-nop here ; immediate
: until postpone 0= postpone if postpone drop swap jmp swap
   ,slot-word drop postpone then postpone drop ; immediate
: d0= 0= swap 0= and ;
: #s begin # 2dup d0= until ;
: ( ')' parse 2drop ; immediate
: d+ >r swap r> + >r 2dup + swap over swap u< >r swap over swap
   u< r> or r> swap if drop 1+ ; then drop ;
: um+ dup 1 and if drop >r 2dup >r >r d+ r> r> r> ; then drop ;
: d2* 2* over msb and if drop 1 or swap 2* swap ; then drop
   swap 2* swap ;
: um-shift dup 1 u>= if drop um+ u2/ >r d2* r> um-shift ; then
   2drop 2drop ;
: um* >r >r 0 0 r> r> swap >r 0 r> um-shift ;
: abs dup 0< if drop negate ; then drop ;
: d- >r swap r> - >r 2dup u< if drop 0 invert swap - + 1+ r> 1-
   ; then drop - r> ;
: dnegate dup 0< if drop invert swap invert swap 1 0 d+ ; then
   drop 1 0 d- invert swap invert swap ;
: m* over 0< over 0< <> >r abs swap abs um* r> if drop dnegate
   ; then drop ;
: um/mod ud/ swap drop swap ;
: sm/rem over 0 >= over 0 >= and if drop um/mod ; then drop
   over 0< over 0 >= and if drop >r dnegate r> um/mod negate >r
   negate r> ; then drop over 0 >= over 0< and if drop negate
   um/mod >r negate r> ; then drop negate >r dnegate r> um/mod
   negate ;
: */ >r m* r> sm/rem swap drop ;
: */mod >r m* r> sm/rem ;
: +! a! @a + !a ;
variable leaves
: leave postpone r>drop postpone r>drop jmp -1 ,slot-word
   leaves @ over ! leaves ! ; immediate
: while jz -1 ,slot-word swap postpone drop ; immediate
: repeat jmp swap ,slot-word drop ,fill-nop here swap !
   postpone drop ; immediate
: reslove ,fill-nop begin leaves @ dup while dup @ leaves !
   here swap ! repeat drop ;
: +loop postpone r> postpone + postpone r> postpone 2dup
   postpone >r postpone >r postpone >= postpone until
   postpone r>drop postpone r>drop reslove ; immediate
: , here ! 1 cells allot ;
: dabs dup 0< if drop dnegate ; then drop ;
: sign 0< if drop '-' hold ; then drop ;
: space bl emit ;
: for postpone >r postpone ,fill-nop here ; immediate
: next drjne swap ,slot-word drop ; immediate
: type swap a! for c@+ emit next ;
: <# /pictured #pictured ! ;
: d. dup >r dabs <# #s r> sign #> type space ;
: . dup 0< if drop -1 d. ; then drop 0 d. ;
: 0> 0 > ;
: cmove dup 0> if drop >r swap a! r> for c@+ swap >r c!r r>
   next drop ; then 2drop 2drop ;
: s" jmp -1 ,slot-word ,fill-nop '"' parse dup >r here dup >r
   over chars allot swap cmove align here swap ! r>
   postpone literal r> postpone literal ; immediate
: ." postpone s" postpone type ; immediate
: 2! swap over ! cell+ ! ;
: 2@ dup cell+ @ swap @ ;
: 2over 3 pick 3 pick ;
: 2swap >r -rot r> -rot ;
: >body parameter + @ ;
: ?digit dup 47 > swap 58 < and ;
: ?lower dup 96 > swap 123 < and ;
: ?upper dup 64 > swap 91 < and ;
: ?alpha dup ?lower swap ?upper or ;
: c>d dup ?digit if drop '0' - ; then drop 'A' - 10 + ;
: char+ 1 chars + ;
: >number begin dup 0> while over c@ dup ?digit over ?alpha or
   0= if 2drop ; then drop c>d dup base @ >= if 2drop ; then
   drop swap 1- >r swap char+ >r >r base @ um* drop >r base @
   um* r> + r> 0 d+ r> r> repeat ;
: ?dup if dup ; then ;
: ?terminator dup 0= swap 10 = or ;
: accept swap a! 0 swap for key dup ?terminator if 2drop r>drop
   ; then drop c!+ 1+ next ;
: parse-name bl skip dup >r input + bl stop r> - ;
: next-char 1- swap char+ swap ;
: signnum dup 0= if drop -13 throw ; then drop over c@ '-' = if
   drop next-char -1 ; then drop 1 ;
: basenum signnum swap >r swap >r 0 0 r> r> dup 0= if drop -13
   throw ; then drop >number ;
: decnum base @ >r 10 base ! basenum r> base ! ;
: hexnum base @ >r 16 base ! basenum r> base ! ;
: binnum base @ >r 2 base ! basenum r> base ! ;
: cnum 2 = if drop c@ 1 0 0 0 ; then drop -13 throw ;
: anynum over c@ dup '-' = if 2drop basenum ; then drop dup '#'
   = if 2drop next-char decnum ; then drop dup '$' = if 2drop
   next-char hexnum ; then drop dup '%' = if 2drop next-char
   binnum ; then drop ''' = if drop next-char cnum ; then drop
   basenum ;
: ?number anynum 0= if drop 2drop * state @ 0= if drop
   postpone literal ; then drop ; then drop -13 throw ;
variable 'name [ 1 cells allot ]
: ?word 'name a! over !+ dup !+ 2dup find-word dup 0= if 2drop
   ?number ; then drop >r 2drop r> dup flag + @ state @ or if
   drop execute ; then drop code + @ ,call ;
: interpret ?eoi if drop ; then drop parse-name
   dup 0= if drop 2drop interpret ; then drop ?word interpret ;
: cr 10 emit ;
: read-eval cr read-input interpret ." OK" read-eval ;
: quit clear-returns terminal 'input ! read-eval ;
[ 5 cells ] constant /frame
[ 4 /frame * ] constant /exception-stack
variable exception-stack [ /frame cells /exception-stack *
   allot exception-stack dup cell+ swap ! ]
: +frame exception-stack a! @a dup /frame + !a ;
: -frame exception-stack a! @a /frame - !a ;
: message dup -1 = if 2drop ." ABORT" ; then drop dup -2 = if
   2drop ." ABORT" '"' emit ; then drop dup -3 = if 2drop
   ." STACK OVERFLOW" ; then drop dup -4 = if 2drop
   ." STACK UNDERFLOW" ; then drop dup -5 = if 2drop
   ." RETURN STACK OVERFLOW" ; then drop dup -6 = if 2drop
   ." RETURN STACK UNDERFLOW" ; then drop dup -7 = if 2drop
   ." DO-LOOPS NESTED TOO DEEPLY DURING EXECUTION" ; then drop
   dup -8 = if 2drop ." DICTIONARY OVERFLOW" ; then drop dup
   -9 = if 2drop ." INVALID MEMORY ADDRESS" ; then drop dup -10
   = if 2drop ." DIVISION BY ZERO" ; then drop dup -11 = if
   2drop ." RESULT OUT OF RANGE" ; then drop dup -12 = if 2drop
   ." ARGUMENT TYPE MISMATCH" ; then drop dup -13 = if 2drop
   ." UNDEFINED WORD" ; then drop dup -14 = if 2drop
   ." INTERPRETING A COMPILE-ONLY WORD" ; then drop dup -15 =
   if 2drop ." INVALID FORGET" ; then drop dup -16 = if 2drop
   ." ATTEMPT TO USE ZERO-LENGTH STRING AS A NAME" ; then drop
   dup -17 = if 2drop
   ." PICTURED NUMERIC OUTPUT STRING OVERFLOW" ; then drop dup
   -18 = if 2drop ." PARSED STRING OVERFLOW" ; then drop dup
   -19 = if 2drop ." DEFINITION NAME TOO LONG" ; then drop dup
   -20 = if 2drop ." WRITE TO A READ-ONLY LOCATION" ; then drop
   dup -21 = if 2drop ." UNSUPPORTED OPERATION" ; then drop dup
   -22 = if 2drop ." CONTROL STRUCTURE MISMATCH" ; then drop
   dup -23 = if 2drop ." ADDRESS ALIGNMENT EXCEPTION" ; then
   drop dup -24 = if 2drop ." INVALID NUMERIC ARGUMENT" ; then
   drop dup -25 = if 2drop ." RETURN STACK IMBALANCE" ; then
   drop dup -26 = if 2drop ." LOOP PARAMETERS UNAVAILABLE" ;
   then drop dup -27 = if 2drop ." INVALID RECURSION" ; then 
   drop dup -28 = if 2drop ." USER INTERRUPT" ; then drop dup
   -29 = if 2drop ." COMPILER NESTING" ; then drop dup -30 = if
   2drop ." OBSOLESCENT FEATURE" ; then drop dup -31 = if 2drop
   ." >BODY USED ON NON-CREATED DEFINITION" ; then drop dup -32
   = if 2drop ." INVALID NAME ARGUMENT" ; then drop dup -33 =
   if 2drop ." BLOCK READ EXCEPTION" ; then drop dup -34 = if
   2drop ." BLOCK WRITE EXCEPTION" ; then drop dup -35 = if
   2drop ." INVALID BLOCK NUMBER" ; then drop dup -36 = if
   2drop ." INVALID FILE POSITION" ; then drop dup -37 = if
   2drop ." FILE I/O EXCEPTION" ; then drop dup -38 = if 2drop
   ." NON-EXISTENT FILE" ; then drop dup -39 = if 2drop
   ." UNEXPECTED END OF FILE" ; then drop dup -40 = if 2drop
   ." INVALID BASE FOR FLOATING POINT CONVERSION" ; then drop
   dup -41 = if 2drop ." LOSS OF PRECISION" ; then drop dup
   -42 = if 2drop ." FLOATING-POINT DIVIDE BY ZERO" ; then drop
   dup -43 = if 2drop ." FLOATING-POINT RESULT OUT OF RANGE" ;
   then drop dup -44 = if 2drop
   ." FLOATING-POINT STACK OVERFLOW" ; then drop dup -45 = if
   2drop ." FLOATING-POINT STACK UNDERFLOW" ; then drop dup -46
   = if 2drop ." FLOATING-POINT INVALID ARGUMENT" ; then drop
   dup -47 = if 2drop ." COMPILATION WORD LIST DELETED" ; then
   drop dup -48 = if 2drop ." INVALID POSTPONE" ; then drop dup
   -49 = if 2drop ." SEARCH-ORDER OVERFLOW" ; then drop dup -50
   = if 2drop ." SEARCH-ORDER UNDERFLOW" ; then drop dup -51 =
   if 2drop ." COMPILATION WORD LIST CHANGED" ; then drop dup
   -52 = if 2drop ." CONTROL-FLOW STACK OVERFLOW" ; then drop
   dup -53 = if 2drop ." EXCEPTION STACK OVERFLOW" ; then drop
   dup -54 = if 2drop ." FLOATING-POINT UNDERFLOW" ; then drop
   dup -55 = if 2drop ." FLOATING-POINT UNIDENTIFIED FAULT" ;
   then drop dup -56 = if 2drop ." QUIT" ; then drop dup -57 =
   if 2drop ." EXCEPTION IN SENDING OR RECEIVING A CHARACTER" ;
   then drop dup -58 = if 2drop
   ." [IF], [ELSE], OR [THEN] EXCEPTION" ; then drop dup -59 =
   if 2drop ." ALLOCATE" ; then drop dup -60 = if 2drop
   ." FREE" ; then drop dup -61 = if 2drop ." RESIZE" ; then
   drop dup -62 = if 2drop ." CLOSE-FILE" ; then drop dup -63 =
   if 2drop ." CREATE-FILE" ; then drop dup -64 = if 2drop
   ." DELETE-FILE" ; then drop dup -65 = if 2drop
   ." FILE-POSITION" ; then drop dup -66 = if 2drop
   ." FILE-SIZE" ; then drop dup -67 = if 2drop ." FILE-STATUS"
   ; then drop dup -68 = if 2drop ." FLUSH-FILE" ; then drop
   dup -69 = if 2drop ." OPEN-FILE" ; then drop dup -70 = if
   2drop ." READ-FILE" ; then drop dup -71 = if 2drop
   ." READ-LINE" ; then drop dup -72 = if 2drop ." RENAME-FILE"
   ; then drop dup -73 = if 2drop ." REPOSITION-FILE" ; then
   drop dup -74 = if 2drop ." RESIZE-FILE" ; then drop dup -75
   = if 2drop ." WRITE-FILE" ; then drop dup -76 = if 2drop
   ." WRITE-LINE" ; then drop dup -77 = if 2drop
   ." MALFORMED XCHAR" ; then drop dup -78 = if 2drop
   ." SUBSTITUTE" ; then drop -79 = if ." REPLACES" ; then drop
   ." UNKNOWN EXCEPTION" ;
: upcase dup ?lower if drop 32 - ; then drop ;
: uptype swap a! for c@+ upcase emit next ;
: name? 'name a! @+ @+ uptype '?' emit ;
: no-frame dup -1 = if 2drop clear-parameters quit ; then drop
   dup -2 = if 2drop type clear-parameters quit ; then drop
   dup -13 = if 2drop name? clear-parameters quit ; then drop
   message '!' emit quit ;
: resume exception-stack dup cell+ swap @ = if drop no-frame ;
   then drop restore ;
[ ' resume 'resume ! ]
: abort -1 throw ;
: abort" postpone if postpone drop postpone s" -2
   postpone literal postpone throw postpone then postpone drop
   ; immediate
: c, here c! 1 chars allot ;
: char parse-name drop c@ ;
: count a! c@+ a swap ;
variable created [ 0 created ! ]
: nothing ;
: create open jmp [ ' nothing ] literal code + @ ,slot-word
   created ! align here swap ! ; immediate
: decimal 10 base ! ;
: do postpone swap postpone >r postpone >r 0 leaves ! ,fill-nop
   here ; immediate
: does> align here ,fill-nop created @ ! ; immediate
: else jmp -1 ,slot-word ,fill-nop swap here swap ! ; immediate
[ 128 ] constant /scratch
variable scratch [ /scratch chars allot ]
: ?shorter < if drop -1 ; then drop 1 ;
: ?same 2dup = if drop 2drop 0 ; then drop ?shorter ;
: min 2dup > if drop swap drop ; then 2drop ;
: compare >r swap >r r> r> 2dup >r >r min dup 0= if 2drop 2drop
   r> r> ?same ; then drop swap a! for >r c@r c@+ 2dup <> if
   drop r> r> 2drop r> r> 2drop ?shorter ; then drop 2drop r>
   char+ next drop r> r> ?same ;
[ 0 drop-true ] constant true
[ 0 drop-false ] constant false
: environment? 2dup s" /COUNTED-STRING" compare 0= if drop
   2drop 255 true ; then drop 2dup s" /HOLD" compare 0= if drop
   2drop /pictured true ; then drop 2dup s" /PAD" compare 0= if
   drop 2drop /scratch true ; then drop 2dup
   s" ADDRESS-UNIT-BITS" compare 0= if drop 2drop 8 true ; then
   drop 2dup s" FLOORED" compare 0= if drop 2drop 0 true ; then
   drop 2dup s" MAX-CHAR" compare 0= if drop 2drop 255 true ;
   then drop 2dup s" MAX-D" compare 0= if drop 2drop -1 -1 msb
   invert and true ; then drop 2dup s" MAX-N" compare 0= if
   drop 2drop -1 msb invert and true ; then drop 2dup s" MAX-U"
   compare 0= if drop 2drop -1 true ; then drop 2dup s" MAX-UD"
   compare 0= if drop 2drop -1 -1 true ; then drop 2dup
   s" RETURN-STACK-CELLS" compare 0= if drop 2drop 32 true ;
   then drop s" STACK-CELLS" compare 0= if drop 32 true ; then
   drop false ;
: evaluate state @ >r input >r #input >r in >r '#input ! 'input
   ! 0 >in ! 1 state ! interpret r> >in ! r> '#input ! r> 'input
   ! r> state ! ;
: fill over 0> if drop >r swap a! r> swap for dup c!+ next drop
   ; then drop 2drop ;
: find dup count find-word dup 0= if drop ; then drop swap drop
   dup flag @ 1 = if drop -1 ; then drop 1 ;
: fm/mod dup >r sm/rem over dup 0 <> swap 0< r@ 0< xor and if
   drop 1- swap r> + swap ; then drop r>drop ;
: i postpone r@ ; immediate
: j 1 postpone literal postpone rpick ; immediate
: loop postpone r> postpone 1+ postpone r> postpone 2dup
   postpone >r postpone >r postpone = postpone until
   postpone r>drop postpone r>drop reslove ; immediate
: max 2dup < if drop swap drop ; then 2drop ;
: move dup 0> if drop >r swap a! r> for @+ swap >r !r r> next
   drop ; then 2drop 2drop ;
: s>d dup 0< if drop -1 ; then drop 0 ;
: source input #input ;
: space bl emit ;
: spaces dup 0> if drop for space next ; then 2drop ;
: ud. <# #s #> type space ;
: u. 0 ud. ;
: unloop postpone r>drop postpone r>drop ; immediate
: word dup skip dup >r input + swap stop r> - dup #pictured a!
   c!+ a swap cmove #pictured ;
: ['] ' postpone literal ; immediate
: [char] char postpone literal ; immediate
: erase-chars dup 0= if 2drop ; then drop for 0 c!+ next ;
: erase-cells dup 0= if 2drop ; then drop for 0 !+ next ;
: erase 1 cells u/mod >r swap a! erase-cells r> 1 chars /
   erase-chars ;
variable blk [ 0 blk ! ]
variable current [ 0 current ! ]
[ here 4 cells allot ] constant assigned
[ here 4 cells allot ] constant updated [ assigned 4 cells
   erase updated 4 cells erase ]
variable buffers [ 4096 chars allot ]
: \ blk @ if drop >in a! @a dup 64 mod - 64 + !a ; then drop
   #input >in ! ; immediate
: #assigned cells assigned + ;
: #updated cells updated + ;
: #buffer dup current ! 1024 * buffers + ;
: update dup #updated @ if drop dup #assigned @ swap 1024 *
   buffers + !block drop ; then 2drop ;
: assign 2dup #buffer @block >r swap over #assigned ! false
   swap #updated ! r> ;
: block 4 for dup r@ 1- #assigned @ = if 2drop r> 1- #buffer ;
   then drop next 4 for r@ 1- #assigned @ 0= if drop r> 1-
   assign ; then drop next 1 update 1 assign ;
: buffer 4 for dup r@ 1- #assigned @ = if 2drop r> 1- #buffer ; 
   then drop next 4 for r@ 1- #assigned @ 0= if 2drop r> 1-
   #buffer ; then drop next 1 update 1 #buffer ;
: save-buffers 4 for r@ 1- dup update #updated false swap !
   next ;
: flush save-buffers 4 for 0 r@ 1- #assigned ! next ;
: core-evaluate evaluate ;
: evaluate blk @ >r 0 blk ! core-evaluate r> blk ! ;
: unassign 4 for dup r@ 1- #assigned @ = if drop r> 1-
   #assigned 0 swap ! drop ; then drop next ;
: load state @ >r blk @ >r input >r #input >r in >r 1 state !
   dup blk ! 1024 '#input ! 0 >in ! block 'input ! interpret
   blk @ unassign r> >in ! r> '#input ! r> 'input ! r> blk ! r>
   state ! ;
: update true current @ #updated ! ;

\ boot                optional file word set               forth
[ 0 ] constant r/o [ 1 ] constant r/w [ 2 ] constant w/o
: bin w/o 1+ + ;
variable 'source-id [ 0 'source-id ! ]
: source-id 'source-id @ ;
[ 1 files 1 cells + /buffer chars + ] constant /fileid
: fileids /fileid * ;
[ here 8 fileids dup allot over swap erase ] constant buffer
variable 'fileid
: fileid 'fileid @ ;
: handle fileid fileids buffer + ;
: using fileid fileids buffer + 1 files + ;
: stream fileid fileids buffer + 1 files + cell+ ;
: close-file 1- 'fileid ! using a! @a 0 !a if drop handle
   fclose ; then drop -1 ;
: ?limit 0 begin dup 8 < while dup 'fileid ! using @ 0= if
   2drop false ; then drop 1+ repeat drop true ;
: open-file ?limit if 2drop 2drop 0 -1 ; then drop 1 using !
   handle fopen fileid 1+ swap ;
: empty-file w/o open-file 0= if drop close-file drop ; then
   2drop ;
: create-file >r 2dup empty-file r> open-file ;
: delete-file fremove ;
: ?id dup 0> over 9 < and if drop 1- 'fileid ! true ; then
   2drop false ;
: file-position ?id if drop handle ftell dup -1 = ; then drop -1
   -1 ;
: reposition-file ?id if drop handle 0 fseek -1 = ; then drop -1
   ;
: file-size dup file-position -1 = if drop -1 ; then drop >r 0
   over reposition-file -1 = if drop r>drop -1 ; then drop dup
   file-position -1 = if drop r>drop -1 ; then drop 0 handle 2
   fseek -1 = if drop r>drop -1 ; then drop over file-position
   -1 = if 2drop r>drop -1 ; then drop swap - swap r> swap
   reposition-file ;
: ?error swap drop true handle ferror ;
: read-line ?id if drop handle feof if drop 0 false 0 ; then
   drop swap >r 0 begin 2dup swap < while handle dup fgetc swap
   feof if 2drop r>drop ?error ; then drop dup c!r 10 = if r>
   2drop ?error ; then drop 1+ repeat r>drop ?error ; then r>
   2drop 0 false 0 ;
: fread-eval source-id 1- 'fileid ! handle feof if drop ;  then
   drop input /buffer source-id read-line 2drop '#input ! 0
   >in ! interpret fread-eval ;
: include-file ?id if drop state @ >r source-id >r blk @ >r
   input >r #input >r in >r fileid 1+ 'source-id ! 0 blk ! 1
   state ! stream 'input ! 0 '#input ! 0 >in ! fread-eval r>
   >in ! r> '#input ! r> 'input ! r> blk ! r> 'source-id ! r>
   state ! ; then drop ;
: included r/o open-file 0= if drop dup >r include-file r>
   close-file drop ; then drop -38 throw ;
: ?error swap drop handle ferror ;
: read-file ?id if drop swap a! 0 begin 2dup swap < while
   handle dup fgetc swap feof if 2drop ?error ; then drop c!+
   1+ repeat ?error ; then drop 0 -1 ;
variable buffer1 [ /buffer chars allot ] 
variable buffer2 [ /buffer chars allot ] 
variable 'buffer [ buffer2 'buffer ! buffer2 buffer1 ! buffer1
   buffer2 !  ]
: buffer 'buffer @ dup @ 'buffer ! cell+ ;
: core-s" postpone s" ;
: file-s" '"' parse 128 min >r buffer r@ over >r cmove r> r> ;
: s" state @ 0= if drop core-s" ; then drop file-s" ; immediate
: write-file ?id if drop for a! c@+ a swap dup handle fputc <>
   if 2drop r>drop -1 ; then drop next drop 0 ; then drop -1 ;
: write-line write-file 0=
   if drop 10 handle fputc 10 <> ; then drop -1 ;

\ boot          optional floating-point word set           forth
[ 32 ] constant /stack variable 'sp [ -1 'sp ! ]
[ here /stack floats allot ] constant stack
: sp 'sp @ ;
: +char 1- swap char+ swap ;
: ?sign dup 0= if drop 1 ; then drop over c@ '+' = if drop
   +char 1 ; then drop over c@ '-' = if drop +char -1 ; then
   drop 1 ;
: +float sp 1+ dup /stack 1- > if drop -3 throw ; then drop dup
   'sp ! floats stack + ;
: u>f +float u>!f ;
: @float sp dup 0< if drop -4 throw ; then drop floats stack +
   ;
: fdrop sp dup 0< if drop -4 throw ; then drop 1- 'sp ! ;
: fbinary @float fdrop @float ;
: f* fbinary !f* ;
: f+ fbinary !f+ ;
: digits 0 u>f begin dup 0> while over c@ dup ?digit over
   ?alpha or 0= if 2drop ; then drop c>d dup base @ >= if 2drop
   ; then drop base @ u>f f* u>f f+ +char repeat ;
: fswap sp 1 < if drop -4 throw ; then drop @float !fswap ;
: ?e dup 'D' = if 2drop true ; then drop dup 'd' = if 2drop
   true ; then drop dup 'E' = if 2drop true ; then drop 'e' =
   if drop true ; then drop false ;
: e over c@ ?e 0= if drop ; then drop +char ;
: fnegate @float !fnegate ;
: *sign 0< if drop fnegate true ; then drop true ;
: ud>f 0 invert u>f u>f f* u>f f+ ;
: fover @float 1 floats - dup stack < if drop -4 throw ; then
   drop +float !f! ;
: f/ fbinary !f/ ;
: **sign base @ 0 ud>f fswap 0> if drop for fover f* next fswap
   fdrop true ; then drop for fover f/ next fswap fdrop true ;
: exponent >r fswap fdrop f+ e ?sign >r >r >r 0 0 r> r> >number
   0> if 2drop 2drop r>drop r>drop false ; then drop 2drop dup
   0= if 2drop r>drop r> *sign ; true ; then drop r> **sign r>
   *sign ;
: >float ?sign >r digits over c@ '.' = if drop +char dup >r
   digits base @ u>f fswap r> over - dup 0= if 2drop r>
   exponent ; then drop for fover f/ next r> exponent ; then
   drop 2drop r>drop fdrop false ;
: d>f dup 0< if drop dnegate ud>f fnegate ; then drop ud>f ;
: f! @float swap !f! fdrop ;
: f- fbinary !f- ;
: f0< @float !f0< fdrop ;
: f0= @float !f0= fdrop ;
: f< fbinary swap !f< fdrop ;
: fdup @float +float !f! ;
: fabs fdup f0< if drop fnegate ; then drop ;
: f>s @float f@>s fdrop ;
: f>d fdup f0< >r fabs fdup
   0 invert u>f 1 u>f f+ fover fover f/ f>s dup u>f
   f*  f- f>s swap r> if drop dnegate ; then drop ;
: f@ +float !f! ;
: faligned 1 floats align-number ;
: falign here faligned 'here ! ;
: fconstant open postpone f@ ret ,slot here swap ! here 1
   floats allot f! ;
: fdepth sp 1+ ;
: fliteral jmp -1 ,slot-word here >r 1 floats allot align here
   swap ! r@ f! ,fill-nop r> postpone literal postpone f@ ;
   immediate
: float+ 1 floats + ;
: fmax fover fover f< if drop fswap fdrop ; then drop fdrop ;
: fmin fover fover f< 0= if drop fswap fdrop ; then drop fdrop
   ;
: frot sp 2 < if drop -4 throw ; then drop @float !frot ;
: fvariable open ret ,slot align here swap ! 1 floats allot ;
   immediate
: fhalf s" 0.5" >float drop ;
fvariable 'fhalf [ fhalf 'fhalf f! ]
: fzero s" 0.0" >float drop ;
fvariable 'fzero [ fzero 'fzero f! ]
: s>f dup 0< if drop u>f fnegate ; then drop u>f ;
: digit fover fover f/ f>s fswap fover dup s>f f* f- fswap base
   @ u>f f/ ;
: ?round 1 = if drop digit 5 < if drop ; then drop 1+ ; then
   drop ;
: write-significand >r swap >r swap for r@ ?round d>c over c!
   char+ digit next 2drop r> r> fdrop fdrop true ;
: write-digits >r fover f0= if drop 0 r> write-significand ;
   then drop digit begin dup 0= while drop 1- digit repeat r>
   write-significand ;
: represent fdup f0< >r fabs fdup 1 u>f f- f0< if drop 0 1 u>f
   r> write-digits ; then drop 1 1 u>f begin fover fover f/
   base @ u>f f- f0< 0= while base @ u>f f* 1+ repeat
   r> write-digits ;
: floor #pictured 64 represent drop >r dup 0= if 2drop r>drop 0
   u>f ; then drop dup 0< if 2drop r>drop 0 u>f ; then drop
   base @ u>f 0 u>f #pictured a! for fover f* c@+ c>d u>f f+
   next fswap fdrop r> if drop fnegate ; then drop ;
: fround 'fhalf f@ fover 'fzero f@ f< if drop f- floor ; then
   drop f+ floor ;
: ?sign if drop '-' emit ; then drop ;
: -f. ." 0." 1+ dup 0= if 2drop #pictured 6 type space ; then
   drop -6 max dup negate for '0' emit next 6 + dup 0= if drop
   space ; then drop #pictured swap type space ;
: +f. #pictured over type '.' emit chars #pictured + 6 type
   space ;
: f. #pictured /pictured represent drop ?sign dup 0= if 2drop
   ." 0.000000" space ; then drop dup 0< if drop -f. ; then
   drop +f. ;
: ?e dup 0= if 2drop false ; then drop swap a! for c@+ dup 'E'
   = swap 'e' = or if drop true r>drop ; then drop next false ;
: fnumber >float if drop state @ 0= if drop postpone fliteral ;
   then drop ; then drop -13 throw ;
: ?fnumber 2dup ?e if drop fnumber ; then drop ?number ;
: ?word 'name a! over !+ dup !+ 2dup find-word dup 0= if 2drop
   ?fnumber ; then drop >r 2drop r> dup flag + @ state @ or if
   drop execute ; then drop code + @ ,call ;
: interpret ?eoi if drop ; then drop parse-name
   dup 0= if drop 2drop interpret ; then drop ?word interpret ;
: cr 10 emit ;
: read-eval cr read-input interpret ." OK" read-eval ;
: quit clear-returns terminal 'input ! read-eval ;
: no-frame dup -1 = if 2drop clear-parameters quit ; then drop
   dup -2 = if 2drop type clear-parameters quit ; then drop
   dup -13 = if 2drop name? clear-parameters quit ; then drop
   message '!' emit quit ;
: resume exception-stack dup cell+ swap @ = if drop no-frame ;
   then drop restore ;
[ ' resume 'resume ! quit ]
