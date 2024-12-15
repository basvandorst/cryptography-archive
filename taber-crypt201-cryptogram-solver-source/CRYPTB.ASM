cryptb	segment 'cryb'
vcontrol db	2Ch,28h,2Dh,29h,2Ah,2Eh,1Eh,29h
err1	db	'Crypt cannot run; no co-processor',0Dh,0Ah,'$'
cryptb	ends
