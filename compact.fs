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

\ compact             debugs                                 lst
: .s depth 0= if drop ; then drop depth for r@ 1- pick . next ;

\ compact             interfaces                             lst
defer collect-garbage
defer allocate-template
defer @corrected-pointer

\ compact             bitmap                                 lst
[ 256 ] constant /heap
variable heap [ /heap cells allot ] variable hp [ heap hp ! ]
[ here /heap 8 / allot ] constant bitmap
[ 8 cells ] constant /bits [ 0 invert ] constant mask
: reset-map bitmap a! /heap 8 cells / for 0 !+ next ;
: on-bound  >r drop a! mask /bits r> - lshift @a or !a ;
: bound over + >r mask swap rshift dup /bits r> - lshift and
   swap a! @a or !a ;
: single-word over 0= if drop on-bound ; then drop bound ;
: on-bound >r drop a! mask r> /bits /mod >r for dup !+ next
   /bits r> - lshift @a or !a ;
: bound over + /bits - >r mask swap rshift swap a! @a or !+ a 0
   r> dup /bits < if drop single-word ; then drop on-bound ;
: cross-words over 0= if drop on-bound ; then drop bound ;
: ?cross 2dup + /bits <= ;
: set-map >r heap - 1 cells / /bits /mod swap cells bitmap +
   swap r> ?cross if drop single-word ; then drop cross-words ;

\ compact             stack                                  lst
variable fp [ 0 fp ! ] variable sp [ heap /heap cells + sp ! ]
: frame-template nop ;
: previous-fp [ 0 frame-template cell+ ] literal + ;
[ 0 previous-fp cell+ ] constant /frame
: [+stack] sp @ swap - dup hp @ < if drop false ; then drop sp
   ! true ;
: +stack [+stack] if drop ; then drop collect-garbage [+stack]
   if drop ; then drop true abort" STACK OVERFLOW" ;
: -stack sp @ + dup heap /heap cells + > if
   abort" STACK UNDERFLOW" ; then drop sp ! ;
: +frame fp @ swap /frame +stack sp @ dup fp ! a! !+ !a ;
: -frame fp @ dup 0= if abort" STACK UNDERFLOW" ; then drop
   dup previous-fp @ fp ! sp ! ;

\ compact             allocator                              lst
[ here /bits cells allot ] constant masks
: lambda masks /bits cells + 1 /bits for swap 1 cells - 2dup !
   swap 2* next 2drop ; [ lambda ]
: ?on /bits /mod cells masks + @ swap cells bitmap + @ and ;
: ?empty-/bits /bits / cells bitmap + @ 0 = ;
: ?+/bits dup ?empty-/bits 0= if drop 1+ ; then drop /bits + ;
: +count dup /bits mod 0= if drop ?+/bits ; then drop 1+ ;
: +1bits begin dup /heap < over ?on and while +count repeat ;
: ?off ?on 0= ;
: ?+/bits dup ?empty-/bits if drop /bits + ; then drop 1+ ;
: +count dup /bits mod 0= if drop ?+/bits ; then drop 1+ ;
: ?0bits >r dup begin dup /heap < over ?off and while +count
   2dup swap - r@ >= if 2drop r>drop true ; then drop repeat
   swap drop r>drop false ;
: at-least 2dup < if drop swap drop ; then 2drop ;
: map-width 1 cells /mod 0= if drop ; then drop 1+ ;
: ?overflow cells + dup hp a! @a max !a sp @ > ;
: allocate map-width 2 at-least >r 0 begin dup /heap = if 2drop
   r>drop 0 ; then drop +1bits r@ ?0bits until cells heap + dup
   r@ ?overflow if 2drop r>drop 0 ; then drop dup r> set-map ;

\ compact             template                               lst
[ 8 cells ] constant /template
[ 1 ] constant %pointer [ 2 ] constant %any
[ 3 ] constant %array [ 4 ] constant %flexible
[ 5 ] constant %record [ 6 ] constant %variants
: template-kind nop ;
: object-template [ 0 template-kind cell+ ] literal + ;
: object-size [ 0 object-template cell+ ] literal + ;
[ 0 object-size cell+ ] constant /pointer
: element-template [ 0 template-kind cell+ ] literal + ;
: element-size [ 0 element-template cell+ ] literal + ;
: bound-specifiers [ 0 element-size cell+ ] literal + ;
: elements-number [ 0 bound-specifiers cell+ ] literal + ;
: element-bounds [ 0 elements-number cell+ ] literal + ;
[ 0 element-bounds cell+ ] constant /array
: upper-bound nop ;
: lower-bound [ 0 upper-bound 2 cells + ] literal + ;
[ 0 lower-bound 2 cells + ] constant /bound
: field-specifiers [ 0 template-kind cell+ ] literal + ;
: fields-number [ 0 field-specifiers cell+ ] literal + ;
: field-bounds [ 0 fields-number cell+ ] literal + ;
[ 0 field-bounds cell+ ] constant /fields
: field-offset nop ;
: field-template [ 0 field-offset cell+ ] literal + ;
[ 0 field-template cell+ ] constant /field

