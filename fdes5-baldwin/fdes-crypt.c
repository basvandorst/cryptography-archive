/* Fast password transform.
 */

#include	"fdes.h"
 

extern	unsb	E[];
extern	unsb	FP[];
extern	fsetkey();
extern	xform();


/*
 * The current block, divided into 2 halves.
 */
static	obpb1	L[32], R[32];



/* Final permutation.  Takes input from globals L and R.
 */
Fperm(block)
	unsb *block;
{
	reg j;

	for (j = 0 ; j < 64 ; j++)
		block[j] = L[FP[j]];
}


/* E permutation.
 */
doe(bitarray, ptrlw, ptrhw)
     unsb *bitarray;
     unsl *ptrlw, *ptrhw;
{

}


/* Inverse of E permutation.
 */
undoe(fromarr, toarr)
     obpb1	*fromarr;
     obpb1	*toarr;
{
	reg j;

	for (j = 0 ; j < 32 ; j++)
	  toarr[j] = fromarr[1 + (j & 03) + 6 * (j >> 2)];
}


toBA64(quarters, onebits64)
reg	sbpb24	*quarters;
	obpb1	*onebits64;
{
	int	j;
static	obpb1 tmpE[48];
reg	unsb	*onebits48;
reg	sbpb24	quarter;
	
	onebits48 = tmpE;
	quarter = sixbitTOtf(*quarters++);
	for (j = 0 ; j < 24 ; j++)
		*onebits48++ = (quarter >> j) & 01;
	quarter = sixbitTOtf(*quarters++);
	for ( j = 0 ; j < 24 ; j++)
		*onebits48++ = (quarter >> j) & 01;
	undoe(tmpE,L);
	
	onebits48 = tmpE;
	quarter = sixbitTOtf(*quarters++);
	for ( j = 0 ; j < 24 ; j++)
		*onebits48++ = (quarter >> j) & 01;
	quarter = sixbitTOtf(*quarters++);
	for ( j = 0 ; j < 24 ; j++)
		*onebits48++ = (quarter >> j) & 01;
	undoe(tmpE,R);

	Fperm(onebits64);
}


char *fcrypt(pw,salt)
char *pw;
char *salt;
{
reg	int  i, j, c;
static	obpb1 block[66];
static	char iobuf[16];
static	sbpb24 out96[4];
	sbpb24 saltvalue;

	for (i = 0 ; i < 66 ; i++)
		block[i] = 0;
	for (i = 0 ; (c = *pw) && i < 64 ; pw++)  {
		for (j = 0 ; j < 7 ; j++, i++)
			block[i] = (c >> (6 - j)) & 01;
		i++;		/* Skip parity bit. */
		}
	
	fsetkey(block);
	
	for (i = 0 ; i < 66 ; i++)
		block[i] = 0;

	saltvalue = 0;
	for (i = 0 ; i < 2 ; i++)  {
		c = *salt++;
		iobuf[i] = c;
		if (c > 'Z')
			c -= 6;
		if (c > '9')
			c -= 7;
		c -= '.';
		for (j = 0 ; j < 6 ; j++)
			saltvalue |= ((c >> j) & 01) << (6 * i + j);
		}
	saltvalue = tfTOsixbit(saltvalue);
	
	xform(out96, saltvalue);
	toBA64(out96, block);
	
	for (i = 0 ; i < 11 ; i++)  {
		c = 0;
		for (j = 0 ; j < 6 ; j++)  {
			c <<= 1;
			c |= block[6 * i + j];
			}
		c += '.';
		if (c > '9')
			c += 7;
		if (c > 'Z')
			c += 6;
		iobuf[i+2] = c;
	}
	iobuf[i+2] = 0;
	if (iobuf[1] == 0)
		iobuf[1] = iobuf[0];
	return(iobuf);
}
