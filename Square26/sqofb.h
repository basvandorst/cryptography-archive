#ifndef __SQOFB_H
#define __SQOFB_H

/*
	Output Feedback (OFB) mode support

	IMPORTANT REMARK: input buffers to the encryption/decryption functions
	must be word-aligned on machines which have this as a requirement.
*/

#include "square.h"

typedef struct {
	squareKeySchedule roundKeys;
	squareBlock mask;
	unsigned avail; /* number of available mask bytes */ 
} squareOfbContext;

void squareOfbInit  (squareOfbContext *ctxOfb, const squareBlock key);
void squareOfbSetIV (squareOfbContext *ctxOfb, const squareBlock iv);
void squareOfbMask  (squareOfbContext *ctxOfb, byte *buffer, unsigned length);
#define squareOfbEncrypt squareOfbMask
#define squareOfbDecrypt squareOfbMask
void squareOfbFinal (squareOfbContext *ctxOfb);

#endif /* __SQOFB_H */
