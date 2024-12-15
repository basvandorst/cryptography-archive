#ifndef __SQCBC_H
#define __SQCBC_H

/*
	Cipher Block Chaining (CBC) mode support

	IMPORTANT REMARKS:
	
	1.	Input buffers to the encryption/decryption functions
		must be word-aligned on machines which have this as a requirement.
	2.	The buffer lengths must be multiples of SQUARE_BLOCKSIZE.
*/

#include "square.h"

typedef struct {
	squareKeySchedule roundKeys_e, roundKeys_d;
	squareBlock mask;
} squareCbcContext;

void squareCbcInit    (squareCbcContext *ctxCbc, const squareBlock key);
void squareCbcSetIV   (squareCbcContext *ctxCbc, const squareBlock iv);
void squareCbcEncrypt (squareCbcContext *ctxCbc, byte *dataBuffer, unsigned dataLength);
void squareCbcDecrypt (squareCbcContext *ctxCbc, byte *dataBuffer, unsigned dataLength);
void squareCbcFinal   (squareCbcContext *ctxCbc);

#endif /* __SQCBC_H */
