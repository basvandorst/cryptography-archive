	TITLE	setenv - 12.29.93  Large Model
	.MODEL large
setenv_TEXT   SEGMENT WORD PUBLIC 'CODE'
	ASSUME	CS:setenv_TEXT,DS:setenv_TEXT
;
;setenv(char * varname,char * strint)
;
command db	356 dup (?)
set	db	"set "
CR	equ	0dh
EnvPtr	EQU	2CH	; Offset in PSP
ComInt	EQU	2EH	; entry point into first Command.Com
			; through interpreter

SegPtr	EQU	ComInt*4 + 2
sssav	dw	0
spsav	dw	0

s_path		dw	05h		; Size of Path.
d_path		db	"PATH="
s_comspec	dw	08h		; Size of Comspec=.
d_comspec	db	"COMSPEC="
s_prompt	dw	07h		; Size of Prompt=.
d_prompt	db	"PROMPT="
s_setenv	dw	07h		; Size of Setenv=.
d_setenv	db	"SETENV="
s_config	dw	07h		; Size of Config=.
d_config	db	"CONFIG="
equals		equ	$-1

szdat	dw	0	;Size of environment data.
szvar	dw	0	;Environment variable size.

zero	dw	0	;constant

exit_code	db	00h		; Normal return
setok		db	00h		; set NZ on some OK set 1.3b

envseg dw      0       ;Environment segment address.
envsz	dw	0	;Environment size in bytes.

get4d_err	db	"Error - can't find/recognize environment space.$"

msg00		db	"$"
msg01		db	"$"
msg02		db	"environment variable erased$"
msg03		db	"environment variable created$"
msg04		db	"$"
msg05		db	"$"
msg06		db	"$"
msg07		db	"$"
msg08		db	"$"
msg09		db	"$"
msg10		db	"environment space full$"
msgtab		dw	msg00
		dw	msg01
		dw	msg02
		dw	msg03
		dw	msg04
		dw	msg05
		dw	msg06
		dw	msg07
		dw	msg08
		dw	msg09
		dw	msg10

	public	_setenv
_setenv proc far
	push	bp
	mov	bp,sp
	push	si
	push	di
	push	es
	push	ds

	push	cs
	pop	ds

	mov	setok,0
	les	di,ss:[bp+6]	;char * varname
	mov	bx,di
	mov	cx,255
	mov	al,0
	cld
	repne scasb
	sub	di,bx
	dec	di
	mov	szvar,di	;size of env var
	les	di,ss:[bp+10]	 ;char * string
	mov	bx,di
	mov	cx,255
	mov	al,0
	cld
	repne scasb
	sub	di,bx
	dec	di
	mov	szdat,di	;size of data var

	xor   si,si		;Point to segment 0
	mov   es,si
	mov   si, word ptr es:[SegPtr]
	mov   ax,si
	dec   ax
	mov   es,ax


get4d:
;	find memory descriptor + PSP
	cmp byte ptr es:0,4dh
	jnz inc_seg
	cmp word ptr es:10h,20cdh
	jz  got4d

inc_seg:
	mov ax, es		; Increment
	inc ax			;   to the next
	mov es, ax		;      segment boundary.

;	limit search
	mov bx,cs
	cmp ax, bx		; Have we gone passed current PSP?
	jbe get4d		; No. Continue.
	push cs
	pop  ds
	cmp  envseg,0
	jnz  nor_ts
	mov  si,offset get4d_err
	jmp  exitmsg
nor_ts:
	jmp  nor_term		 ; Exit.


got4d:
	mov ax,es:EnvPtr+10h	; Offset in PSP - Environment
	or ax,ax
	jz inc_seg
	push es
	dec ax
	mov es,ax
	cmp byte ptr es:0,4dh
	jnz nxt_env
	mov ah, 00h
	mov al, es:3		;Get the number of segments.
	mov bx, 10h
	mul bx			;Get the number of bytes.
	mov cs:envsz, ax	;Save it.

	mov ax, es		;Increment
	inc ax			;  to the environment
	mov es, ax		;     segment boundary.

;	Just a double check for environment space.

	xor di,di
	mov si, offset d_path	; Check
	mov cx, s_path		;   for
	rep cmpsb		;     PATH=.
	jz got_env

	xor di,di
	mov si, offset d_prompt ; Check
	mov cx, s_prompt	;   for
	rep cmpsb		;     PROMPT=.
	jz got_env

	xor di,di
	mov si, offset d_comspec ;Check
	mov cx, s_comspec	;    for
	rep cmpsb		;      COMSPEC=.
	jz got_env

	xor di,di
	mov si, offset d_setenv ; Check
	mov cx, s_setenv	;   for
	rep cmpsb		;     SETENV=.
	jz got_env

	xor di,di
	mov si, offset d_config ; Check
	mov cx, s_config	;   for
	rep cmpsb		;     CONFIG=.
	jz got_env

	pop es
	jmp inc_seg		;	to scan some more.

got_env:
	mov ax,es		;don't change same env twice
	cmp cs:envseg,ax       ; may be pointed to by >1 PSP
	je  nxt_env
	mov cs:envseg,es
	call set_env
