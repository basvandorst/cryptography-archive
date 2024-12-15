cryptgb segment 'cryb'
c2	dw	2
c1416	dw	1416		;max x for little x routine, e^-1416/2 +
;				 causes underflow in little x
m1	dw	-1
m16	dw	-16
halfx	dq	0.0	;a in algorithm 299
yexp	dq	0.0	;y in algorithm 299
s	dq	0.0
bigc	dq	0.0	;c for bigx
invsqrpi dq	0.564189583547756	;1/sqrt(pi)
lnsqrpi dq	0.5723649429247 	;ln sqrt(pi)
half	dq	0.5	;z for odd degrees of freedom
;PNORM polynomial coefficients
coef	dq	0.5383E-5	;+fz^6
	dq	0.488906E-4	;+ez^5
	dq	0.380036E-4	;+dz^4
	dq	0.32776263E-2	;+cz^3
	dq	0.0211410061	;+bz^2
	dq	0.049867347	;az		or something like that
status	dw	0	;8087 status bits
flag1	db	0	;01 = big x caused underflow for exp(-x/2)
			;02 = degrees of freedom is odd number
cryptgb ends
