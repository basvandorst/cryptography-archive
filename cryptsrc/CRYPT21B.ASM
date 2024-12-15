crypt21b segment 'cryb'
ascfp	db 26 dup(' ')          ;floating point conversion area
;keep following four fields together
saveb	dw	0		;beginning freespace size
savea	dw	0
freeb	dw	0		;begining freespace origin
freea	dw	0

oldpos	dw	0		;old cursor position

;keep cursor positions together
rtepos	dw	0002h		;route cursor position
dimpos	dw	000Bh		;dimension cursor position
cippos	dw	1003h		;cipher cursor position
solpos	dw	3203h		;solving cursor position
logpos	dw	1002h		;log choices cursor position
savepos dw	0

;clearing areas parameters
;cipher display
klarcip dw	160+26		;row 2, col 14 is origin
	dw	20		;twenty rows
	dw	33		;33 columns
;solution display
klarsol dw	160+94		;row 2, col 48 origin
	dw	20		;twenty rows
	dw	33		;33 columns
;computed permutation display
klarsug dw	22*160+26	;row 23, column 14 origin
	dw	2		;two rows
	dw	67		;67 columns

curdim	dw	0		;current top of box dimension

curcol1 dw	0		;current starting cipher column
currow1 dw	0		;current starting cipher row

curcol2 dw	0		;current starting solving column
currow2 dw	0		;current starting solving row

thisrow dw	0		;either cipher or solution current row
thiscol dw	0		;either cipher or solution current col
screenp dw	0		;either cipher or solution screen pos
sourcep dw	0		;either cipher or solution source letters

solcols dw	0		;total of columns moved to solving area
solrows dw	0		;total of rows moved to solving area

;dynamic storage anchors
cipherb dw	0		;ciphertext	offset
ciphera dw	0		;		segment
rectb	dw	0		;permuted cipher work area
refresh dw	0		;permuted cipher refresh area
solveb	dw	0		;cipher solving area
indexb	dw	0		;permutation index offset
workb	dw	0		;permutation work area
colptrb dw	0		;start of solved permuted col indexes
rowptrb dw	0		;start of solved permuted row indexes

;currently selected route, direction, and row and col dimensions
chosen1 dw	0		;route chosen
chosen2 dw	0		;..direction
chosen3 dw	0		;row dimension
chosen4 dw	0		;col dimension
;current transform, direction null, row and col dimensions
chosen5 dw	0		;transform
	dw	0		;direction, don't care
	dw	0		;row dimension for this transform
	dw	0		;col dimension for this transform
;out route
chosen9 dw	0		;route
	dw	0		;direction
	dw	0		;row dimension
	dw	0		;col dimension
chosen	dw	0		;current route and direction

;index generator routine table
iotagen dd	i_col		;column order generator
	dd	i_row		;row order generator
	dd	i_diag		;diagonal order generator
	dd	i_spiral	;cw spiral order generator
	dd	i_zcol		;zigzag column order generator
	dd	i_zrow		;zigzag row order generator
	dd	i_zdiag 	;zigzag diagonal order generator
	dd	i_revrow	;row axis reverser
	dd	i_revcol	;col axis reverser
	dd	i_tran		;transpose
	dd	i_atran 	;anti-transpose

letcnt	dw	0		;number of letters in cryptogram
vowel	dw	0		;number of vowels in cryptogram
vcount	dw	0		;number of vowels in this row
factn	dw	0		;number of pairs of factors
factb	dw	0		;->factors	offset
facta	dw	0		;		segment
dimn	dw	0		;number of possible dimensions
dimenb	dw	0		;->dimension table	offset
dimena	dw	0		;			segment
markdcol dw	0		;column index marked for moving
markarea db	0
	db	?
scol	dw	0		;squeezed out solution column
c10	dw	10
c12	dw	12
c33	dw	33
c100	dw	100
c160	dw	160

flag1	db	0		;01 = cipher rectangle displayed
				;04 = dimension list selection enabled
				;08 = displayed message to clear
				;10 = outbound route
				;20 = route/dimension 1st toggle
				;40 = extant solution
				;80 = going forward

