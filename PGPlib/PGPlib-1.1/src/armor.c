/*********************************************************************
 * Filename:      armor.c
 * Description:
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Thu Dec 11 19:44:36 1997
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 ********************************************************************/

#include <pgplib.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
#  ifdef HAVE_SYS_ERRNO_H
#    include <sys/errno.h>
#  endif
#endif


/* I have taken some lines of code directly from PGP.  It didn't
 * feel quite right to paste it in underneath my Copyright so I
 * include it In Verbatim instead.
 *
 * The file includes the code necessary to calculate the CRC (that is,
 * the routines mk_crctbl, crcbytes() and init_crc).
 */

#include "armor.inc"

/* ====================================================================== */

/* The following code has been donated to PGPlib by
 * 	Peter Gutmann <pgut001@cs.auckland.ac.nz>
 * by means of an e-mail dated Fri, 9 May 1997 21:57:26 (NZST),
 * to the pgplib-dev@petium.rhein.de mailing list,
 *
 * Someone down there, hand that man some flowers.
 */


 /* Begin ASCII armor routines.
   This converts a binary file into printable ASCII characters, in a
   radix-64 form mostly compatible with the MIME format.
   This makes it easier to send encrypted files over a 7-bit channel.
   
   Encode/decode data from binary to the printable format specified in
   RFC 1113 (this should really be called base64 encoding, the names used
   here show the age of this code) */

/* The padding value used to pad odd output bytes */

#define PEM_PAD         '='

/* Non-PEM character values */

#define PERR            0xFF                    /* Illegal char */
#define PEOF            0x7F                    /* PEM-EOF (padding char) */

/* Binary <-> PEM translation tables */

static char binToAscii[] = 
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static unsigned char 
asciiToBin[] = { 
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, 0x3E, PERR, PERR, PERR, 0x3F,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
    0x3C, 0x3D, PERR, PERR, PERR, PEOF, PERR, PERR,
    PERR, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
    0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    0x17, 0x18, 0x19, PERR, PERR, PERR, PERR, PERR,
    PERR, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
    0x31, 0x32, 0x33, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR,
    PERR, PERR, PERR, PERR, PERR, PERR, PERR, PERR
};

/* Basic single-char en/decode functions */

#define encode(data)    binToAscii[ data ]
#define decode(data)    asciiToBin[ data ]

/****************************************************************************
*                                                                       *
*PEM Decoding Routines                                                  *
*                                                                       *
****************************************************************************/

/* Decode a NULL-terminated block of binary data from the PEM format.
 * Four (4) chars are packed into three (3) bytes; we must receive
 * at least 4 chars, inluding the termimnating 0.
 * If this isn't ARMORED (properly) return 0.
 * Return -1 on error 
 */

int 
pemDecode( char 	*inBuffer, 
	   u_char 	*outBuffer, 
	   int 		*outbuflen )
{
    int srcIndex, destIndex;
    int remainder;
    int	count;
    u_char c0, c1, c2, c3, *p;
    u_char *outBufferPtr;

    remainder = srcIndex = destIndex = 0;
    outBufferPtr = outBuffer;

    /* This is the only thing we can get */
    pgplib_errno = PGPLIB_ERROR_EINVAL;
    
    if ( inBuffer == NULL || outBuffer == NULL || outbuflen == NULL )
	return(-1);
    
    p = (u_char *)strchr(inBuffer, '\0');
    count = (char *)p - inBuffer;
    if ( count < 4 )
	return(-1);
    
    while( srcIndex < count )
    {
	/* There must be more than 4 chars left */
	if ( count - srcIndex < 4 )
	    return(1);
        /* Decode a block of data from the input buffer */
        c0 = decode( (int)inBuffer[ srcIndex++ ] );
        c1 = decode( (int)inBuffer[ srcIndex++ ] );
        c2 = decode( (int)inBuffer[ srcIndex++ ] );
        c3 = decode( (int)inBuffer[ srcIndex++ ] );
        if( ( c0 | c1 | c2 | c3 ) == PERR )
            return( 1 );
	
        /* Copy the decoded data to the output buffer */
        outBufferPtr[ destIndex++ ] = ( c0 << 2 ) | ( c1 >> 4 );
	if ( c2 != PEOF )
	    outBufferPtr[ destIndex++ ] = ( c1 << 4 ) | ( c2 >> 2);
	if ( c3 != PEOF )
	    outBufferPtr[ destIndex++ ] = ( c2 << 6 ) | ( c3 );
    }

    /* Return count of decoded bytes */
    *outbuflen = destIndex - remainder;
    return(0);
}


