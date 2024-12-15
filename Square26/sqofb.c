/*----------------------*/
/* Cipher Feedback Mode */
/*----------------------*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "square.h"
#include "sqofb.h"

#define D(p) ((word32 *)(p))


void squareOfbInit (squareOfbContext *ctxOfb, const squareBlock key)
{
	assert (ctxOfb != NULL);
	assert (key != NULL);
	memset (ctxOfb, 0, sizeof (squareOfbContext));
	squareExpandKey (key, ctxOfb->roundKeys);
} /* squareOfbInit */


void squareOfbSetIV (squareOfbContext *ctxOfb, const squareBlock iv)
{
	assert (ctxOfb != NULL);
	if (iv) {
		memcpy (ctxOfb->mask, iv, SQUARE_BLOCKSIZE);
	}
} /* squareOfbSetIV */


void squareOfbMask (squareOfbContext *ctxOfb, byte *buffer, unsigned length)
{
	unsigned i;
	byte *mask;

	assert (ctxOfb != NULL);
	assert (buffer != NULL);
	mask = ctxOfb->mask + (SQUARE_BLOCKSIZE - ctxOfb->avail);

	/* if there are enough available mask bytes
	 * to encrypt/decrypt the whole buffer, just use them:
	 */
	if (length <= ctxOfb->avail) {
		for (i = 0; i < length; i++) {
			buffer[i] ^= mask[i];
		}
		ctxOfb->avail -= length;
		return;
	}
	/* use all available (always 0 to SQUARE_BLOCKSIZE-1)
	 * mask bytes to encrypt/decrypt the first bytes of the buffer:
	 */
	for (i = 0; i < ctxOfb->avail; i++) {
		buffer[i] ^= mask[i];
	}
	buffer += ctxOfb->avail;
	length -= ctxOfb->avail;
	/* encrypt/decrypt the middle of the buffer in
	 * blocks of SQUARE_BLOCKSIZE bytes:
	 */
	while (length > SQUARE_BLOCKSIZE) {
		squareEncrypt ((word32 *)ctxOfb->mask, ctxOfb->roundKeys);
		D(buffer)[0] ^= D(ctxOfb->mask)[0]; \
		D(buffer)[1] ^= D(ctxOfb->mask)[1]; \
		D(buffer)[2] ^= D(ctxOfb->mask)[2]; \
		D(buffer)[3] ^= D(ctxOfb->mask)[3]; \
		buffer += SQUARE_BLOCKSIZE;
		length -= SQUARE_BLOCKSIZE;
	}
	/* encrypt/decrypt the last (always 1 to SQUARE_BLOCKSIZE) bytes:
	 */
	squareEncrypt ((word32 *)ctxOfb->mask, ctxOfb->roundKeys);
	mask = ctxOfb->mask;
	for (i = 0; i < length; i++) {
		buffer[i] ^= mask[i];
	}
	ctxOfb->avail = SQUARE_BLOCKSIZE - length;
} /* squareOfbMask */


void squareOfbFinal (squareOfbContext *ctxOfb)
{
	assert (ctxOfb != NULL);
	memset (ctxOfb, 0, sizeof (squareOfbContext));
} /* squareOfbFinal */
