\ gc               title                                09-08-20
\ name: compact garbage collector
\ release: 0.01
\ author: lst-lang <coldmoon@mailfence.com>
\ license: GNU GPL 3.0 or later











\ gc               bitmap#1                             09-13-20
1 cells 8 * constant cell-bits variable 'this-bitmap
variable left-bits variable middle-cells variable right-bits

: this-bitmap 'this-bitmap @ ;
: bits cell-bits /mod ;
: bitmap 0> if 1+ then dup 1+ cells allot swap over ! ;
: clear-bitmap this-bitmap a! @+ for 0 !+ next ;
: bits+ bits swap 1+ cells this-bitmap + a! ;
: bit-mask dup 0= if 1+ exit then 1 swap lshift ;
: set-bit bit-mask @a or !a ;
: @bit bit-mask @a and 0= 1+ ;




\ gc               bitmap#2                             09-13-20
: ?cross-cell 2dup + cell-bits > ;
: split-left dup 0= 0= if cell-bits swap - then left-bits ! ;
: split-middle left-bits @ - cell-bits /mod ;
: split-right right-bits ! middle-cells ! ;
: split swap split-left split-middle split-right ;
: bits-mask 0 invert swap lshift ;
: left-mask cell-bits @a - bits-mask ;
: or-mask! over 0= if swap drop exit then a! @a or !+ a ;
: set-left left-bits a! left-mask swap or-mask! ;
: !middles swap a! 0 invert swap for dup !+ next drop a ;
: set-middle middle-cells @ dup 0= if drop exit then !middles ;
: right-mask @a 0= if 0 exit then @a bits-mask invert ;
: set-right right-bits a! right-mask swap or-mask! ;
: set-slices set-left set-middle set-right ;

\ gc               bitmap#3                             09-13-20
: split-set a >r split r> set-slices drop ;
: loop-set for dup set-bit 1+ next drop ;
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
variable fp sp cell+ fp ! variable 'this-frame

: /frame 3 cells ;
: this-frame 'this-frame @ ;
: this-locals this-frame /frame + ;
: frame-template this-frame ;
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






\ gc               storage#1                            09-13-20
: parcels 2 cells * ;
800 parcels constant /storage
/storage allot constant storage
/storage 1 parcels / bits bitmap constant 'storage-map
variable free 1 cells allot constant current

: storage-map 'storage-map 'this-bitmap ! ;
: round-units 1 parcels /mod 0> if 1+ then ;
: set-walkers 0 dup free ! current ! ;
: no-memory true abort" NOT ENOUGH MEMORY" ;
: ?end-of-map current @ 800 < 0= ;
: ?found-enough dup current @ free @ - < ;
: ?walk ?end-of-map if false exit then ?found-enough 0= ;


\ gc               storage#2                            09-13-20
: ?cell-bound current @ cell-bits /mod swap drop 0= ;
: ?bit-set current @ bits+ @bit 0= 0= ;
: step-both free a! @a over + !+ @a + !a ;
: step-current current a! @a + !a ;
: walk-bit ?bit-set if 1 step-both exit then 1 step-current ;
: cells-offset current @ cell-bits / ;
: cell-address 'storage-map cells-offset 1+ cells + ;
: ?cell-not-set cell-address @ dup a! 0= ;
: cell-current cell-bits step-current ;
: cell-both cell-bits step-both ;





\ gc               storage#3                            09-13-20
: ?all-bits-set a invert 0= ;
: cell-set ?all-bits-set if cell-both exit then walk-bit ;
: walk-cell ?cell-not-set if cell-current exit then cell-set ;
: update-walkers ?cell-bound if walk-cell exit then walk-bit ;
: walk-map begin ?walk while update-walkers repeat ;
: check-result ?found-enough if free @ exit then no-memory ;
: set-map >r r@ bits+ swap set-bits r> parcels storage + ;
: search-empty set-walkers walk-map check-result set-map ;
: allocate round-units storage-map search-empty ;






\ gc               mark stack#1                         09-14-20
variable mp stack /stack + mp ! variable 'this-mark

: /mark 3 cells ;
: this-mark 'this-mark @ ;
: marking-template this-mark ;
: marking-locals this-mark 1 cells + ;
: marking-recurse this-mark 2 cells + ;
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






\ gc               template#1                           09-14-20
variable 'this-template

