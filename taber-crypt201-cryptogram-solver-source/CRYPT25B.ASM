;6			to permit accented chars to print
crypt25b segment 'cryb'
root	dw	0	;root of keysquare, 5 or 6
square	dw	0	;square of root, 25 or 36
oldpos	dw	0	;caller's cursor position
savechar dw	0	;saved keyboard char

saveb	dw	0	;original memory save area
	dw	0
freeb	dw	0
	dw	0

save2b	dw	0	;site table/matched ct memory save area
	dw	0
free2b	dw	0
	dw	0

save3b	dw	0	;fragment table memory save area
	dw	0
free3b	dw	0
	dw	0

c160	dw	160
savecurs dw	0	;save previous cursor position

cmdcurs label	word
cmdcol	db	0	;command cursor column
cmdrow	db	12	;command cursor row (unvarying)

sitcurs label	word
sitcol	db	0	;site list cursor column
sitrow	db	14

percurs label	word
percol	db	11	;period list cursor column
perrow	db	14	;..cursor row

keycurs label	word
keycol	db	25	;key cursor column
keyrow	db	16	;..cursor row

cmdtabb dw	site		;site crib
	dw	noj		;restore english, no j alphabet
	dw	alphabet	;define a user alphabet
	dw	nok		;set french, spanish, no k alphabet
	dw	permute 	;permute keysquare
	dw	bloc		;determine block length

;keyboard char branch table
fanout1 dw	extended	;0	extended code
	dw	useasis 	;1	no assigned meaning
	dw	rubout		;2	rubout
	dw	tab		;3	tab forward
	dw	endline 	;4	carriage return
	dw	escape		;5	escape, flip cursors
	dw	blank		;6	blank
	dw	punct		;7	marks and punctuation
	dw	numeral 	;8	numeral
	dw	enguc		;9	english upper case
	dw	englc		;10	english lower case
	dw	del		;11	delete line
	dw	euruc		;12	foreign upper case
	dw	eurlc		;13	foreign lower case

;keyboard extended character branch table
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

;cursor right function table
ftab1	dw	subrite ;cursor right in substitution area
	dw	forever ;no cursor right in site area
	dw	forever ;no cursor right in period list
	dw	keyrite ;cursor right in key area
;cursor left function table
ftab2	dw	subleft ;cursor left in substitution area
	dw	forever ;no cursor left in site area
	dw	forever ;no cursor left in period list
	dw	keyleft ;cursor left in key area
;enter function table
ftab4	dw	subend	;enter in substitution area
	dw	sitend	;enter in site area
	dw	perend	;enter in period list
	dw	keyend	;enter in keysquare area
;cursor down function table
ftab5	dw	subdown ;in substitution area
	dw	sitdown ;in site list
	dw	perdown ;in period list
	dw	keydown ;in key
;cursor up function table
ftab6	dw	subup	;in sub area
	dw	situp	;in site list
	dw	perup	;in period list
	dw	keyup	;in key
;previous area function table
ftab7	dw	area3	;0 -> 3 	;from sub to key
	dw	area0	;1 -> 0 	;from sites to sub
	dw	area1	;2 -> 1 	;from periods to sites
	dw	area2	;3 -> 2 	;from key to periods
;next area function table
ftab8	dw	area1	;0 -> 1 	;from sub to sites
	dw	area2	;1 -> 2 	;from sites to periods
	dw	area3	;2 -> 3 	;from periods to key
	dw	area0	;3 -> 0 	;from key to sub

tipblock dw	0	;length of 1st part of a crib

;begin area cleared in one swoop
cleara	equ	$
bifa	bif	<>	;1st bifid parameters
bifb	bif	<>	;2nd bifid parameters
cmdprm	befehl	<>	;command line parsing parameters
pairs	bifsite <>	;siting, pairing parameters
workb	dw	0	;->work area [expanded twin],expanded ct,pt
worka	dw	0
worklen dw	0	;..byte length of work area
culldim dw	0	;row or col dimension to cull
cullcord dw	0	;coordinate being culled
pairlen dw	0	;number of paired pt/ct dimensioned
permvec dw 6 dup(0)	;keysquare permutation vector

;area key control
area	dw	0	;0 = substitution area
			;1 = site list
			;2 = period list
			;3 = key