\ compact             template constructors                  lst
variable [template] variable field variable bound
: lambda here swap allot ;
[ ' lambda ' allocate-template defer! ]
: template /template allocate-template dup [template] ! dup
   postpone constant ! ; immediate
: more-fields dup /field * allocate-template dup field !
   [template] @ field-specifiers a! !+ dup !+ !+ 1 !a ;
: !field /field * field @ + a! swap !+ !a ;
: array-element [template] @ element-template a! !+ !a ;
: array-size [template] @ elements-number ! ;
: array-bounds dup /bound * allocate-template dup bound !
   [template] @ bound-specifiers a! !+ 0 !+ !a ;
: !upper /bound * bound @ + a! !+ !a ;
: !lower /bound * bound @ + 2 cells + a! !+ !a ;

\ compact             nil template                           lst
[ 0 ] constant nil-template
[ %pointer ] template reference-template

\ compact             pointer template                       lst
[ %record ] template pointer-template [ 2 more-fields ]
[ 0 reference-template 0 !field ]
[ 1 cells nil-template 1 !field ]

\ compact             array template                         lst
[ %record ] template bound-template [ 4 more-fields ]
[ 0 nil-template 0 !field ]
[ 1 cells nil-template 1 !field ]
[ 2 cells nil-template 2 !field ]
[ 3 cells nil-template 3 !field ]
[ %flexible ] template flexb-template
[ 4 cells bound-template array-element ]
[ 1 array-bounds 1 array-size ]
[ 0 1 0 !upper 0 1 0 !lower ]
[ %record ] template array-template [ 3 more-fields ]
[ 0 cells reference-template 0 !field ]
[ 1 cells nil-template 1 !field ]
[ 2 cells flexb-template 2 !field ]

\ compact             fields template                        lst
[ %record ] template .field-template [ 2 more-fields ]
[ 0 nil-template 0 !field ]
[ 1 cells reference-template 1 !field ]
[ %flexible ] template fields-template
[ 2 cells .field-template array-element ]
[ 1 array-bounds 1 array-size ]
[ 0 1 0 !upper 0 1 0 !lower ]

\ compact             reference template                     lst
[ %variants ] template template-template [ 7 more-fields ]
[ 0 nil-template 0 !field ]
[ 0 pointer-template 1 !field ]
[ 0 pointer-template 2 !field ]
[ 0 array-template 3 !field ]
[ 0 array-template 4 !field ]
[ 0 fields-template 5 !field ]
[ 0 fields-template 6 !field ]
[ /template reference-template object-size ! ]
[ template-template reference-template object-template ! ]

\ compact             unscanned queue                        lst
variable up [ sp @ up ! ]
: unscanned-mark sp @ ;
: unscanned-number sp @ cell+ ;
: unscanned-object sp @ 2 cells + ;
: unscanned-template sp @ 3 cells + ;
[ 4 cells ] constant /unscanned
: reset-unscanned sp @ up ! ;
: ?unscanned sp @ up @ <> ;
: +unscanned /unscanned +stack ;
: -unscanned /unscanned -stack ;
: put-unscanned +unscanned unscanned-mark a! 0 !+ !+ !+ !a ;
: put-pointer swap @ 1 put-unscanned ;
: put-fields >r swap r> put-unscanned ;

\ compact             mark                                   lst
: ?marked @corrected-pointer dup 0= if 2drop true ; then drop
   heap - 1 cells / ?on ;
: [mark] over @ over object-size @ map-width set-map
   put-pointer ;
: mark-pointer over ?marked if drop 2drop ; then drop [mark] ;
: mark-any over ?marked if drop 2drop ; then 2drop drop dup a!
   cell+ @a [mark] ;
: mark-array swap over elements-number @ put-unscanned ;
: mark-flexible over ?marked if drop 2drop ; then drop dup
   element-size @ >r swap a! @+ @a 2dup r> * map-width
   set-map put-unscanned ;
: mark-record dup fields-number @ put-fields ;
: mark-variants 1 put-fields ;
: [mark-object] dup template-kind @
   dup %pointer = if 2drop mark-pointer ; then drop
   dup %any = if 2drop mark-any ; then drop
   dup %array = if 2drop mark-array ; then drop
   dup %flexible = if 2drop mark-flexible ; then drop
   dup %record = if 2drop mark-record ; then drop
   %variants = if drop mark-variants ; then drop 2drop ;
: mark-object dup 0= if drop 2drop ; then drop [mark-object] ;
: @1+ @a dup 1+ !+ ;
: dereference-pointer swap drop object-template @ mark-object ;
: dereference-any 2drop dup 1 cells - @ mark-object ;
: next-element dup >r element-size @ * + r> element-template @
   mark-object ;
: next-field field-specifiers @ swap /field * + dup >r
   field-offset @ + r> field-template @ mark-object ;