: this-template 'this-template @ ;
: template-kind this-template ;
: template-marking this-template 1 cells + ;
: /pointer 4 cells ;
: object-template this-template 2 cells + ;
: object-size this-template 3 cells + ;
: /array 5 cells ;
: element-template this-template 2 cells + ;
: element-size this-template 3 cells + ;
: element-number this-template 4 cells + ;



\ gc               template#2                           09-14-20
: record 3 cells + ;
: fields-number this-template 2 cells + ;
: tagfield 3 cells + ;
: variants-number this-template 2 cells + ;
: /field 2 cells ;
: fields /field * ;
: field-offset fields this-template + 3 cells + ;
: field-template field-offset cell+ ;







\ gc               mark phase#1                         09-15-20
: parcels-offset marking-locals @ @ storage - 1 parcels / ;
: ?null marking-locals @ @ 0= ;
: ?marked parcels-offset dup bits+ @bit swap a! ;
: ?unmarked ?null if false exit then ?marked 0= ;
: set-map a bits+ object-size @ 1 parcels / set-bits ;
: this-template! 'this-template ! ;
: parcels-template object-template @ this-template! ;
: @pointer parcels-template marking-locals @ ;
: ?recurse template-marking a! @a 0= 0= ;
: reversal-marking @a 'this-mark ! marking-locals @ ;





\ gc               mark phase#2                         09-15-20
: swap-object over @ marking-locals ! ;
: swap-recurse swap a! marking-recurse >r @r !a r> ! ;
: pointer-reversal swap-object swap-recurse ;
: end-pointer leave-mark drop current-mark ;
: reversal reversal-marking pointer-reversal end-pointer ;
: !mark a! !+ !+ 0 !a current-mark ;
: overwrite-marking this-mark dup template-marking ! ;
: object-mark @ this-template overwrite-marking !mark ;
: push-object ?recurse if reversal exit then object-mark ;
: mark-object set-map @pointer push-object ;
: mark-pointer ?unmarked if mark-object exit then end-pointer ;




\ gc               mark phase#3                         09-15-20
: ?element marking-locals @ a! @+ dup @a < ;
: increase-counter dup 1+ marking-locals @ ! ;
: next-offset element-size @ * 2 cells + ;
: @element next-offset marking-locals @ + ;
: next-object increase-counter @element ;
: next-template element-template @ this-template! ;
: push-mark this-template enter-mark !mark ;
: next-element next-object next-template push-mark ;
: end-array drop end-pointer ;
: mark-array ?element if next-element exit then end-array ;





\ gc               mark phase#4                         09-15-20
: ?field marking-locals @ @ dup fields-number @ < ;
: @field dup field-offset @ marking-locals @ + ;
: next-object increase-counter @field ;
: next-template swap field-template @ this-template! ;
: next-field next-object next-template push-mark ;
: ?recursing marking-recurse @ dup 0= 0= ;
: parent-pointer dup @ 1- field-offset @ + ;
: parent-recurse parent-pointer @ marking-recurse ! ;
: parent-locals dup marking-locals ! ;
: parent parent-locals parent-recurse ;





\ gc               mark phase#5                         09-15-20
: current-template marking-template @ this-template! ;
: clear-marking current-template 0 template-marking ! ;
: end-recurse clear-marking end-array ;
: end-record drop ?recursing if parent exit then end-recurse ;
: mark-record ?field if next-field exit then end-record ;

: @tag marking-locals @ dup @ ;
: tagfield-template @tag field-template @ marking-template ! ;
: tagfield-object cell+ marking-locals ! ;
: mark-tagfield tagfield-template tagfield-object ;





\ gc               mark phase#6                         09-15-20
variable markers 3 cells allot drop
: >> markers a! ['] mark-pointer !+ ['] mark-array !+ a ;
: >>> a! ['] mark-record !+ ['] mark-tagfield !+ ; >> >>>

: marker template-kind @ cells markers + @ ;
: call-marker current-template marker execute ;









\ gc               mark phase#7                         09-18-20
: ?frame current-frame 0= 0= ;
: @frame frame-template @ this-template! this-locals ;
: push-frame @frame !mark previous-frame ;
: ?has-mark mp @ stack /stack + < ;
: start-marking begin ?has-mark while call-marker repeat ;
: mark-frame push-frame current-mark start-marking ;
: mark-root begin ?frame while mark-frame repeat ;
: mark current-frame mark-root ;







