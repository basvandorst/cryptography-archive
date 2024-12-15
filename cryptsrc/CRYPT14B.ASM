crypt14b segment 'cryb'
freeb	dw	0
freea	dw	0
saveb	dw	0
savea	dw	0
dath	dw	0		;statistics file handle
bufl	dw	0		;remaining characters in buff
tokp	dw	0		;start of token
tokl	dw	0		;length of token
alfl	dw	0		;length of language's alphabet
cipherb dw	0		;temp holding area for cipher alphabet
ciphera dw	0
plainb	dw	0		;temp holding area for plain alphabet
monob	dw	0		;temp holding area for mono statistics
monotab dw	mkey1,8,alph
	dw	mkey2,6,ciph
	dw	mkey3,5,plain
	dw	mkey4,4,mstat
monotabz equ	$

buff	db 256 dup(' ')
bufp	dw	buff		;current position in buffer
flag	db	0		;01 = end of file pending
				;02 = end of file
;mono statistics parsing flag
flagm	db	0		;01 = alphabet length specified
				;02 = cipher alphabet specified
				;04 = plain alphabet specified
				;08 = mono stats defined

mkey1	db	'ALPHABET'
mkey2	db	'CIPHER'
mkey3	db	'PLAIN'
mkey4	db	'MONO'

;error messages, display using either DOS or crypt screen
msg1	db	'Press any key to continue...',0Dh,0Ah,'$'
err1	db	19,'Language file error',0Dh,0Ah,'$'
err2	db	23,'Language file not found',0Dh,0Ah,'$'
err3	db	34,'Respecification of alphabet length',0Dh,0Ah,'$'
err4	db	38,'Alphabet length expected but not found',0Dh,0Ah,'$'
err5	db	23,'Alphabet length invalid',0Dh,0Ah,'$'
err6	db	38,'Insufficient memory to load statistics',0Dh,0Ah,'$'
err7	db	38,'Cipher alphabet specified out of order',0Dh,0Ah,'$'
err8	db	34,'Respecification of cipher alphabet',0Dh,0Ah,'$'
err9	db	36,'End of file before cipher definition',0Dh,0Ah,'$'
err10	db	30,'Cipher definition length error',0Dh,0Ah,'$'
err11	db	37,'Plain alphabet specified out of order',0Dh,0Ah,'$'
err12	db	33,'Respecification of plain alphabet',0Dh,0Ah,'$'
err13	db	35,'End of file before plain definition',0Dh,0Ah,'$'
err14	db	29,'Plain definition length error',0Dh,0Ah,'$'
err15	db	28,'Mono statistics out of order',0Dh,0Ah,'$'
err16	db	34,'Respecification of mono statistics',0Dh,0Ah,'$'
err17	db	34,'End of file before mono definition',0Dh,0Ah,'$'
err18	db	22,'Invalid mono statistic',0Dh,0Ah,'$'
err19	db	34,'Spec errors; using default English',0Dh,0Ah,'$'
err20	db	23,'Cipher alphabet missing',0Dh,0Ah,'$'
err21	db	22,'Plain alphabet missing',0Dh,0Ah,'$'
crypt14b ends
