!
! SPARC assembler primitives for rsalib.c
! Taken from sparc.s by Branko Lankester, lankeste@fwi.uva.nl   91/08/29
! updated for SunOS 5.x by Philip C. Kizer, pckizer@tamu.edu    94/01/06
!   according to _Assm._Lang._Ref._Man._for_SPARC, p. 31
!
! For those that care, here are the rules from the manual:
!   o Remove leading underscore from symbol names
!   o Prefix local names with a dot (.).
!   o Change the pseudo-op <.seg> to <.section>
! Trivial, huh?
!
! Tested with gcc 2.5, and SunOS cc 2.0.1
!
! other sources must be compiled with UNIT32 and HIGHFIRST defined
!
	.section	".text"
        .proc   4
        .global P_SETP
P_SETP:
	retl
	nop


        .proc   4
        .global P_ADDC
P_ADDC:
	sethi	%hi(global_precision), %o3
	ldsh	[%o3+%lo(global_precision)], %o3
	and	%o2, 1, %o2
	dec	%o3
	sll	%o3, 2, %o3
	ld	[%o0+%o3], %o4
add_loop:
	ld	[%o1+%o3], %o5
	subcc	%g0, %o2, %g0
	addxcc	%o4, %o5, %o4
	addx	%g0, %g0, %o2
	st	%o4, [%o0+%o3]
	subcc	%o3, 4, %o3
	bge,a	add_loop
	ld	[%o0+%o3], %o4
	retl
	mov	%o2, %o0		! return carry


        .proc   4
        .global P_SUBB
P_SUBB:
	sethi	%hi(global_precision), %o3
	ldsh	[%o3+%lo(global_precision)], %o3
	and	%o2, 1, %o2
	dec	%o3
	sll	%o3, 2, %o3
	ld	[%o0+%o3], %o4
sub_loop:
	ld	[%o1+%o3], %o5
	subcc	%g0, %o2, %g0
	subxcc	%o4, %o5, %o4
	addx	%g0, %g0, %o2
	st	%o4, [%o0+%o3]
	subcc	%o3, 4, %o3
	bge,a	sub_loop
	ld	[%o0+%o3], %o4
	retl
	mov	%o2, %o0


        .proc   4
        .global P_ROTL
P_ROTL:
	sethi	%hi(global_precision), %o3
	ldsh	[%o3+%lo(global_precision)], %o3
	and	%o1, 1, %o1
	dec	%o3
	sll	%o3, 2, %o3
	ld	[%o0+%o3], %o4
rot_loop:
	sll	%o4, 1, %o5
	or	%o5, %o1, %o5
	st	%o5, [%o0+%o3]
	srl	%o4, 31, %o1
	subcc	%o3, 4, %o3
	bge,a	rot_loop
	ld	[%o0+%o3], %o4
	retl
	mov	%o1, %o0

	.section	".data"

