crypt8b segment 'cryb'
wrdpos	dw	111Ah	;row 18, col 27 for word index cursor position
l1pos	dw	0F2Dh	;row 16, col 46 for primary list cursor position
motlang dw	type motndx
c16	dw	16
c160	dw	160
sptr	dw	17*160+40	;1st screen line for list
lstptr	dw	0		;primary or alternate list ptrs
itemb	dw	0		;-> cipher word index entry
itema	dw	0
status	dw	0		;NDP status word
scale	dw	0		;scale of probable fit
area	dw	eclata		;going to substitution area
	dw	eclatb		;going to cipher word list area
	dw	eclatc		;going to primary list area
	dw	eclatd		;going to secondary list area
;memory ptr and size for resetting word indexes
freeb	dw	0	;start of freespace
freea	dw	0
saveb	dw	0	;size of freespace
savea	dw	0
msg1	db	20,'..... of ..... words'
items	db	06,'..... '
msg2	db	14,'..... of .....'

w1len	dw	0		;length of cursor crypt word
w2len	dw	0		;length of primary crypt word
wort1	db 16 dup(' ')          ;..cursor crypt word
wort2	db 16 dup(' ')          ;..primary crypt word
w1onof	db 16 dup(07h)		;low or hi for cursor word letters
w2onof	db 16 dup(07h)		;low or hi for primary word letters
crypt8b ends
