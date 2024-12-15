/* This program produces C source containing static initializations
 * for the combined S and P boxes in DES.
 *
 * Usage: gensp c > dessp.c (for C version)
 *        gensp a > desspa.c (for assembler versions)	
 *
 * Note! There are two different formats of the output, one for the portable
 * C version and the other for the 386/486 inline assembler version. In the
 * portable version, each entry is rotated 1 bit to the left to match the
 * format of the L and R halves in that code; this avoids one of the left
 * rotates that would otherwise be required in each round.
 *
 * In the assembler version, each entry is rotated by 2 more bits (3 total)
 * to match that version's convention of rotating the L and R halves left
 * 3 bits to avoid two shift operations that would otherwise be required
 * in each round to convert the 6-bit index into the SP array into a
 * memory offset. So be sure to run this program with the correct argument.
 *
 * August 1994, Phil Karn, KA9Q
 */

#include <stdio.h>

unsigned long Spbox[8][64];		/* Combined S and P boxes */

/* 32-bit permutation function P used on the output of the S-boxes */
static unsigned char p32i[] = {	
	16,  7, 20, 21,
	29, 12, 28, 17,
	 1, 15, 23, 26,
	 5, 18, 31, 10,
	 2,  8, 24, 14,
	32, 27,  3,  9,
	19, 13, 30,  6,
	22, 11,  4, 25
};

/* The (in)famous S-boxes */
static unsigned char sbox[8][64] = {
	/* S1 */
	14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
	 0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
	 4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
	15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13,

	/* S2 */
	15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
	 3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
	 0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
	13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9,

	/* S3 */
	10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
	13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
	13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
	 1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12,

	/* S4 */
	 7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
	13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
	10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
	 3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14,

	/* S5 */
	 2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
	14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
	 4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
	11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3,

	/* S6 */
	12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
	10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
	 9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
	 4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13,

	/* S7 */
	 4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
	13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
	 1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
	 6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12,

	/* S8 */
	13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
	 1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
	 7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
	 2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
};

int desinit(int);

main(argc,argv)
int argc;
char *argv[];
{
	int i,j,k;
	int shifts;

	if(argc != 2){
		fprintf(stderr,"Usage: %s a|c\n",argv[0]);
		return 0;
	}
	switch(argv[1][0]){
	case 'a':	/* Generate for assembler version */
		shifts = 3;
		printf("unsigned long Spboxa[8][64] = {\n");
		break;
	case 'c':	/* Generate for portable C version */
		shifts = 1;
		printf("unsigned long Spbox[8][64] = {\n");
		break;
	default:
		printf("Invalid code %s\n",argv[1]);
		exit(1);
	}

	desinit(0);
	k = 0;

	for(i=0;i<8;i++){
		for(j=0;j<64;j++){
			Spbox[i][j] = (Spbox[i][j] << shifts) | (Spbox[i][j] >> (32-shifts));
			printf("0x%08lx,",Spbox[i][j]);
			if(++k == 4){
				printf("\n");
				k = 0;
			}
		}
	}
	printf("};\n");
	return 0;
}

/* Initialize the lookup table for the combined S and P boxes
 * "mode" no longer used; only standard DES supported
 */
int
desinit(mode)
int mode;
{
	unsigned char pbox[32];
	int p,i,s,j,rowcol;

	/* Compute pbox, the inverse of p32i.
	 * This is easier to work with
	 */
	for(p=0;p<32;p++){
		for(i=0;i<32;i++){
			if(p32i[i]-1 == p){
				pbox[p] = i;
				break;
			}
		}
#ifdef	DEBUG
		printf("pbox[%d] = %d\n",p,pbox[p]);
#endif
	}
	for(s = 0; s < 8; s++){			/* For each S-box */
		for(i=0; i<64; i++){		/* For each possible input */
			Spbox[s][i] = 0;
			/* The row number is formed from the first and last
			 * bits; the column number is from the middle 4
			 */
			rowcol = (i & 32) | ((i & 1) ? 16 : 0) | ((i >> 1) & 0xf);
			for(j=0;j<4;j++){	/* For each output bit */
				if(sbox[s][rowcol] & (8 >> j)){
				 Spbox[s][i] |= 1L << (31 - pbox[4*s + j]);
				}
			}
#ifdef	DEBUG
			printf("Spbox[%d][%2d] = %08lx\n",s,i,Spbox[s][i]);
#endif
		}
	}
	return 0;
}

