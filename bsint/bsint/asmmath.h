#if __cplusplus
extern "C" {
#endif

/*
	NOTATION:
		U1:U2:...:Un	32n bit number. U1 is most significant 32 bits.
*/	

/*
		W[0]:W[1] = U * V
*/
void	mul32( unsigned long * W, unsigned long U, unsigned long V );

/*
	K:W = W + U * V + K
*/
void	mulstep( unsigned long * W, unsigned long * K, unsigned long U, unsigned long V );

/*
	K:W = K + U + V
*/
void	addstep( unsigned long * W, unsigned long * K, unsigned long U, unsigned long V );

/*
	while ( Q * V1:V2 > U0:U1:U2 )
		Q--;
*/
void	qstep( unsigned long * Q, unsigned long V1, unsigned long V2, unsigned long U0, unsigned long U1, unsigned long U2 );

/*
	Q = U0:U1 / V, R = U0:U1 % V
	
	If Q overflows, it is set to 0xFFFFFFFF.
*/
void	div32( unsigned long * Q, unsigned long * R, unsigned long U0, unsigned long U1, unsigned long V );

#if __cplusplus
}
#endif

