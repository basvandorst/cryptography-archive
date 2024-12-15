#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "rsaref.h"
#include "ripem.h"
#include "bfstream.h"

/* Initialize the buffer stream so that writes will reallocate the
     buffer as necessary.
 */
void BufferStreamConstructor (stream)
BufferStream *stream;
{
  stream->buffer = (unsigned char *)NULL;
  stream->maxBufferLen = 0;
  stream->point = 0;
}

/* Zeroize and free the buffer.
 */
void BufferStreamDestructor (stream)
BufferStream *stream;
{
  if (stream->buffer != (unsigned char *)NULL) {
    R_memset ((POINTER)stream->buffer, 0, stream->maxBufferLen);
    free (stream->buffer);
  }
}

/* Copy block to stream's buffer at the point given by stream->point and
     update point for more writes.  This resizes the buffer as needed
     by allocating a new one, copying, zeroizing and freeing the old one.
     We don't use realloc since this will leave sensitive data lying
     around.
   Return NULL for success or error string for error.
 */
char *BufferStreamWrite (block, blockLen, stream)
unsigned char *block;
unsigned int blockLen;
BufferStream *stream;
{
  unsigned int newSize;
  unsigned char *newBuffer;
  
  if (stream->point + blockLen > stream->maxBufferLen) {
    /* Resize the buffer to accomodate the bytes being written.
       Also allocate some extra bytes to allow more writes before another
         realloc.
     */
    newSize = stream->maxBufferLen + blockLen + 1024;
    if ((newBuffer = (unsigned char *)malloc (newSize))
        == (unsigned char *)NULL)
      return (ERR_MALLOC);

    /* Copy and zeroize and free the old buffer.
     */
    R_memcpy ((POINTER)newBuffer, (POINTER)stream->buffer, stream->point);
    R_memset ((POINTER)stream->buffer, 0, stream->maxBufferLen);
    free (stream->buffer);
    stream->buffer = newBuffer;
    stream->maxBufferLen = newSize;
  }

  memcpy (stream->buffer + stream->point, block, blockLen);
  stream->point += blockLen;

  return ((char *)0);
}

/* Write the string to stream's buffer (without end of line).
 */
char *BufferStreamPuts (string, stream)
char *string;
BufferStream *stream;
{
  return (BufferStreamWrite
          ((unsigned char *)string, strlen (string), stream));
}

/* Convert the character to a char and write it to stream's buffer.
 */
char *BufferStreamPutc (c, stream)
int c;
BufferStream *stream;
{
  char block[1];

  block[0] = (char)c;
  return (BufferStreamWrite ((unsigned char *)block, 1, stream));
}

/* Rewind the stream by setting the point back to zero.
 */
void BufferStreamRewind (stream)
BufferStream *stream;
{
  stream->point = 0;
}
