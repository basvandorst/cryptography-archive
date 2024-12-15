crypt9b segment 'cryb'
wrdpos	dw	111Ah	;row 18, col 27 for cursor position
olditem dw	0	;index of this 1st word index
calcpos dw	0
motlang dw	type motndx
c16	dw	16
c160	dw	160
itemb	dw	0		;-> cipher word index entry
itema	dw	0
;save memory ptr and space for restoring, 1st list
freeb	dw	0
freea	dw	0
saveb	dw	0
savea	dw	0

;save memory ptr and space for restoring, 2nd list
free2b	dw	0	;begin freespace for 2nd list, offset
free2a	dw	0	;			       segment
save2b	dw	0	;remaining freespace bytes, hi order 16 bits
save2a	dw	0	;			    lo order 16 bits

lstptr	dw	0	;which list to get
;combined word indexes
w1len	dw	0	;length of 1st cipher word
w1cnt	dw 16 dup (0)	;letter counts
word1	db 16 dup(' ')  ;1st cipher word
w2len	dw	0	;length of 2nd cipher word
word2	db 16 dup(' ')  ;2nd cipher word
shrcnt	dw	0	;count of letters shared by both words
shroff	db 16 dup(0,0)	;offsets to position of shared letter in each word
colordr dw	0	;number of columns
	db 16 dup(0)	;offsets to columns
joinln	dw	0	;joint length of both words
joinwd	db 32 dup(' ')  ;join of both words
joinpta db 32 dup(0)	;joint pattern of cipher words
joinptb db 32 dup(0)	;joint pattern of plaintext words
i	dw	0	;index to first list
j	dw	0	;index to second list
m	dw	0	;save i for m x n search
n	dw	0	;save j for m x n search
list1n	dw	0	;count of words in first list
list2n	dw	0	;count of words in second list
sub1	dw	0	;count of list1 words with same letters in common cols
sub2	dw	0	;ditto for list2 words
list1b	dw	0	;segment:offset -> word in 1st list
list1a	dw	0
list2b	dw	0	;segment:offset -> word in 2nd list
list2a	dw	0
maxlen	dw	0	;max memory needed for final list
destb	dw	0	;list ptr after filtering and sort	offset
desta	dw	0	;					segment
flag1	db	0	;01 = reduction in list1
			;02 = reduction in list2
msg1	db	25,'Pattern not in dictionary'
msg2	db	18,'This list is empty'
msg3	db	31,'Not enough memory for word list'
msg4	db	25,'Pattern dictionary broken'
msg5	db	30,'Not enough memory to sort list'
msg6	db 5 dup(' ')   ;updated count of words
msg7	db	27,'Choose a primary word first'
msg8	db	37,'Primary word.  Pick another secondary'
msg9	db	33,'Not enough memory for second list'
msg10	db	23,'Primary word list empty'
msg11	db	25,'Alternate word list empty'
msg12	db	36,'File error, new list request ignored'
crypt9b ends
