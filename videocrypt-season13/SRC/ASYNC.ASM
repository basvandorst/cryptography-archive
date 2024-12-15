;  interrupt driven serial port I/O module.
;  written by David Kessner
;  modified for MASM 5.1 compatibility on 1994-04-11 by WK

.MODEL SMALL

EOI		equ	020h			; 8259 end-of-interupt
Ctrl8259_0	equ	020h			; 8259 port
Ctrl8259_1	equ	021h			; 8259 port (Masks)
BufSize		equ	8196			; Buffer Size


        _DATA      SEGMENT WORD    PUBLIC  'DATA'

                   assume  cs:DGROUP, ds:DGROUP, ss:DGROUP

        .DATA


; Various things to be set upon AsyncInit()
VectorNum		db	?		; Vector Number
EnableIRQ		db	?		; Mask to enable 8259 IRQ
DisableIRQ	        db	?		; Mask to disable 8259 IRQ
VectorSeg		dw	?		; Old Vector Segment
VectorOfs		dw	?		; Old Vector Offset

; Register Addresses for the 8250 UART
Port			dw	?		; Port Base Address
      RegStart LABEL WORD
THR			dw	?		; Transmitter Holding Register
RDR			dw	?		; Reciever Data Register
BRDL			dw	?		; Baud Rate Divisor, Low byte
BRDH			dw	?		; Baud Rate Divisor, High Byte
IER			dw	?		; Interupt Enable Register
IIR			dw	?		; Interupt Identification Register
LCR			dw	?		; Line Control Register
MCR			dw	?		; Modem Control Register
LSR			dw	?		; Line Status Register
MSR			dw	?		; Modem Status Register

; Buffer Data
RecBuffer		db	BufSize DUP (?)	; Recieve Buffer
RecHead		        dw	?		; Buffer Head Pointer
RecTail		        dw	?		; Buffer Tail Pointer
TransBuffer	        db	BufSize DUP (?)	; Transmit Buffer
TransHead		dw	?		; Buffer Head Pointer
TransTail		dw	?		; Buffer Tail Pointer

; Register Offsets for the UART
RegOffsets	dw	0, 0, 0, 1, 1, 2, 3, 4, 5, 6

_DATA       ENDS


        _TEXT  SEGMENT WORD PUBLIC 'CODE'

               assume  cs:_TEXT, ds:DGROUP, ss:DGROUP

	PUBLIC	_AsyncInit,	_AsyncClear, 	_AsyncStop
	PUBLIC	_AsyncIn,	_AsyncOut, 	_AsyncSet
	PUBLIC	_AsyncHand,	_AsyncStat,	_AsyncInStat
	PUBLIC	_AsyncOutStat


;-----------------------------------------------------------------------------
; AsyncClear			Empty the recieve buffer
;-----------------------------------------------------------------------------
; void	AsyncClear( void);
;
;
;-----------------------------------------------------------------------------
_AsyncClear PROC NEAR
	cli
	push	ax
	mov	ax, offset RecBuffer
	mov	[RecHead], ax
	mov	[RecTail], ax
	mov	ax, offset TransBuffer
	mov	[TransHead], ax
	mov	[TransTail], ax
	pop	ax
	sti
	ret
_AsyncClear ENDP



;-----------------------------------------------------------------------------
; AsyncInit			Initalize Serial Port and install ISR
;-----------------------------------------------------------------------------
;	void	AsyncInit( int port)
;
;	Where Port is
;		0  =  COM1
;		1  =  COM2
;		2  =  COM3
;		3  =  COM4
;
;-----------------------------------------------------------------------------
_AsyncInit PROC NEAR
       CommPort equ bp+4

        push    bp
        mov     bp, sp

;---- Set various things according to com port number
        mov     ax, [CommPort]


;----- COM1
	cmp	ax, 0
        jne     _AsyncInit_1
	mov	[Port], 03F8h
	mov	[VectorNum], 0Ch
	mov	[EnableIRQ], 0EFh
	mov	[DisableIRQ], 10h
        jmp     _AsyncInit_Done
