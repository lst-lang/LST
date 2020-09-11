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

: mframe-kind nop ;
: mframe-template [ 0 mframe-kind cell+ ] literal + ;
: mframe-object [ 0 mframe-template cell+ ] literal + ;
: mframe-number [ 0 mframe-object cell+ ] literal + ;
: mark-frame [ 0 mframe-number cell+ ] literal ;





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
variable sp 128 cells allot dup sp ! constant stack
variable fp sp cell+ fp !

: enter-stack ( size -- addr )
   sp @ + dup sp cell+ - 128 cells >
   if abort" STACK OVERFLOW" then sp @ swap sp ! ;
: out-stack ( size -- addr )
   sp @ - dup sp cell+ <
   if abort" STACK UNDERFLOW" then dup sp ! ;






\ gc               storage#1                            09-10-20
800 make-bitmap constant map

: clear-map ( -- ) map 800 clear-bitmap ;
: set-map ( offset size -- ) >r map swap bits+ r> bits-set ;
: ?set-map ( offset -- flag ) map swap bits+ bit@ ;

: parcels 2 cells * ;
800 parcels allot constant storage

: units>parcels ( units -- parcels )
   1 parcels /mod 0> if 1+ then ;




\ gc               storage#2                            09-10-20
: forward-1bit ( freep1 currp1 -- freep2 currp2 )
   1+ map over bits+ bit@ if swap drop dup then ;
: forward-cells ( freep1 currp1 cells -- freep2 currp2 )
   cells map + @ dup 0= if drop cell-bits + exit then invert 0=
   if cell-bits + swap drop dup exit then forward-1bit ;
: update-freep ( freep1 currp1 -- freep2 currp2 )
   dup cell-bits /mod 0= if forward-cells exit
   then drop forward-1bit ;
: more-units ( size -- addr )
   units>parcels 1+ >r 0 dup begin dup 800 < while
   update-freep 2dup swap - r@ > if drop dup r> set-map
   parcels storage + exit then repeat r> drop 2drop 0 ;



\ gc               mark phase#1                         09-10-20
: enter-mark ( -- addr ) mark-frame enter-stack ;
: out-mark ( -- addr ) mark-frame out-stack ;
: push-stack ( -- ) enter-mark a! 0 !+ -1 !+ fp @ !+ -1 !+ ;
: push-pointer ( template addr -- )
   over object-size + @ over swap set-map
   swap object-template + @ enter-mark a! -1 !+ !+ !+ -1 !a ;
: push-array ( template addr -- )
   over dup element-number + @ >r element-size + @ r@ *
   swap set-map swap enter-mark a! -2 !+ !+ !+ r> !a ;
: push-record ( template addr -- )
   swap dup record-fields + @
   enter-mark a! -3 !+ >r !+ !+ r> !a ;
: push-tagfield ( template addr -- )
   swap cells + @ enter-mark a! -4 !+ !+ !+ -1 !a ;

\ gc               mark phase#2                         09-10-20
: push-object ( template addr -- )
   dup @ if 2drop exit then over template-kind @
   dup -1 = if drop push-pointer exit then
   dup -2 = if drop push-array exit then
   dup -3 = if drop push-record exit then
   -4 = if push-tagfield exit then ;
: pop-object ( markframe -- object template number )
   mframe-template + a! @+ @+ swap @a 1- dup 0= 0=
   if 1- !a enter-mark then drop ;






\ gc               mark phase#3                         09-10-20
: mark-pointer ( markframe -- )
   mframe-template + a! @+ @a
   >r object-template + @ r> @ push-object ;
: mark-array ( markframe -- )
   pop-object over element-size + @ * swap element-template + @
   >r + r> swap push-object ;
: mark-record ( markframe -- )
   pop-object template-field * + dup field-template + @
   >r field-offset + @ + r> push-object ;
: mark-stack ( markframe -- )
   mframe-object + a! @a dup 0= 0=
   if dup !a enter-mark then stack + dup @ swap stack-frame +
   push-object ;


\ gc               mark phase#3                         09-10-20
: mark-object ( -- ) out-mark dup @
   dup 0 = if drop mark-stack exit then
   dup -1 = if drop mark-pointer exit then
   dup -2 = if drop mark-array exit then
   -3 = if mark-record then ;
: mark ( -- ) clear-map push-stack sp @
   begin sp @ = 0= while mark-object repeat ;








\ gc               compaction#1                         09-11-20
variable break-table 0 break-table !
variable break-entrys 0 break-entrys !
variable break-point 1 cells allot drop 0 break-point !

: current-entry ( -- addr )
   break-table @ break-entrys @ parcels + ;
: append-point ( -- )
   break-point a! @+ @a current-entry a! dup !+ swap - !a
   break-entrys a! @a 1+ !a ;
: add-entry ( currp -- )
   break-table a! @a 0= if 1- parcels storage + !a
   append-point exit then drop append-point ;
: save-point ( newp currp -- ) swap break-point a! !+ !a ;


\ gc               compaction#2                         09-11-20
: before-table ( -- size )
   break-table @ storage - break-point @ parcels - ;
: move-size ( newp -- size ) parcels break-point @ parcels - ;
: ?no-space ( newp -- flag ) before-table swap move-size < ;
: ?need-roll ( newp -- flag )
   break-table @ 0= if drop false exit then ?no-space ;
: roll-entry ( end1 -- end2 )
   1 parcels - >r break-table @ a! @+ @+ a break-table !
   swap r@ a! !+ !a r> ;
: roll-table ( end1 -- end2 )
   dup current-entry - 1 parcels / for roll-entry next ;
: move-partial ( from1 to1 size -- from2 to2 )
   >r 2dup r@ 1 cells / move r@ + swap r> + swap ;


\ gc               compaction#3                         09-11-20
: [rolling-move]  ( from1 to1 size1 -- from2 to2 size2 )
   >r >r dup roll-table over swap - r> swap
   dup r> swap - >r move-partial r> ;
: rolling-move ( from to size -- )
   begin dup 0 > while [rolling-move] repeat ;
: new-position ( -- addr ) @+ parcels storage + ;
: old-position ( -- addr ) @a parcels storage + ;
: move-parcels ( newp -- )
   >r break-point a! new-position old-position swap
   r@ move-size r> ?need-roll
   if rolling-move exit then 1 cells / move ;




\ gc               compaction#5                         09-11-20
: on-break ( newp currp -- )
   break-point @ 0= if save-point exit then
   over move-parcels dup add-entry save-point ;
: check-point ( newp currp -- )
   dup 0= if 2drop exit then
   dup 1- ?set-map if 2drop exit then on-break ;
: forward-1bit ( newp1 currp1 -- newp2 currp2 )
   dup ?set-map 0= if 1+ exit then
   2dup check-point swap 1+ swap 1+ ;
: forward-1cell ( newp1 currp1 addr -- newp2 currp2 )
   a! @a 0= if cell-bits + exit then
   @a invert 0= 0= if forward-1bit exit then
   2dup check-point swap cell-bits + swap cell-bits + ;


\ gc               compaction#6                         09-11-20
: step ( newp1 currp2 -- newp2 currp2 )
   dup cell-bits /mod if drop forward-1bit exit then
   cells map + forward-1cell ;
: compact ( -- )
   0 0 begin dup 800 < while step repeat on-break ;










