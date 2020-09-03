\ This file is part of LST.

\ LST is free software; you can redistribute it and/or
\ modify it under the terms of the GNU General Public License
\ as published by the Free Software Foundation,
\ either version 3 of the License, or (at your option)
\ any later version.

\ This program is distributed in the hope that it will
\ be useful, but WITHOUT ANY WARRANTY; without even the
\ implied warranty of MERCHANTABILITY or FITNESS FOR
\ A PARTICULAR PURPOSE.  See the GNU General Public License
\ for more details.

\ You should have received a copy of the
\ GNU General Public License along with this program.
\ If not, see http://www.gnu.org/licenses/.

number 0 number -1
begin, parse-name number, 32 postpone, skip-delimiters
number, 32 postpone, find-delimiter ret, end,
number 0 number -1
begin, define, postpone, parse-name
postpone, begin, ret, end,
define, <true> number, 0 true, ret, end,
define, <false> number, 0 false, ret, end,
define, char+ char-size literal plus, ret, end,
define, cell+ cell-size literal plus, ret, end,
define, chars char-size literal star, ret, end,
define, cells cell-size literal star, ret, end,
define, inc dup, fetch,
one-plus, swap, store, ret, end,
define, dec dup, fetch,
one-minus, swap, store, ret, end,
define, fill-nop, number, 1 postpone, fill, ret, end,
define, if number, 55 number, -1
postpone, slot-instruction, ret, end, immediate
define, then postpone, fill-nop, postpone, here fetch,
swap, store, ret, end, immediate
define, else number, 54 number, -1
postpone, slot-instruction, swap,
postpone, then ret, end, immediate
define, ?lower dup, number, 96 greater-than,
swap, number, 123 less-than, and, ret, end,
define, ?upper dup, number, 64 greater-than,
swap, number, 91 less-than, and, ret, end,
define, ?alpha dup, postpone, ?upper
swap, postpone, ?lower or, ret, end,
define, ?digit dup, number, 47 greater-than,
swap, number, 58 less-than, and, ret, end,
define, ?space dup, number, 10 equals, over,
number, 13 equals, or, over, number, 9 equals,
or, swap, number, 32 equals, or, ret, end,
define, downcase dup, postpone, ?upper
if number, 32 plus, then ret, end,
define, upcase dup, postpone, ?lower
if number, 32 minus, then ret, end,
define, incin in literal postpone, inc ret, end,
define, @digit over, in literal fetch,
postpone, chars plus, c-fetch, ret, end,
define, sign postpone, @digit number, 45 equals,
if postpone, incin number, -1 ret, then
number, 1 ret, end,
define, <basenum> base literal dup, fetch, to-r,
store, postpone, sign to-r, to-r, to-r,
r-from, in literal fetch, postpone, chars
plus, r-from, in literal fetch, minus,
postpone, parse-number
r-from, r-from, base literal store, to-r,
r-from, star, ret, end,
define, basenum drop, base literal fetch,
postpone, <basenum> ret, end,
define, decnum drop, postpone, incin number, 10
postpone, <basenum> ret, end,
define, hexnum drop, postpone, incin number, 16
postpone, <basenum> ret, end,
define, binnum drop, postpone, incin number, 2
postpone, <basenum> ret, end,
define, cnum drop, number, 3 not-equals,
if number, -13 throw, then postpone, char+ dup, c-fetch,
to-r, postpone, char+ c-fetch, number, 39 not-equals,
if number, -13 throw, then r-from, ret, end,
define, ?bdigit dup, postpone, ?digit swap,
postpone, ?alpha or, ret, end,
define, anynum postpone, @digit
dup, number, 45 equals, if postpone, basenum ret, then
dup, number, 35 equals, if postpone, decnum ret, then
dup, number, 36 equals, if postpone, hexnum ret, then
dup, number, 37 equals, if postpone, binnum ret, then
dup, number, 39 equals, if postpone, cnum ret, then
dup, postpone, ?bdigit if postpone, basenum ret, then
number, -13 throw, ret, end,
define, do-number in literal dup, fetch, to-r,
number, 0 swap, store, dup, zero-equals,
if number, -13 throw, then postpone, anynum
r-from, in literal store, state literal
fetch, if ret, then postpone, literal ret, end,
define, @code entry-code literal plus, fetch,
ret, end,
define, do-word state literal fetch, over,
entry-flag literal plus, fetch, or,
if postpone, @code to-r, ex,
fill-nop, ret, then postpone, @code
number, 57 swap, postpone, slot-instruction, drop,
postpone, fill-nop, ret, end,
define, interpret postpone, parse-name
dup, zero-equals, if drop, drop, ret, then
over, over, postpone, find-word dup, zero-equals,
if drop, postpone, do-number tail-recurse, then
to-r, drop, drop, r-from, postpone, do-word
tail-recurse, ret, end,
define, <quit> tib literal dup, input literal store,
/tib literal postpone, accept-input
#input literal store, number, 0 in literal
store, postpone, interpret
number, 79 postpone, emit number, 75 postpone, emit
number, 10 postpone, emit tail-recurse, ret, end,
define, quit clear-return-stack, postpone, <true>
state literal store, postpone, <quit> ret, end,
define, : postpone, parse-name postpone, begin,
postpone, <false> state literal store, ret, end,
define, ; number, 58 postpone, slot, postpone, end,
postpone, <true> state literal store, ret, end, immediate
define, <abortx> postpone, quit ret, end,
define, abortx dup, number, -1 equals,
over, number, -2 equals, or, zero-equals,
if postpone, display-syserr then
postpone, <abortx> ret, end,
define, [ postpone, <true> state literal
store, ret, end, immediate
define, ] postpone, <false> state literal store, ret, end,
quit

