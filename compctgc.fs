\ lst              title                                05-28-20
\ name: lst
\ release: 0.01
\ author: lst-lang <coldmoon@mailfence.com>
\ license: gnu gpl 3.0 or later











\ lst              load#1                               05-28-20
: thru do i dup . load loop ;
45 30 thru













\ lst              load#2                               05-28-20















\ lst              load#3                               05-28-20















\ lst              system messages#1                    05-28-20















\ lst              system messages#2                    05-28-20















\ lst              system messages#3                    05-28-20















\ lst              system messages#4                    05-28-20















\ lst              system messages#5                    05-28-20















\ lst              system messages#6                    05-28-20















\ lst              utilities#1                          05-28-20















\ lst              utilities#2                          05-28-20















\ lst              utilities#3                          05-28-20















\ lst              utilities#4                          05-28-20















\ lst              utilities#5                          05-28-20















\ lst              utilities#6                          05-28-20















\ lst              utilities#7                          05-28-20















\ lst              utilities#8                          05-28-20















\ lst              utilities#9                          05-28-20















\ lst              utilities#10                         05-28-20















\ lst              utilities#11                         05-28-20















\ lst              utilities#12                         05-28-20















\ lst              utilities#13                         05-28-20















\ lst              utilities#14                         05-28-20















\ lst              utilities#15                         05-28-20















\ lst              utilities#16                         05-28-20















\ lst              utilities#17                         05-28-20















\ lst              utilities#18                         05-28-20















\ lst              utilities#19                         05-28-20















\ lst              utilities#20                         05-28-20















\ lst              template#1                           05-28-20
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

\ lst              template#2                           05-28-20
: record-fields [ 0 template-kind cell+ ] literal + ;
: template-record ( fields -- size )
   template-field * [ 0 record-fields cell+ ] literal + ;

: tagfield-variants [ 0 template-kind cell+ ] literal + ;
: template-tagfield ( tags -- size )
   cells [ 0 tagfield-variants cell+ ] literal + ;








\ lst              frame                                05-28-20
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




\ lst              bitmap#1                             05-28-20
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


\ lst              bitmap#2                             05-28-20
: split ( lhalf size -- lhalf whole rhalf )
   over cell-bits swap - - cell-bits /mod ;
: make-mask ( n -- 111...000... ) 0 invert swap for 2* next ;
: or-mask! ( mask addr -- ) swap over @ or swap ! ;
: [bits-set] ( map offset size -- )
   split >r >r make-mask over or-mask!
   cell+ a! 0 invert r> for dup !+ next drop a
   r> make-mask invert swap or-mask! ;
: bits-set ( map offset size -- )
   dup 2 cell-bits * > if [bits-set] exit then
   for 2dup bit-set 1+ next 2drop ;




\ lst              bitmap#3                             05-28-20
: [type-bits] ( x bits -- )
   dup 0= if 2drop exit then
   for dup 1 and . u2/ next drop ;
: <type-bits> ( cells -- )
   dup 0= if drop exit then
   for @+ cell-bits [type-bits] next ;
: type-bits ( map bits -- )
   cell-bits /mod >r swap a!
   <type-bits> @+ r> [type-bits] ;






\ lst              registers & stack                    05-28-20
variable sp 128 cells allot drop sp cell+ sp !
variable fp sp cell+ fp !

: enter-stack ( size -- addr )
   sp @ + dup sp cell+ - 128 cells >
   if abort" STACK OVERFLOW" then sp @ swap sp ! ;
: out-stack ( size -- addr )
   sp @ - dup sp cell+ <
   if abort" STACK UNDERFLOW" then dup sp ! ;






\ lst              storage                              05-28-20
variable up 1600 cells allot drop up cell+ up !
1600 make-bitmap constant map map 1600 clear-bitmap

: cell/ ( addr -- ) 1 cells / ;
: cell/-set ( addr size -- )
   cell/ >r map swap cell/ bits+ r> bits-set ;