nxt_env:
	pop es
	jmp inc_seg

nor_term:
	push cs
	pop  ds
	mov al,0
	cmp setok,0
	jne nort1
	mov al, exit_code
	jmp short nort2
nort1:
	mov exit_code,0
nort2:
	mov ah,0
	add ax,ax		; index to msg table
	mov si, offset msgtab
	add si,ax
	mov dx,0[si]
exitmsg:
	mov ah, 09h		; display exit msg
	int 21h
	mov al, exit_code	; Set termination code.
	cbw
comrtn:
	pop	ds
	pop	es
	pop	di
	pop	si
	pop	bp
	ret
_setenv endp

set_env proc  near
	mov ax, es		; environment to
	mov ds, ax		;   data segment
	xor si,si		; environment offset

	les	di,ss:[bp+6]	;char * varname
	mov bl, es:[di] 	;1st char of environment variable.
	jmp short zero1


;	Scan through environment for the end,
;	two null bytes.
lp1:
	lodsb			; Get a byte.
	cmp al, 00h		; Is it null?
	jz zero1		; Yes. Got first null.
	jmp lp1 		; Keep looking.

;	Is the variable there?
zero1:
	lodsb			; Get a byte.
	cmp al, 00h		; Is it zero?
	jz write_near		; Yes.	Jump out.
	cmp al, bl		; Maybe the env. variable?
	jnz lp1 		; No.  Jump out.

	dec si			; We are N+1.
	mov ax, ds		; Get the data segment
	mov es, ax		; and put it in extra segment.
	mov di, si		; This is our destination.
	mov dx, si		; Save the address of env. var.

	lds	si,ss:[bp+6]	;char * varname
	mov	cx, cs:szvar	;Length of variable.
	cld
	rep cmpsb		; Does it exist?
	jz update

	mov ax, es		; Get the extra segment.
	mov ds, ax		; and put it in data segment.
	mov si, di		; Setup the source.
	jmp lp1

write_near:
	jmp write

;	The environment variable is there.
;	Now update the environment variable.
update:

update0:
	mov ax, es		; Get the extra segment.
	mov ds, ax		; and put it in data segment.
	mov di, dx		; This is our destination.
	mov si, di		; Setup the source.

update1:
	lodsb			; Skip
	cmp al, 00h		;   over
	jnz update1		;     the variable.

	lodsb			; Look ahead to see
	dec si			;   if the next byte
	cmp al, 00h		;     is zero if not
	jz update3		;	compress the data.

update2:
	lodsb			; Compress
	stosb			;   the data
	cmp al, 00h		;     for a
	jnz update2		;	variable.

	lodsb			; Get a byte.
	dec si
	cmp al, 00h		; Is it zero?
	jnz update2		; No.  Get next.

update3:
update4:
	mov cx, cs:szdat	; Length of data.
	cmp cx, 00h		; null data?
	jnz write1		; No. Jump out.

	mov cx, si		; End of old environment.
	mov dx, di		; Current position.
	sub cx, dx
	mov al, 00h		; Zero
	rep stosb		;  the rest.
	ret

;	The environment variable isn't there.
;	Now move the data into place.
write:
	mov ax, ds		; Get the data segment
	mov es, ax		; and put it in extra segment.
	dec si			; We are N+1.
	mov di, si		; This is our destination.
	mov dx, si		; save for reset

write1:
	mov cx, cs:szdat	; Environment data length
	jcxz write1a		; delete if none
	jmp short write2	; No. Jump out.
write1a:
	mov cs:[exit_code], 08h ; Environment variable not found.
	ret

write2:
	lds	si,ss:[bp+6]	;char * varname
	mov	cx, cs:szvar	;Set the length.
	call	cpytenv 	;copy to env
	push	cs
	pop	ds
	mov	si, offset equals
	mov	cx,1
	call	cpytenv
	lds	si,ss:[bp+10]	;char * data
	mov	cx, cs:szdat	;Set the length.
	call	cpytenv
	push	cs
	pop	ds
	mov	si,offset zero
	mov	cx,2
	call	cpytenv
	mov	setok,1
	ret

cpytenv label near
write3:
	mov ah, es:[di] 	; Get destination byte.
write4:
	cmp ah, 00h		; Is it a zero?
	jnz not_zero		; No.  Jump out.
	lodsb			; Get byte.
	stosb			; Save byte.
	loop write3		; Loop.
	ret


not_zero:
	mov ax, di		; Get the current offset.
	mov bx, cs:envsz	; Get environment size.
	cmp ax, bx		; Greater than environment size.
	jge reset		; Yes.	Jump.
	mov ah, 00h		; Make it zero.
	jmp write4

reset:
	mov di, dx
	mov al, 00h		; Put zero back.
	stosb
	stosb
	mov exit_code, 0Ah	; Set code for out of env.,
	pop  ax 		; discard inner return
	ret



	public lastloc
lastloc 	label	byte		; End of program.
set_env endp
setenv_TEXT   ENDS
	END