: dump [ halt, ] ; immediate
: ?interp state [ fetch, ] ;
: [literal] literal ; immediate
: literal ?interp if -14 [ throw, ] then literal ; immediate
: postpone postpone, ; immediate
: exit ret, ; immediate
: a ?interp if [ a, ] exit then a, ; immediate
: a! ?interp if [ a-store, ] exit then a-store, ; immediate
: !a ?interp if [ store-a, ] exit then store-a, ; immediate
: @a ?interp if [ fetch-a, ] exit then fetch-a, ; immediate
: !+ ?interp if [ store-plus, ]
   exit then store-plus, ; immediate
: @+ ?interp if [ fetch-plus, ]
   exit then fetch-plus, ; immediate
: !r ?interp if [ store-r, ] exit then store-r, ; immediate
: @r ?interp if [ fetch-r, ] exit then fetch-r, ; immediate
: c!a ?interp if [ c-store-a, ] exit then c-store-a, ; immediate
: c@a ?interp if [ c-fetch-a, ] exit then c-fetch-a, ; immediate
: c!+ ?interp if [ c-store-plus, ]
   exit then c-store-plus, ; immediate
: c@+ ?interp if [ c-fetch-plus, ]
   exit then c-fetch-plus, ; immediate
: c!r ?interp if [ c-store-r, ] exit then c-store-r, ; immediate
: c@r ?interp if [ c-fetch-r, ] exit then c-fetch-r, ; immediate
: ! ?interp if [ store, ] exit then store, ; immediate
: @ ?interp if [ fetch, ] exit then fetch, ; immediate
: c! ?interp if [ c-store, ] exit then c-store, ; immediate
: c@ ?interp if [ c-fetch, ] exit then c-fetch, ; immediate
: dup ?interp if [ dup, ] exit then dup, ; immediate
: drop ?interp if [ drop, ] exit then drop, ; immediate
: over ?interp if [ over, ] exit then over, ; immediate
: swap ?interp if [ swap, ] exit then swap, ; immediate
: pick ?interp if [ pick, ] exit then pick, ; immediate
: 2dup ?interp if [ over, over, ] exit
   then over, over, ; immediate
: 2drop ?interp if [ drop, drop, ] exit
   then drop, drop, ; immediate
: 2over 3 pick 3 pick ;
: >r ?interp if [ to-r, ] exit then to-r, ; immediate
: r> ?interp if [ r-from, ] exit then r-from, ; immediate
: rpick ?interp if [ rpick, ] exit then rpick, ; immediate
: 2swap >r swap r> swap >r >r swap r> swap >r r> r> ;
: r>drop ?interp if [ r-from-drop, ] exit
   then r-from-drop, ; immediate
