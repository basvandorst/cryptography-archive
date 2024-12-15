	public	fndx,fdct
crypt10b segment 'cryb'
fndx	db	'ENGLISH.NDX',0         ;index filespec
	db 69 dup(0)
fdct	db	'ENGLISH.DCT',0         ;dictionary filespec
	db 69 dup(0)
;keep together, one index entry buffer
hdr	db 12 dup(' ')                  ;index buffer
hdrcnt	dw	0			;number of patterns
hdrlen	dw	0			;length of longest pattern

filemark db	'PatternIndex'
saveb	dw	0		;save memory size
savea	dw	0		;..in case of bailout

motemp	motndx <>		;word index work area
;for extracting cipher letters
cryword db 80 dup(' ')  ;cipher word stripped of decorations
cryoff	db 80 dup(0)	;offsets of letters in cipher alphabet
patwork db 16 dup(0)	;make pattern in byte format
top	dw	0	;top of binary search
bottom	dw	0	;bottom of binary search
c16	dw	16
msg1	db	32,'Dictionary index file read error'
msg2	db	37,'Insufficient memory for pattern lists'
msg3	db	23,'Empty cipher word index'
crypt10b ends