flag2	db	0		;01 = logging mode
				;02 = pending row or column mark

;next area branch table
avance	dw	area1		;go to dimension list
	dw	area2		;go to cipher rectangle if possible
	dw	area3		;go to solving work area if possible
	dw	area0		;go to route list

;previous area branch table
reverse dw	area3		;go to solving work area if possible
	dw	area0		;go to route list
	dw	area1		;go to dimension list
	dw	area2		;go to cipher rectangle if possible

;cursor down branch table
cursdn	dw	rtdown		;route area cursor down
	dw	dmdown		;dimension down
	dw	r1down		;rectangle 1 area down
	dw	r2down		;rectangle 2 area down

;cursor up branch table
cursup	dw	rtup
	dw	dmup
	dw	r1up
	dw	r2up

;cursor right branch table
curight dw	forever 	;not meaningful in route list
	dw	forever 	;..nor in dimension list
	dw	ciprit		;go right in cipher
	dw	solrit		;go right in solution area

;cursor left branch table
curlef	dw	forever 	;not meaningful in route list
	dw	forever 	;..nor in dimension list
	dw	ciplef		;go left in cipher
	dw	sollef		;go left in cipher

;select branch table
select	dw	rtsel		;route selected
	dw	dmsel		;dimension selected
	dw	forever 	;not meaningful in cipher rectangle area
	dw	forever 	;not meaningful in cipher solving area

;logging branch table
logkind dw	logrte		;log a route transposition
	dw	logcol		;log a columnar transposition
	dw	lognih		;log a nihilist transpostion

savechar dw	0		;keystroke and scan code

logarea dw 6 dup(0)

;direction flags for routes
rtflag	db	0		;01 = inverse column
				;02 = inverse row
				;04 = inverse diagonal
				;08 = inverse spiral
				;10 = inverse zigzag column
				;20 = inverse zigzag row
				;40 = inverse zigzag diagonal

area	db	0		;0 = route list
				;1 = dimension list
				;2 = transposition rectangle
				;3 = solving rectangle

group0	dw	6		;number of characters constant per string
	db	'AEIOUY'        ;..of text in English
	db 20 dup(' ')          ;..space for other languages

panelid db	13,'Transposition'

		;012345678901
routes	db	'D  ROUTES   '
	db	' Column     '  ;ravel in column order + or -
	db	' Row        '  ;ravel in row order + or -
	db	' Diagonal   '  ;ravel in diagonal order + or -
	db	' Spiral     '  ;ravel in spiral order + or -
	db	' Zig column '  ;      in zig-zag column order + or -
	db	' Zig row    '  ;      in zig-zag row order + or -
	db	' Zig diag.  '  ;      in zig-zag diagonal order + or -

dimtit	db	' R  C   VAR '

kinds	db	'…ÕÕÕÕÕÕÕÕÕÕª'
	db	'∫ Route    ∫'
	db	'∫ Columnar ∫'
	db	'∫ Nihilist ∫'
	db	'»ÕÕÕÕÕÕÕÕÕÕº'

trankey db	0	;length of key
	db 80 dup(' ')

msg1	db	19,'Insufficient memory'
msg2	db	25,'Cipher length indivisible'
msg3	db	20,'Select a route first'
msg4	db	26,'No current solution to log'
msg5	db	31,'Choose dimension for this route'
msg6	db	22,'Choose a takeout route'
msg7	db	21,'Choose a put-in route'
msg8	db	37,'Dimension chosen, choose put-in route'
msg9	db	18,'Choose a dimension'
msg10	db	37,'Designate type, or press F2 to cancel'
msg11	db	27,'Conflicts with pending mark'
msg12	db	12,'Empty column'
msg13	db	37,'Must be in solution area to move mark'
msg14	db	14,'No marked area'
msg15	db	29,'Incomplete column anagramming'
msg16	db	23,'Nihilist must be square'

;6		  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
;		ext	  not-assigned		rub tab 	     cr
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
crypt21b ends