: ?set ( addr -- ) map swap cell/ bits+ bit@ ;
: cell-round ( bytes -- cells ) 1 cells /mod 0> if 1+ then ;
: update-freep ( freep1 currp1 -- freep2 currp2 )
   map over bits+ bit@ if swap drop dup then ;
: more-units ( size -- addr )
   cell-round >r 0 dup begin dup 1600 < while
   update-freep 2dup swap - r@ = if drop dup r> cell/-set
   1+ cells up + exit then repeat 2drop 0 ;

\ lst              mark phase#1                         05-28-20
variable msp 128 cells allot drop msp cell+ msp !

: top-markstack ( -- addr )
   msp @ markstack-frame - dup msp cell+ <
   abort" MARK STACK UNDERFLOW" ;
: enter-markstack ( -- addr )
   msp @ dup markstack-frame + dup msp 129 cells + >
   abort" MARK STACK OVERFLOW" msp ! ! ;
: out-markstack ( -- x ) top-markstack dup msp ! @ ;
: ?mark-stack ( -- flag ) msp @ msp cell+ > ;





\ lst              mark phase#2                         05-28-20
: putback-frame ( markstack-frame -- )
   enter-markstack >r a!
   @+ !r @+ @+ swap over + !r !r @+ 1- !r @+ !r ;
: putback-fieldframe ( markstack-frame -- )
   enter-markstack >r a!
   @+ template-field + !r @+ !r @+ !r @+ 1- !r @+ !r ;
: ?marked ( addr -- flag )
   dup nil = if drop true exit then
   cell/ ?set if true exit then false ;
: set-object ( template pointer -- )
   swap object-size @ swap cell/-set ;




\ lst              mark phase#3                         05-28-20
: [mark-field] ( markstack-frame -- )
   dup msframe-number @ 1- if putback-fieldframe then
   dup r@ msframe-object @ swap msframe-template @
   enter-markstack >r dup field-template @ !r
   field-offset @ + !r 0 !r 1 !r 0 !r r> drop ;
: [mark-record] ( markstack-frame -- )
   dup msframe-number @ 1- if putback-frame then
   enter-markstack >r a! @+ record-fields cell+ !r
   @+ !r @+ !r @a !r 1 !r r> drop ;






\ lst              mark phase#4                         05-28-20
: mark-pointer ( markstack-frame -- )
   dup msframe-number @ 1- if putback-frame then
   swap msframe-object @ dup ?marked if 2drop exit then
   2dup set-object enter-markstack >r
   over !r !r object-size @ !r 1 !r 0 !r r> drop ;
: mark-array ( markstack-frame -- )
   dup msframe-number @ 1- if putback-frame then
   swap msframe-object @ enter-markstack >r
   over element-template @ !r !r dup element-size @ !r
   element-number !r 0 !r r> drop ;





\ lst              mark phase#5                         05-28-20
: mark-record ( markstack-frame -- )
   dup msframe-isfield @
   if [mark-field] exit then [mark-record] ;
: mark-tagfield ( markstack-frame -- )
   dup msframe-number @ 1- if putback-frame then
   tagfield-variants cell+ swap msframe-object @
   swap over @ cells + @ enter-markstack r>
   !r cell+ !r 0 !r 1 !r r> drop ;







\ lst              mark phase#6                         05-28-20
variable mark-x 3 cells allot drop
mark-x a! ' mark-pointer !+ ' mark-array !+
' mark-record !+ ' mark-tagfield !a

: do-mark-x ( markstack-frame )
   dup msframe-template @ template-kind @
   dup 3 > if 2drop exit then cells mark-x + @ execute ;
: mark-frame ( fp -- old-fp )
   out-markstack >r dup stack-frame - sframe-template @
   !r !r 0 !r 1 !r r> drop
   begin ?mark-stack while top-markstack do-mark-x repeat ;
: mark ( -- )
   fp @ begin dup sp cell+ <> while mark-frame repeat ;