_AsyncInit_1:
;----- COM2
	cmp	ax, 1
        jne     _AsyncInit_2
	mov	[Port], 02F8h
	mov	[VectorNum], 0Bh
	mov	[EnableIRQ], 0F7h
	mov	[DisableIRQ], 08h
        jmp     _AsyncInit_Done
_AsyncInit_2:
;----- COM3
	cmp	ax, 2				; 2
        jne     _AsyncInit_3
	mov	[Port], 03E8h			; 03E8
	mov	[VectorNum], 0Ch		; 0C
	mov	[EnableIRQ], 0EFh		; EF
	mov	[DisableIRQ], 10h		; 10
        jmp     _AsyncInit_Done
_AsyncInit_3:
;----- COM4
	mov	[Port], 02E8h			; 02E8
	mov	[VectorNum], 0Bh		; 0B
	mov	[EnableIRQ], 0F7h		; F7
	mov	[DisableIRQ], 08h		; 08
_AsyncInit_Done:

;---- Compute Register locations
	mov	cx, 10
	mov	bx, offset RegOffsets
	push	di
	mov	di, offset RegStart
_AsyncInit_4:
	mov	ax, [bx]
	add	ax, [Port]
	mov	[di], ax
	add	bx, 2
	add	di, 2
        loop    _AsyncInit_4

	pop	di


;----- Initalize Buffer
	call	_AsyncClear

;--- Save and reassign interrupt vector
	push	ds					; Save Old Vector
	mov	al,[VectorNum]
	mov	ah,35h
	int	21h
	mov	[VectorSeg], es
	mov	[VectorOfs], bx

	mov	al, [VectorNum]
	push	cs					; Set New Vector
	pop	ds
	mov	dx, offset AsyncISR
	mov	ah, 25h
	int	21h
	pop	ds

;----- Enable 8259 interrupt (IRQ) line for this async adapter
	in	al, Ctrl8259_1
	and	al, [EnableIRQ]
	out	Ctrl8259_1, al

;----- Enable 8250 Interrupt-on-data-ready
	mov	dx, [LCR]		; Read Line control register and clear
	in	al, dx                  ; bit 7, the Divisor Latch Address
	and	al, 07Fh
	out	dx, al
	mov	dx, [IER]

        mov al, 0               ; we're gonna test the UART first
        out dx, al
        in al, dx               ; if this isn't 0, there's no UART
        cmp al, 0
        jnz _AsyncInit_222

	mov	al, 3
	out	dx, al

;----- Clear 8250 Status and data registers
_AsyncInit_10:
	mov	dx, [RDR]		; Clear RDR by reading port
	in	al, dx
	mov	dx, [LSR]		; Clear LSR
	in	al, dx
	mov	dx, [MSR]		; Clear MSR
	in	al, dx
	mov	dx, [IIR]		; Clear IIR
	in	al, dx
	test	al, 1
        jz      _AsyncInit_10

;----- Set Bit 3 of MCR -- Enable interupts
	mov	dx, [MCR]
	in	al, dx
	or	al, 08h
	out	dx, al

;----- Clear Buffer Just in case
	call	_AsyncClear

;----- Return
	xor	ax, ax
_AsyncInit_222:
	pop	bp
	ret
_AsyncInit ENDP



;-----------------------------------------------------------------------------
;  AsyncStop				Uninstall ISR
;-----------------------------------------------------------------------------
;	void  AsyncStop( void)
;-----------------------------------------------------------------------------
_AsyncStop PROC NEAR

	push	bp
	mov	bp, sp

;----- Mask (disable) 8259 IRQ Interrupt
	in	al, Ctrl8259_1
	or	al, [DisableIRQ]
	out	Ctrl8259_1, al

;----- Disable 8250 interrupt
	mov	dx, [LCR]
	in	al, dx
	and	al, 07Fh
	out	dx, al
	mov	dx, [IER]
	xor	al, al
	out	dx, al

;----- Set bit 3 in MCR to 0
	mov	dx, [MCR]
	in	al, dx
	and	al, 0F7h
	out	dx, al

