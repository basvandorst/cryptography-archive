/*---------------------------------------------------------------------------

  file_io.c

  This file contains routines for doing direct input/output, file-related
  sorts of things.  Most of the system-specific code for unzip is contained
  here, including the non-echoing password code for decryption (bottom).

  Modified: 24 Jun 92 - HAJK
  Fix VMS support
  ---------------------------------------------------------------------------*/


#include "zunzip.h"

/****************************/
/* Function FillBitBuffer() */
/****************************/

int FillBitBuffer()
{
    /*
     * Fill bitbuf, which is 32 bits.  This function is only used by the
     * READBIT and PEEKBIT macros (which are used by all of the uncompression
     * routines).
     */
    UWORD temp;

    zipeof = 1;
    while (bits_left < 25 && ReadByte(&temp) == 8)
    {
      bitbuf |= (ULONG)temp << bits_left;
      bits_left += 8;
      zipeof = 0;
    }
    return 0;
}

/***********************/
/* Function ReadByte() */
/***********************/

int ReadByte(x)
UWORD *x;
{
    /*
     * read a byte; return 8 if byte available, 0 if not
     */


    if (csize-- <= 0)
        return 0;

    if (incnt == 0) {
        if ((incnt = read(zipfd, (char *) inbuf, INBUFSIZ)) <= 0)
            return 0;
        /* buffer ALWAYS starts on a block boundary:  */
        inptr = inbuf;
    }
    *x = *inptr++;
    --incnt;
    return 8;
}


/**************************/
/* Function FlushOutput() */
/**************************/

int FlushOutput()
{
    /*
     * flush contents of output buffer; return PK-type error code
     */
    int len;

    if (outcnt) {
			len = outcnt;
            if (write(outfd, (char *) outout, len) != len)
#ifdef MINIX
                if (errno == EFBIG)
                    if (write(fd, outout, len/2) != len/2  ||
                        write(fd, outout+len/2, len/2) != len/2)
#endif /* MINIX */
                {
                    return 50;    /* 50:  disk full */
                }
        outpos += outcnt;
        outcnt = 0;
        outptr = outbuf;
    }
    return (0);                 /* 0:  no error */
}
