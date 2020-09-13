\ gc               title                                09-08-20
\ name: compact garbage collector
\ release: 0.01
\ author: lst-lang <coldmoon@mailfence.com>
\ license: GNU GPL 3.0 or later











\ gc               template#1                           09-08-20
: field-offset nop ;
: field-template [ 0 field-offset cell+ ] literal + ;
: /field [ 0 field-template cell+ ] literal ;

: template-kind nop ;

: object-template [ 0 template-kind cell+ ] literal + ;
: object-size [ 0 object-template cell+ ] literal + ;
: /pointer [ 0 object-size cell+ ] literal ;

: element-template [ 0 template-kind cell+ ] literal + ;
: element-size [ 0 element-template cell+ ] literal + ;
: element-number [ 0 element-size cell+ ] literal + ;
: /array [ 0 element-number cell+ ] literal ;

\ gc               template#2                           09-08-20
: record-fields [ 0 template-kind cell+ ] literal + ;
: fields /field * ;
: /record fields [ 0 record-fields cell+ ] literal + ;

: tagfield-variants [ 0 template-kind cell+ ] literal + ;
: /tagfield cells [ 0 tagfield-variants cell+ ] literal + ;









\ gc               frame                                09-08-20
: sframe-template nop ;
: sframe-sp [ 0 sframe-template cell+ ] literal + ;
: sframe-fp [ 0 sframe-sp cell+ ] literal + ;
: stack-frame [ 0 sframe-fp cell+ ] literal ;

: mframe-kind nop ;
: mframe-template [ 0 mframe-kind cell+ ] literal + ;
: mframe-object [ 0 mframe-template cell+ ] literal + ;
: mframe-number [ 0 mframe-object cell+ ] literal + ;
: mark-frame [ 0 mframe-number cell+ ] literal ;





\ gc               bitmap#1                             09-13-20
1 cells 8 * constant cell-bits
variable 'this-bitmap 0 'this-bitmap !

: this-bitmap 'this-bitmap @ ;
: bits cell-bits /mod ;
: bitmap 0> if 1+ then dup 1+ cells allot swap over ! ;
: clear-bitmap this-bitmap a! @+ for 0 !+ next ;
: bits+ bits swap 1+ cells this-bitmap + a! ;
: bit-mask dup 0= if 1+ exit then 1 swap for 2* next ;
: set-bit bit-mask @a or !a ;
: @bit bit-mask @a and 0= 1+ ;




\ gc               bitmap#2                             09-13-20
variable left-bits variable middle-cells variable right-bits

: split-left cell-bits swap - left-bits ! ;
: split-middle left-bits @ - cell-bits /mod ;
: split-right right-bits ! middle-cells ! ;
: split swap split-left split-middle split-right ;
: bits-mask 0 invert swap for 2* next ;
: or-mask! a! @a or !+ a ;
: set-left cell-bits left-bits @ - bits-mask swap or-mask! ;
: !middles 0 invert swap a! for dup !+ next drop a ;
: set-middle middle-cells @ dup 0= if drop exit then !middles ;
: set-right right-bits @ bits-mask invert swap or-mask! ;



\ gc               bitmap#3                             09-13-20
: set-slices set-left set-middle set-right ;
: split-set a >r split r> set-slices drop ;
: loop-set for dup set-bit 1+ next drop ;
: ?cross-cell 2dup + cell-bits > ;
: set-bits ?cross-cell if split-set exit then loop-set ;
: type-cell dup 0= if 2drop exit then
   for dup 1 and . u2/ next drop ;
: type-cells dup 0= if drop exit then
   for dup @ cell-bits type-cell cell+ next ;
: type-bits this-bitmap cell+ swap
   cell-bits /mod >r type-cells @ r> type-cell ;