;tip drag parameters
draginc dw	1	;increment for dragging, 1 or block length
draga	dw	0	;offset to start dragging
dragn	dw	?	;number of times to drag
sitea	dw	0	;top of screen site number
siten	dw	0	;number of non-conflicting sites for tip
sitabb	dw	0	;->site table, offset
sitaba	dw	0	;segment
sitib	dw	0	;offset of picked site
sitia	dw	0	;segment of picked site
forbif	dw	0	;sites for which bifid

;fragment list control
fragn	dw	0	;number of fragments
fragb	dw	0	;offset of fragments list
fraga	dw	0	;segment
frags	dw	0	;1st fragment entry on screen

;marked block controls
markcnt dw	0	;number of marks made, 1,2,3,4
marklen dw	0	;length of mark
marksvb dw	0	;offset of dimensioned letters
marksva dw	0	;segment
mark1	bifmark <>	;1st block mark control
mark2	bifmark <>	;2nd block mark control

;period-dic table
dictab	dw	5	;block length, 5
	dd	0.0	;dic
	dw	6	;6
	dd	0.0
	dw	7	;7
	dd	0.0
	dw	8	;8
	dd	0.0
	dw	9	;9
	dd	0.0
	dw	10	;10
	dd	0.0
	dw	11	;11
	dd	0.0
	dw	12
	dd	0.0
	dw	13
	dd	0.0
	dw	14
	dd	0.0
oldblk	dw	0		;save old period
forbif2 dw	0
forbif3 dw	0

flag1	db	0	;01 = twin bifid
			;02 = we have a keysquare
			;04 = we have a key permutation
			;08 = period list up
			;10 = show short block
			;20 = message pending
			;40 = command mode
			;80 = any must quit error

flag2	db	0	;01 = site list up
			;02 = 2nd bifid up
			;04 = mark pending
			;08 = matched twin ciphertext
			;80 =

flager	db	0	;01 = 1st bifid empty
			;02 = no memory
			;04 = 2nd bifid empty
			;08 = cipher(s) too big
			;10 = broken transaction log

	db	0	;word boundary alignment
clearb	equ	$

collect db 26 dup(' ')  ;collected equivalents, fp conversion area

traplet db	'  '    ;user specified trap

;these two stay together in this order
bifalf	db 36 dup(' ')  ;current alphabet
eligible db 36 dup(' ') ;eligible letters for substitution

specalf db 36 dup(' ')  ;user specified alphabet
j2i	db	'JI'    ;trap Js, translate to Is
alf5	db	'ABCDEFGHIKLMNOPQRSTUVWXYZ'     ;5x5 no j alphabet
alf6	db	'A1B2C3D4E5F6G7H8I9J0KLMNOPQRSTUVWXYZ' ;6x6 alphabet
alfnok	db	'ABCDEFGHIJLMNOPQRSTUVWXYZ'     ;5x5 no k alphabet

toasc	db 5 dup(' ')   ;work area for converting integers to ascii
savebar db 160 dup(0)	;save screen line

cmdlen	db	0	;command length
cmdbuf	db 80 dup(' ')  ;command keyboard buffer
cmdtaba db	2,'site    '    ;minimum abbreviation, site crib
	db	3,'noj     '    ;min abbrev, set no j alphabet
	db	1,'alphabet'    ;min abbrev., define bifid alphabet
	db	3,'nok     '    ;no k alphabet
	db	1,'permute '    ;permute keysquare
	db	1,'block   '    ;determine block length
cmdtabn equ	($-cmdtaba)/9

tip	db	0,80 dup(' ')
panel1	db	09,'Bifid .x.'
panel2	db	12,'Twin Bifid 1'
sititle db	09,'Rate  Loc'
blktit	db	'Blk  DIC'