/****************************************************************************
*                                                                       *
* PEM Encoding Routines                                                 *
*                                                                       *
****************************************************************************/

/* Encode a block of binary data into the PEM format.  The result is 
 * padded if necessary.
 * If all is well, 0 is returned.  On error, -1 is returned.
 */
 

int
pemEncode( u_char 	*inBuffer,  
	   int		inbuflen,
	   char		*outBuffer,
	   int 		*outbuflen)
{
    int srcIndex, destIndex;
    int remainder;
    u_char *inBufferPtr;

    if (inBuffer==NULL || inbuflen<1 || outBuffer==NULL || outbuflen == NULL )
	return(-1);

    srcIndex = destIndex = 0;
    inBufferPtr = inBuffer;
    remainder =  inbuflen % 3;

    while( srcIndex < inbuflen )
    {
        outBuffer[ destIndex++ ] = encode( inBufferPtr[ srcIndex ] >> 2 );
        outBuffer[ destIndex++ ] = encode(((inBufferPtr[ srcIndex ] << 4 ) & 0x30 ) | ( ( inBufferPtr[ srcIndex + 1 ] >> 4) & 0x0F ) );
        srcIndex++;
        outBuffer[ destIndex++ ] = encode((( inBufferPtr[ srcIndex ] << 2 ) & 0x3C ) | ( ( inBufferPtr[ srcIndex + 1 ] >> 6) & 0x03 ) );
        srcIndex++;
        outBuffer[ destIndex++ ] = encode( inBufferPtr[ srcIndex++ ] & 0x3F );
    }

    /* Go back and add padding if we've encoded too many characters */
    if( remainder == 2 )
        /* There were only 2 bytes in the last group */
        outBuffer[ destIndex - 1 ] = PEM_PAD;
    else
        if( remainder == 1 )
            /* There was only 1 byte in the last group */
            outBuffer[ destIndex - 2 ] = outBuffer[ destIndex - 1 ] = PEM_PAD;
    outBuffer[ destIndex ] = '\0';

    /* Return count of encoded bytes */
    *outbuflen = destIndex;
    return( 0 );
}


/*
 * End of code donated by Peter Gutmann
 *======================================================================
 */



static void
skip_armor_headers(flio_t *p)
{
    char line[MAXLINE];

    while (flio_gets(line, MAXLINE, p) != NULL ) {
	if ( strlen(line) > 1 )
	    continue;
	if (line[0] == '\n')
	    break;
    }
}    

/* dearmor an input file.  The file stream will be positioned on the 
 * first line after the armored block.
 * NB: There is a fixed maximum line length
 */

