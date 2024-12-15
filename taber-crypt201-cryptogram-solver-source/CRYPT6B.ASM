crypt6b segment 'cryb'
oldcur	dw	0	;editor's current line index
oldgram dw	0	;editor's current line ptr
oldpos	dw	0	;previous panel's cursor position
alfnow	dw	0	;index into displayed alphabet
;keep following 4 fields together in this order
oldszb	dw	0	;old free space hi order bytes
oldsza	dw	0	;		lo order bytes
oldfrb	dw	0	;old free space offset
oldfra	dw	0	;		segment

posnow	dw	0	;substitution area cursor position
wrdnow	dw	111Ah	;row 18, col 27 	cipher word list
l1now	dw	0F2Dh	;row 16, col 46 	1st pattern list
l2now	dw	0F40h	;row 16, col 65 	2nd pattern list
savpos	dw	0	;save current cursor position for skipping
savechar dw	0	;entered keyboard character

wordb	dw	0	;offset to plaintext word to be substituted
worda	dw	0	;seg of plaintext word to be substituted
c16	dw	16
c160	dw	160
flag1	db	0	;01 = message to be cleared
			;02 = switch backwards
			;04 = trial substitution

area	db	0	;0 = I'm in substitution work area
			;1 = I'm in cipher word list area
			;2 = I'm in first plain word area
			;3 = I'm in second plain word area
;cursor up
ucurs	dw	subup	;cursor up for substitution
	dw	wup	;cursor up for cipher word list
	dw	pup	;cursor up for primary pattern list
	dw	aup	;cursor up for alternate pattern list
;move cursor down
dcurs	dw	subdn	;cursor down for substitution
	dw	wdn	;cursor down for cipher word list
	dw	pdn	;cursor down for primary pattern list
	dw	adn	;cursor down for alternate pattern list
;switch areas
areasw	dw	gosub	;substitution work area
	dw	gowrd	;cipher list area
	dw	go1st	;1st list area
	dw	go2nd	;2nd list area

savalfs db 256 dup(' ') ;save area for solved letters
savalfi db 256 dup(' ') ;save area for inverse of solved letters

panelid db	19,'Simple substitution'
msg1	db	30,'Not a defined plaintext letter'
msg2	db	27,'Not a defined cipher letter'
msg3	db	19,'Contrast filter off'
msg4	db	18,'Contrast filter on'
msg5	db	21,'No pattern dictionary'
msg6	db	20,'Agreement filter off'
msg7	db	19,'Agreement filter on'
msg8	db	24,'Probable fit sorting off'
msg9	db	23,'Probable fit sorting on'
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
crypt6b ends
