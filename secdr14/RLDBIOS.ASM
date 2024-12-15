	TITLE	rldbios - 01.21.94  Large Model
	.MODEL large
rldbios_TEXT   SEGMENT WORD PUBLIC 'CODE'
	ASSUME	CS:rldbios_TEXT
;
;rldbios(int,drive,head,track, strint)
;int rldbios(int cmd, int drive, int head,
;	     int track, int sector,
;	     int nsects, void *buffer);
;  (same as C++ biosdisk)

;Function: to bypass SECDRV TSR (if installed) to "real" disk bios

	public firstbyte,rdbvect
firstbyte label byte
rdbvect label  dword
	 dw	0,0

i13rtnp dw     offset i13rtn

tsrstat db     0

	public	_rldbios
_rldbios proc far
	push	bp
	mov	bp,sp
	mov	ah,[bp+06h]
	mov	al,[bp+10h]
	les	bx,[bp+12h]
	mov	cx,[bp+0Ch]
	shr	cx,1
	shr	cx,1
	and	cl,0C0h
	add	cl,[bp+0Eh]
	mov	ch,[bp+0Ch]
	mov	dh,[bp+0Ah]
	mov	dl,[bp+08h]
	pushf
	cmp	byte ptr cs:tsrstat,0
	jne	byptsr
	popf
	int	13h
i13rtn:
	cmp	byte ptr [bp+06h],08h
	jne	l1350
	mov	es:[bx],cx
	mov	es:[bx+02h],dx
l1350:
	mov	al,ah
	mov	ah,0
	pop	bp
	ret

byptsr:
	push	cs
	push	cs:i13rtnp
	jmp	dword ptr cs:rdbvect

	public	_setrldb
_setrldb label	far
	push	bp
	mov	bp,sp
	push	ds
	push	si
	push	ax
	mov	byte ptr cs:tsrstat,1
	lds	si,[bp+06h]
	mov	ax,ds:[si]
	mov	word ptr cs:rdbvect,ax
	mov	ax,ds:[si+2]
	mov	word ptr cs:rdbvect+2,ax
	nop
	pop	ax
	pop	si
	pop	ds
	pop	bp
	ret

iretx:	iret

_rldbios endp
rldbios_TEXT   ENDS
	END
