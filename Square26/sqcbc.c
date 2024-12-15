/*----------------------------*/
/* Cipher Block Chaining Mode */
/*----------------------------*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "square.h"
#include "sqcbc.h"

#define D(p) ((word32 *)(p))

#define COPY_BLOCK(target, source) \
{ \
	(target)[0] = (source)[0]; \
	(target)[1] = (source)[1]; \
	(target)[2] = (source)[2]; \
	(target)[3] = (source)[3]; \
} /* COPY_BLOCK */


void squareCbcInit (squareCbcContext *ctxCbc, const squareBlock key)
{
	assert (ctxCbc != NULL);
	assert (key != NULL);
	memset (ctxCbc, 0, sizeof (squareCbcContext));
	squareGenerateRoundKeys (key, ctxCbc->roundKeys_e, ctxCbc->roundKeys_d);
} /* squareCbcInit */


void squareCbcSetIV (squareCbcContext *ctxCbc, const squareBlock iv)
{
	assert (ctxCbc != NULL);
	if (iv) {
		memcpy (ctxCbc->mask, iv, SQUARE_BLOCKSIZE);
	}
} /* squareCbcSetIV */


void squareCbcEncrypt (squareCbcContext *ctxCbc, byte *buffer, unsigned length)
{
	byte *mask;

	assert (ctxCbc != NULL);
	assert (buffer != NULL);
	mask = ctxCbc->mask;
	while (length >= SQUARE_BLOCKSIZE) {
		/* mask and encrypt the current block: */
		D(buffer)[0] ^= D(mask)[0];
		D(buffer)[1] ^= D(mask)[1];
		D(buffer)[2] ^= D(mask)[2];
		D(buffer)[3] ^= D(mask)[3];
		squareEncrypt (D(buffer), ctxCbc->roundKeys_e);
		/* chain the block into the mask: */
		mask = buffer;
		/* proceed to the next block, if any: */
		buffer += SQUARE_BLOCKSIZE;
		length -= SQUARE_BLOCKSIZE;
	}
	assert (length == 0);
	/* chain the block into the mask: */
	COPY_BLOCK (D(ctxCbc->mask), D(mask));
	mask = NULL;
} /* squareCbcEncrypt */


void squareCbcDecrypt (squareCbcContext *ctxCbc, byte *buffer, unsigned length)
{
	squareBlock temp;
 
	assert (ctxCbc != NULL);
	assert (buffer != NULL);
	while (length >= SQUARE_BLOCKSIZE) {
		/* save the current block for chaining: */
		COPY_BLOCK (D(temp), D(buffer));
		/* decrypt and unmask the block: */
		squareDecrypt (D(buffer), ctxCbc->roundKeys_d);
		D(buffer)[0] ^= D(ctxCbc->mask)[0];
		D(buffer)[1] ^= D(ctxCbc->mask)[1];
		D(buffer)[2] ^= D(ctxCbc->mask)[2];
		D(buffer)[3] ^= D(ctxCbc->mask)[3];
		/* update the mask: */
		COPY_BLOCK (D(ctxCbc->mask), D(temp));
		/* proceed to the next block, if any: */
		buffer += SQUARE_BLOCKSIZE;
		length -= SQUARE_BLOCKSIZE;
	}
	assert (length == 0);
#ifdef DESTROY_TEMPORARIES
	/* destroy potentially sensitive data: */
	memset (temp, 0, sizeof (temp));
	/* N.B. this cleanup is in principle unnecessary */
	/* as temp only contains encrypted (public) data */
#endif /* ?DESTROY_TEMPORARIES */
} /* squareCbcDecrypt */


void squareCbcFinal (squareCbcContext *ctxCbc)
{
	assert (ctxCbc != NULL);
	memset (ctxCbc, 0, sizeof (squareCbcContext));
} /* squareCbcFinal */
