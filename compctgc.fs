\ gc               title                                09-08-20
\ name: compact garbage collector
\ release: 0.01
\ author: lst-lang <coldmoon@mailfence.com>
\ license: GNU GPL 3.0 or later











\ gc               template#1                           09-14-20
variable 'this-template 0 'this-template !

: this-template 'this-template @ ;
: template-kind this-template ;
: template-copying this-template 1 cells + ;
: /pointer 4 cells ;
: object-template this-template 1 cells + ;
: object-size this-template 2 cells + ;
: /array 5 cells ;
: element-template this-template 1 cells + ;
: element-size this-template 2 cells + ;
: element-number this-template 3 cells + ;



\ gc               template#2                           09-14-20
: record 3 cells + ;
: record-fields this-template 1 cells + ;
: tagfield 3 cells + ;
: tagfield-variants this-template 1 cells + ;
: /field 2 cells ;
: fields /field * ;
: field-offset fields this-template record-fields + ;
: field-template field-offset cell+ ;







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

: split-left dup 0= 0= if cell-bits swap - then left-bits ! ;
: split-middle left-bits @ - cell-bits /mod ;
: split-right right-bits ! middle-cells ! ;
: split swap split-left split-middle split-right ;
: bits-mask 0 invert swap for 2* next ;
: or-mask! over 0= if swap drop exit then a! @a or !+ a ;
: left-mask cell-bits @a - bits-mask ;
: set-left left-bits a! left-mask swap or-mask! ;
: !middles swap a! 0 invert swap for dup !+ next drop a ;
: set-middle middle-cells @ dup 0= if drop exit then !middles ;
: right-mask @a 0= if 0 exit then @a bits-mask invert ;
: set-right right-bits a! right-mask swap or-mask! ;

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




\ gc               runtime stack#1                      09-14-20
128 cells constant /stack
variable sp /stack allot dup sp ! constant stack
variable fp sp cell+ fp !
variable 'this-frame 0 'this-frame !

: this-frame 'this-frame @ ;
: /frame 3 cells ;
: frame-template this-frame ;
: frame-locals this-frame /frame + ;
: previous-sp this-frame 1 cells + ;
: previous-frame this-frame 2 cells + ;
: current-frame fp @ 'this-frame ! ;



\ gc               runtime stack#2                      09-14-20
: top sp a! @a over ;
: ?overflow top + stack /stack + > ;
: overflow true abort" STACK OVERFLOW" ;
: increase-sp @a swap over + !a ;
: enter-stack ?overflow if overflow then increase-sp ;
: ?underflow top - stack < ;
: underflow true abort" STACK UNDERFLOW" ;
: decrease-sp @a swap - dup !a ;
: leave-stack ?underflow if underflow then decrease-sp ;






\ gc               mark stack#1                         09-14-20
variable mp stack /stack + mp !
variable 'this-mark 0 'this-frame !

: this-mark 'this-mark @ ;
: /mark 4 cells ;
: mark-template this-mark ;
: mark-object this-mark /mark + ;
: mark-recurse this-mark /mark + ;
: current-mark mp @ 'this-mark ! ;






\ gc               mark stack#2                         09-14-20
: top mp a! @a /mark ;
: ?overflow sp @ top - > ;
: overflow true abort" MARK STACK OVERFLOW" ;
: decrease-mp /mark decrease-sp ;
: enter-mark ?overflow if overflow then decrease-mp ;
: ?underflow top + stack /stack + > ;
: underflow true abort" MARK STACK UNDERFLOW" ;
: increase-mp /mark increase-sp ;
: leave-mark ?underflow if underflow then increase-mp ;
: ?has-mark mp @ stack /stack + < ;





\ gc               storage#1                            09-13-20
: parcels 2 cells * ;
800 parcels constant /storage
/storage allot constant storage
/storage 1 parcels / bits bitmap constant 'storage-map
variable free 1 cells allot constant current

: storage-map 'storage-map 'this-bitmap ! ;
: ?cell-bound current @ cell-bits /mod swap drop 0= ;
: step-both free a! @a over + !+ @a + !a ;
: step-current current a! @a + !a ;
: ?bit-set current @ bits+ @bit 0= 0= ;
: walk-bit ?bit-set if 1 step-both exit then 1 step-current ;



\ gc               storage#2                            09-14-20
: cells-offset current @ cell-bits / ;
: cell-address 'storage-map cells-offset 1+ cells + ;
: ?cell-not-set cell-address @ dup a! 0= ;
: cell-current cell-bits step-current ;
: cell-both cell-bits step-both ;
: ?all-bits-set a invert 0= ;
: cell-set ?all-bits-set if cell-both exit then walk-bit ;
: walk-cell ?cell-not-set if cell-current exit then cell-set ;







\ gc               storage#3                            09-13-20
: round-units 1 parcels /mod 0> if 1+ then ;
: set-pointers 0 dup free ! current ! ;
: ?end-of-map current @ 800 < 0= ;
: ?found-enough dup current @ free @ - < ;
: ?walk ?end-of-map if false exit then ?found-enough 0= ;
: update-pointers ?cell-bound if walk-cell exit then walk-bit ;
: walk-map begin ?walk while update-pointers repeat ;
: no-memory true abort" NOT ENOUGH MEMORY" ;
: check-result ?found-enough if free @ exit then no-memory ;
: parcels-offset parcels storage + ;
: set-map >r r@ bits+ swap set-bits r> parcels-offset ;
: search-empty set-pointers walk-map check-result set-map ;
: allocate-units round-units storage-map search-empty ;
