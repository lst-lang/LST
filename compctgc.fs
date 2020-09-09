\ gc               title                                09-08-20
\ name: compact garbage collector
\ release: 0.01
\ author: lst-lang <coldmoon@mailfence.com>
\ license: GNU GPL 3.0 or later











\ gc               template#1                           09-08-20
: field-offset nop ;
: field-template [ 0 field-offset cell+ ] literal + ;
: template-field [ 0 field-template cell+ ] literal ;

: template-kind nop ;

: object-template [ 0 template-kind cell+ ] literal + ;
: object-size [ 0 object-template cell+ ] literal + ;
: template-pointer [ 0 object-size cell+ ] literal ;

: element-template [ 0 template-kind cell+ ] literal + ;
: element-size [ 0 element-template cell+ ] literal + ;
: element-number [ 0 element-size cell+ ] literal + ;
: template-array [ 0 element-number cell+ ] literal ;

\ gc               template#2                           09-08-20
: record-fields [ 0 template-kind cell+ ] literal + ;
: template-record ( fields -- size )
   template-field * [ 0 record-fields cell+ ] literal + ;

: tagfield-variants [ 0 template-kind cell+ ] literal + ;
: template-tagfield ( tags -- size )
   cells [ 0 tagfield-variants cell+ ] literal + ;








\ gc               frame                                09-08-20
: sframe-template nop ;
: sframe-sp [ 0 sframe-template cell+ ] literal + ;
: sframe-fp [ 0 sframe-sp cell+ ] literal + ;
: stack-frame [ 0 sframe-fp cell+ ] literal ;

: msframe-template nop ;
: msframe-object [ 0 msframe-template cell+ ] literal + ;
: msframe-size [ 0 msframe-object cell+ ] literal + ;
: msframe-number [ 0 msframe-size cell+ ] literal + ;
: msframe-kind [ 0 msframe-number cell+ ] literal + ;
: markstack-frame [ 0 msframe-kind cell+ ] literal ;




\ gc               bitmap#1                             09-08-20
1 cells 8 * constant cell-bits
1 chars 8 * constant char-bits

: bitmap-size ( bits -- size ) cell-bits /mod 0> if 1+ then ;
: make-bitmap ( bits -- map ) bitmap-size cells allot ;
: clear-bitmap ( map bits -- )
   bitmap-size swap a! for 0 !+ next ;
: bits+ ( map offset1 -- addr offset2 )
   cell-bits /mod >r cells + r> ;
: make-mask ( offset -- mask )
   dup 0= if 1+ exit then 1 swap for 2* next ;
: bit-set ( addr offset -- ) make-mask over @ or swap ! ;
: bit@ ( map offset -- ) make-mask swap @ and 0= 1+ ;


\ gc               bitmap#2                             09-08-20
: split ( lhalf size -- lhalf whole rhalf )
   over cell-bits swap - - cell-bits /mod ;
: make-mask ( n -- 111...000... ) 0 invert swap for 2* next ;
: or-mask! ( mask addr -- ) swap over @ or swap ! ;
: set-cells ( addr1 cells -- addr2 )
   dup 0= if drop exit then
   swap a! 0 invert swap for dup !+ next drop a ;
: [bits-set] ( map offset size -- )
   split >r >r make-mask over or-mask!
   cell+ r> set-cells r> make-mask invert swap or-mask! ;
: bits-set ( map offset size -- )
   2dup + cell-bits > if [bits-set] exit then
   for 2dup bit-set 1+ next 2drop ;


\ gc               bitmap#3                             09-08-20
: [type-bits] ( x bits -- )
   dup 0= if 2drop exit then
   for dup 1 and . u2/ next drop ;
: <type-bits> ( addr cells -- )
   dup 0= if drop exit then
   for dup @ cell-bits [type-bits] cell+ next ;
: type-bits ( map bits -- )
   cell-bits /mod >r <type-bits> @ r> [type-bits] ;







\ gc               registers & stack                    09-08-20
variable sp 128 cells allot drop sp cell+ sp !
variable fp sp cell+ fp !

: enter-stack ( size -- addr )
   sp @ + dup sp cell+ - 128 cells >
   if abort" STACK OVERFLOW" then sp @ swap sp ! ;
: out-stack ( size -- addr )
   sp @ - dup sp cell+ <
   if abort" STACK UNDERFLOW" then dup sp ! ;






\ gc               storage                              09-08-20
1600 cells allot constant storage
1600 make-bitmap constant map map 1600 clear-bitmap

: set-map ( addr size -- ) >r map swap bits+ r> bits-set ;
: ?set ( addr -- ) map swap 1 cells / bits+ bit@ ;
: units>cells ( units -- cells ) 1 cells /mod 0> if 1+ then ;
: update-freep ( freep1 currp1 -- freep2 currp2 )
   map over bits+ bit@ if swap drop dup then
   cr 2dup swap ." curr:" . ." free:" . ;
: more-units ( size -- addr )
   units>cells 1+ >r 0 dup begin dup 1600 < while
   update-freep 2dup swap - r@ = if drop dup r> set-map
   cells storage + exit then 1+ repeat 2drop 0 ;



\ gc               storage                              09-08-20
