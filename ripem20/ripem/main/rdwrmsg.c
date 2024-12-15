/*--- rdwrmsg.c -- Routine to help read and write messages. 
 *
 *  Mark Riordan  May 92 - July 92
 *
 *  This code is placed into the public domain.
 */
 
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "rsaref.h"
#include "ripem.h"
#include "headers.h"
#include "bfstream.h"
#include "rdwrmsgp.h"
#include "strutilp.h"
#include "p.h"

#define INLINESIZE 1024

/*--- function CodeAndWriteBytes ------------------------------
 *
 *  Convert a buffer of bytes to RFC1113 format and write
 *  the encoded data to a file.  In nbytes is a multiple of ENCODED_CHUNKSIZE
 *  then each line encodes ENCODED_CHUNKSIZE bytes and all output lines
 *  are the same length.
 *
 *   Entry: buf   is the buffer of data.
 *         nbytes is the number of bytes in the buffer.
 *        prefix  is a zero-terminated string to write at the
 *              beginning of each line.  Typically either
 *              empty string or one space.
 *        pstream is the stream to which to write the lines.
 *
 *   Exit:   Returns null normally, else error string if error.
 */
char *CodeAndWriteBytes (buf, nbytes, prefix, pstream)
unsigned char *buf;
unsigned int nbytes;
char *prefix;
FILE *pstream;
{
  unsigned int lineBytes, encodedBlockLen;
  char line[2*ENCODE_CHUNKSIZE];

  while (nbytes != 0) {
    lineBytes = nbytes<ENCODE_CHUNKSIZE ? nbytes : ENCODE_CHUNKSIZE;

    R_EncodePEMBlock ((unsigned char *)line, &encodedBlockLen, buf, lineBytes);
    /* Zero terminate the line */
    line[encodedBlockLen] = 0;

    fputs (prefix, pstream);
    fputs (line, pstream);
    fputc ('\n', pstream);
    buf += lineBytes;
    nbytes -= lineBytes;
  }
  return ((char *)NULL);
}

/* Same as CodeAndWriteBytes except writes to a BufferStream.
 */
char *BufferCodeAndWriteBytes (buf, nbytes, prefix, pstream)
unsigned char *buf;
unsigned int nbytes;
char *prefix;
BufferStream *pstream;
{
  unsigned int lineBytes, encodedBlockLen;
  char line[2*ENCODE_CHUNKSIZE], *errorMessage;

  while (nbytes != 0) {
    lineBytes = nbytes<ENCODE_CHUNKSIZE ? nbytes : ENCODE_CHUNKSIZE;

    R_EncodePEMBlock ((unsigned char *)line, &encodedBlockLen, buf, lineBytes);
    /* Zero terminate the line */
    line[encodedBlockLen] = 0;

    if ((errorMessage = BufferStreamPuts (prefix, pstream)) != (char *)NULL)
      return (errorMessage);
    if ((errorMessage = BufferStreamPuts (line, pstream)) != (char *)NULL)
      return (errorMessage);
    if ((errorMessage = WriteEOL (pstream)) != (char *)NULL)
      return (errorMessage);
    buf += lineBytes;
    nbytes -= lineBytes;
  }
  return ((char *)NULL);
}

/*--- function WriteCoded ----------------------------------------------
 *
 *  Write a chunk of coded bytes to the specified stream.
 *  Split the chunk into lines of "PR_CHUNKSIZE" characters.
 *
 *  Entry:  buf   is a buffer of ASCII characters.
 *        nbytes   is the number of bytes in buf.
 *        prefix   is a prefix to write at the beginning of each line.
 *        pstream  is the stream to write to.
 *
 *  Exit:
 */
void
WriteCoded(buf,nbytes,prefix,pstream)
unsigned char *buf;
unsigned int nbytes;
char *prefix;
FILE *pstream;
{
#define PR_CHUNKSIZE 64
  unsigned char line_bytes;
  unsigned int j;

   while(nbytes > 0) {
      line_bytes = nbytes<PR_CHUNKSIZE ? nbytes : PR_CHUNKSIZE;
    fputs(prefix,pstream);
      for(j=0; j<line_bytes; j++) putc((char)buf[j],pstream);
    putc('\n',pstream);
      buf += line_bytes;
      nbytes -= line_bytes;
   }
}

/* Read a header up to the first blank line.
   If headerList is not NULL, add header lines to the list.  The calling
     routine must initialize the list.
   If recipientNames is not NULL, add email addresses in all To: and
     Cc: fields.  The calling routine must initialize the list.
 */
