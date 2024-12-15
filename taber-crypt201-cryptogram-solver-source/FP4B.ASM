fp4b	segment 'cryb'
shortxm equ	-37		;smallest negative exponent for shorts
shortxp equ	38		;largest positive exponent for shorts
longxm	equ	-307		;smallest negative exponent for longs
longxp	equ	308		;largest positive exponent for longs
indef	dd	0FFC00000R	;single precision real for indefinite
infin	dd	07FF80000R	;single precision real for +infinity
strngp	dw	0		;beginning of string after sign
strcnt	dw	0		;string length without sign
decp	dw	0		;->decimal point
decnt	dw	0		;count of fraction digits
exp	dw	0		;->e or E
expcnt	dw	0		;length of exponent
p10	dw	0		;power of 10
intp	dw	0		;->start of integer portion
intcnt	dw	0		;->count of integer portion
bcdp	dw	bcd		;->current position in bcd template
case	dw	xx		;string with no decimal, no exponent
	dw	xxd		;string with decimal, no exponent
	dw	xxe		;string with exponent, no decimal
	dw	xxde		;string with decimal and exponent

c10	dw	10		;constant 10
flag	db	0		;01 = decimal point found
				;02 = exponent found
				;80 = negative exponent
digcnt	db	0		;count of digits put into template
bcd	dt	0		;bcd template
fp4b	ends