\ gc               compaction#1                         09-18-20
variable break-table variable break-entrys
variable break-point 1 cells allot drop
variable compacted variable uncompacted variable moved-units
storage dup compacted ! uncompacted !

: set-table storage break-table ! 0 break-entrys ! ;
: ?object ?end-of-map if false exit then ?bit-set ;
: walk-bit 1 step-current ;
: cell-set ?all-bits-set if cell-current exit then walk-bit ;
: walk-cell ?cell-not-set if cell-current exit then cell-set ;
: update-walkers ?cell-bound if walk-cell exit then walk-bit ;




\ gc               compaction#2                         09-18-20
: next-gap begin ?object while update-walkers repeat ;
: ?gap ?end-of-map if false exit then ?bit-set 0= ;
: next-object begin ?gap while update-walkers repeat ;
: new-uncompacted current @ parcels storage + ;
: reset-uncompacted new-uncompacted uncompacted ! ;
: next-break next-gap next-object reset-uncompacted ;
: ?uncompacted next-break ?end-of-map 0= ;
: live-pointer current @ parcels storage + ;
: block-length next-gap live-pointer uncompacted @ - ;
: before-table break-table @ compacted @ - ;
: ?roll-table before-table over < ;
: current-offset break-entrys @ parcels ;
: current-entry break-table @ current-offset + ;


\ gc               compaction#3                         09-18-20
: after-table uncompacted @ current-entry - ;
: need-move after-table break-entrys @ parcels min ;
: source break-table @ ;
: destination uncompacted @ over - ;
: move-table source swap destination swap 1 cells / move ;
: update-table uncompacted @ current-offset - break-table ! ;
: roll-table need-move move-table update-table ;
: roll-cells dup before-table dup 1 cells / min ;
: move-cells >r uncompacted @ compacted @ r> move ;
: update-uncompacted uncompacted a! @a over + !a ;
: update-compacted compacted a! @a over + !a ;
: update-state update-uncompacted update-compacted ;



\ gc               compaction#4                         09-18-20
: move-parcels roll-cells move-cells update-state ;
: roll-move ?roll-table if roll-table then move-parcels ;
: move-block begin dup 0> while roll-move - repeat drop ;
: @break-point break-point a! @+ dup @a - ;
: add-entry @break-point swap current-entry a! !+ !a ;
: increase-counter break-entrys a! @a 1+ !a ;
: after-move add-entry increase-counter ;
: space-state compacted @ uncompacted @ ;
: save-point space-state break-point a! !+ !a ;
: build save-point block-length move-block after-move ;
: build-table begin ?uncompacted while build repeat ;




\ gc               compaction#5                         09-18-20
variable root variable child variable end

: ?need-sort break-entrys a! @a 1 > ;
: last-parent 1- dup end ! 1- 2/ ;
: left 2* 1+ dup child ! ;
: ?has-child root @ left end @ > 0= ;
: entry parcels break-table @ + ;
: less-than swap entry @ swap entry @ < ;
: ?left-greater root @ child @ 2dup less-than ;
: left-max ?left-greater if swap then drop ;
: ?has-right child @ 1+ dup end @ > 0= ;
: ?right-greater ?has-right if 2dup less-than exit then false ;
: right-max ?right-greater if swap then drop ;


\ gc               compaction#6                         09-18-20
: ?need-swap dup root @ = 0= ;
: two-values 2dup >r >r >r a! @+ @+ r> a! @+ @+ r> r> ;
: swap! >r a! swap !+ !a swap !r !r r> drop ;
: swap-entry entry swap entry two-values swap! ;
: swap-root root @ over swap-entry root ! false ;
: swap-parent ?need-swap if swap-root exit then drop true ;
: down-child left-max right-max swap-parent ;
: root-largest ?has-child if down-child exit then true ;
: sift-down root ! begin root-largest until ;





\ gc               compaction#7                         09-18-20
: heapify last-parent 1+ for r@ 1- sift-down next ;
: sort dup 0 swap-entry 1- end ! 0 sift-down ;
: heapsort heapify end @ for r@ sort next ;
: sort-table ?need-sort if @a heapsort then ;
: table-compact set-table build-table sort-table ;
: compact set-walkers table-compact ;










\ gc               collection                           09-15-20
: collect storage-map clear-bitmap mark compact ;