: case-field >r drop dup a! cell+ @a r> next-field ;
: mark-next @+ swap @a dup template-kind @
   dup %pointer = if 2drop dereference-pointer ; then drop
   dup %any = if 2drop dereference-any ; then drop
   dup %array = if 2drop next-element ; then drop
   dup %flexible = if 2drop next-element ; then drop
   dup %record = if 2drop next-field ; then drop
   %variants = if drop case-field ; then drop 2drop ;
: mark-unscanned unscanned-mark a! @1+ @+ over > if drop
   mark-next ; then 2drop -unscanned ;
: mark begin ?unscanned while mark-unscanned repeat ;
: prepare-marking ['] @corrected-pointer defer! reset-map
   reset-unscanned fp @ ;
: mark-frame dup frame-template reference-template mark-object
   mark dup a! /frame + a frame-template @ mark-object mark ;
: lambda @ ;
: mark-root ['] lambda prepare-marking begin dup 0=
   0= while dup mark-frame previous-fp @ repeat drop ;

\ compact             build table                            lst
variable break-table variable break-entrys
variable break-point [ 1 cells allot ]
variable compacted variable uncompacted variable moved-units
: entrys 2 cells * ;
: reset-table heap dup compacted ! dup uncompacted !
   break-table ! 0 break-entrys ! ;
: +0bits begin dup /heap < over ?off and while +count repeat ;
: next-break +1bits +0bits dup cells heap + uncompacted ! ;
: save-point compacted @ uncompacted @ break-point a! !+ !a ;
: block-length +1bits dup cells heap + uncompacted @ - ;
: before-table break-table @ compacted @ - ;
: current-offset break-entrys @ entrys ;
: current-entry break-table @ current-offset + ;
: need-move uncompacted @ current-entry - break-entrys @ entrys
   min ;
: move-table break-table @ swap uncompacted @ over - swap 1
   cells / move ;
: update-table uncompacted @ current-offset - break-table ! ;
: roll-table need-move move-table update-table ;
: move-cells before-table min dup >r uncompacted @ compacted @
   r> 1 cells / move dup uncompacted +! dup compacted +! ;
: ?roll-table before-table over < if drop roll-table ; then
   drop ;
: move-block begin dup 0> while ?roll-table dup move-cells -
   repeat drop ;
: @break-point break-point a! @+ dup @a - ;
: add-entry @break-point swap current-entry a! !+ !a
   break-entrys a! @a 1+ !a ;
: +compacted 0 +1bits dup cells heap + compacted ! ;
: build-table reset-table +compacted begin next-break dup /heap
   < while save-point block-length move-block add-entry repeat
   drop compacted @ hp ! ;

\ compact             sort table                             lst
: parent 1- 2/ ;
: left-child 2* 1+ ;
: right-child 2* 2 + ;
: ?offset< >r entrys r@ + @ swap entrys r> + @ < ;
: ?swap-left >r 2dup r> ?offset< if 2drop dup ; then drop ;
: ?swap-right swap >r >r over 1+ r> > if drop r>drop ; then
   drop over 1+ over r> ?offset< if 2drop 1+ dup ; then drop ;
: swap+ >r a! @r @a swap !+ !r r> a ;
: swap-entry >r entrys over + swap r> entrys + swap+ swap+
   2drop ;
: sift-down >r begin dup left-child r@ <= while over >r dup
   left-child over r@ ?swap-left r> r@ ?swap-right >r drop dup
   r@ = if drop 2drop r>drop r>drop ; then drop over swap r@
   swap-entry r> repeat 2drop r>drop ;
: heapify 1- >r r@ parent begin dup 0 >= while 2dup r@
   sift-down 1- repeat 2drop r>drop ;
: heap-sort 2dup heapify 1- begin dup 0 > while 2dup 0
   swap-entry 1- >r dup 0 r@ sift-down r> repeat 2drop ;
: sort-table break-table @ break-entrys @ heap-sort ;

\ compact             search table                           lst
variable target
: ?found target @ >r dup @ r@ < swap 1 entrys + @ r> > and ;
: found >r 2drop r> dup ;
: narrow-down dup ?found if 2drop found ; then drop dup @
   target @ < if 2drop 1+ swap drop ; then drop @ target @ >
   if drop 1- >r swap drop r> swap ; then drop found ;
: binary-search swap >r 1- 0 begin 2dup > while 2dup + 2/ dup
   entrys r@ + narrow-down repeat drop r>drop ;
: search-table target ! break-table @ break-entrys @
   binary-search ;

\ compact             update                                 lst
: lambda >r @r dup 0= if drop r>drop ; then drop dup
   search-table entrys break-table @ + a! @+ @+ >r over <= if
   drop r> - dup r> ! ; then drop r>drop r>drop ;
: update-root break-entrys @ 0= if drop ; then drop
   ['] lambda prepare-marking begin dup 0= 0= while dup
   mark-frame previous-fp @ repeat drop ;

\ compact             garbage collector                      lst
: compact build-table sort-table ;
: lambda mark-root compact update-root ;
[ ' lambda ' collect-garbage defer! ]