: r@ ?interp if [ r-fetch, ] exit then r-fetch, ; immediate
: + ?interp if [ plus, ] exit then plus, ; immediate
: - ?interp if [ minus, ] exit then minus, ; immediate
: * ?interp if [ star, ] exit then star, ; immediate
: / ?interp if [ slash, ] exit then slash, ; immediate
: mod ?interp if [ mod, ] exit then mod, ; immediate
: 1+ ?interp if [ one-plus, ] exit
   then one-plus, ; immediate
: 1- ?interp if [ one-minus, ] exit
   then one-minus, ; immediate
: 2* ?interp if [ two-star, ] exit
   then two-star, ; immediate
: 2/ ?interp if [ two-slash, ] exit
   then two-slash, ; immediate
: 0= ?interp if [ zero-equals, ] exit
   then zero-equals, ; immediate
: 0< ?interp if [ zero-less, ] exit
   then zero-less, ; immediate
: = ?interp if [ equals, ] exit then equals, ; immediate
: <> ?interp if [ not-equals, ] exit
   then not-equals, ; immediate
: > ?interp if [ greater-than, ] exit
   then greater-than, ; immediate
: >= ?interp if [ greater-equals, ] exit
   then greater-equals, ; immediate
: < ?interp if [ less-than, ] exit
   then less-than, ; immediate
: <= ?interp if [ less-equals, ] exit
   then less-equals, ; immediate
: u+ ?interp if [ u-plus, ] exit then u-plus, ; immediate
: u- ?interp if [ u-minus, ] exit then u-minus, ; immediate
: u* ?interp if [ u-star, ] exit then u-star, ; immediate
: u/ ?interp if [ u-slash, ] exit then u-slash, ; immediate
: umod ?interp if [ u-mod, ] exit then u-mod, ; immediate
: u2/ ?interp if [ u-two-slash, ] exit
   then u-two-slash, ; immediate
: u> ?interp if [ u-greater-than, ] exit
   then u-greater-than, ; immediate
: u>= ?interp if [ u-greater-equals, ] exit
   then u-greater-equals, ; immediate
: u< ?interp if [ u-less-than, ] exit
   then u-less-than, ; immediate
: u<= ?interp if [ u-less-equals, ] exit
   then u-less-equals, ; immediate
: invert ?interp if [ invert, ] exit
   then invert, ; immediate
: and ?interp if [ and, ] exit then and, ; immediate
: or ?interp if [ or, ] exit then or, ; immediate
: xor ?interp if [ xor, ] exit then xor, ; immediate
: negate ?interp if [ negate, ] exit
   then negate, ; immediate
: begin fill-nop, [ here ] literal @ ; immediate
: while 55 -1 slot-instruction, swap ; immediate
: repeat 54 swap slot-instruction, drop
   fill-nop, here @ swap ! ; immediate
: until 55 swap slot-instruction, drop ; immediate
: for to-r, postpone begin ; immediate
: next 56 swap slot-instruction, drop ; immediate
: <<d->> >r swap r> - >r 2dup u<
   if 0 invert swap - + 1+ r> 1- exit
   then - r> exit ;
: d+ >r swap r> + >r 2dup + swap over swap u< >r swap
   over swap u< r> or r> swap if 1+ then ;
: dnegate dup 0<
   if invert swap invert swap 1 0 d+ exit
   then 1 0 <<d->> invert swap invert swap ;
: dabs dup 0< if dnegate then ;
: d0= 0= swap 0= and ;
: d< >r swap r> 2dup <
   if 2drop 2drop 0 [ true, ] exit then u<
   if 2drop 0 [ false, ] exit then u> ;
: ud> >r swap r> 2dup u>
   if 2drop 2drop 0 [ true, ] exit then u<
   if 2drop 0 [ false, ] exit then u> ;
: d- 2 pick 0 > 1 pick 0< and
   if dabs d+ exit then 2 pick 0< 1 pick 0 > and
   if dnegate 2swap d+ exit then 2 pick 0<
   1 pick 0< and if dabs 2swap dabs then
   2over 2over d< if 2swap <<d->> dnegate exit then <<d->> ;
: highest 0 swap
   begin dup [ lmb ] literal and 0=
   while 2* swap 1+ swap
   repeat drop [ cell-size ] literal 8 * swap - ;
: d2* 2* over [ lmb ] literal and
   if 1 or then swap 2* swap ;
: d2/ swap u2/ over 1 and
   if [ lmb ] literal or then swap 2/ ;
: du2/ swap u2/ over 1 and
   if [ lmb ] literal or then swap u2/ ;
