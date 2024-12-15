/*---------------------------*/
/* Electronic Code Book Mode */
/*---------------------------*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "square.h"
#include "sqecb.h"

#define D(p) ((word32 *)(p))


void squareEcbInit (squareEcbContext *ctxEcb, const squareBlock key)
{
	assert (ctxEcb != NULL);
	assert (key != NULL);
	memset (ctxEcb, 0, sizeof (squareEcbContext));
	squareGenerateRoundKeys (key, ctxEcb->roundKeys_e, ctxEcb->roundKeys_d);
} /* squareEcbInit */


void squareEcbEncrypt (squareEcbContext *ctxEcb, byte *buffer, unsigned length)
{
	assert (ctxEcb != NULL);
	assert (buffer != NULL);
	while (length >= SQUARE_BLOCKSIZE) {
		/* encrypt this block: */
		squareEncrypt (D(buffer), ctxEcb->roundKeys_e);
		/* proceed to the next block, if any: */
		buffer += SQUARE_BLOCKSIZE;
		length -= SQUARE_BLOCKSIZE;
	}
	assert (length == 0);
} /* squareEcbEncrypt */


void squareEcbDecrypt (squareEcbContext *ctxEcb, byte *buffer, unsigned length)
{
	assert (ctxEcb != NULL);
	assert (buffer != NULL);
	while (length >= SQUARE_BLOCKSIZE) {
		/* decrypt this block: */
		squareDecrypt (D(buffer), ctxEcb->roundKeys_d);
		/* proceed to the next block, if any: */
		buffer += SQUARE_BLOCKSIZE;
		length -= SQUARE_BLOCKSIZE;
	}
	assert (length == 0);
} /* squareEcbDecrypt */


void squareEcbFinal (squareEcbContext *ctxEcb)
{
	assert (ctxEcb != NULL);
	memset (ctxEcb, 0, sizeof (squareEcbContext));
} /* squareEcbFinal */
