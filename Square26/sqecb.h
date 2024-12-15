#ifndef __SQECB_H
#define __SQECB_H

/*
	Electronic Code Book (ECB) mode support

	IMPORTANT REMARKS:
	
	1.	Input buffers to the encryption/decryption functions
		must be word-aligned on machines which have this as a requirement.
	2.	The buffer lengths must be multiples of SQUARE_BLOCKSIZE.
*/

#include "square.h"

typedef struct {
	squareKeySchedule roundKeys_e, roundKeys_d;
} squareEcbContext;

void squareEcbInit    (squareEcbContext *ctxEcb, const squareBlock key);
void squareEcbEncrypt (squareEcbContext *ctxEcb, byte *buffer, unsigned length);
void squareEcbDecrypt (squareEcbContext *ctxEcb, byte *buffer, unsigned length);
void squareEcbFinal   (squareEcbContext *ctxEcb);

#endif /* __SQECB_H */
