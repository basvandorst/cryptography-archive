        .seg "text"
        .global _addll,_subll,_addllx,_subllx,_shiftl,_shiftlr,_bfffo
        .global _mulll,_overflow,_hiremainder,_addmul,_divll

_addll: sethi   %hi(_overflow),%o3
        addcc   %o0,%o1,%o0
        addx    %g0,%g0,%o2
        retl
        st      %o2,[%o3+%lo(_overflow)]
_subll: sethi   %hi(_overflow),%o3
        subcc   %o0,%o1,%o0
        addx    %g0,%g0,%o2
        retl
        st      %o2,[%o3+%lo(_overflow)]
_addllx: sethi  %hi(_overflow),%o3
        ld      [%o3+%lo(_overflow)],%o2
        subcc   %g0,%o2,%g0
        addxcc  %o0,%o1,%o0
        addx    %g0,%g0,%o2
        retl
        st      %o2,[%o3+%lo(_overflow)]
_subllx: sethi  %hi(_overflow),%o3
        ld      [%o3+%lo(_overflow)],%o2
        subcc   %g0,%o2,%g0
        subxcc  %o0,%o1,%o0
        addx    %g0,%g0,%o2
        retl
        st      %o2,[%o3+%lo(_overflow)]
_shiftl: sethi  %hi(_hiremainder),%o3
        neg     %o1,%o4
        srl     %o0,%o4,%o2
        st      %o2,[%o3+%lo(_hiremainder)]
        retl
        sll     %o0,%o1,%o0
_shiftlr: sethi %hi(_hiremainder),%o3
        neg     %o1,%o4
        sll     %o0,%o4,%o2
        st      %o2,[%o3+%lo(_hiremainder)]
        retl
        srl     %o0,%o1,%o0

_bfffo: sethi   %hi(0xffff0000),%o1
        andcc   %o1,%o0,%g0
        bnz,a   1f
        clr     %o2
        sll     %o0,16,%o0
        mov     16,%o2
1:      sethi   %hi(0xff000000),%o1
        andcc   %o1,%o0,%g0
        bnz     2f
        sethi   %hi(0xf0000000),%o1
        sll     %o0,8,%o0
        add     %o2,8,%o2
2:      andcc   %o1,%o0,%g0
        bnz,a   3f
        srl     %o0,28,%o0
        add     %o2,4,%o2
        srl     %o0,24,%o0
3:      set     _tabshi,%o3
	sll	%o0,2,%o0
        ld      [%o3+%o0],%o1
        retl
        add     %o2,%o1,%o0

_mulll: sethi   %hi(_hiremainder),%o3
	umul	%o0,%o1,%o0
	rd	%y,%o2
        retl
	st      %o2,[%o3+%lo(_hiremainder)]

_addmul: sethi  %hi(_hiremainder),%o3
	ld	[%o3+%lo(_hiremainder)],%o2
	umul	%o0,%o1,%o0
	rd	%y,%o4
        addcc   %o0,%o2,%o0
        addx    %g0,%o4,%o4
        retl
	st      %o4,[%o3+%lo(_hiremainder)]

_divll: sethi  %hi(_hiremainder),%o4
        ld      [%o4+%lo(_hiremainder)],%o2
        mov     %o0,%o3
        wr      %o2,%g0,%y
        udivcc  %o0,%o1,%o0
        bvc     1f
        umul    %o0,%o1,%o5
        mov     0x2f,%o0
        call    _err,1
        nop
1:      subcc   %o3,%o5,%o2
        retl
        st      %o2,[%o4+%lo(_hiremainder)]

/* _divll: sethi  %hi(_hiremainder),%o4
*	ld	[%o4+%lo(_hiremainder)],%o2
*	wr	%o2,%g0,%y
*	udivcc	%o0,%o1,%o0
*	bvc	1f
*	rd	%y,%o2
*	mov	0x2f,%o0
*	call	_err,1
*	nop
* 1:	retl
*	st	%o2,[%o4+%lo(_hiremainder)]
*/
 
       .seg    "data"
        .align  4
_tabshi: .word  4,3,2,2,1,1,1,1,0,0,0,0,0,0,0,0

	.seg    "bss"
	.align  4
_hiremainder: .skip  4
_overflow: .skip  4

