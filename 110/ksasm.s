.text
.align 16

.macro  ONE_UPDATE box1,box2
	movl (4*\box1)(%edi),%eax
	movl (4*\box2)(%edx),%ebx
	xorl %eax,(4*\box1)(%esi)
	xorl %ebx,(4*\box2)(%ecx)
	.endm

.macro SCHED_UPDATE_ZERO
	ONE_UPDATE 1,16
	ONE_UPDATE 2,20
	ONE_UPDATE 4,23
	ONE_UPDATE 6,25
	ONE_UPDATE 11,27
	ONE_UPDATE 12,28
	movl  60(%edi),%eax
	xorl  %eax,60(%esi)		
.endm
	
.macro SCHED_UPDATE_ONE
	ONE_UPDATE 1,17
	ONE_UPDATE 3,18
	ONE_UPDATE 7,21
	ONE_UPDATE 8,22
	ONE_UPDATE 10,25
	ONE_UPDATE 13,28
	movl 56(%edi),%eax
	xorl %eax,56(%esi)
.endm

.macro SCHED_UPDATE_TWO
	ONE_UPDATE 0,15
	ONE_UPDATE 2,16
	ONE_UPDATE 5,18
	ONE_UPDATE 7,23
	ONE_UPDATE 9,24
	ONE_UPDATE 10,27
	ONE_UPDATE 13,28
.endm

.macro SCHED_UPDATE_OTHER
	ONE_UPDATE 0,14
	ONE_UPDATE 2,17
	ONE_UPDATE 3,18
	ONE_UPDATE 4,20
	ONE_UPDATE 5,21
	ONE_UPDATE 6,22
	ONE_UPDATE 7,23
	ONE_UPDATE 8,24
	ONE_UPDATE 9,25
	ONE_UPDATE 10,26
	ONE_UPDATE 11,27
	ONE_UPDATE 13,28
.endm
	
.globl sched_update
.type sched_update,@function
/* sched_update(unsigned char *ks, unsigned char *ksmod,grayval) */
sched_update:
	pushl %ebp
	pushl %ebx
	pushl %esi
	pushl %edi
	
	movl 28(%esp),%eax			/* eax = gray8[jj] */
	movl 20(%esp),%esi			/* esi = lp */
	movl 24(%esp),%edi			/* edi = lp2 */
	movl %esi,%ecx				/* ecx = lp */
	movl %edi,%edx				/* edx = lp2 */
	
	orl  %eax,%eax				/* Check the value of gray8[jj] */
	jnz  Not_Level_Zero
	SCHED_UPDATE_ZERO
	jmp  OUT
Not_Level_Zero:	
	cmp  $1,%eax				/* Check if we are on bit 1*/
	jne  Not_Level_One
	SCHED_UPDATE_ONE
	jmp  OUT
Not_Level_One:	
	cmp  $2,%eax				/* Check if we are on bit 2*/
	jne  Not_Level_Two
	SCHED_UPDATE_TWO
	jmp OUT

Not_Level_Two:	
	SCHED_UPDATE_OTHER
				
OUT:	
	popl %edi
	popl %esi
	popl %ebx
	popl %ebp
	ret
.sched_update_end:
	.size sched_update,.sched_update_end-sched_update