;----- Interrupts are disables.  Restore saved interrupt vector.
	push	ds
	mov	al, [VectorNum]
	mov	ah, 25h
	mov	dx, [VectorOfs]
	mov	ds, [VectorSeg]
	int	21h
	pop	ds

;----- Return
	pop	bp
	ret
_AsyncStop ENDP



;-----------------------------------------------------------------------------
; AsyncISR				Async Interrupt Service Routine
;-----------------------------------------------------------------------------
;	To be called only as an interrupt.
;-----------------------------------------------------------------------------
AsyncISR PROC NEAR

        push    ax                              ; Save Registers
	push	bx
	push	ds
	push	dx

	mov	ax, @data			; Address local data with ds
	mov	ds, ax

	mov	dx, [IIR]			; Check if data actually recieved
	in	al, dx
	and	al, 06h
	cmp	al, 04h
        je      AsyncISR_recieve
	cmp	al, 02h
        jne     AsyncISR_end

;----- Transmit A byte
AsyncISR_transmit:
	mov	bx, [TransTail]
	cmp	bx, [TransHead]
        jne     AsyncISR_1

	mov	dx, [IER]			; Buffer empty
	mov	al, 1
	out	dx, al				; Disable THR empty interrupt
        jmp     AsyncISR_end

AsyncISR_1:
	mov	al, byte ptr [bx]		; Get Byte
	inc	[TransTail]			; Update buffer pointer
	cmp	[word ptr TransTail], offset TransBuffer + BufSize
        jb      AsyncISR_2
	mov	[TransTail], offset TransBuffer
AsyncISR_2:
	mov	dx, [THR]
	out	dx, al
        jmp     AsyncISR_end

;----- Recieve a byte
AsyncISR_recieve:
	mov	dx, [RDR]			; Get Byte
	in	al, dx
	mov	bx, [RecHead]			; Store Byte in buffer
	mov	byte ptr [bx], al
	inc	bx				; Update RecHead
	cmp	bx, offset RecBuffer + BufSize
        jb      AsyncISR_10
	mov	bx, offset RecBuffer
AsyncISR_10:
	cmp	bx, [RecTail]
        jne     AsyncISR_20
	mov	bx, [RecHead]			; Cancel Pointer advance on overflow
AsyncISR_20:
	mov	[RecHead], bx			; Store new pointer

AsyncISR_end:
	mov	al, EOI				; Signal end ot interrupt
	out	Ctrl8259_0, al

        ; Disable and re-enable interrupts so that there
        ; is an interrupt edge.

        mov     dx,[IER]                ; Point to Interrupt Enable Register.
        in      al,dx                   ; Read the current value.
        push    ax                      ; Save it.
        mov     al,0                    ; Disable the interrupts.
        out     dx,al
        pop     ax                      ; Restore original mask.
        out     dx,al                   ; Re-enable interrupts.

        pop     dx                      ; Restore saved registers.
        pop     ds
        pop     bx
        pop     ax

	iret

AsyncISR ENDP



;-----------------------------------------------------------------------------
;	AsyncIn			Gets a byte from the input buffer
;-----------------------------------------------------------------------------
;	int	AsyncIn( void)
;-----------------------------------------------------------------------------
_AsyncIn PROC NEAR

	push	bp
	mov	bp, sp

	xor	ax, ax				; Pre-Set result to 0
	mov	bx, [RecTail]
	cmp	bx, [RecHead]
        je      _AsyncIn_return
	mov	al, byte ptr [bx]
	inc	[RecTail]
	cmp	[word ptr RecTail], offset RecBuffer + BufSize
        jb      _AsyncIn_return
	mov	[RecTail], offset RecBuffer

_AsyncIn_return:
	pop	bp
	ret
_AsyncIn ENDP



;-----------------------------------------------------------------------------
;	AsyncOut		Output a byte
;-----------------------------------------------------------------------------
;	void	AsyncOut( int c)
;-----------------------------------------------------------------------------
_AsyncOut PROC NEAR
        push    bp
        mov     bp,sp
        mov     ax,[bp+4]               ; get argument
	mov	bx, [TransHead]
	mov	cx, bx
	inc	cx				; Compute NEW buffer position
	cmp	cx, offset TransBuffer + BufSize
        jb      _AsyncIn_1
	mov	cx, offset TransBuffer
