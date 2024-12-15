!
! On the SPARC, %o0..%o5 are available.  %o6 is the stack pointer and
! %o7 is the return address.
! Some of the globals are also available.  %g0 is hardwired 0 and
! %g1 is definitely available.  %g2-%g4 appear to be, as well.
! Arguments are in %o0 upwards.  Return values are in %o0.
!
! The SPARC has delayed branches.  Unless the ,a option is added to an
! instruction, the instruction after a branch is executed.
!
! SPARC multipllies are variable-time in the second input
! (The one that can possibly be an immediate.)
! That should correspond to the "k" argument, %o3
!
.section	".text"

! BNWORD32
! bniMulAdd1_32(BNWORD32 *out, BNWORD32 const *in, unsigned len, BNWORD32 k)
!                        %o0                  %o1           %o2          %o3
! out += k * in, of length len, and returns the word of carry.
!
! Len is guaranteed greater than 0.
! %g1 is the current input word
! %g2 is the current output word
! %o4 is the current product word
! %o5 is the carry word
!
! For doing multiply-add, the (pre-v9) SPARC is a bit annoying because
! it only has one set of condition codes, which are needed both by the
! add-with-carry and the loop indexing.
!
! The basic sequence of the loop is:
! 1. Multiply k * *in++ -> high, low
! 2. Addxcc carry word and carry bit to low
! 3. Addx carry bit to high, producing carry word (can't overflow)
! 4. Addcc low to *out++
!
! Note that the carry bit set in step 4 is used in step 2.  The only place
! in this loop that the carry flag isn't in use is between steps 3 and 4,
! so we have to rotate the loop to place the loop indexing operations here.
! The loop above ignores the details of when to do loads, stores and the
! pointer increments, which have some flexibility, but should be scheduled
! to avoid stalls.
!
! The first iteration has no carry word in, so it requires only steps 1 and 4,
! and since we begin the loop with step 4, it boils down to just step 1
! followed by the loop indexing.
!
! This loop is unrolled once to simplify 


	.align 4
	.global bniMulAdd1_32
	.type	bniMulAdd1_32,#function
bniMulAdd1_32:
	ld	[%o1],%g1	! Fetch *in
	subcc	%o2,1,%o2
	umul	%g1,%o3,%o4
	rd	%y,%o5

	be	Lma_once
	ld	[%o0],%g2	! Fetch *out
	andcc	%o2,1,%g0
	ld	[%o1+4],%g1	! Fetch *in
	bne	Lma_loop1
	addcc	%g2,%o4,%o4	! Add to *out, setting carry bit

	sub	%o1,4,%o1
	sub	%o0,4,%o0

Lma_loop:
	st	%o4,[%o0+4]	! Store prev *out
	add	%o1,8,%o1
	umul	%g1,%o3,%o4	! Multiply, low half of product in %o4
	ld	[%o0+8],%g2	! Fetch *out
	ld	[%o1+4],%g1	! Fetch *in
	addxcc	%o5,%o4,%o4	! Add carry word and bit from last step
	rd	%y,%o5		! High half of product is new carry word
	addx	%g0,%o5,%o5	! Propagate carry bit

	add	%o0,8,%o0
	addcc	%g2,%o4,%o4	! Add to *out, setting carry bit

Lma_loop1:
	st	%o4,[%o0]	! Store prev *out
	umul	%g1,%o3,%o4	! Multiply, low half of product in %o4
	ld	[%o0+4],%g2	! Fetch *out
	ld	[%o1+8],%g1	! Fetch *in (may be redundant)
	addxcc	%o5,%o4,%o4	! Add carry word and bit from last step
	rd	%y,%o5		! High half of product is new carry word
	addx	%g0,%o5,%o5	! Propagate carry bit

	subcc	%o2,2,%o2	! Loop overhead
	bgu	Lma_loop
	addcc	%g2,%o4,%o4	! Add to *out, setting carry bit

!Lma_done:
	st	%o4,[%o0+4]	! Store *out
	retl
	addx	%g0,%o5,%o0	! Compute carry word to return

Lma_once:
	addcc	%g2,%o4,%o4	! Add to *out, setting carry bit
	st	%o4,[%o0]	! Store *out
	retl
	addx	%g0,%o5,%o0	! Compute carry word to return

ma32_end:
	.size	 bniMulAdd1_32,ma32_end-bniMulAdd1_32


! BNWORD32
! bniMulSub1_32(BNWORD32 *out, BNWORD32 const *in, unsigned len, BNWORD32 k)
! out -= k * in, of length len, and returns the word of carry.
! This is similar, but we subtract.  The SPARc uses a borrow bit,
! so we can add the borrow from a subtract to the carry word.
!
	.align 4
	.global bniMulSub1_32
	.type	bniMulSub1_32,#function

bniMulSub1_32:
	ld	[%o1],%g1	! Fetch *in
	umul	%g1,%o3,%o4
	rd	%y,%o5

	subcc	%o2,1,%o2
	ld	[%o0],%g2	! Fetch *out
	be	Lms_once
	andcc	%o2,1,%g0
	bne	Lms_loop1
	subcc	%g2,%o4,%o4	! Subtact from *out, setting carry bit

Lms_loop:
	ld	[%o1+4],%g1	! Fetch *in
	st	%o4,[%o0]	! Store prev *out
	add	%o1,4,%o1
	umul	%g1,%o3,%o4	! Multiply, low half of product in %o4
	addxcc	%o5,%o4,%o4	! Add carry word and bit from last step
	rd	%y,%o5		! High half of product is new carry word
	addx	%g0,%o5,%o5	! Propagate carry bit

	ld	[%o0+4],%g2	! Fetch *out
	add	%o0,4,%o0
	subcc	%g2,%o4,%o4	! Subtact from *out, setting carry bit

Lms_loop1:
	ld	[%o1+4],%g1	! Fetch *in
	st	%o4,[%o0]	! Store prev *out
	add	%o1,4,%o1
	umul	%g1,%o3,%o4	! Multiply, low half of product in %o4
	addxcc	%o5,%o4,%o4	! Add carry word and bit from last step
	rd	%y,%o5		! High half of product is new carry word
	addx	%g0,%o5,%o5	! Propagate carry bit

	subcc	%o2,2,%o2	! Loop overhead
	ld	[%o0+4],%g2	! Fetch *out
	add	%o0,4,%o0
	bgu	Lms_loop
	subcc	%g2,%o4,%o4	! Subtract from *out, setting carry bit

!Lms_done:
	st	%o4,[%o0]	! Store *out
	retl
	addx	%g0,%o5,%o0	! Compute carry word to return

Lms_once:
	subcc	%g2,%o4,%o4	! Add to *out, setting carry bit
	st	%o4,[%o0]	! Store *out
	retl
	addx	%g0,%o5,%o0	! Compute carry word to return

ms_end:
	.size	 bniMulSub1_32,ms_end-bniMulSub1_32

! void
! bniMulN1_32(BNWORD32 *out, BNWORD32 const *in, unsigned len, BNWORD32 k)
!                       %o0                  %o1          %o2           %o3
! out = k * in.  The carry word is stored.
! This is similar to the previous cases, but simpler.
! If we didn't have to use the carry flag to do the loop overhead,
! we could just re-use the carry around the loop.
	.align 4
	.global bniMulN1_32
	.type	bniMulN1_32,#function
bniMulN1_32:
	ld	[%o1],%g1	! Fetch *in
	umul	%g1,%o3,%o4
	rd	%y,%o5

	subcc	%o2,1,%o2	! Loop overhead
	be	Lm_done
	st	%o4,[%o0]	! Store *out

Lm_loop:
	ld	[%o1+4],%g1	! Fetch *in
	add	%o1,4,%o1
	add	%o0,4,%o0
	umul	%g1,%o3,%o4	! Multiply, low half of product in %o4
	addcc	%o5,%o4,%o4	! Add carry word from last step
	rd	%y,%o5		! High half of product is new carry word
!
!	st	%o4,[%o0]	! Store *out
!Lm_loop1:
!	ld	[%o1+4],%g1	! Fetch *in
!	add	%o1,4,%o1
!	add	%o0,4,%o0
!	umul	%g1,%o3,%o4	! Multiply, low half of product in %o4
!	addxcc	%o5,%o4,%o4	! Add carry bit and word from last step
!	rd	%y,%o5		! High half of product is new carry word
	addx	%g0,%o5,%o5	! Propagate carry bit

!	subcc	%o2,2,%o2	! Loop overhead
	subcc	%o2,1,%o2	! Loop overhead
	bgu	Lm_loop
	st	%o4,[%o0]	! Store *out

Lm_done:
	retl
	st	%o5,[%o0+4]	! Store carry word
m_end:
	.size	 bniMulN1_32,m_end-bniMulN1_32
