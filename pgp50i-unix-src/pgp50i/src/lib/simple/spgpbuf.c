/*
* spgpbuf.c -- Simple PGP API helper Buffer reading and writing functions
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: spgpbuf.c,v 1.5.2.1 1997/06/07 09:51:52 mhw Exp $
*/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "spgp.h"
#include "spgpint.h"

/*
 * Pump data from the memory buffer at InputBuffer, of size InputBufferLen,
 * into head. Do the terminating sizeAdvise but don't do the teardown.
 */
int
spgpMemPump (PgpPipeline *head, byte *InputBuffer, size_t InputBufferLen,
			SPGPProgressCallBack callBack, void *callBackArg)
{
	size_t totalSize = InputBufferLen;
	int err = head->sizeAdvise (head, InputBufferLen);
	while (InputBufferLen && !err) {
		size_t written;
		written = head->write (head, InputBuffer, InputBufferLen, &err);
		InputBuffer += written;
		InputBufferLen -= written;
		if (err)
			break;
		if (callBack != NULL)
			err = callBack (callBackArg, totalSize - InputBufferLen,
							totalSize);
	}
	head->sizeAdvise (head, 0);
	return err;
}

/*
 * Set *OutputBufferLen from pipebuf, using defwrit if pipebuf is NULL
 * (which can happen in a multi part PGP message; we may have closed
 * pipebuf already in the callback, but in that case we saved the size).
 * Return 0 if OK or an error if there was not enough room.
 */
int
spgpMemOutput (PgpPipeline *pipebuf, size_t defwrit, size_t *OutputBufferLen)
{
	size_t written = defwrit;
	size_t bufsize = *OutputBufferLen;
	if (pipebuf)
		pipebuf->annotate(pipebuf, NULL,
				PGPANN_MEM_BYTECOUNT, (byte *)&written, sizeof(written));
	*OutputBufferLen = written;
	if (written > bufsize)
		return SIMPLEPGPRECEIVEBUFFER_OUTPUTBUFFERTOOSMALL;
	return 0;
}

/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
