cryptsb segment 'cryb'
indexb	dw	0	;indexes of elements to be sorted
indexa	dw	0
aptr	dw	0	;top partition ptr to indexes
midptr	dw	0	;middle of partition ptr to indexes
zptr	dw	0	;bottom partition ptr to indexes
destptr dw	0	;destination ptr to indexes
haufen	dw	0	;count of stacked partitions
pivot	dw	0	;pivot index
pivb	dw	0	;pivot element, offset
piva	dw	0	;		segment
compb	dw	0	;left or right element, current  offset
compa	dw	0	;..comparand for pivot element	 segment
dir	dw	0	;up or down address
firstb	dw	0	;->first element, current partition
firsta	dw	0
midb	dw	0	;->middle element, current partition
mida	dw	0
lastb	dw	0	;->last element, current partition
lasta	dw	0
c16	dw	16
half1	dw	0	;count of elements in left partition
half2	dw	0	;count of elements in right partition

saveb	dw	0
savea	dw	0
freeb	dw	0	;save freeptr origin
freea	dw	0

holdb	dw	0	;->temp hold area
holda	dw	0
aptrb	dw	0	;->one list item
aptra	dw	0
bptrb	dw	0	;->another list item
bptra	dw	0

isortn	dw	0	;number of comparisons



inptr	dw	0	;which buffer is index into list
outptr	dw	0	;which buffer is output indexes after column sort
headptr dw	0	;which index entry is head of chain

colordn dw	0	;number of columns to sort on
	db 16 dup(0)	;offsets to columns

flag	db	0	;0 = use right as comparand
			;1 = use left as comparand
cryptsb ends
