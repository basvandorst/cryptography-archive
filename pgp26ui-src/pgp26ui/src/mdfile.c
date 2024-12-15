/*	mdfile.c  - Message Digest routines for PGP.
	PGP: Pretty Good(tm) Privacy - public key cryptography for the masses.

	(c) Copyright 1990-1992 by Philip Zimmermann.  All rights reserved.
	The author assumes no liability for damages resulting from the use
	of this software, even if the damage results from defects in this
	software.  No warranty is expressed or implied.

	All the source code Philip Zimmermann wrote for PGP is available for
	free under the "Copyleft" General Public License from the Free
	Software Foundation.  A copy of that license agreement is included in
	the source release package of PGP.  Code developed by others for PGP
	is also freely available.  Other code that has been incorporated into
	PGP from other sources was either originally published in the public
	domain or was used with permission from the various authors.  See the
	PGP User's Guide for more complete information about licensing,
	patent restrictions on certain algorithms, trademarks, copyrights,
	and export controls.  
*/

#include <stdio.h>
#include "mpilib.h"
#include "mdfile.h"
#include "fileio.h"
#include "language.h"
#include "pgp.h"

/* Begin MD5 routines */

/* Note - the routines in this module, except for MD_addbuffer,
 * do not "finish" the MD5 calculation.  MD_addbuffer finishes the
 * calculation in each case, usually to append the timestamp and class info.
 */

/* Computes the message digest for a file from current position for
   longcount bytes.
   Uses the RSA Data Security Inc. MD5 Message Digest Algorithm */
int MDfile0_len(MD5_CTX *mdContext, FILE *f, word32 longcount)
{	int bytecount;
	unsigned char buffer[1024];

	MD5Init(mdContext);
	/* Process 1024 bytes at a time... */
	do
	{
		if (longcount < (word32) 1024)
			bytecount = (int)longcount;
		else
			bytecount = 1024;
		bytecount = fread(buffer, 1, bytecount, f);
		if (bytecount>0)
		{	MD5Update(mdContext, buffer, bytecount);
			longcount -= bytecount;
		}
		/* if text block was short, exit loop */
	} while (bytecount==1024);
	return(0);
}	/* MDfile0_len */


/* Computes the message digest for a file from current position to EOF.
   Uses the RSA Data Security Inc. MD5 Message Digest Algorithm */

static int MDfile0(MD5_CTX *mdContext,FILE *inFile)
{	int bytes;
	unsigned char buffer[1024];

	MD5Init(mdContext);
	while ((bytes = fread(buffer,1,1024,inFile)) != 0)
		MD5Update(mdContext,buffer,bytes);
	return(0);
}

/* Computes the message digest for a specified file */

int MDfile(MD5_CTX *mdContext,char *filename)
{
	FILE *inFile;
	inFile = fopen(filename,FOPRBIN);

	if (inFile == NULL)
	{	fprintf(pgpout,PSTR("\n\007Can't open file '%s'\n"),filename);
		return(-1);
	}
	MDfile0(mdContext,inFile);
	fclose (inFile);
	return(0);
}

/* Add a buffer's worth of data to the MD5 computation.  If a digest
 * pointer is supplied, complete the computation and write the digest.
 */
void MD_addbuffer (MD5_CTX *mdContext, byte *buf, int buflen, byte digest[16])
{
	MD5Update(mdContext,buf,buflen);
	if (digest) {
		MD5Final(digest, mdContext);
		burn(*mdContext);	/* Paranoia */
	}
}

/* End MD5 routines */
