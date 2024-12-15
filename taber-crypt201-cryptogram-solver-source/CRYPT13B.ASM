	extrn	fndx:far,fdct:far
crypt13b segment 'cryb'
hiwater dw	0
stringl dw	0		;remaining string length
parml	dw	0		;length of isolated parameter
dnamel	dw	0		;length of display name
part1	dw	0		;length of display name up to path
partb	dw	0		;offset into dname
ndxspec dd	fndx		;address of index file specification
dctspec dd	fdct		;address of pattern dictionary file spec
;option table	string ptr, routine name
optab	dw	lopt,tongue	;just one option for now
optabz	equ	$
lopt	db	9,'-language'   ;language specification

default1 db	12,'ENGLISH.DAT',0 ;default language data file
default2 db	12,'ENGLISH.NDX',0 ;default dictionary index file
default3 db	12,'ENGLISH.DCT',0 ;default pattern dictionary file
string	db 81 dup(' ')          ;string to be parsed - string,blank
parm	db 81 dup(' ')          ;isolated parameter from string
dname	db 14 dup(' ')          ;isolated display name from parm
crypt13b ends
