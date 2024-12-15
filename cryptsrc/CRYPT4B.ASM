crypt4b segment 'cryb'
function dw	0		;function code, what is being logged
logh	dw	0	;file handle
savegrm dw	0	;save caller's current cipher line
;log function table
logtab	dw	simple	;log a simple substitution solution
	dw	rail	;log a rail fence solution
	dw	xpose	;log a complete transposition
	dw	bifid	;log a bifid

fposb	dw	0	;position of last byte in log file
fposa	dw	0

bkeyl	dw	0	;lines of bifid key, if any
bkeyw	dw	0	;width of a key line
bkeyd	dw	0	;distance to next key line
bkeypb	dw	0	;offset to key line
bkeypa	dw	0	;segment

bifptl	dw	0	;length of bifid pt
bifptb	dw	0	;offset of current bifid pt
bifpta	dw	0	;segment

flag1	db	0	;01 = on 2nd bifid pt logged
logfile db	'crypt.log',0
msg1	db	14,'Log file error'

cline	db	0,80 dup(' ')   ;cipher letters line
newline db	0Dh,0Ah
crypt4b ends
