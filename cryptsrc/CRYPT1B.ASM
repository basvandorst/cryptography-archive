crypt1b segment 'cryb'
poscmd	label	word	;command line cursor position
colcmd	db	0
rowcmd	db	12
posgrm	label	word	;cryptogram cursor position
colgrm	db	0
rowgrm	db	1
poshdr	label	word	;title line cursor position
colhdr	db	0
rowhdr	db	0
statline dw	24*160	;offset to line 25, status/message line
savechar dw	0	;keyboard character input
normal	db	07h	;normal video attribute
reverse db	70h	;reverse video attribute
under	db	01h	;underline video attribute
blink	db	80h	;blink video attribute
bright	db	08h	;intense video attribute
language db 80 dup(' ') ;fileid of language data file
langname db 13 dup(' ') ;language display name without path or drive
gramid	db 80 dup (' ') ;fileid of cryptogram
gramid2 db 15 dup (' ') ;cryptogram display name without path
flag1	db	0Ch

			;04 = fold crypt letters to upper case
			;08 = fold substitution letters to lower
			;40 = quit ok
flag2	db	0	;01 = pending punctuation
input	equ	07h	;modes that permit character input
command db	0,80 dup(' ')   ;command length and buffer
titre	db	0,80 dup(' ')   ;title line length and buffer
rdbuf	db	81 dup(' ')
alphac	dw	26	;number of cipher alphabet characters
	db	'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
	db 230 dup(' ') ;for now up to 256 characters
alphap	db	'abcdefghijklmnopqrstuvwxyz'
	db 230 dup(' ')
alphas	db 256 dup(' ') ;solved alphabet characters
alphai	db 256 dup(' ') ;inverse solved alphabetic characters
;normal alphabet probabilites
prob	dd     0.0805	;A
	dd     0.0153	;B
	dd     0.0310	;C
	dd     0.0397	;D
	dd     0.125	;E
	dd     0.0231	;F
	dd     0.0195	;G
	dd     0.0542	;H
	dd     0.0729	;I
	dd     0.0016	;J
	dd     0.0066	;K
	dd     0.0414	;L
	dd     0.0254	;M
	dd     0.0710	;N
	dd     0.0760	;O
	dd     0.0202	;P
	dd     0.0011	;Q
	dd     0.0613	;R
	dd     0.0655	;S
	dd     0.0925	;T
	dd     0.0272	;U
	dd     0.01	;V
	dd     0.0188	;W
	dd     0.002	;X
	dd     0.0172	;Y
	dd     0.001	;Z
	dd 230 dup (0.0)
;all other ascii characters are separators, ignored for counting, respected
;for appearance when present
panelid db	4,'Edit'
msg1	db	14,'New cryptogram'
msg2	db	11,'Title entry'
msg3	db	16,'Cryptogram entry'
msg4	db	11,'..... .....'        ;row and column, cursor position
err5	db	36,'Changed.  Use shift-F2 to force quit'
err6	db	19,'Cryptogram too long'
err7	db	32,'Not enough memory for cryptogram'
err8	db	10,'File error'
err9	db	09,'Disk full'
err10	db	12,'No file name'
derr1	db	'Error reading cryptogram file',0Dh,0Ah,'$'
merr1	db	'Cryptogram file too big',0Dh,0Ah,'$'
merr2	db	'Not enough memory for cryptogram',0Dh,0Ah,'$'
cmdret	dw	escret		;return from get command execution
	dw	escret		;return from name command execution
	dw	edit02		;return from start command execution
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
crypt1b ends
