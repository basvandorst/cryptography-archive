crypt26b segment 'cryb'
hold	db 80 dup(' ')

rowcol	dw	ctrow	;top line, even index, even block
	dw	ctrow	;top line, even index, odd block
	dw	ctcol	;top line, odd index, even block
	dw	ctcol	;top line, odd index, odd block

	dw	ctrow	;bot line, even index, even block
	dw	ctcol	;bot line, even index, odd block
	dw	ctcol	;bot line, odd index, even block
	dw	ctrow	;bot line, odd index, odd block

keycase dw	keyl0	;entirely new keysquare letter
	dw	keylr	;new keysquare letter matches a row letter
	dw	keylc	;new keysquare letter matches a col letter
	dw	keylrc	;new keysquare letter matches both a row and col letter

keycas2 dw	keyl005 ;both original cords in root, conflict
	dw	keylr10  ;original row in pale, col in root
	dw	keylc10  ;original col in pale, row in root
	dw	keylrc30 ;both original row and col in pale

root	dw	0	;passed root
parmb	dw	0	;offset to frag table parms
parma	dw	0	;segment
save2b	dd	?	;mem parms save area
free2b	dd	?
oldkeyp dd	?	;save old key ptr in case of key build failure
sidehold dw	?	;original N of NxN
fitsize  dw	?	;size of a fits table
counters dw	?	;size of counters
digtot	dw	0	;number of digraphs according to root
digcntb dw	0	;->digraphic counters
digcnta dw	0	;..segment

keycurs label	word	;keysquare position
keycol	db	?
keyrow	db	?

destrow dw	?	;destination row
destcol dw	?	;destination column
orgrow	dw	?	;original row
orgcol	dw	?	;original col
voncord dw	?	;from coordinate for pale merging
zucord	dw	?	;to coordinate

filendx db	'CR00001.YPT',0        ;transaction index
filelog db	'CR00002.YPT',0        ;transaction log
ndxid	dw	0	;transaction index handle
logid	dw	0	;transaction log handle
logl	dw	0	;total log entry length
ndxbuf	bifxact <>	;log index file buffer

flag1	db	0	;01 = broken transaction log
			;02 = display major diagonal permutation
			;04 = key letter exists in row square
			;08 = key letter exists in col square

savechar db	' '     ;key sub char
msg1	db	34,'Position already solved.  Use undo'
msg2	db	38,'Keysquare conflict, invalid substitute'
crypt26b ends