int
dearmor_flio (flio_t *fin,
	      flio_t *fout)
{
    char	line[MAXLINE];
    u_char	buf[MAXLINE];
    int		len;
    int		found_checksum;
    u_long	found_crc, made_crc;
    /* scratch */
    char	*p;
    int		i;

    if ( fin == NULL || fout == NULL ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return -1;
    }

    while ( (p = flio_gets(line, MAXLINE, fin)) != NULL) {
	if ( strncmp(line, "-----BEGIN PGP ", 15) == 0)
	    break;
    }

    if ( p == NULL ) {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(1);
    }
	
    if(strncmp(line + 15, "SIGNED MESSAGE-----", 19) == 0)
    {
	int n = 0;
	flio_pos_t pos;

	skip_armor_headers(fin);
	flio_getpos(fin, &pos);
	while((p = flio_gets(line, MAXLINE, fin)) != NULL)
	{
	    char *s, *t;

	    if(!strncmp(line, "-----BEGIN PGP ", 15))
	    {
		flio_setpos(fin, &pos);
		break;
	    } else
		flio_getpos(fin, &pos);

	    if(!strncmp(line, "- ", 2))
		s = line + 2;
	    else
		s = line;

	    for (t = s + strlen(s); t >= s; t--)
	    {
		if(isspace(*t))
		    *t = '\0';
		else if(*t)
		    break;
	    }

	    if(n) 
	    {
		flio_putc('\r', fout);
		flio_putc('\n', fout);
	    }
	    else 
		n = 1;

	    flio_puts(s, fout);
	}

	if(p == NULL)
	{
	    pgplib_errno = PGPLIB_ERROR_EINVAL;
	    return 1;
	}
	
	return 0;
    }


    /* Start of something else.  Let us check the alternatives.  We don't
     * care what it is, as long as it seems to be correct */
    if ( strncmp(line+15,    "PUBLIC KEY BLOCK-----", 21) != 0
	 && strncmp(line+15, "MESSAGE-----", 12) != 0
	 && strncmp(line+15, "SIGNATURE-----", 14) != 0)
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(1);
    }

    /* Skip any header lines.  We simply don't care whether
     * they are valid or not (see armor.c in PGP to understand how
     * validity is decided.)
     */
    skip_armor_headers(fin);

    found_checksum = 0;
    found_crc = 0;
    init_crc();
    made_crc = CRCINIT;

    /* Read and read, until we reach the checksum */
    for ( ;; ) {
	p = flio_gets(line, MAXLINE, fin);
	if ( p == NULL ) {
	    if ( found_checksum ) {
		break;
	    }
	    /* Not missing END, then it is an error */
	    pgplib_errno = PGPLIB_ERROR_EINVAL;
	    return(1);
	}

	/* Get rid of trailing whitespace (and things) */
	p = strrchr(line, 0);
	/* This shouldn't be possible (I think) */
	if ( strlen(line) == 1)
	    return(-1);
	while( *(p-1) == ' ' || *(p-1) == '\n' ) {
	    *(p-1) = 0;
	    if (p == line) {
		/* a line with just whitespace */
		continue;
	    }
	    else
		p -= 1;
	}

	if ( found_checksum == 1 ) {
	    /* This line should be the end of things */
	    if ( strncmp(line, "-----END PGP", 12) ) {
		;
	    }
	    /* This is how we normally depart from here (the only break)*/
	    break;
	}
	if ( line[0] == PEM_PAD ) {

	    /* Here comes the checksum.
	     * MIME (the stupind thing, don't even know the difference
	     * between a clean transport-service and a broken
	     * front end; enough said) converts the already armored
	     * format into an even better armored format (more
	     * stupid every time).
	     * '=' is converted into '=3D'; go figure.
	     */
	    memset(buf, 0, 4);

	    if ( strlen(line) == 5 )
		i = pemDecode(line+1, buf, &len);
	    else 
	    {
		if ((strlen(line) == 7 && line[1] == '3' && line[2] == 'D'))
		    i = pemDecode(line+3, buf, &len);
		else {
		    pgplib_errno = PGPLIB_ERROR_EINVAL;
		    return(1);
		}
	    }
	    if ( i != 0 )
		return(1);
	    found_crc = buf[0] << 16;
	    found_crc |= buf[1] << 8;
	    found_crc |= buf[2];
	    /* We will read one more line, looking for the END */
	    found_checksum = 1;
	    continue;
	}

	/*i = darmor_line(line, buf, &len);*/
	i = pemDecode(line, buf, &len);
	if ( i != 0 ) {
	    return(i);
	}
	/* A new line has been decoded.  Write it out */
	i = flio_write(buf, 1, len, fout);
	if ( i != len ) {
	    /* Well, what to do ?
	     * Bug found by tzeruch@ceddec.com */
	    return(-1);
	}
	/* accumulate the crc */
	made_crc = crcbytes(buf, len, made_crc);
	continue;

    } /* while */

    if ( found_checksum == 0 ) {
	/* fprintf(stderr, "Incorrect termination of package\n");*/
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(1);
    }

    if ( made_crc != found_crc ) {
	/* fprintf(stderr, "Checksum incorrect\n");*/
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return(1);
    }
    return(0);
}


/* Public routines below here */

