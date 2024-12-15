crypt7b segment 'cryb'
totals	dw	0		;sum of buckets
bucketn dw	0		;number of items to count
bucketi db 256 dup(0,0,0)	;buckets to hold the counts
freqs	db 256 dup('.....')     ;frequencies in percent
	db	?		;guard byte, we move 1 more than needed
zahlen	dw 256 dup(0)		;counts in alphabetic order
icfp	dq	0.0		;index of coincidence in floating point
icasc	db	'0.0  '         ;IC in ascii, 0.0dd
lnasc	db 5 dup(' ')           ;length of cryptogram in ascii
c3	dw	3
c10	dw	10
c100	dw	100
msga	dw	24*160+32	;panel function
msgb	dw	24*160+50	;subfunction
msgc	dw	24*160+80	;errors
panelid db	8,'Counting'
msg2	db	9,'frequency'
err1	db	16,'empty cryptogram'
lang	db	'Length'
icname	db	'IC'
fpret	db 26 dup(' ')
scantab db	1,0,0,1 	;nulls and mystery		0-3
	db	0,0,0,0,0,0,0,0,0,0,0,2  ;backtab		4-15
	db	3,4,5,6,7,8,9,10,11,12	;alt-Q W E R T Y U I O P  16-25
	db	0,0,0,0 	;mysteries			26-29
	db	13,14,15,16,17,18,19,20,21 ;alt-A S D F G H J K L  30-38
	db	0,0,0,0,0	;mysteries			39-43
	db	22,23,24,25,26,27,28	;alt-Z X C V B N M	44-50
	db	0,0,0,0,0,0,0,0 ;mysteries			51-58
	db	29,30,31,32,33,34,35,36,37,38 ;f1-f10		59-68
	db	0,0		;mysteries			69-70
	db	39		;home				71
	db	40		;cursor up			72
	db	41		;page up			73
	db	0		;mystery			74
	db	42		;cursor left			75
	db	0		;mystery			76
	db	43		;cursor right			77
	db	0		;mystery			78
	db	44		;end				79
	db	45		;cursor down			80
	db	46		;page down			81
	db	47		;insert 			82
	db	48		;delete 			83
	db	49,50,51,52,53,54,55,56,57,58 ;sf1-sf10 	84-93
	db	59,60,61,62,63,64,65,66,67,68 ;cf1-cf10 	94-103
	db	69,70,71,72,73,74,75,76,77,78 ;af1-af10 	104-113
	db	0		;cntrl-PrtSc  don't care        114
	db	79		;ctrl-cursor left		115
	db	80		;ctrl-cursor right		116
	db	81		;ctrl-end			117
	db	82		;ctrl-PgDn			118
	db	83		;ctrl-home			119
	db	84,85,86,87,88,89,90,91,92,93,94,95 ;atoprow	120-131
	db	96		;ctrl-PgUp			132
;		   0	 1     2
fanout2 dw	forever,null,backtab
;		 3    4    5	6    7	  8    9   10	 11   12
	dw	altq,altw,alte,altr,altt,alty,altu,alti,alto,altp
;		 13   14   15	16   17   18   19   20	 21
	dw	alta,alts,altd,altf,altg,alth,altj,altk,altl
;		 22   23   24	25   26  27    28
	dw	altz,altx,altc,altv,altb,altn,altm
;		29 30 31 32 33 34 35 36 37 38
	dw	f1,f2,f3,f4,f5,f6,f7,f8,f9,f10
;		39    40    41	  42	  43	  44   45
	dw	home,curup,pagup,curleft,curite,endkey,curdn
;		46     47     48
	dw	pagdn,insert,delete
;		49  50	 51  52  53  54  55  56  57  58
	dw	sf1,sf2,sf3,sf4,sf5,sf6,sf7,sf8,sf9,sf10
;		59   60  61 62	63  64	65  66	67   68
	dw	cf1,cf2,cf3,cf4,cf5,cf6,cf7,cf8,cf9,cf10
;		69  70	 71  72  73  74 75  76	77   78
	dw	af1,af2,af3,af4,af5,af6,af7,af8,af9,af10
;		79	 80	 81	82   83
	dw	prewind,nxtwind,bottom,botpg,top
;		84 85 86 87 88 89 90 91 92 93 94    95
	dw	a1,a2,a3,a4,a5,a6,a7,a8,a9,a0,adash,aequal
;		 96
	dw	toppg
crypt7b ends
