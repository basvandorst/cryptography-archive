crypt3b segment 'cryb'
oldpos	dw	0	;old cursor position
saveline db 80 dup (' ',07)     ;to save current video line for command
cmdptr	dw	0	;command offset
cmdseg	dw	0	;command segment
nxtptr	dw	0	;current command line position
nxtlen	dw	0	;length of remaining command line
tokptr	dw	0	;start of this token
toklen	dw	0	;length of this token
cmdtab	db	3,'get     '
	dw	get
	db	4,'name    '
	dw	nom
	db	5,'start   '
	dw	start
cmdtabn equ	($-cmdtab)/11
err1	db	11,'Bad command'
crypt3b ends
