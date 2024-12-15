/*----------------------*/
/* Cipher Feedback Mode */
/*----------------------*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "square.h"
#include "sqcfb.h"

#define D(p) ((word32 *)(p))


#define ENCRYPT_CFB_BLOCK(buffer, mask) \
{ \
	D(buffer)[0] = (D(mask)[0] ^= D(buffer)[0]); \
	D(buffer)[1] = (D(mask)[1] ^= D(buffer)[1]); \
	D(buffer)[2] = (D(mask)[2] ^= D(buffer)[2]); \
	D(buffer)[3] = (D(mask)[3] ^= D(buffer)[3]); \
} /* ENCRYPT_CFB_BLOCK */

#define DECRYPT_CFB_BLOCK(buffer, mask, z) \
{ \
	(z) = D(mask)[0]; D(buffer)[0] = (z) ^ (D(mask)[0] = D(buffer)[0]); \
	(z) = D(mask)[1]; D(buffer)[1] = (z) ^ (D(mask)[1] = D(buffer)[1]); \
	(z) = D(mask)[2]; D(buffer)[2] = (z) ^ (D(mask)[2] = D(buffer)[2]); \
	(z) = D(mask)[3]; D(buffer)[3] = (z) ^ (D(mask)[3] = D(buffer)[3]); \
} /* DECRYPT_CFB_BLOCK */


void squareCfbInit (squareCfbContext *ctxCfb, const squareBlock key)
{
	assert (ctxCfb != NULL);
	assert (key != NULL);
	memset (ctxCfb, 0, sizeof (squareCfbContext));
	squareExpandKey (key, ctxCfb->roundKeys);
} /* squareCfbInit */


void squareCfbSetIV (squareCfbContext *ctxCfb, const squareBlock iv)
{
	assert (ctxCfb != NULL);
	if (iv) {
		memcpy (ctxCfb->mask, iv, SQUARE_BLOCKSIZE);
	}
} /* squareCfbSetIV */


void squareCfbEncrypt (squareCfbContext *ctxCfb, byte *buffer, unsigned length)
{
	unsigned i;
	byte *mask;

	assert (ctxCfb != NULL);
	assert (buffer != NULL);
	mask = ctxCfb->mask + (SQUARE_BLOCKSIZE - ctxCfb->avail);

	/* if there are enough available mask bytes
	 * to encrypt the whole buffer, just use them:
	 */
	if (length <= ctxCfb->avail) {
		for (i = 0; i < length; i++) {
			buffer[i] = (mask[i] ^= buffer[i]);
		}
		ctxCfb->avail -= length;
		return;
	}
	/* use all available (always 0 to SQUARE_BLOCKSIZE-1)
	 * mask bytes to encrypt the first bytes of the buffer:
	 */
	for (i = 0; i < ctxCfb->avail; i++) {
		buffer[i] = (mask[i] ^= buffer[i]);
	}
	buffer += ctxCfb->avail;
	length -= ctxCfb->avail;
	/* encrypt the middle of the buffer in
	 * blocks of SQUARE_BLOCKSIZE bytes:
	 */
	while (length > SQUARE_BLOCKSIZE) {
		squareEncrypt ((word32 *)ctxCfb->mask, ctxCfb->roundKeys);
		ENCRYPT_CFB_BLOCK (buffer, ctxCfb->mask);
		buffer += SQUARE_BLOCKSIZE;
		length -= SQUARE_BLOCKSIZE;
	}
	/* encrypt the last (always 1 to SQUARE_BLOCKSIZE) bytes:
	 */
	squareEncrypt ((word32 *)ctxCfb->mask, ctxCfb->roundKeys);
	mask = ctxCfb->mask;
	for (i = 0; i < length; i++) {
		buffer[i] = (mask [i] ^= buffer[i]);
	}
	ctxCfb->avail = SQUARE_BLOCKSIZE - length;
} /* squareCfbEncrypt */


void squareCfbDecrypt (squareCfbContext *ctxCfb, byte *buffer, unsigned length)
{
	word32 z;
	unsigned i;
	byte *mask;
	byte t;

	assert (ctxCfb != NULL);
	assert (buffer != NULL);
	mask = ctxCfb->mask + (SQUARE_BLOCKSIZE - ctxCfb->avail);

	/* if there are enough available mask bytes
	 * to decrypt the whole buffer, just use them:
	 */
	if (length <= ctxCfb->avail) {
		for (i = 0; i < length; i++) {
			t = mask[i]; buffer[i] = t ^ (mask[i] = buffer[i]);
		}
		ctxCfb->avail -= length;
		return;
	}
	/* use all available (always 0 to SQUARE_BLOCKSIZE-1)
	 * mask bytes to decrypt the first bytes of the buffer:
	 */
	for (i = 0; i < ctxCfb->avail; i++) {
		t = mask[i]; buffer[i] = t ^ (mask[i] = buffer[i]);
	}
	buffer += ctxCfb->avail;
	length -= ctxCfb->avail;
	/* decrypt the middle of the buffer in
	 * blocks of SQUARE_BLOCKSIZE bytes:
	 */
	while (length > SQUARE_BLOCKSIZE) {
		squareEncrypt ((word32 *)ctxCfb->mask, ctxCfb->roundKeys);
		DECRYPT_CFB_BLOCK (buffer, ctxCfb->mask, z);
		buffer += SQUARE_BLOCKSIZE;
		length -= SQUARE_BLOCKSIZE;
	}
	/* decrypt the last (always 1 to SQUARE_BLOCKSIZE) bytes:
	 */
	squareEncrypt ((word32 *)ctxCfb->mask, ctxCfb->roundKeys);
	mask = ctxCfb->mask;
	for (i = 0; i < length; i++) {
		t = mask[i]; buffer[i] = t ^ (mask[i] = buffer[i]);
	}
	ctxCfb->avail = SQUARE_BLOCKSIZE - length;
	/* destroy potentially sensitive data: */
	t = 0; z = 0L;
} /* squareCfbDecrypt */


void squareCfbFinal (squareCfbContext *ctxCfb)
{
	assert (ctxCfb != NULL);
	memset (ctxCfb, 0, sizeof (squareCfbContext));
} /* squareCfbFinal */
