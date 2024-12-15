#ifndef __SQCFB_H
#define __SQCFB_H

/*
	Cipher Feedback (CFB) mode support

	IMPORTANT REMARK: input buffers to the encryption/decryption functions
	must be word-aligned on machines which have this as a requirement.
*/

#include "square.h"

typedef struct {
	squareKeySchedule roundKeys;
	squareBlock mask;
	unsigned avail; /* number of available mask bytes */ 
} squareCfbContext;

void squareCfbInit    (squareCfbContext *ctxCfb, const squareBlock key);
void squareCfbSetIV   (squareCfbContext *ctxCfb, const squareBlock iv);
void squareCfbEncrypt (squareCfbContext *ctxCfb, byte *buffer, unsigned length);
void squareCfbDecrypt (squareCfbContext *ctxCfb, byte *buffer, unsigned length);
void squareCfbFinal   (squareCfbContext *ctxCfb);

#endif /* __SQCFB_H */
