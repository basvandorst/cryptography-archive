crypthb segment 'cryb'
helpno	dw	0		;number of help screen wanted
freeb	dw	0		;save current memory
freea	dw	0
spaceb	dw	0
spacea	dw	0
savescrb dw	0		;save screen area
savescra dw	0
helph	dw	0		;help file handle
count	dw	0		;count of bytes read
buffptr dw	0		;current position in read buffer
hlines	dw	0		;number of help lines
help1b	dw	0		;offset to 1st help line
help1a	dw	0		;segment
currh	dw	0		;number of top help line
c23	dw	23		;help lines per screen
c80	dw	80		;length of a help line
hscreens dw	0		;number of screens for help file
folge	dw	0		;current screen number shown
flag	db	0		;01 = eof encountered
				;02 = end of line encountered
				;04 = pending end of file
				;80 = i/o error

hname	db	'CRYPTXXX.HLP',0        ;hname+3 start of digits
	db	80		;length of title
helptit db	80 dup(' ')   ;help title
helpbuf db 256 dup(' ')         ;read buffer for help file
line25	db	80,'F2 Quit  PgDn Next screen  PgUp Previous screen'
	db	'            Screen '
l25ith	db	'      of '
l25tot	db	'     '
msg1	db	32,'Insufficient memory to show help'
msg2	db	17,'No help available'
msg3	db	23,'File error reading help'
crypthb ends