msg1	db	19,'Insufficient memory'
msg2	db	11,'Empty Bifid'
msg3	db	22,'Assumed block(s) .. ..'
msg4	db	11,'Bad command'
msg5	db	20,'Crib cannot be sited'
msg6	db	'   of    '
msg7	db	30,'Unexpected keysquare collision'
msg8	db	37,'Plain- ciphertext coordinate conflict'
msg9	db	31,'Character not in Bifid alphabet'
msg10	db	31,'Letter conflicts with keysquare'
msg11	db	37,'Complete 1st mark before starting 2nd'
msg12	db	28,'Marked texts not same length'
msg13	db	36,'Keysquare conflict, matching invalid'
msg14	db	36,'1st pair matching text marks pending'
msg15	db	25,'End marking 1st pair text'
msg16	db	37,'Matching text too long, chopped to 75'
msg17	db	23,'Invalid alphabet length'
msg18	db	27,'Duplicate alphabet letter .'
msg19	db	23,'Trap letters not a pair'
msg20	db	32,'Unable to create transaction log'
msg21	db	32,'Error writing to transaction log'
msg22	db	34,'Error reading from transaction log'
msg23	db	35,'Unrecoverable transaction log error'
msg24	db	24,'No prior changes to undo'
msg25	db	37,'Cipher(s) too big by ..... characters'
msg26	db	39,"Can't change solved position.  Use undo"
msg27	db	36,'2nd pair matching text marks pending'
msg28	db	14,'Match len     '
msg29	db	24,'Invalid alphabet for 6x6'
msg30	db	19,'Invalid permutation'
msg31	db	20,'Invalid block length'
msg32	db	30,'Block length bigger than bifid'
msg33	db	'   of  10'

;		  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
;6		ext	  not-assigned		rub tab 	     cr
keytab	db	000,001,001,001,001,001,001,001,002,003,001,001,001,004,001,001

;		 16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31
;							    esc
	db	001,001,001,001,001,001,001,001,001,001,001,005,001,001,001,001

;		 32  33  34  35  36  37  38  39  40  41  42  43  44  45  46  47
;		bln   !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /
	db	006,007,007,007,007,007,007,007,007,007,007,007,007,007,007,007

;		 48  49  50  51  52  53  54  55  56  57  58  59  60  61  62  63
;		  0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?
	db	008,008,008,008,008,008,008,008,008,008,007,007,007,007,007,007

;		 64  65  66  67  68  69  70  71  72  73  74  75  76  77  78  79
;		  @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
	db	007,009,009,009,009,009,009,009,009,009,009,009,009,009,009,009

;		 80  81  82  83  84  85  86  87  88  89  90  91  92  93  94  95
;		  P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _
	db	009,009,009,009,009,009,009,009,009,009,009,007,007,007,007,007

;		 96  97  98  99 100 101 102 103 104 105 106 107 108 109 110 111
;		  `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o
	db	007,010,010,010,010,010,010,010,010,010,010,010,010,010,010,010

;		112 113 114 115 116 117 118 119 120 121 122 123 124 125 126 127
;		  p   q   r   s   t   u   v   w   x   y   z  {	 |   }	 ~  del
	db	010,010,010,010,010,010,010,010,010,010,010,007,007,007,007,011

;		128 129 130 131 132 133 134 135 136 137 138 139 140 141 142 143
;		  Ä   Å   Ç   É   Ñ   Ö   Ü   á   à   â   ä   ã   å   ç   é   è
	db	012,013,013,013,013,013,013,013,013,013,013,013,013,013,012,012

;		144 145 146 147 148 149 150 151 152 153 154 155 156 157 158 159
;		  ê   ë   í   ì   î   ï   ñ   ó   ò   ô   ö   õ   ú   ù   û   ü
	db	012,013,012,013,013,013,013,013,013,012,012,007,007,007,007,007

;		160 161 162 163 164 165 166 167 168 169 170 171 172 173 174 175
;		  †   °   ¢   £   §   • 	  ®   ©   ™   ´   ¨   ≠   Æ   Ø
	db	013,013,013,013,013,012,001,001,007,007,007,007,007,007,007,007

;		176 177 178 179 180 181 182 183 184 185 186 187 188 189 190 191
	db	001,001,001,001,001,001,001,001,001,001,001,001,001,001,001,001

;		192 193 194 195 196 197 198 199 200 201 202 203 204 205 206 207
	db	001,001,001,001,001,001,001,001,001,001,001,001,001,001,001,001

;		208 209 210 211 212 213 214 215 216 217 218 219 220 221 222 223
	db	001,001,001,001,001,001,001,001,001,001,001,001,001,001,001,001

;		224 225 226 227 228 229 230 231 232 233 234 235 236 237 238 239
;		      ·
	db	001,013,001,001,001,001,001,001,001,001,001,001,001,001,001,001

;		240 241 242 243 244 245 246 247 248 249 250 251 252 253 254 255
	db	001,001,001,001,001,001,001,001,001,001,001,001,001,001,001,001

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
crypt25b ends