_AsyncIn_1:
	cmp	cx, [TransTail]			; Wait for space in buffer
        je      _AsyncIn_1

	mov	byte ptr [bx], al		; Add byte to buffer
	mov	[TransHead], cx			; Update pointer

	mov	dx, [IER]			; Enable THR empty interrupt
	mov	al, 3
	out	dx, al

	pop	bp
	ret
_AsyncOut ENDP



;-----------------------------------------------------------------------------
;	AsyncSet		Set communication paramaters
;-----------------------------------------------------------------------------
;	void	AsyncSet( int Baud, int Control)
;
;	Baud = 150, 300, 600, 1200, 2400, 4800, 9600, 19200, 28800, 38400, 57600
;	Control = The valure to place in the LCR
;-----------------------------------------------------------------------------
_AsyncSet PROC NEAR
       Baud   equ  bp+4
       Control equ bp+6

	push	bp
	mov	bp, sp

	mov	bx, [Baud]
	cmp	bx, 0
        je      _AsyncSet_abort

	mov	ax, 0C200h	        ; Baud rate divisor = 115200 / Baud
	mov	dx, 0001h		; 115200 = 0001C200h
	div	bx
	mov	cx, ax

	cli
	mov	dx, [LCR]		; Set Port Toggle to BRDL/BRDH registers
	mov	al, 0ffh
	out	dx, al

	mov	dx, [BRDL]		; Set Baud Rate
	mov	al, cl
	out	dx, al
	mov	dx, [BRDH]
	mov	al, ch
	out	dx, al

	mov	dx, [LCR]		; Set LCR and Port Toggle
	mov	ax, [Control]
	and	al, 07Fh
	out	dx, al

	sti
_AsyncSet_abort:
	pop	bp
	ret
_AsyncSet ENDP



;-----------------------------------------------------------------------------
;	AsyncInStat		Returns the # of characters in buffer
;-----------------------------------------------------------------------------
;	int	AsyncInStat( void)
;-----------------------------------------------------------------------------
_AsyncInStat PROC NEAR
	push	bp
	mov	bp, sp

	mov	ax,[RecHead]
	sub	ax, [RecTail]
        jge     _AsyncInStat_10
	add	ax, BufSize
_AsyncInStat_10:
	pop	bp
	ret
_AsyncInStat ENDP



;-----------------------------------------------------------------------------
;	AsyncOutStat		Returns the # of characters in buffer
;-----------------------------------------------------------------------------
;	int	AsyncOutStat( void)
;-----------------------------------------------------------------------------
_AsyncOutStat PROC NEAR
	push	bp
	mov	bp, sp

	mov	ax,[TransHead]
	sub	ax, [TransTail]
        jge     _AsyncOutStat_10
	add	ax, BufSize
_AsyncOutStat_10:

	pop	bp
	ret
_AsyncOutStat ENDP



;-----------------------------------------------------------------------------
;	AsyncHand		Sets various handshaking lines
;-----------------------------------------------------------------------------
;	void	AsyncHand( int Hand)
;-----------------------------------------------------------------------------
_AsyncHand PROC NEAR
       Hand  equ bp+4

	push	bp
	mov	bp, sp

	mov	dx, [MCR]
	mov	ax, [Hand]
	or	al, 08h				; Keep interrupt enable ON
	out	dx, al

	pop	bp
	ret
_AsyncHand ENDP


;-----------------------------------------------------------------------------
;	AsyncStat		Returns Async/Modem status
;-----------------------------------------------------------------------------
;	unsigned	AsyncStat( void)
;
;	MSR is returned in the high byte, LSR in the low byte
;-----------------------------------------------------------------------------
_AsyncStat PROC NEAR
	push	bp
	mov	bp, sp

	mov	dx, [MSR]
	in	al, dx
	mov	cl, al
	mov	dx, [LSR]
	in	al, dx			; LSR in low byte
	mov	ah, cl			; MSR in high byte

	pop	bp
	ret
_AsyncStat ENDP


        _TEXT ENDS

	END