: /mod 2dup mod >r / r> ;
: u/mod 2dup umod >r u/ r> ;
: ud/ swap over u/mod swap >r 2* over
   0 lmb or swap u/mod >r over >r * r> r> * swap >r >r
   swap over u/mod r> + swap >r swap u/mod swap
   r> + r> + swap r> swap ;
: d>c dup 9 > if 10 - 'A' + exit then '0' + ;
: hold [ #pictured ] literal @ 0= if -17 [ throw, ]
   then [ pictured ] literal [ #pictured ] literal
   dup dec @ chars + c! exit ;
: depth ?interp if [ depth, ] exit then depth, ; immediate
: # [ base ] literal @ ud/ d>c hold ;
: <# [ /pictured ] literal [ #pictured ] literal ! ;
: #> 2drop [ pictured ] literal [ #pictured ] literal @ +
   [ /pictured ] literal [ #pictured ] literal @ - ;
: <#s> begin # 2dup d0= until ;
: #s <# <#s> #> ;
: type dup if for dup char+ swap c@ emit next then drop ;
: d. dup 0< dup >r if dnegate then
   <# <#s> r> if '-' hold then #> type 32 emit ;
: . dup 0< if -1 else 0 then d. ;
: ' parse-name find-word dup 0= if -13 [ throw, ] then ;
: ( ')' find-delimiter incin drop ; immediate
: um/mod ud/ swap drop swap ;
: sm/rem over 0 >= over 0 >= and if um/mod exit
   then over 0< over 0 >= and
   if >r dnegate r> um/mod
   negate >r negate r> exit
   then over 0 >= over 0< and
   if negate um/mod >r negate r> exit then
   negate >r dnegate r> um/mod negate ;
: <um*> dup 1 u>= if dup 1 and
   if >r 2dup >r >r d+ r> r> r> then u2/ >r d2* r>
   [ tail-recurse, ] then ;
: um* >r >r 0 0 r> r> swap >r 0 r> <um*> 2drop drop ;
: m* over 0< over 0< <> >r dup 0< if negate then
   swap dup 0< if negate then um* r> if dnegate then ;
: */ >r m* r> sm/rem swap drop ;
: */mod >r m* r> sm/rem ;
: +! >r r@ @ + r> ! ;
: , [ cell-size ] literal allot ! ;
: fm/mod dup >r sm/rem over dup 0 <> swap 0< r@ 0<
   xor and if 1- swap r> + swap else r>drop then ;
: <postpone> 57 swap slot-instruction, drop fill-nop, ;
: postpone parse-name find-word dup 0=
   if -13 [ throw, ] then dup [ entry-flag ] literal + @
   if 57 swap @code slot-instruction, drop fill-nop, exit
   then @code postpone literal 57 [ ' <postpone> @code ]
   literal slot-instruction, drop fill-nop, ; immediate
: " incin input @ in @ + '"' find-delimiter incin ;
: s" ?interp if -14 [ throw, ] then "
   dup >r static-allot r@ over >r cmove
   r> postpone literal r> postpone literal ; immediate
: ." postpone s" postpone type ; immediate
: 2! swap over ! cell+ ! ;
: 2@ dup cell+ @ swap @ ;
: dm* swap over m* >r >r m* r> or r> ;
: >number begin dup while
   over c@ dup ?digit over ?alpha or 0=
   if drop, exit
   then parse-digit dup [ base ] literal @ >=
   if drop, exit then swap 1- >r swap char+ >r >r
   [ base ] literal @ um* drop >r
   [ base ] literal @ um* r> +
   r> 0 d+ r> r> repeat ;
: ?dup dup if dup exit then ;
: abort 10 emit -1 [ throw, ] ;
: <abort"> type 10 emit -2 [ throw, ] ;
: abort" postpone s" postpone <abort"> ; immediate
: abs dup 0< if negate then ;
: aligned [ cell-size ] literal align-number ;
: align aligned [ here ] literal ! ;
: c, [ char-size ] literal allot c! ;
: char parse-name drop c@ ;
: <constant> ?interp 0= if postpone literal then ;
: constant >r 32 word count begin,
   53 r> slot-instruction, drop
   postpone <constant> ret, end, immediate ;
base constant base 32 constant bl
: cr 10 emit ;
: decimal 10 base ! ;
input constant input #input constant #input in constant >in
: execute @code >r [ ex, fill-nop, ] ;
<false> constant false
: 0> 0 > ;
: fill over dup 0> if swap >r swap r>
   for 2dup c! char+ next then 2drop ;
: find dup count find-word dup 0=
   if exit then swap drop
   dup [ entry-flag ] literal + @ dup 0=
   if 1- then ;
here constant here
: lshift dup if for 2* next exit then drop ;
: max 2dup < if swap then drop ;
: min 2dup > if swap then drop ;
: quit 10 emit quit ;
: rshift dup if for 2/ next exit then drop ;
: s>d dup 0< if -1 else 0 then ;
: sign 0< if '-' hold then ;
: source input @ #input @ ;
: space 32 emit ;
: spaces dup 0> if for 32 emit next exit then drop ;
<true> constant true
: ud. <# <#s> #> type 32 emit ;
: u. 0 ud. ;
: variable 32 word count
   begin, 53 0 slot-instruction, >r
   postpone <constant> ret, end, immediate
   [ cell-size ] literal allot r> ! ;
: ['] ' postpone literal ; immediate
: [char] char postpone literal ; immediate
variable leaves
: do 0 leaves ! over, to-r, minus,
   postpone for ; immediate
: leave r-from-drop, r-from-drop,
   54 -1 slot-instruction,
   leaves @ over ! leaves ! ; immediate
: reslove-leaves fill-nop,
   begin leaves @ dup
   while dup @ leaves ! here @ swap !
   repeat drop ;
: loop postpone next
   reslove-leaves r-from-drop, ; immediate
: <+loop> 1 rpick swap 2dup < if 2drop 1 else - then
   r> swap r>drop >r >r ;
: +loop postpone <+loop> postpone loop ; immediate
: unloop r-from-drop, r-from-drop, ; immediate
: i 2 rpick 1 rpick - ;
: j 4 rpick 3 rpick - ;
variable created 0 created !
: nop ;
: create parse-name begin, 53 -1 slot-instruction,
   54 ['] nop @code slot-instruction, created !
   fill-nop, ret, swap dup >r end, align here @ dup
   r> [ entry-parameter ] literal + ! swap ! ;
: [does>] created @ dup 0= if drop exit then
   r> swap ! 0 created ! ;
: does> ?interp if -14 [ throw, ] then
   postpone [does>] ; immediate
: >body [ entry-parameter ] literal + @ ;
: environment?
   2dup s" /COUNTED-STRING" compare 0=
   if 2drop 255 true exit then
   2dup s" /HOLD" compare 0=
   if 2drop [ /pictured ] literal true exit then
   2dup s" /PAD" compare 0=
   if 2drop [ /pad ] literal true exit then
   2dup s" ADDRESS-UNIT-BITS" compare 0=
   if 2drop 8 true exit then
   2dup s" FLOORED" compare 0= if 2drop 0 true exit then
   2dup s" MAX-CHAR" compare 0= if 2drop 255 true exit then
   2dup s" MAX-D" compare 0=
   if 2drop -1 -1 [ lmb ] literal invert and true exit then
   2dup s" MAX-N" compare 0=
   if 2drop -1 [ lmb ] literal invert and true exit then
   2dup s" MAX-U" compare 0= if 2drop -1 true exit then
   2dup s" MAX-UD" compare 0= if 2drop -1 -1 true exit then
   2dup s" RETURN-STACK-CELLS" compare 0=
   if 2drop 32 true exit then
   s" STACK-CELLS" compare 0= if 32 true exit then false ;
: macro parse-name begin,
   82 over entry-name + slot-instruction, drop
   fill-nop, swap if drop, then ret, end, ;
variable blk 0 blk !
variable current-buffer 0 current-buffer !
4 cells allot constant assigned-blocks
assigned-blocks 4 cells erase
4 cells allot constant updated-buffers
updated-buffers 4 cells erase
4096 chars allot constant block-buffers
: \line >in @ dup 64 mod - 64 + >in ! ;
: \ blk @ if \line exit then #input @ >in ! ; immediate
: assigned-block cells assigned-blocks + ;
: updated-buffer cells updated-buffers + ;
: [block-buffer] 1024 * block-buffers + ;
: block-buffer dup current-buffer ! [block-buffer] ;
: update dup updated-buffer @ if dup assigned-block @
   swap [block-buffer] !block then drop ;
: assign 2dup block-buffer
   @block >r
   swap over assigned-block !
   false swap updated-buffer ! r> ;
: block 4 for dup r@ 1- assigned-block @ =
   if drop r> 1- block-buffer exit then next
   4 for r@ 1- assigned-block @ 0=
   if r> 1- assign exit then next
   1 update 1 assign ;
: buffer 4 for dup r@ 1- assigned-block @ =
   if drop r> 1- block-buffer exit then next
   4 for r@ 1- assigned-block @ 0=
   if drop r> 1- block-buffer exit then next
   1 update 1 block-buffer ;
: save-inputs r> >in @ >r input @ >r
   #input @ >r blk @ >r >r ;
: restore-inputs r> r> blk ! r> #input !
   r> input ! r> >in ! >r ;
: evaluate save-inputs #input ! input !
   0 >in ! 0 blk ! interpret restore-inputs ;
: save-buffers 4 for r@ 1- dup update
   updated-buffer false swap ! next ;
: flush save-buffers
   4 for 0 r@ 1- assigned-block ! next ;
: unassign 4 for dup r@ 1- assigned-block @ =
   if r> 1- assigned-block 0 swap ! drop exit then
   next ;
: load save-inputs dup blk ! 1024 #input !
   0 >in ! block input ! interpret
   blk @ unassign restore-inputs ;
: update true current-buffer @ updated-buffer ! ;
: [search] - dup >r for 2dup >r >r >r >r
   over r> r> swap over compare
   0= if r> drop r> drop r> drop r> drop true exit then
   1- >r char+ r> r> r> next
   2drop r@ + swap r> chars - swap false ;
: search >r over r> swap over
   2dup = if 2drop >r 2dup r> r> compare 0= exit then
   2dup < if 2drop r> 2drop false exit then [search] ;
: bye [ HALT, ] ;

\ optional file word set.
0 macro fptrs 0 macro fopen 0 macro fclose
0 macro fremove 0 macro fread 0 macro fwrite
0 macro feof 0 macro ferror 0 macro fseek 0 macro ftell
8 fptrs allot 1 fptrs - constant files
8 cells allot dup 1 cells - constant file-flags
a! 0 !+ 0 !+ 0 !+ 0 !+ 0 !+ 0 !+ 0 !+ 0 !a
0 constant r/o 1 constant r/w 2 constant w/o
variable line-term 10 line-term c!
variable [source-id] 0 [source-id] !
variable string-buffer1 128 chars allot drop
variable string-buffer2 128 chars allot drop
string-buffer1 string-buffer2 !
string-buffer2 string-buffer1 !
variable strbuf-pointer string-buffer1 strbuf-pointer !
: bin 3 + ;
: find-unused file-flags 1 cells + a!
   8 for @+ 0= if 9 r> - exit then next -1 ;
: id>file fptrs files + ;
: id>flag cells file-flags + ;
: open-file find-unused dup -1 =
   if dup exit then dup >r id>file fopen r> swap
   over if exit then true over id>flag ! ;
: ?bad-id dup 0 < swap 7 > or ;
: close-file dup ?bad-id if drop 0 exit then
   dup id>flag dup >r @ if 0 r> ! id>file fclose exit then
   r> 2drop 0 ;
: empty-file w/o open-file 0= if close-file then drop ;
: create-file >r 2dup empty-file r> open-file ;
: delete-file fremove ;
: file-position dup ?bad-id if -1 exit then
   id>file ftell dup -1 = ;
: reposition-file dup ?bad-id if drop -1 exit then
   id>file 0 fseek ;
: [file-size] 0 over id>file 2 fseek 0=
   if file-position exit then drop -1 ;
: file-size dup file-position if drop -1 then
   over [file-size] >r >r swap reposition-file r> or r> ;
: read-file dup ?bad-id if 2drop -1 exit then
   id>file dup >r fread r> ferror 0= 0= ;
: read-char 1 swap read-file swap 1 = 0= or ;
: maybe-error swap >r >r drop r> true r> id>file ferror ;
: read-line dup file-position >r over file-size >r
   = r> r> or or if 2drop drop 0 false 0 exit then
   over >r swap for 2dup read-char
   if r> r> swap - maybe-error exit then
   over c@ 10 = if 2drop r> r> swap - true 0 exit then
   swap char+ swap next 2drop r> true 0 ;
: write-file dup ?bad-id if 2drop -1 exit then
   id>file dup >r over >r fwrite r> =
   if r> ferror 0= 0= exit then -1 r> drop ;
: write-line dup >r write-file dup 0=
   if drop line-term 1 r> write-file exit then r> drop ;
: source-id [source-id] @ ;
: evaluate source-id >r -1 [source-id] !
   evaluate r> [source-id] ! ;
: read-input tib /tib 2 - source-id read-line ;
: check-io-exception 0= 0= if -37 [ throw, ] then ;
: ?eof check-io-exception 0= ;
: ?incomplete-line dup /tib 2 - = ;
: line-too-long true abort" INPUT LINE TOO LONG!" ;
: ?more-input read-input ?eof if drop false exit then
   ?incomplete-line if line-too-long then true ;
: finterpret begin true while ?more-input 0= if exit then
   #input ! 0 >in ! interpret repeat ;
: include-file source-id >r save-inputs
   [source-id] ! tib input ! 0 #input ! 0 >in ! 0 blk !
   finterpret restore-inputs r> [source-id] ! ;
: included r/o open-file check-io-exception
   dup >r include-file r> close-file check-io-exception ;
: file( begin true while ')' find-delimiter drop
   >in @ #input @ < if incin exit then
   ?more-input 0= if exit then #input ! 0 >in ! repeat ;
: ( source-id 0 > if file( exit then postpone ( ; immediate
: next-strbuf strbuf-pointer @ @ dup strbuf-pointer ! ;
: s" ?interp if -14 [ throw, ] then "
   dup >r static-allot r@ over >r cmove
   r> postpone literal r> postpone literal ; immediate
: file-s" " 128 min dup >r next-strbuf
   cell+ dup >r swap cmove r> r> ;
: s" ?interp if file-s" exit then postpone s" ; immediate

\ optional floating-point word set.
0 macro [f!] 0 macro [f*] 0 macro [f+] 0 macro [f-]
0 macro [f/] 0 macro [f<] 0 macro [f/] 0 macro floats
0 macro [floor] 0 macro [fnegate] 0 macro [frot]
0 macro [fround] 0 macro [fswap] 0 macro [f0<]
0 macro [f0=] 0 macro [ud>f] 0 macro [float.]
variable fsp 32 floats allot constant fstack
-1 fsp !
: one-more-float fsp @ 1+ dup 31 > if -3 [ throw, ] then
   dup fsp ! floats fstack + ;
: fdrop fsp @ dup 0< if -4 [ throw, ] then 1- fsp ! ;
: @float fsp @ dup 0< if -4 [ throw, ] then floats fstack + ;
: f! @float swap [f!] drop ;
: f@ one-more-float [f!] drop ;
: fbinary @float fdrop @float ;
: f* fbinary [f*] drop ;
: f+ fbinary [f+] drop ;
: f- fbinary [f-] drop ;
: f/ fbinary [f/] drop ;
: f< fbinary swap [f<] fdrop ;
: floor @float [floor] fdrop ;
: fround @float [fround] drop ;
: fnegate @float [fnegate] drop ;
: ud>f one-more-float [ud>f] drop ;
: d>f dup 0< if dnegate ud>f fnegate exit then ud>f ;
: fdup @float one-more-float [f!] drop ;
: fover @float 1 floats - dup fstack <
   if -4 [ throw, ] then one-more-float [f!] drop ;
: fdepth fsp @ 1+ ;
: float+ 1 floats + ;
: fswap fsp @ 1 < if -4 [ throw, ] then @float [fswap] drop ;
: frot fsp @ 2 < if -4 [ throw, ] then @float [frot] drop ;
: uf>d fdup 0 1 d>f f< if floor exit then
   fdup 0 1 d>f fover fover f/ f* f- floor 0 1 d>f f/ floor ;
: f0< @float [f0<] fdrop ;
: f0= @float [f0=] fdrop ;
: f>d fdup f0< if fnegate uf>d dnegate exit then uf>d ;
: fmin fover fover f< 0= if fswap then fdrop ;
: fmax fover fover f< if fswap then fdrop ;
: float. @float [float.] drop fdrop ;

\ optional tools word set.
1 macro dump 1 macro see 1 macro words
: .s ?interp if [ dot-s, ] exit then dot-s, ; immediate
: ? @ . ;