int 
dearmor_buffer(char 	*a_buf,
	       long 	a_len,
	       int 	*used,
	       u_char 	**buffer,
	       int 	*buf_len)
{
    flio_t in, *out;
    flio_pos_t pos;
    int rv;
    
    in.magic = FLIO_BUFF;
    in.d.buff.b = (u_char *)a_buf;
    in.d.buff.p = 0;
    in.d.buff.size = a_len;

    out = flio_newbuff();

    rv = dearmor_flio(&in, out);

    *buffer = out->d.buff.b;
    *buf_len = out->d.buff.p;

    flio_getpos(&in, &pos);
    *used = pos.d.p;

    free(out);

    return rv;
}


int 
dearmor_file(FILE *in, FILE *out)
{
    flio_t inf, outf;

    inf.magic = FLIO_FILE; inf.d.f = in;
    outf.magic = FLIO_FILE; outf.d.f = out;
    
    return dearmor_flio(&inf, &outf);
}

int 
read_armored_package(FILE 	*fin, 
		     u_char 	**buffer, 
		     int *buf_len)
{
    flio_t in, *out;
    int rv;

    in.magic = FLIO_FILE;
    in.d.f = fin;

    if((out = flio_newbuff()) == NULL)
	return -1;

    rv = dearmor_flio(&in, out);
    
    *buffer = out->d.buff.b;
    *buf_len = out->d.buff.p;

    free(out);
    return rv;
}

/*
 * Take a file-like I/O stream and armor it.
 * All OK : return 0.  Syntax error, return 1, error, return -1.
 */

int
armor_flio(flio_t *in, flio_t *out)
{

    u_long crc;
    u_char wkbuf[3];
    char tgt[5];
    u_char crcbuf[3];
    int i,j,n, m;

    if(in == NULL || out == NULL)
    {
	pgplib_errno = PGPLIB_ERROR_EINVAL;
	return -1;
    }
    
    flio_puts("-----BEGIN PGP MESSAGE-----\n", out);
    flio_puts(PGP_VERSION, out); flio_putc('\n', out);
    flio_puts(PGP_COMMENT, out); flio_putc('\n', out);
    flio_puts("Comment: created by " , out);
    flio_puts(LIB_VERSION, out);
    flio_puts("\n\n", out);
    

    init_crc();
    crc = CRCINIT;

    i = 0; n = 0; 
    
    do
    {
	j = flio_getc(in);

	if(j != EOF)
	    wkbuf[i++] = j;
	
	if(i == 3 || (j == EOF && i))
	{
	    int k;
	    for(k = i; k < 3; k++)
		wkbuf[k] = 0;

	    crc = crcbytes(wkbuf, i, crc);
	    memset(tgt, 0, sizeof(tgt));
	    if ( pemEncode(wkbuf, i, tgt, &m) != 0 && m != 5)
		return(-1);
	    flio_puts(tgt, out);

	    if((n += 4) >= ARMORLINE || j == EOF)
	    {
		n = 0;
		flio_putc('\n', out);
	    }

	    i = 0;
	} 
	else 
	    if(j == EOF) 
		flio_putc('\n', out);
    } while(j != EOF);
    
    flio_putc(PEM_PAD, out);

    crcbuf[0] = (crc >> 16) & 0xff;
    crcbuf[1] = (crc >> 8)  & 0xff;
    crcbuf[2] = crc         & 0xff;
    memset(tgt, 0, sizeof(tgt));
    (void)pemEncode(crcbuf, 3, tgt, &i);
    flio_puts(tgt, out);
    flio_putc('\n', out);

    flio_puts("-----END PGP MESSAGE-----\n", out);

    return 0;
}

int
armor_buffer(u_char	*buf,
	     int	buf_len,
	     int	*used,
	     char	**a_buf,
	     int	*a_len)

{
    flio_t in, *out;
    flio_pos_t pos;
    int rv;
    
    in.magic = FLIO_BUFF;
    in.d.buff.b = buf;
    in.d.buff.p = 0;
    in.d.buff.size = buf_len;

    out = flio_newbuff();

    rv = armor_flio(&in, out);

    *a_buf = (char *)out->d.buff.b;
    *a_len = out->d.buff.p;

    flio_getpos(&in, &pos);
    *used = pos.d.p;

    free(out);

    return rv;
}


int 
armor_file(FILE *in, FILE *out)
{
    flio_t inf, outf;
    
    inf.magic = FLIO_FILE; inf.d.f = in;
    outf.magic = FLIO_FILE; inf.d.f = out;
    
    return armor_flio(&inf, &outf);
}

