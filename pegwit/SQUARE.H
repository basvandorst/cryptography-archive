#ifndef __SQUARE_H
#define __SQUARE_H

#define R 6	/* number of rounds */
#define SQUARE_BLOCKSIZE (4*sizeof(word32))

#ifndef USUAL_TYPES
#define USUAL_TYPES
typedef unsigned char	byte;	/*  8 bit */
typedef unsigned short	word16;	/* 16 bit */
typedef unsigned long	word32;	/* 32 bit */
#endif /* ?USUAL_TYPES */

void squareGenerateRoundkeys (word32 key[4],
	word32 roundkeys_e[R+1][4], word32 roundkeys_d[R+1][4]);
void squareEncrypt (word32 text[4], word32 roundkeys_e[R+1][4]);
void squareDecrypt (word32 text[4], word32 roundkeys_d[R+1][4]);

#endif /* __SQUARE_H */
