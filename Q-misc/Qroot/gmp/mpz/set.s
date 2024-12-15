gcc2_compiled.:
___gnu_compiled_c:
.text
	.align 4
	.global _mpz_set
	.proc	020
_mpz_set:
	!#PROLOGUE# 0
	save %sp,-112,%sp
	!#PROLOGUE# 1
	ld [%i1+4],%l1
	cmp %l1,0
	bge L45
	mov %l1,%l0
	sub %g0,%l1,%l0
L45:
	ld [%i0],%o0
	cmp %o0,%l0
	bge,a L54
	ld [%i0+8],%o3
	mov %i0,%o0
	call __mpz_realloc,0
	mov %l0,%o1
	ld [%i0+8],%o3
L54:
	cmp %l0,0
	ble L48
	ld [%i1+8],%i1
	mov 0,%o2
	sll %l0,2,%o1
	ld [%o2+%i1],%o0
L55:
	st %o0,[%o2+%o3]
	add %o2,4,%o2
	cmp %o2,%o1
	bl,a L55
	ld [%o2+%i1],%o0
L48:
	st %l1,[%i0+4]
	ret
	restore
