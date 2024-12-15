;ppmp.h ver2.0

;	1991/92 by Yuji KIDA

FMR		equ	0

HigherPower	equ	4096	;if p is larger than this then
				;higer powers are not checked.
				;used in mpqshd2p
ansDword	equ	10	;dword length of sieve answer


maxprimes	equ	0f000h
maxbufferbytes	equ	maxprimes/4
maxbufferwords	equ	maxprimes/8

primeunitbytes	equ	16

mask16bit	equ	0000ffffh
mask24bit	equ	00ffffffh
deletedmark4B	equ	80000000h

;pmodeworkadr	equ	00100000h
pmodeworkadr	equ	00110000h

;
; data area for each prime
;
primeadr	equ	pmodeworkadr

  if FMR
mainRAMover	equ	0c0000h
primeadr2	equ	80000h	;0c0000h-80000h >=4bytes*maxprimes
primeseg2	equ	8000h	;primeadr2/10h
  else
mainRAMover	equ	0a0000h
primeadr2	equ	60000h	;0a0000h-60000h >= 4bytes*maxprimes
primeseg2	equ	6000h	;primeadr2/10h
  endif
optionRAM	equ	mainRAMover-primeadr2	;must be a multiple of
						;primeunitbytes

;
; sieve constants
;
sieveRep	equ	8
sieveRepLog	equ	3	;= log of sieveRep

;
; LPV constants
;
QSORTCUTOFF	EQU	40

R1_BYTES	EQU	4
R2_BYTES	EQU	4
maxXbytes	EQU	80

COMBIMASK	EQU	80000000h
MAXCOMBI	EQU	16
COMBIUNITBYTES	EQU	MAXCOMBI*4

RAMUNITBYTES	EQU	12
RAMUNITDWORDS	EQU	3	;RAMUNITBYTES/4

;
; Gauss constants
;

matrixunitsize	equ	00020h
matrixunitmask	equ	0ffe0h

GaussAdr	equ	pmodeworkadr

;
; real-protect common parameter area
;

_memoseg	equ	primeseg2-200h
_memoadr	equ	_memoseg*10h

_result		equ	0+_memoadr
_primes		equ	4+_memoadr
_primesR	equ	4
_primes1	equ	8+_memoadr
_primes2	equ	0ch+_memoadr
_primes3A	equ	10h+_memoadr
_primes3B	equ	14h+_memoadr
_primes4	equ	18h+_memoadr

_inilog		equ	20h+_memoadr
_inilogR	equ	20h
_cutlog		equ	24h+_memoadr
_cutlogR	equ	24h
_offset		equ	28h+_memoadr
_sievewidth	equ	2ch+_memoadr
_sievewidthR	equ	2ch
_start		equ	30h+_memoadr
_absQ		equ	34h+_memoadr
_absQR		equ	34h

_sievetop	equ	40h+_memoadr	;= pmodeworkadr
					;+ maxprimes * primeunitbytes
_sievetopR	equ	40h
_sieveBsize	equ	44h+_memoadr	;= sieveover - sievetop
_sieveBsizeR	equ	44h
_sieveConst	equ	48h+_memoadr	;= sieveBsize - sievetop
_sieveConstR	equ	48h
_sieveover	equ	4ch+_memoadr
_sieveoverR	equ	4ch
_sieveXsize	equ	50h+_memoadr	;= sieveBsize\sieveRep
_sieveXsizeR	equ	50h
_sieveWsize	equ	54h+_memoadr	;= sieveBsize\2
_sieveWsizeR	equ	54h
_sieveDsize	equ	58h+_memoadr	;= sieveBsize\4
_sieveDsizer	equ	58h

_sortingtop	equ	60h+_memoadr

_D		equ	80h+_memoadr
_DR		equ	80h
_MIA2P		equ	0c0h+_memoadr

_bigpivot	equ	_D

_B		equ	0100h+_memoadr
_BR		equ	0100h

_W		equ	0140h+_memoadr	;also used by ISORT
_WR		equ	0140h

		;200h-2ffh are gdttbl

_gdttblseg	equ	_memoseg+20h
_gdttbladr	equ	_gdttblseg*10h

;
; sieve answer area
;
sieveansarea	equ	_memoadr+300h