char *ReadEmailHeader (stream, headerList, recipientNames)
FILE *stream;
TypList *headerList;
TypList *recipientNames;
{
  unsigned char line[INLINESIZE], *linecp;
  unsigned int to_field=FALSE;
  char *errorMessage = (char *)NULL, *readStatus;

  /* For error, break to end of do while (0) block. */
  do {
    while ((readStatus = fgets ((char *)line, INLINESIZE, stream)) != NULL) {
      /* Strip end-of-line CR and/or NL */
      for (linecp=line; *linecp && *linecp!='\r' && *linecp!='\n'; linecp++);
      *linecp = '\0';
      
      if(line[0] == '\0')
        break;

      if (recipientNames != (TypList *)NULL) {
        if (matchn ((char *)line,"To:", 3) ||
            matchn ((char *)line,"cc:",3)) {
          to_field = TRUE;
          CrackRecipients ((char *)line + 3, recipientNames);
        } else if (to_field) {
          if (WhiteSpace (line[0]))
            CrackRecipients ((char *)line, recipientNames);
          else
            to_field = FALSE;
        }
      }
      if (headerList != (TypList *)NULL) {
        if ((errorMessage = AppendLineToList ((char *)line, headerList))
            != (char *)NULL)
          break;
      }
    }
    if (errorMessage != (char *)NULL)
      /* broke because of error */
      break;

    if (readStatus == (char *)NULL) {
      /* Make sure we got a NULL read status becuase of end of file. */
      if (!feof (stream)) {
        errorMessage = "Error reading header from stream";
        break;
      }

      /* Note: we have reached the end of stream before reading a blank line */
    }
  } while (0);

  /* Zeroize the line */
  R_memset ((POINTER)line, 0, sizeof (line));  
  return (errorMessage);
}

/* line is a null-terminated, comma delimited list of email addresses.
   Extract each and add to recipientNames as a null-terminated string.
   This will extract the address properly, even with <>  and () addresses.
 */
char *
CrackRecipients (line, recipientNames)
char *line;
TypList *recipientNames;
{
  char *cptr=line, *targptr, *nptr, *beg_addr, *errorMessage;
  char recip[INLINESIZE];
  BOOL good_recip;

  while (*cptr) {
    /* Skip white space at beginning of recipient name */
    while(WhiteSpace(*cptr) && *cptr) cptr++;
    
    /* Copy characters from recipient name to next delimiter */
    
    for(targptr=recip;*cptr && *cptr!=',' && *cptr!='\n';) {
      *(targptr++) = *(cptr++);
    }
    *targptr = '\0';
    if(*cptr) cptr++;
    /* Extract the address properly, even with <>  and () addresses. */
    beg_addr = ExtractEmailAddr(recip);
    /* Store the name of this recipient, if non-empty.  */
    for(nptr=beg_addr,good_recip=FALSE; !good_recip && *nptr; nptr++) {
      if(!WhiteSpace(*nptr)) good_recip = TRUE;
    }
    if (good_recip) {
      if ((errorMessage = AppendLineToList (beg_addr, recipientNames))
          != (char *)NULL)
        return (errorMessage);
    }
  }

  return NULL;
}

/* Copy a \n to the buffer stream.
   This returns null for success or an error string if the buffer is
     too small.
 */
char *WriteEOL (stream)
BufferStream *stream;
{
  return (BufferStreamPutc ('\n', stream));
}

/* quoteHyphens means add "- " before lines which start with a hyphen.
   This translates <CR><LF> to '\n'.
 */
char *WriteMessage (text, textLen, quoteHyphens, stream)
unsigned char *text;
unsigned int textLen;
BOOL quoteHyphens;
BufferStream *stream;
{
  register int ch;
  char *errorMessage = (char *)NULL;
  
  if (quoteHyphens && textLen > 1 && *text == '-') {
    if ((errorMessage = BufferStreamPuts ("- ", stream)) != (char *)NULL)
      return (errorMessage);
  }

  while(textLen--) {
    ch = (int) *(text++);
    if(ch == '\r') {
      /* Ignore CR's */
    } else if (ch == '\n') {
      if ((errorMessage = WriteEOL(stream)) != (char *)NULL)
        return (errorMessage);
      if(quoteHyphens && textLen >= 1 && *text == '-') {
        if ((errorMessage = BufferStreamPuts ("- ", stream)) != (char *)NULL)
          return (errorMessage);
      }
    } else {
      if ((errorMessage = BufferStreamPutc (ch, stream)) != (char *)NULL)
        return (errorMessage);
    }
  }

  return ((char *)NULL);
}
