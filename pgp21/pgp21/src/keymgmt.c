/*	keymgmt.c  - Key management routines for PGP.
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
#include <stdlib.h>
#ifdef UNIX
#include <sys/types.h>
#endif
#include <time.h>
#include <ctype.h>
#include "mpilib.h"
#include "idea.h"
#include "random.h"
#include "crypto.h"
#include "fileio.h"
#include "keymgmt.h"
#include "genprime.h"
#include "rsagen.h"
#include "mpiio.h"
#include "language.h"
#include "pgp.h"
#include "md5.h"


/*
**	Convert to or from external byte order.
**	Note that convert_byteorder does nothing if the external byteorder
**  is the same as the internal byteorder.
*/
#define convert2(x,lx)	convert_byteorder( (byteptr)&(x), (lx) )
#define convert(x)		convert2( (x), sizeof(x) )


/*
 * check if userid matches the substring, magic characters ^ and $
 * can be used to match start and end of userid.
 * if n is NULL, only return TRUE if substr is an exact match of
 * userid, a substring does not match in this case.
 * the comparison is always case insensitive
 */
boolean userid_match(char *userid, char *substr,unitptr n)
{
	boolean match_end = FALSE;
	int id_len, sub_len, i;
	char buf[256], sub[256], *p;

	if (substr == NULL || *substr == '\0')
		return(TRUE);
	if (userid == NULL || *userid == '\0')
		return(FALSE);

	/* Check whether we have an ASCII or hex userID to check for */
	if (n != NULL && substr[ 0 ] == '0' && to_lower( substr[ 1 ] ) == 'x' )
	{
		userid = key2IDstring( n );
		substr += 2;
	}

	id_len = strlen(userid);
	for (i = 0; i <= id_len; ++i)
		buf[i] = to_lower(userid[i]);

	sub_len = strlen(substr);
	for (i = 0; i <= sub_len; ++i)
		sub[i] = to_lower(substr[i]);

	if (n == NULL)
	{
		return !strcmp(buf, sub);
	}
#ifdef MAGIC_MATCH
	if (sub_len > 1 && sub[sub_len - 1] == '$')
	{
		match_end = TRUE;
		sub[--sub_len] = '\0';
	}
	if (*sub == '^')
	{	if (match_end)
			return(!strcmp(buf, sub + 1));
		else
			return(!strncmp(buf, sub + 1, sub_len - 1));
	}
#endif
	if (sub_len > id_len)
		return(FALSE);

	if (match_end)
		return(!strcmp(buf + id_len - sub_len, sub));

	p = buf;
	while ((p = strchr(p, *sub)) != NULL)
	{
		if (strncmp(p, sub, sub_len) == 0)
			return(TRUE);
		++p;
	}
	return(FALSE);
}

int
is_key_ctb (byte ctb)
{
	return(ctb == CTB_CERT_PUBKEY  ||  ctb == CTB_CERT_SECKEY);
}


char *keyIDstring(byte *keyID)
/*	Return printable key fragment, which is an abbreviation of 
	the public key.
	Show LEAST significant 64 bits (KEYFRAGSIZE bytes) of modulus,
	LSB last.  Yes, that's LSB LAST.
*/
{	short i,j;
	static char keyIDbuf[2*KEYFRAGSIZE+1];
	char *bufptr;	/* ptr to Key ID string */
	bufptr = keyIDbuf;
	fill0(bufptr,sizeof(keyIDbuf));
#ifdef XLOWFIRST	/* LSB-first keyID format */
	j = KEYFRAGSIZE;
	for (i=KEYFRAGSIZE-1; i>=0; i--)	/* print LSB last */
	{	if (--j < 3)	/* only show bottom 3 bytes of keyID */
		{	sprintf(bufptr,"%02X",keyID[i]);
			bufptr += 2;
		}
		*bufptr = 0;
	}
#else		/* MSB-first keyID format */
	j = KEYFRAGSIZE;
	for (i=0; i<KEYFRAGSIZE; i++)	/* print LSB last */
	{	if (--j < 3)	/* only show bottom 3 bytes of keyID */
		{	sprintf(bufptr,"%02X",keyID[i]);
			bufptr += 2;
		}
		*bufptr = 0;
	}
#endif
	return(keyIDbuf);
}	/* keyIDstring */



void extract_keyID(byteptr keyID, unitptr n)
/*	Extract key fragment from modulus n.  keyID byte array must be
	at least KEYFRAGSIZE bytes long.
*/
{	byte buf[MAX_BYTE_PRECISION+2];
	short i, j;

	fill0(buf,KEYFRAGSIZE+2); /* in case n is too short */
	reg2mpi(buf,n);	/* MUST be at least KEYFRAGSIZE long */
#ifdef XLOWFIRST
	i = reg2mpi(buf,n);	/* MUST be at least KEYFRAGSIZE long */
	/* For LSB-first keyID format, start of keyID is: */
	i = 2;	/* skip over the 2 bytes of bitcount */
	for (j=0; j<KEYFRAGSIZE; )
		keyID[j++] = buf[i++];
#else
	i = reg2mpi(buf,n);	/* MUST be at least KEYFRAGSIZE long */
	/* For MSB-first keyID format, start of keyID is: */
	i = i + 2 - KEYFRAGSIZE;
	for (j=0; j<KEYFRAGSIZE; )
		keyID[j++] = buf[i++];
#endif

}	/* extract_keyID */



char *key2IDstring(unitptr n)
/*	Derive the key abbreviation fragment from the modulus n,
	and return printable string of key ID.
	n is key modulus from which to extract keyID.
*/
{	byte keyID[KEYFRAGSIZE];
	extract_keyID(keyID, n);
	return (keyIDstring(keyID));
}	/* key2IDstring */



void showkeyID(byteptr keyID)
/*	Print key fragment, which is an abbreviation of the public key. */
{
	fprintf(pgpout,"%s",keyIDstring(keyID));
}	/* showkeyID */



void writekeyID(unitptr n, FILE *f)
/*	Write message prefix keyID to a file.
	n is key modulus from which to extract keyID.
*/
{	byte keyID[KEYFRAGSIZE];
	extract_keyID(keyID, n);
	fwrite(keyID,1,KEYFRAGSIZE,f);
}	/* writekeyID */



boolean checkkeyID(byte *keyID, unitptr n)
/*	Compare specified keyID with one derived from actual key modulus n. */
{
	byte keyID0[KEYFRAGSIZE];
	if (keyID==NULL) /* no key ID -- assume a good match */
		return (TRUE);
	extract_keyID(keyID0, n);
	return(equal_buffers(keyID,keyID0,KEYFRAGSIZE));
}	/* checkkeyID */



/* external function prototype, from mpiio.c */
void dump_unit_array(string s, unitptr r);

void write_trust (FILE *f, byte trustbyte)
/*	Write a key control packet to f, with the specified trustbyte data.
 */
{
	byte ctb;
	byte keyctrllen;

	ctb = CTB_KEYCTRL;
	fwrite(&ctb,1,1,f);		/* write key control header byte */
	keyctrllen = 1;
	fwrite(&keyctrllen,1,1,f);	/* write key control length */
	fwrite(&trustbyte,1,1,f);	/* write key control */
}

short writekeyfile(char *fname, boolean hidekey, byte *timestamp, byte *userid, 
	unitptr n, unitptr e, unitptr d, unitptr p, unitptr q, unitptr u)
/*	Write key components p, q, n, e, d, and u to specified file.
	hidekey is TRUE iff key should be encrypted.
	userid is a length-prefixed Pascal-type character string. 
	We write three packets: a key packet, a key control packet, and
	a userid packet.  We assume the key being written is our own,
	so we set the control bits for full trust.
*/
{	FILE *f;
	byte ctb;
	byte alg, version;
	word16 validity;
	word16 cert_length;
	extern word16 mpi_checksum;
	byte iv[8];
	int i;

	/* open file f for write, in binary (not text) mode...*/
	if ((f = fopen(fname,FOPWBIN)) == NULL)
	{	fprintf(pgpout,PSTR("\n\007Unable to create key file '%s'.\n"), fname);
		return(-1);
	}
	/*** Begin key certificate header fields ***/
	if (d==NULL)
	{	/* public key certificate */
		ctb = CTB_CERT_PUBKEY;
		cert_length = 1 + SIZEOF_TIMESTAMP + 2 + 1 + (countbytes(n)+2)
			+ (countbytes(e)+2);
	}	/* public key certificate */
	else
	{	/* secret key certificate */
		ctb = CTB_CERT_SECKEY;
			cert_length = 1 + SIZEOF_TIMESTAMP + 2 + 1
			+ (countbytes(n)+2)
			+ (countbytes(e)+2)
			+ 1 + (hidekey ? 8 : 0)		/* IDEA algorithm byte and IV */
			+ (countbytes(d)+2)
			+ (countbytes(p)+2)	+ (countbytes(q)+2) 
			+ (countbytes(u)+2) + 2;

	}	/* secret key certificate */

	fwrite(&ctb,1,1,f);		/* write key certificate header byte */
	convert(cert_length);	/* convert to external byteorder */
	fwrite(&cert_length,1,sizeof(cert_length),f);
	version = VERSION_BYTE;
	fwrite(&version,1,1,f);		/* set version number */
	convert_byteorder(timestamp,4);	/* convert to external form */
	fwrite(timestamp,1,4,f); /* write certificate timestamp */
	convert_byteorder(timestamp,4);	/* convert back to internal form */
	validity = 0;
	fwrite (&validity,1,sizeof(validity),f);	/* validity period */
	alg = RSA_ALGORITHM_BYTE;
	fwrite(&alg,1,1,f);
	write_mpi(n,f,FALSE);
	write_mpi(e,f,FALSE);

	if (is_secret_key(ctb))	/* secret key */
	{
		/* Write byte for following algorithm */
		alg = (hidekey)?IDEA_ALGORITHM_BYTE:0;
		fwrite(&alg,1,1,f);

		if (hidekey)  /* store encrypted IV */
		{	for (i=0; i<8; i++)
				iv[i] = randombyte();
			if (hidekey)  /* encrypt the IV */
				ideacfb(iv,8);
			fwrite(iv,1,8,f);	/* write out the IV */
		}
		mpi_checksum = 0;
		write_mpi(d,f,hidekey);
		write_mpi(p,f,hidekey);
		write_mpi(q,f,hidekey);
		write_mpi(u,f,hidekey);
		/* Write checksum here - based on plaintext values */
		convert(mpi_checksum);
		fwrite (&mpi_checksum,1,sizeof(mpi_checksum),f);
	} else {
		/* Keyring control packet, public keys only */
		write_trust (f, KC_OWNERTRUST_ULTIMATE | KC_BUCKSTOP);
	}
	/* User ID packet */
	ctb = CTB_USERID;
	fwrite(&ctb,1,1,f);		/* write userid header byte */
	fwrite(userid,1,userid[0]+1,f);	/* write user ID */
	if (d == NULL)	/* only on public keyring */
		write_trust (f, KC_LEGIT_COMPLETE);
	if (write_error(f))
	{	fclose(f);
		return -1;
	}
	fclose(f);
	if (verbose)
		fprintf(pgpout,"%d-bit %s key written to file '%s'.\n",
			countbits(n),
			is_secret_key(ctb) ? "secret" : "public" ,
			fname);
	return(0);
}	/* writekeyfile */



/* Return -1 on EOF, else read next key packet, return its ctb, and
 * advance pointer to beyond the packet.
 * This is short of a "short form" of readkeypacket
 */
short nextkeypacket(FILE *f, byte *pctb)
{
	word32 cert_length;
	int count;
	byte ctb;

	*pctb = 0;	/* assume no ctb for caller at first */
	count = fread(&ctb,1,1,f);	/* read key certificate CTB byte */
	if (count==0) return(-1);	/* premature eof */
	*pctb = ctb;	/* returns type to caller */
	if ((ctb != CTB_CERT_PUBKEY) && (ctb != CTB_CERT_SECKEY)  &&
			(ctb != CTB_USERID)  &&  (ctb != CTB_KEYCTRL)  &&
			!is_ctb_type(ctb,CTB_SKE_TYPE) &&
			!is_ctb_type(ctb,CTB_COMMENT_TYPE))
		/* Either bad key packet or X/Ymodem padding detected */
		return ((ctb == 0x1A) ? -1 : -2);

	cert_length = getpastlength(ctb, f); /* read certificate length */

	if (cert_length > MAX_KEYCERT_LENGTH-3)
		return(-3);	/* bad length */

	fseek(f, cert_length, SEEK_CUR);
	return(0);
} /* nextkeypacket */


short readkeypacket(FILE *f, boolean hidekey, byte *pctb,
	byte *timestamp, char *userid,
	unitptr n ,unitptr e, unitptr d, unitptr p, unitptr q, unitptr u,
	byte *sigkeyID, byte *keyctrl)
/*	Reads a key certificate from the current file position of file f.
	Depending on the certificate type, it will set the proper fields
	of the return arguments.  Other fields will not be set.
	pctb is always set.
	If the packet is CTB_CERT_PUBKEY or CTB_CERT_SECKEY, it will
	return timestamp, n, e, and if the secret key components are
	present and d is not NULL, it will read, decrypt if hidekey is
	true, and return d, p, q, and u.
	If the packet is CTB_KEYCTRL, it will return keyctrl as that byte.
	If the packet is CTB_USERID, it will return userid.
	If the packet is CTB_COMMENT_TYPE, it won't return anything extra.
	The file pointer is left positioned after the certificate.

	If the key could not be read because of a version error or bad
	data, the return value is -6 or -4, the file pointer will be
	positioned after the certificate, only the arguments pctb and
	userid will valid in this case, other arguments are undefined.
	Return value -3 means the error is unrecoverable.

*/
{
	byte ctb;
	word16 cert_length;
	int count;
	byte version, alg, mdlen;
	word16 validity;
	word16 chksum;
	extern word16 mpi_checksum;
	long next_packet;
	byte iv[8];

	/*** Begin certificate header fields ***/
	*pctb = 0;	/* assume no ctb for caller at first */
	count = fread(&ctb,1,1,f);	/* read key certificate CTB byte */
	if (count==0) return(-1);	/* premature eof */
	*pctb = ctb;	/* returns type to caller */
	if ((ctb != CTB_CERT_PUBKEY) && (ctb != CTB_CERT_SECKEY)  &&
			(ctb != CTB_USERID)  &&  (ctb != CTB_KEYCTRL)  &&
			!is_ctb_type(ctb,CTB_SKE_TYPE) &&
			!is_ctb_type(ctb,CTB_COMMENT_TYPE))
		/* Either bad key packet or X/Ymodem padding detected */
		return ((ctb == 0x1A) ? -1 : -2);

	cert_length = getpastlength(ctb, f); /* read certificate length */

	if (cert_length > MAX_KEYCERT_LENGTH-3)
		return(-3);	/* bad length */

	next_packet = ftell(f) + cert_length;

	/* skip packet and return, keeps us in sync when we hit a
	   version error or bad data */
#define	SKIP_RETURN(x) \
		{fseek(f,next_packet,SEEK_SET); return(x);}

	if (ctb == CTB_USERID)
	{	if (cert_length > 255)
			return(-3);			/* Bad length error */
		if (userid)
		{	userid[0] = cert_length;		/* Save user ID length */
			fread(userid+1,1,cert_length,f); /* read rest of user ID */
		} else
			fseek (f, (long)cert_length, SEEK_CUR);
		return(0);	/* normal return */
	}
	else if (is_ctb_type (ctb, CTB_SKE_TYPE))
	{	if (sigkeyID)
		{	fread(&version,1,1,f);		/* Read version of sig packet */
			if (version_error(version, VERSION_BYTE))
				SKIP_RETURN(-6);			/* Need a later version */
			/* Skip timestamp, validity period, and type byte */
			fread(&mdlen, 1, 1, f);
			fseek(f, (long) mdlen, SEEK_CUR);
			/* Read and return KEY ID */
			fread(sigkeyID,1,KEYFRAGSIZE,f);
		}
		SKIP_RETURN(0);	/* normal return */
	}
	else if (ctb == CTB_KEYCTRL)
	{	if (cert_length != 1)
			return(-3);			/* Bad length error */
		if (keyctrl)
			fread(keyctrl,1,cert_length,f);	/* Read key control byte */
		else
			fseek (f, (long)cert_length, SEEK_CUR);
		return(0);	/* normal return */
	}
	else if (!is_key_ctb(ctb))	/* comment or other packet */
		SKIP_RETURN(0);	/* normal return */

	/* Here we have a key packet */
	if (n != NULL)
		set_precision(MAX_UNIT_PRECISION);	/* safest opening assumption */
	fread(&version,1,1,f);	/* read and check version */
	if (version_error(version, VERSION_BYTE))
		SKIP_RETURN(-6);			/* Need a later version */
	if (timestamp)
	{	fread(timestamp,1,SIZEOF_TIMESTAMP,f);	/* read certificate timestamp */
		convert_byteorder(timestamp,SIZEOF_TIMESTAMP); /* convert from external form */
	} else
		fseek(f, (long)SIZEOF_TIMESTAMP, SEEK_CUR);
	fread(&validity,1,sizeof(validity),f);	/* Read validity period */
	convert(validity);	/* convert from external byteorder */
	/* We don't use validity period yet */
	fread (&alg, 1, 1, f);
	if (version_error(alg, RSA_ALGORITHM_BYTE))
		SKIP_RETURN(-6);			/* Need a later version */
	/*** End certificate header fields ***/

	/* We're past certificate headers, now look at some key material...*/

	cert_length -= 1 + SIZEOF_TIMESTAMP + 2 + 1;

	if (n==NULL)	/* Skip key certificate data */
		SKIP_RETURN(0);

	if (read_mpi(n,f,TRUE,FALSE) < 0)
		SKIP_RETURN(-4);	/* data corrupted, return error */

	/* Note that precision was adjusted for n */

	if (read_mpi(e,f,FALSE,FALSE) < 0)
		SKIP_RETURN(-4);	/* data corrupted, error return */

	cert_length -= (countbytes(n)+2) + (countbytes(e)+2);

	if (d==NULL)	/* skip rest of this key certificate */
		SKIP_RETURN(0);			/* Normal return */
	if (is_secret_key(ctb))
	{	byte	alg = 0;
		fread (&alg,1,1,f);
		if (alg && version_error(alg,IDEA_ALGORITHM_BYTE))
			SKIP_RETURN(-6);			/* Unknown version */

		if (!hidekey && alg)
			/* Don't bother trying if hidekey is false and alg is true */
			SKIP_RETURN(-5);

		if (alg)	/* if secret components are encrypted... */
		{	/* process encrypted CFB IV before reading secret components */
			count = fread(iv,1,8,f);
			if (count < 8)
				return(-4);	/* data corrupted, error return */
			ideacfb(iv,8);
			cert_length -= 8;	/* take IV length into account */
		}

		/* Reset checksum before these reads */
		mpi_checksum = 0;

		if (read_mpi(d,f,FALSE,hidekey) < 0)
			return(-4);	/* data corrupted, error return */
		if (read_mpi(p,f,FALSE,hidekey) < 0)
			return(-4);	/* data corrupted, error return */
		if (read_mpi(q,f,FALSE,hidekey) < 0)
			return(-4);	/* data corrupted, error return */

		/* use register 'u' briefly as scratchpad */
		mp_mult(u,p,q);	/* compare p*q against n */
		if (mp_compare(n,u)!=0)	/* bad pass phrase? */
			return(-5);	/* possible bad pass phrase, error return */
		/* now read in real u */
		if (read_mpi(u,f,FALSE,hidekey) < 0)
			return(-4);	/* data corrupted, error return */

		/* Read checksum, compare with mpi_checksum */
		fread (&chksum,1,sizeof(chksum),f);
		convert(chksum);
		if (chksum != mpi_checksum)
			return(-5);	/* possible bad pass phrase */

		cert_length -= 1 + (countbytes(d)+2) + (countbytes(p)+2)
			+ (countbytes(q)+2) + (countbytes(u)+2) + 2;

	}	/* secret key */
	else /* not a secret key */
	{	mp_init(d,0);
		mp_init(p,0);
		mp_init(q,0);
		mp_init(u,0);
	}

	if (cert_length != 0)
	{	fprintf(pgpout,"\n\007Corrupted key.  Bad length, off by %d bytes.\n",
			(int) cert_length);
		SKIP_RETURN(-4);	/* data corrupted, error return */
	}

	return(0);	/* normal return */

}	/* readkeypacket */



int getpublickey(boolean giveup, boolean showkey, char *keyfile,
	long *file_position, int *pktlen, byte *keyID, byte *timestamp, 
	byte *userid, unitptr n, unitptr e)
/*	keyID contains key fragment we expect to find in keyfile.
	If keyID is NULL, then userid contains a C string search target of
	userid to find in keyfile.
	keyfile is the file to begin search in, and it may be modified
	to indicate true filename of where the key was found.  It can be
	either a public key file or a secret key file.
	file_position is returned as the byte offset within the keyfile
	that the key was found at.  pktlen is the length of the key packet.
	These values are for the key packet itself, not including any
	following userid, control, signature, or comment packets.
	giveup is TRUE iff we are just going to do a single file search only.

	Returns -6 if the key was found but the key was not read because of a
	version error or bad data.  The arguments timestamp, n and e are
	undefined in this case.
*/
{
	byte ctb;	/* returned by readkeypacket */
	FILE *f;
	int status, keystatus = -1;
	boolean keyfound = FALSE;
	boolean secret = FALSE;
	char userid0[256];	/* C string format */
	long fpos, userid_pos;

	userid0[0] = '\0';

	if (keyID==NULL)	/* then userid has search target */
		strcpy(userid0,(char *)userid);

top:
	if (strlen(keyfile) == 0)	/* null filename */
		return(-1);	/* give up, error return */

	default_extension(keyfile,PGP_EXTENSION);

	if (!file_exists(keyfile))
	{	if (giveup)
			return(-1);	/* give up, error return */
		fprintf(pgpout,PSTR("\n\007Keyring file '%s' does not exist. "),keyfile);
		goto nogood;
	}
	if (verbose)
	{	fprintf(pgpout,"searching key ring file '%s' ",keyfile);
		if (keyID)
			fprintf(pgpout, "for keyID %s\n", keyIDstring(keyID));
		else
			fprintf(pgpout, "for userid \"%s\"\n", userid);
	}

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(keyfile,FOPRBIN)) == NULL)
		return(-1);	/* error return */

	while (TRUE) 
	{
		fpos = ftell(f);
		status = readkeypacket(f,FALSE,&ctb,timestamp,(char *)userid,n,e,
				NULL,NULL,NULL,NULL,NULL,NULL);
		/* Note that readkeypacket has called set_precision */

		if (status == -1)	/* end of file */
			break;

		if (status < -1 && status != -4 && status != -6)
		{	fprintf(pgpout,PSTR("\n\007Could not read key from file '%s'.\n"),
				keyfile);
			fclose(f);	/* close key file */
			return(status);
		}

		/* Remember packet position and size for last key packet */
		if (is_key_ctb(ctb))
		{	*file_position = fpos;
			*pktlen = (int)(ftell(f) - fpos);
			secret = is_ctb_type(ctb, CTB_CERT_SECKEY_TYPE);
			keystatus = status;
		}

		/* Only check for matches when we find a USERID packet */
		if (ctb == CTB_USERID)
		{	/* keyID contains key fragment.  Check it against n from keyfile. */
			if (keyID!=NULL)
			{	if (keystatus == 0)
					keyfound = checkkeyID(keyID,n);
			}
			else
			{	/* userid0 is already a C string */
				PascalToC((char *)userid);	/* for C string functions */
				/* Accept any matching subset */
				keyfound = userid_match((char *)userid,userid0,n);
				CToPascal((char *)userid);
			}
		}

		if (keyfound)
		{	if (showkey)
			{	PascalToC((char *)userid);	/* for display */
				fprintf(pgpout,PSTR("\nKey for user ID: %s\n"),
					LOCAL_CHARSET((char *)userid));
				switch (keystatus)
				{	case 0: fprintf(pgpout,PSTR("%d-bit key, Key ID %s, created %s\n"),
						countbits(n), key2IDstring(n), cdate((word32 *)timestamp) );
						break;
					case -4: fprintf(pgpout,PSTR("Bad key format.\n")); break;
					case -6: fprintf(pgpout,PSTR("Unrecognized version.\n")); break;
				}
				CToPascal((char *)userid);	/* restore after display */
				userid_pos = fpos;
				if (keyID==NULL)
					fseek(f, *file_position + *pktlen, SEEK_SET);
				while (((status = readkeypacket(f,FALSE,&ctb,NULL,(char *)userid0,
						NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL) == 0) ||
						status == -4 || status == -6) && !is_key_ctb(ctb))
				{	if (ctb == CTB_USERID)
					{	PascalToC ((char *)userid0);
						if (fpos != userid_pos)
							fprintf (pgpout,PSTR("Also known as: %s\n"),
												userid0);
					}
					fpos = ftell(f);
				}
			}
			fseek(f, *file_position, SEEK_SET);
			if (keystatus == 0 && is_compromised(f))
			{	fclose(f);
				return(1);
			}
			fclose(f);
			return(keystatus);
		}
	}	/* while TRUE */

	fclose(f);	/* close key file */

	if (giveup)
		return(-1);	/* give up, error return */

	if (keyID!=NULL)
	{
		fprintf(pgpout,PSTR("\n\007Key matching expected Key ID %s not found in file '%s'.\n"),
			keyIDstring(keyID),keyfile);
	}
	else
	{	fprintf(pgpout,PSTR("\n\007Key matching userid '%s' not found in file '%s'.\n"),
			LOCAL_CHARSET(userid0),keyfile);
	}

nogood:
	if (giveup || filter_mode)
		return(-1);	/* give up, error return */

	if (secret)
	{	/* Look in public key file and see if it's there. */
		char keyfilename[MAX_PATH];	/* for getpublickey */
		buildfilename(keyfilename,PUBLIC_KEYRING_FILENAME);
		if (getpublickey(TRUE,FALSE,keyfilename,file_position,pktlen,
				keyID,timestamp,userid,n,e) >= 0)
		{	PascalToC((char * )userid);
			fprintf(pgpout,PSTR("\nThis message can only be read by:\n"));
			fprintf(pgpout,"\"%s\"\n\n",LOCAL_CHARSET((char *)userid));
			CToPascal((char *)userid);
		}
		fprintf(pgpout,PSTR("Enter secret key filename: "));
	}
	else
		fprintf(pgpout,PSTR("Enter public key filename: "));

	getstring(keyfile,59,TRUE);	/* echo keyboard input */
	goto top;

}	/* getpublickey */


int getpubuserid(char *keyfile, long key_position, byte *userid,
	long *userid_position, int *userid_len, boolean exact_match)
/*  Start at key_position in keyfile, and scan for the key packet
	that contains userid.  Return userid_position and userid_len.
	Return 0 if OK, -1 on error.  Userid should be a C string.
	If exact_match is TRUE, the userid must match for full length,
	a substring is not enough.
*/
{
	unit n[MAX_UNIT_PRECISION];
	unit e[MAX_UNIT_PRECISION];
	byte ctb;	/* returned by readkeypacket */
	FILE *f;
	int status;
	char userid0[256];	/* C string format */
	long fpos;

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(keyfile,FOPRBIN)) == NULL)
		return(-1);	/* error return */

	/* Start off at correct location */
	fseek (f, key_position, SEEK_SET);
	(void)nextkeypacket(f, &ctb);	/* Skip key */
	while (TRUE)
	{
		fpos = ftell(f);
		status = readkeypacket(f,FALSE,&ctb,NULL,(char *)userid0,n,e,
				NULL,NULL,NULL,NULL,NULL,NULL);

		if (status < 0  ||  is_key_ctb(ctb))
			break;

		/* Only check for matches when we find a USERID packet */
		if (ctb == CTB_USERID)
		{	/* userid is already a C string */
			PascalToC((char *)userid0);	/* for C string functions */
			/* Accept any matching subset if exact_match is FALSE */
			if (userid_match((char *)userid0, (char *) userid,
				(exact_match ? NULL : n)))
			{	*userid_position = fpos;
				*userid_len = ( int ) ( ftell(f) - fpos );
				fclose(f);
				return(0);	/* normal return */
			}
		}
	}	/* while TRUE */

	fclose(f);	/* close key file */
	return(status ? status : -1);	/* give up, error return */
}	/* getpubuserid */


int getpubusersig(char *keyfile, long user_position, byte *sigkeyID,
	long *sig_position, int *sig_len)
/*  Start at user_position in keyfile, and scan for the signature packet
	that matches sigkeyID.  Return sig_position and sig_len.
	Return 0 if OK, -1 on error.
*/
{
	byte ctb;	/* returned by readkeypacket */
	FILE *f;
	int status;
	byte keyID0[KEYFRAGSIZE];
	long fpos;

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(keyfile,FOPRBIN)) == NULL)
		return(-1);	/* error return */

	/* Start off at correct location */
	fseek (f, user_position, SEEK_SET);
	(void)nextkeypacket(f, &ctb);	/* Skip userid packet */
	while (TRUE) 
	{
		fpos = ftell(f);
		status = readkeypacket(f,FALSE,&ctb,NULL,NULL,NULL,NULL,
				NULL,NULL,NULL,NULL,keyID0,NULL);

		if (status < 0  ||  is_key_ctb(ctb)  ||  ctb==CTB_USERID)
			break;

		/* Only check for matches when we find a signature packet */
		if (is_ctb_type(ctb,CTB_SKE_TYPE))
		{	if (equal_buffers(sigkeyID,keyID0,KEYFRAGSIZE))
			{	*sig_position = fpos;
				*sig_len = ( int ) ( ftell(f) - fpos );
				fclose(f);
				return(0);	/* normal return */
			}
		}
	}	/* while TRUE */

	fclose(f);	/* close key file */
	return(status ? status : -1);	/* give up, error return */
}	/* getpubusersig */


int getsecretkey(boolean giveup, boolean showkey, char *keyfile, byte *keyID,
	byte *timestamp, char *passp, boolean *hkey,
	byte *userid, unitptr n, unitptr e, unitptr d, unitptr p, unitptr q,
	unitptr u)
/*	keyID contains key fragment we expect to find in keyfile.
	If keyID is NULL, then userid contains search target of
	userid to find in keyfile.
	giveup controls whether we ask the user for the name of the
	secret key file on failure.  showkey controls whether we print
	out the key information when we find it.  keyfile, if non-NULL,
	is the name of the secret key file; if NULL, we use the
	default.  passp and hkey, if non-NULL, get returned with a copy
	of the pass phrase and hidekey variables.
*/
{
	byte ctb;	/* returned by readkeypacket */
	FILE *f;
	char keyfilename[MAX_PATH];	/* for getpublickey */
	long file_position;
	int pktlen;	/* unused, just to satisfy getpublickey */
	int status;
	boolean hidekey = FALSE;	/* TRUE iff secret key is encrypted */
	char passphrase[256];
	word16 iv[4];		/* initialization vector for encryption */
	byte ideakey[16];
	extern char password[];
	int guesses = 3;

	if (keyfile == NULL)
	{	/* use default pathname */
		buildfilename(keyfilename,SECRET_KEYRING_FILENAME);
		keyfile = keyfilename;
	}

	status = getpublickey(giveup, showkey, keyfile, &file_position, &pktlen,
			keyID, timestamp, userid, n, e);
	if (status < 0)
		return(status);	/* error return */


	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(keyfile,FOPRBIN)) == NULL)
		return(-1);	/* error return */

	/* First guess is null password, so hidekey is FALSE */

	do	/* until good password */
	{	if (hkey != NULL)
			*hkey = hidekey;
		/* Initialize IDEA key */
		if (hidekey)
		{	if (passp != NULL)
				strcpy (passp, passphrase); /* Save phrase for caller */
			fill0(iv,8);
			initcfb_idea(iv,ideakey,TRUE);
		}
		fseek(f,file_position,SEEK_SET); /* reposition file to key */
		status = readkeypacket(f,hidekey,&ctb,timestamp,(char *)userid,
						n,e,d,p,q,u,NULL,NULL);
		if (hidekey)
			close_idea();	/* Release resources */

		burn((byteptr)passphrase);	/* burn sensitive data on stack */

		if (status == -5)	/* bad pass phrase status */
		{	if (guesses!=3)	/* not first guess of null password? */
				fprintf(pgpout,PSTR("\n\007Error:  Bad pass phrase.\n"));
			if (!hidekey && *password != '\0')
			{ /* Try environment variable second */
				strncpy (passphrase, password, sizeof(passphrase)-1);
				hashpass (passphrase, strlen(passphrase), ideakey);
				hidekey = TRUE;
				continue;
			}
			if (--guesses)	/* not ran out of guesses yet */
			{	fprintf(pgpout,PSTR("\nYou need a pass phrase to unlock your RSA secret key. "));
				if (!showkey && guesses == 2)
				{	/* let user know for which key he should type his password */
					PascalToC((char *)userid);
					fprintf(pgpout, PSTR("\nKey for user ID \"%s\"\n"), 
						LOCAL_CHARSET((char *)userid));
					CToPascal((char *)userid);
				}
				hidekey = (GetHashedPassPhrase(passphrase, (char *) ideakey, 1) > 0);
				continue;	/* take it from the top */
			}	/* more guesses to go */
		}
		if (status < 0)
		{	fprintf(pgpout,PSTR("\n\007Could not read key from file '%s'.\n"),
				keyfile);
			fclose(f);	/* close key file */
			return(-1);
		}
	}	while (status < 0);	/* until key reads OK, with good password */

	fclose(f);	/* close key file */

	/* Note that readkeypacket has called set_precision */

	if (d != NULL)		/* No effective check of pass phrase if d is NULL */
	{	if (!hidekey)
			fprintf(pgpout,PSTR("\nAdvisory warning: This RSA secret key is not protected by a passphrase.\n"));
		else
			fprintf(pgpout,PSTR("Pass phrase is good.  "));

		if (testeq(d,0))	/* didn't get secret key components */
		{	fprintf(pgpout,PSTR("\n\007Key file '%s' is not a secret key file.\n"),keyfile);
			return(-1);
		}
	}

	return(0);	/* normal return */

}	/* getsecretkey */


int is_compromised(FILE *f)
/* check if a key has a compromise certificate, file pointer must
   be positioned at or right after the key packet.
*/
{
	long pos, savepos;
	byte class, ctb;
	int cert_len;
	int status = 0;

	pos = savepos = ftell(f);

	nextkeypacket(f, &ctb);
	if (is_key_ctb(ctb))
	{	pos = ftell(f);
		nextkeypacket(f, &ctb);
	}
	if (ctb != CTB_KEYCTRL)
		fseek(f, pos, SEEK_SET);

	/* file pointer now positioned where compromise cert. should be */
	if (fread(&ctb, 1, 1, f) != 1)
	{	status = -1;
		goto ex;
	}
	
	if (is_ctb_type(ctb, CTB_SKE_TYPE))
	{
		cert_len = ( int ) getpastlength(ctb, f);
		if (cert_len > MAX_SIGCERT_LENGTH)	/* Huge packet length */
		{	status = -1;
			goto ex;
		}

		/* skip version and mdlen byte */
		fseek(f, 2L, SEEK_CUR);
		if (fread(&class, 1, 1, f) != 1)
		{	status = -1;
			goto ex;
		}
		status = (class == KC_SIGNATURE_BYTE);
	}
ex:
	fseek(f, savepos, SEEK_SET);
	return(status);
}


/*	Alfred Hitchcock coined the term "mcguffin" for the generic object 
	being sought in his films-- the diamond, the microfilm, etc. 
*/


/*	Calculate and display a hash for the public components of the key.
	The components are converted to their external (big-endian) 
	representation, concatenated, and an MD5 on the bit values 
	(ie excluding the length value) calculated and displayed in hex.

	The hash, or "fingerprint", of the key is useful mainly for quickly
	and easily verifying over the phone that you have a good copy of 
	someone's public key.  Just read the hash over the phone and have
	them check it against theirs.
*/

void getKeyHash( byte *hash, unitptr n, unitptr e )
{
	MD5_CTX mdContext;
	byte buffer[ MAX_BYTE_PRECISION + 2 ];
	byte mdBuffer[ MAX_BYTE_PRECISION * 2 ];
	int i, mdIndex = 0, bufIndex;

	/* Convert n and e to external (big-endian) byte order and move to mdBuffer */
	i = reg2mpi( buffer, n );
	for( bufIndex = 2; bufIndex < i + 2; bufIndex++ )	/* +2 skips count */
		mdBuffer[ mdIndex++ ] = buffer[ bufIndex ];
	i = reg2mpi( buffer, e );
	for( bufIndex = 2; bufIndex < i + 2; bufIndex++ )	/* +2 skips count */
		mdBuffer[ mdIndex++ ] = buffer[ bufIndex ];

	/* Now evaluate the MD5 for the two MPI's */
	MD5Init( &mdContext );
	MD5Update( &mdContext, mdBuffer, mdIndex );
	MD5Final( &mdContext );

	for( i = 0; i < 16; i++ )
		hash[i] = mdContext.digest[i];

}	/* getKeyHash */


static void showKeyHash( unitptr n, unitptr e )
{
	byte hash[16];
	int i;

	getKeyHash(hash,n,e);	/* compute hash of (n,e) */

/*	Display the hash.  The format is:
pub  1024/xxxxxx yyyy-mm-dd  aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
          Key fingerprint =  xx xx xx xx xx xx xx xx  xx xx xx xx xx xx xx xx 
*/
	fprintf( pgpout, "%27s  ", PSTR("Key fingerprint =" ) );
	for( i = 0; i < 8; i++ )
		fprintf(pgpout, "%02X ", hash[ i ] );
	putc( ' ', pgpout);
	for( i = 8; i < 16; i++ )
		fprintf(pgpout, "%02X ", hash[ i ] );
	putc( '\n', pgpout);
}	/* showKeyHash */


int view_keyring(char *mcguffin, char *ringfile, boolean show_signatures, boolean show_hashes)
/*	Lists all entries in keyring that have mcguffin string in userid.
	mcguffin is a null-terminated C string.
*/
{	FILE *f;
	byte ctb, keyctb;
	long fpr;
	int pktlenr;
	int status;
	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
	byte keyID[KEYFRAGSIZE];
	byte sigkeyID[KEYFRAGSIZE];
	byte userid[256];		/* key certificate userid */
	byte siguserid[256];	/* signator userid */
	char dfltring[MAX_PATH];
	word32 tstamp;
	byte *timestamp = (byte *) &tstamp;		/* key certificate timestamp */
	int keycounter = 0;
	int firstuser = 0;
	int compromised = 0;
	boolean shownKeyHash;
	boolean invalid_key;	/* unsupported version or bad data */
	boolean match = FALSE;
	extern boolean moreflag;

	/* Default keyring to check signature ID's */
	buildfilename(dfltring,PUBLIC_KEYRING_FILENAME);

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(ringfile,FOPRBIN)) == NULL)
	{	fprintf(pgpout,PSTR("\n\007Can't open key ring file '%s'\n"),ringfile);
		return(-1);
	}

/*	Here's a good format for display of key or signature certificates:
Type bits/keyID   Date       User ID
pub  1024/xxxxxx yyyy-mm-dd  aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
sec   512/xxxxxx yyyy-mm-dd  aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
sig   384/xxxxxx yyyy-mm-dd  aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
*/

	if (moreflag)
		open_more();
	fprintf(pgpout,PSTR("\nKey ring: '%s'"),ringfile);
	if (mcguffin && strlen(mcguffin) > 0)
		fprintf(pgpout,PSTR(", looking for user ID \"%s\"."),LOCAL_CHARSET(mcguffin));
	fprintf(pgpout,PSTR("\nType bits/keyID   Date       User ID\n"));
	for ( ; ; )
	{
		status = readkeypacket(f,FALSE,&ctb,timestamp,(char *)userid,n,e,
				NULL,NULL,NULL,NULL,sigkeyID,NULL);
		/* Note that readkeypacket has called set_precision */
		if (status== -1)
		{	status = 0;
			break;	/* eof reached */
		}
		if (status == -4 || status == -6)
		{	/* only ctb and userid are valid */
			memset(sigkeyID, 0, KEYFRAGSIZE);
			tstamp = 0;
		}
		else if (status < 0)
		{	fprintf(pgpout,PSTR("\n\007Could not read key from file '%s'.\n"),
				ringfile);
			break;
		}

		if (is_key_ctb(ctb))
		{
			firstuser = 1;
			keyctb = ctb;
			compromised = is_compromised(f);
			shownKeyHash = FALSE;
			if (status < 0)
			{	invalid_key = TRUE;
				memset(keyID, 0, KEYFRAGSIZE);
			}
			else
			{	invalid_key = FALSE;
				extract_keyID(keyID, n);
			}
		}

		if (ctb != CTB_USERID  &&  !is_ctb_type(ctb, CTB_SKE_TYPE))
			continue;
		if (ctb == CTB_USERID)
		{	PascalToC((char *)userid);
			match = userid_match((char *)userid,mcguffin,n);
		}
		if (match)
		{
			if (ctb == CTB_USERID)
			{	if (firstuser)
				{	keycounter++;
					if (is_ctb_type(keyctb,CTB_CERT_PUBKEY_TYPE))
						fprintf(pgpout,"pub");
					else if (is_ctb_type(keyctb,CTB_CERT_SECKEY_TYPE))
						fprintf(pgpout,"sec");
					else
						fprintf(pgpout,"???");
					if (invalid_key)
						fprintf(pgpout,"? ");
					else
						fprintf(pgpout,"  ");
					fprintf(pgpout,"%4d/%s %s  ",
						countbits(n),keyIDstring(keyID),cdate(&tstamp));
				}
				else
					fprintf(pgpout,"                             ");
				if (compromised && firstuser)
				{	fprintf(pgpout, PSTR("*** KEY REVOKED ***\n"));
					fprintf(pgpout,"                             ");
				}
				firstuser = 0;
				fprintf(pgpout,"%s\n",LOCAL_CHARSET((char *)userid));

				/* Display the hashes for n and e if required */
				if( show_hashes && !shownKeyHash )
				{	showKeyHash( n, e );
					shownKeyHash = TRUE;
				}
			}
			else if (show_signatures && !(firstuser && compromised))	/* Must be sig cert */
			{	fprintf(pgpout,"sig%c      ", status < 0 ? '?' : ' ');
				showkeyID(sigkeyID);
				fprintf(pgpout,"               "); /* Indent signator userid */
				if (getpublickey(TRUE, FALSE, ringfile, &fpr, &pktlenr,
								 sigkeyID, timestamp, siguserid, n, e)>=0 ||
					getpublickey(TRUE, FALSE, dfltring, &fpr, &pktlenr,
								 sigkeyID, timestamp, siguserid, n, e)>=0)
				{	PascalToC((char *)siguserid);
					fprintf(pgpout,"%s\n",LOCAL_CHARSET((char *)siguserid));
				}
				else
					fprintf(pgpout,PSTR("(Unknown signator, can't be checked)\n"));
			} /* printing a sig cert */
		}	/* if it has mcguffin */
	}	/* loop for all packets */

	fclose(f);	/* close key file */
	fprintf(pgpout,PSTR("%d key(s) examined.\n"),keycounter);
	close_more();

	return(status);	/* normal return */

}	/* view_keyring */


int dokeycheck(char *mcguffin, char *ringfile, byte *chk_keyID)
/*	Lists all entries in keyring that have mcguffin string in userid.
	mcguffin is a null-terminated C string.
*/
{	FILE *f, *fixedf;
	byte ctb, keyctb;
	long fpsig, fpkey, fixpos = 0;
	int status, sigstatus;
	int keypktlen, sigpktlen;
	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
	byte keyID[KEYFRAGSIZE];
	byte sigkeyID[KEYFRAGSIZE];
	byte keyuserid[256];		/* key certificate userid */
	byte siguserid[256];		/* sig certificate userid */
	char dfltring[MAX_PATH];
	char *tempring;
	word32 tstamp;
	byte *timestamp = (byte *) &tstamp;		/* key certificate timestamp */
	word32 sigtstamp;
	byte *sigtimestamp = (byte *) &sigtstamp;
	byte sigclass;
	int firstuser = 0;
	int compromised = 0;
	boolean invalid_key;	/* unsupported version or bad data */
	boolean failed=FALSE;

	/* Default keyring to check signature ID's */
	buildfilename(dfltring,PUBLIC_KEYRING_FILENAME);

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(ringfile,FOPRBIN)) == NULL)
	{	fprintf(pgpout,PSTR("\n\007Can't open key ring file '%s'\n"),ringfile);
		return(-1);
	}

/*	Here's a good format for display of key or signature certificates:
Type bits/keyID   Date       User ID
pub  1024/xxxxxx yyyy-mm-dd  aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
sec   512/xxxxxx yyyy-mm-dd  aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
sig   384/xxxxxx yyyy-mm-dd  aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
*/

	if (!chk_keyID)
	{	fprintf(pgpout,PSTR("\nKey ring: '%s'"),ringfile);
		if (mcguffin && strlen(mcguffin) > 0)
			fprintf(pgpout,PSTR(", looking for user ID \"%s\"."),LOCAL_CHARSET(mcguffin));
		fprintf(pgpout,PSTR("\nType bits/keyID   Date       User ID\n"));
	}
	for ( ; ; )
	{	long fpos = ftell(f);
		status = readkeypacket(f,FALSE,&ctb,timestamp,(char *)keyuserid,n,e,
				NULL,NULL,NULL,NULL,sigkeyID,NULL);
		/* Note that readkeypacket has called set_precision */
		if (status== -1 ) break;	/* eof reached */
		if (status == -4 || status == -6)
		{	/* only ctb and userid are valid */
			memset(sigkeyID, 0, KEYFRAGSIZE);
			tstamp = 0;
		}
		else if (status < 0)
		{	fprintf(pgpout,PSTR("\n\007Could not read key from file '%s'.\n"),
				ringfile);
			fclose(f);	/* close key file */
			return(-1);
		}

		if (is_key_ctb(ctb))
		{	firstuser = 1;
			keyctb = ctb;
			fpkey = fpos;
			keypktlen = ( int ) ( ftell(f) - fpkey );
			compromised = is_compromised(f);
			if (status < 0)
			{	invalid_key = TRUE;
				memset(keyID, 0, KEYFRAGSIZE);
			}
			else
			{	invalid_key = FALSE;
				extract_keyID(keyID, n);
			}
		}

		if (ctb != CTB_USERID  &&  !is_ctb_type(ctb, CTB_SKE_TYPE))
			continue;
		if (ctb == CTB_USERID)
			PascalToC((char *)keyuserid);

		if (chk_keyID || userid_match((char *)keyuserid,mcguffin,n))
		{
			if (ctb == CTB_USERID)
			{	if (chk_keyID)
					continue;
				if (firstuser)
				{	if (is_ctb_type(keyctb,CTB_CERT_PUBKEY_TYPE))
						fprintf(pgpout,"pub");
					else if (is_ctb_type(keyctb,CTB_CERT_SECKEY_TYPE))
						fprintf(pgpout,"sec");
					else
						fprintf(pgpout,"???");
					if (invalid_key)
						fprintf(pgpout,"? ");
					else
						fprintf(pgpout,"  ");
					fprintf(pgpout,"%4d/%s %s  ",
						countbits(n),keyIDstring(keyID),cdate(&tstamp));
				}
				else
					fprintf(pgpout,"                             ");
				if (compromised && firstuser)
				{	fprintf(pgpout, PSTR("*** KEY REVOKED ***\n"));
					fprintf(pgpout,"                             ");
				}
				firstuser = 0;
				fprintf(pgpout,"%s\n",LOCAL_CHARSET((char *)keyuserid));
			}
			else /* Must be sig cert */
			{	/* Try checking signature on either this ring or dflt ring */
				if (chk_keyID && memcmp(chk_keyID, sigkeyID, KEYFRAGSIZE))
					continue;	/* only check signatures from chk_keyID */
				fpsig = fpos;
				sigpktlen = ( int ) ( ftell(f) - fpsig );
				CToPascal((char *)keyuserid);
				sigstatus = check_key_sig (f, fpkey, keypktlen, (char *) keyuserid,
					f, fpsig, ringfile, (char *) siguserid, sigtimestamp, &sigclass);
				if (sigstatus == -1  &&  strcmp(ringfile,dfltring) != 0)
					sigstatus = check_key_sig (f, fpkey, keypktlen, (char *) keyuserid,
						f, fpsig, dfltring, (char *) siguserid, sigtimestamp, &sigclass);
				PascalToC((char *)keyuserid);
				fseek (f, fpsig+sigpktlen, SEEK_SET);
				if (sigclass == KC_SIGNATURE_BYTE)
					fprintf(pgpout,"com");
				else
					fprintf(pgpout,"sig");
				if (sigstatus >= 0)
					fprintf(pgpout,"!      ");
				else if (status < 0 || sigstatus == -1)
					fprintf(pgpout,"?      ");
				else
					fprintf(pgpout,"*      ");	/* bad signature */
				showkeyID(sigkeyID);
				if (sigstatus == -1)
				{	fprintf(pgpout,"               "); /* Indent signator userid */
					fprintf(pgpout,PSTR("(Unknown signator, can't be checked)\n"));
				}
				else
				{	PascalToC((char *) siguserid);
					fprintf(pgpout," %s  ",cdate(&sigtstamp));
					if (sigclass != KC_SIGNATURE_BYTE)
						fprintf(pgpout, "  ");
					fprintf(pgpout,"%s\n", LOCAL_CHARSET((char *)siguserid));
					if (sigstatus < 0)
					{	fprintf(pgpout,"                               ");
						fprintf(pgpout,PSTR("\007***** BAD SIGNATURE! *****\n"));
						if (!failed)
						{	/* first bad signature: create scratch file */
							tempring = tempfile(TMP_TMPDIR);
							fixedf = fopen(tempring, FOPWBIN);
							failed = TRUE;
						}
						if (fixedf != NULL)
						{
							copyfilepos(f, fixedf, fpsig - fixpos, fixpos);
							fseek(f, fpsig+sigpktlen, SEEK_SET);
							if (nextkeypacket(f, &ctb) < 0 || ctb != CTB_KEYCTRL)
								fseek(f, fpsig+sigpktlen, SEEK_SET);
							fixpos = ftell(f);
						}
					}
				}
			} /* checking a signature */
		}	/* if it has mcguffin */
	}	/* loop for all packets */

	if (!chk_keyID)
		fputc('\n',pgpout);

	if (failed)
	{
		copyfilepos(f, fixedf, -1L, fixpos);
		if (write_error(fixedf))
		{	fclose(fixedf);
			fclose(f);
			return -1;
		}
		fclose(fixedf);
		fprintf(pgpout, PSTR("Remove bad signatures (Y/n)? "));
		if (getyesno('y'))
		{
			savetempbak(tempring, ringfile);
			failed = 0;
		}
	}
	fclose(f);	/* close key file */

	return(failed?-1:0);	/* normal return */

}	/* dokeycheck */

int backup_rename(char *scratchfile, char *destfile)
{	/* rename scratchfile to destfile after making a backup file */
	char bakfile[MAX_PATH];

	if (is_tempfile(destfile))
	{
		remove(destfile);
	}
	else
	{	if (file_exists(destfile))
		{
			strcpy(bakfile, destfile);
			force_extension(bakfile, BAK_EXTENSION);
			remove(bakfile);
			rename(destfile, bakfile);
		}
	}
	return(rename2(scratchfile, destfile));
}

int remove_sigs(char *mcguffin, char*ringfile)
/* Lists all signatures for keys with specified mcguffin string, and asks
 * if they should be removed.
 */
{	FILE *f, *g;
	byte ctb;
	long fp, fpr, fpuser;
	int packetlength, pktlenr;
	int status;
	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
	byte sigkeyID[KEYFRAGSIZE];
	byte userid[256];		/* key certificate userid */
	byte siguserid[256];	/* signator userid */
	char dfltring[MAX_PATH];
	word32 tstamp;
	byte *timestamp = (byte *) &tstamp;		/* key certificate timestamp */
	int nsigs = 0, nremoved = 0;
	int keeping;
	char *scratchf;

	/* Default keyring to check signature ID's */
	buildfilename(dfltring,PUBLIC_KEYRING_FILENAME);

	if (!mcguffin  ||  strlen(mcguffin) == 0)
		return(-1);

	setoutdir(ringfile);
	scratchf = tempfile(0);

	strcpy((char *)userid,mcguffin);

	fprintf(pgpout,PSTR("\nRemoving signatures from userid '%s' in key ring '%s'\n"),
				LOCAL_CHARSET(mcguffin), ringfile);

	status = getpublickey(TRUE, TRUE, ringfile, &fp, &packetlength, NULL, timestamp, userid, n, e);
	if (status < 0)
	{	fprintf(pgpout,PSTR("\n\007Key not found in key ring '%s'.\n"),ringfile);
		return(0);	/* normal return */
	}

	strcpy((char *)userid,mcguffin);
	getpubuserid (ringfile, fp, userid, &fpuser, &packetlength, FALSE);
	packetlength += ( int ) ( fpuser - fp );

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(ringfile,FOPRBIN)) == NULL)
	{	fprintf(pgpout,PSTR("\n\007Can't open key ring file '%s'\n"),ringfile);
		return(-1);
	}

	/* Count signatures */
	fseek (f, fp+packetlength, SEEK_SET);
	for ( ; ; )
	{	status = nextkeypacket(f, &ctb);
		if (status < 0  ||  is_key_ctb(ctb)  ||  ctb==CTB_USERID)
			break;
		if (is_ctb_type(ctb,CTB_SKE_TYPE))
			++nsigs;
	}
		
	rewind(f);

	if (nsigs == 0)
	{	fprintf (pgpout,PSTR("\nKey has no signatures to remove.\n"));
		fclose (f);
		return (0);		/* Normal return */
	}

	fprintf (pgpout, PSTR("\nKey has %d signature(s):\n"), nsigs);

	/* open file g for writing, in binary (not text) mode...*/
	if ((g = fopen(scratchf,FOPWBIN)) == NULL)
	{	fclose(f);
		return(-1);
	}
	copyfile(f,g,fp+packetlength);	/* copy file f to g up through key */

	/* Now print out any following sig certs */
	keeping = 1;
	for ( ; ; )
	{	fp = ftell(f);
		status = readkeypacket(f, FALSE, &ctb, NULL, NULL, NULL, NULL,
				NULL,NULL,NULL,NULL,sigkeyID,NULL);
		packetlength = ( int ) ( ftell(f) - fp ); 
		if ((status < 0 && status != -6 && status != -4) ||
				is_key_ctb(ctb)  ||  ctb==CTB_USERID)
			break;
		if (is_ctb_type(ctb,CTB_SKE_TYPE))
		{	fprintf(pgpout,"sig%c     ", status < 0 ? '?' : ' ');
			if (status < 0)
				memset(sigkeyID, 0, KEYFRAGSIZE);
			showkeyID(sigkeyID);
			fprintf(pgpout,"               "); /* Indent signator userid */
			if (getpublickey(TRUE, FALSE, ringfile, &fpr, &pktlenr,
							 sigkeyID, timestamp, userid, n, e)>=0 ||
				getpublickey(TRUE, FALSE, dfltring, &fpr, &pktlenr,
							 sigkeyID, timestamp, userid, n, e)>=0)
			{	PascalToC((char *)userid);
				fprintf(pgpout,"%s\n",LOCAL_CHARSET((char *)userid));
			}
			else
				fprintf(pgpout,PSTR("(Unknown signator, can't be checked)\n"));
			fprintf(pgpout, PSTR("Remove this signature (y/N)? "));
			if (!(keeping=!getyesno('n')))
				++nremoved;
		}
		if (keeping)
			copyfilepos (f, g, (long) packetlength, fp);
	}	/* scanning sig certs */
	copyfilepos (f, g, -1L, fp);		/* Copy rest of file */

	fclose(f);	/* close key file */
	if (write_error(g))
	{	fclose(g);
		return -1;
	}
	fclose(g);	/* close scratch file */
	savetempbak(scratchf,ringfile);
	if (nremoved == 0)
		fprintf(pgpout,PSTR("\nNo key signatures removed.\n"));
	else
		fprintf(pgpout,PSTR("\n%d key signature(s) removed.\n"), nremoved);

	return(0);	/* normal return */

}	/* remove_sigs */


int remove_from_keyring(byte *keyID, char *mcguffin, char *ringfile, boolean secring_too)
/*	Remove the first entry in key ring that has mcguffin string in userid.
	Or it removes the first matching keyID from the ring.
	A non-NULL keyID takes precedence over a mcguffin specifier.
	mcguffin is a null-terminated C string.
	If secring_too is TRUE, the secret keyring is also checked.
*/
{
	FILE *f;
	FILE *g;
	long fp, nfp;
	int packetlength;
	byte ctb;
	int status;
	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
	byte userid[256];		/* key certificate userid */
	word32 tstamp; byte *timestamp = (byte *) &tstamp;		/* key certificate timestamp */
	int userids;
	boolean rmuserid = FALSE;
	char *scratchf;

	default_extension(ringfile,PGP_EXTENSION);

	if ((keyID==NULL) && (!mcguffin  ||  strlen(mcguffin)==0))
		return(-1); /* error, null mcguffin will match everything */

top:
	if (mcguffin)
		strcpy((char *)userid,mcguffin);

	fprintf(pgpout,PSTR("\nRemoving from key ring: '%s'"),ringfile);
	if (mcguffin  &&  strlen(mcguffin) > 0)
		fprintf(pgpout,PSTR(", userid \"%s\".\n"),
			LOCAL_CHARSET(mcguffin));

	status = getpublickey(TRUE, TRUE, ringfile, &fp, &packetlength, NULL, timestamp, userid, n, e);
	if (status < 0 && status != -4 && status != -6)
	{	fprintf(pgpout,PSTR("\n\007Key not found in key ring '%s'.\n"),ringfile);
		return(0);	/* normal return */
	}

	/* Now add to packetlength the subordinate following certificates */
	if ((f = fopen(ringfile,FOPRBIN)) == NULL)
	{	fprintf(pgpout,PSTR("\n\007Can't open key ring file '%s'\n"),ringfile);
		return(-1);
	}
	fseek (f, fp+packetlength, SEEK_SET);
	userids = 0;
	do 	/* count user ID's, position nfp at next key */
	{	nfp = ftell(f);
		status = nextkeypacket(f, &ctb);
		if (status == 0 && ctb == CTB_USERID)
			++userids;
	} while (status == 0 && !is_key_ctb(ctb));
	if (status < -1)
	{	fclose(f);
		return(-1);
	}

	if (keyID==NULL)	/* Human confirmation is required. */
	{	/* Supposedly the key was fully displayed by getpublickey */
		if (userids > 1)
		{	fprintf(pgpout, PSTR("\nKey has more than one user ID.\n\
Do you want to remove the whole key (y/N)? "));
			if (!getyesno('n'))
			{	/* find out which userid should be removed */
				rmuserid = TRUE;
				fseek (f, fp+packetlength, SEEK_SET);
				for ( ; ; )
				{	fp = ftell(f);
					status = readkpacket(f, &ctb, (char *) userid, NULL, NULL);
					if (status < 0 && status != -4 && status != -6 || is_key_ctb(ctb))
					{	fclose(f);
						fprintf(pgpout, PSTR("\nNo more user ID's\n"));
						return(-1);
					}
					if (ctb == CTB_USERID)
					{	fprintf(pgpout, PSTR("Remove \"%s\" (y/N)? "), userid);
						if (getyesno('n'))
							break;
					}
				}
				do 	/* also remove signatures and trust bytes */
				{	nfp = ftell(f);
					status = nextkeypacket(f, &ctb);
				} while ((status == 0 || status == -4 || status == -6) &&
						!is_key_ctb(ctb) && ctb != CTB_USERID);
				if (status < -1 && status != -4 && status != -6)
				{	fclose(f);
					return(-1);
				}
			}
		}
		else if (!filter_mode)	/* only one user ID, interpret -f as force flag */
		{	fprintf(pgpout,
				PSTR("\nAre you sure you want this key removed (y/N)? "));
			if (!getyesno('n'))
			{	fclose(f);
				return(-1);	/* user said "no" */
			}
		}
	}
	fclose(f);
	packetlength = ( int ) ( nfp - fp );

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(ringfile,FOPRBIN)) == NULL)
	{	fprintf(pgpout,PSTR("\n\007Can't open key ring file '%s'\n"),ringfile);
		return(-1);
	}

	setoutdir(ringfile);
	scratchf = tempfile(0);
	/* open file g for writing, in binary (not text) mode...*/
	if ((g = fopen(scratchf,FOPWBIN)) == NULL)
	{	fclose(f);
		return(-1);
	}
	copyfilepos(f,g,fp,0L);	/* copy file f to g up to position fp */
	copyfilepos(f,g,-1L,fp+packetlength); /* copy rest of file f */
	fclose(f);	/* close key file */
	if (write_error(g))
	{	fclose(g);
		return -1;
	}
	fclose(g);	/* close scratch file */
	savetempbak(scratchf,ringfile);
	if (rmuserid)
		fprintf(pgpout,PSTR("\nUser ID removed from key ring.\n"));
	else
		fprintf(pgpout,PSTR("\nKey removed from key ring.\n"));
	
	if (secring_too)
	{	secring_too = FALSE;
		buildfilename(ringfile, SECRET_KEYRING_FILENAME);
		strcpy((char *)userid,mcguffin);
		if (getpublickey(TRUE, FALSE, ringfile, &fp, &packetlength, NULL, timestamp, userid, n, e) == 0)
		{	fprintf(pgpout, PSTR("\nKey or user ID is also present in secret keyring.\n\
Do you also want to remove it from the secret keyring (y/N)? "));
			if (getyesno('n'))
				goto top;
		}
	}

	return(0);	/* normal return */

}	/* remove_from_keyring */


int extract_from_keyring (char *mcguffin, char *keyfile, char *ringfile,
				boolean transflag)
/*	Copy the first entry in key ring that has mcguffin string in
	userid and put it into keyfile.
	mcguffin is a null-terminated C string.
*/
{
	FILE *f;
	FILE *g;
	long fp, dummy_fp;
	int packetlength=0, dummy_packetlength;
	byte ctb;
	int status;
	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
	byte keyID[KEYFRAGSIZE];
	byte userid[256];	/* key certificate userid */
	char fname[MAX_PATH], transfile[MAX_PATH], transname[MAX_PATH];
	char *tempf;
	word32 tstamp; byte *timestamp = (byte *) &tstamp; /* key cert tstamp */
	boolean append = FALSE;
	boolean whole_ring = FALSE;

	default_extension(ringfile, PGP_EXTENSION);

	if (!mcguffin  ||  strlen(mcguffin)==0 || strcmp(mcguffin, "*") == 0)
		whole_ring = TRUE;

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(ringfile,FOPRBIN)) == NULL)
	{	fprintf(pgpout,PSTR("\n\007Can't open key ring file '%s'\n"),ringfile);
		return(-1);
	}

	if (!whole_ring)
	{
		strcpy((char *)userid, mcguffin);
		fprintf(pgpout,PSTR("\nExtracting from key ring: '%s'"),ringfile);
		fprintf(pgpout,PSTR(", userid \"%s\".\n"),LOCAL_CHARSET(mcguffin));

		status = getpublickey(TRUE, TRUE, ringfile, &fp, &packetlength, NULL,
					timestamp, userid, n, e);
		if (status < 0 && status != -4 && status != -6)
		{	fprintf(pgpout,PSTR("\n\007Key not found in key ring '%s'.\n"),
									ringfile);
			fclose(f);
			return(0);	/* normal return */
		}
		extract_keyID(keyID, n);
	}
	else
	{
		do	/* set fp to first key packet */
			fp = ftell(f);
		while ((status = nextkeypacket(f, &ctb)) >= 0 && !is_key_ctb(ctb));
		if (status < 0)
		{	fclose(f);
			return(-1);
		}
		packetlength = ( int ) ( ftell(f) - fp );
	}

	if (!keyfile  ||  strlen(keyfile)==0)
	{	fprintf(pgpout, PSTR("\nExtract the above key into which file? "));
		getstring( fname, sizeof(fname)-4, TRUE );
		if (*fname == '\0')
			return(-1);
	}
	else
		strcpy(fname,keyfile);
	default_extension(fname,PGP_EXTENSION);

	/* If transport armoring, use a dummy file for keyfile */
	if (transflag)
	{	strcpy(transname, fname);
		strcpy(transfile, fname);
		force_extension(transfile, ASC_EXTENSION);
		tempf = tempfile(TMP_TMPDIR|TMP_WIPE);
		strcpy(fname, tempf);
	}
	if (file_exists( transflag?transfile:fname ))
	{
		if (!transflag && !whole_ring)
		{	/* see if the key is already present in fname */
			status = getpublickey(TRUE, FALSE, fname, &dummy_fp, &dummy_packetlength,
					keyID, timestamp, userid, n, e);
			if (status >= 0 || status == -4 || status == -6)
			{	fclose(f);
				fprintf(pgpout,PSTR("Key ID %s is already included in key ring '%s'.\n"),
					keyIDstring(keyID), fname);
				return(-1);
			}
		}
		if (whole_ring || transflag || status < -1)
		{ /* if status < -1 then fname is not a keyfile, ask if it should be overwritten */
			fprintf(pgpout,PSTR("\n\007Output file '%s' already exists.  Overwrite (y/N)? "),
				transflag?transfile:fname);
			if (!getyesno( 'n' ))
			{	fclose(f);
				return(-1);	/* user chose to abort */
			}
		}
		else
			append = TRUE;
	}

	if (append)
		g = fopen(fname, FOPRWBIN);
	else
		g = fopen(fname, FOPWBIN);
	if (g == NULL)
	{	if (append)
			fprintf(pgpout,PSTR("\n\007Can't open key ring file '%s'\n"),ringfile);
		else
			fprintf(pgpout,PSTR("\n\007Unable to create key file '%s'.\n"), fname);
		fclose(f);
		return(-1);
	}
	if (append)
		fseek(g, 0L, SEEK_END);
	do
	{
		if (copyfilepos(f, g, (long) packetlength, fp) < 0)	/* Copy key out */
		{	status = -2;
			break;
		}
		/* Copy any following signature or userid packets */
		for ( ; ; )
		{	fp = ftell(f);
			status = nextkeypacket(f, &ctb);
			packetlength = ( int ) ( ftell(f) - fp );
			if (status < 0  ||  is_key_ctb(ctb))
				break;
			if (ctb==CTB_USERID  ||  is_ctb_type(ctb,CTB_SKE_TYPE))
				if (copyfilepos(f, g, (long) packetlength, fp) < 0)
				{	status = -2;
					break;
				}
		}
	}
	while (whole_ring && status >= 0);

	fclose(f);
	if (status < -1 || write_error(g))
	{	fclose(g);
		return(-1);
	}
	fclose(g);

	if (transflag)
	{	status = armor_file (fname, transfile, transname, NULL);
		rmtemp (tempf);
		if (status)
			return(-1);
	}

	fprintf (pgpout,PSTR("\nKey extracted to file '%s'.\n"), transflag?transfile:fname);

	return (0);	/* normal return */
}	/* extract_from_keyring */


/*======================================================================*/

int merge_key_to_ringfile(char *keyfile, char* ringfile, long fp,
				int packetlength, long keylen)
/* Copy the key data in keyfile into ringfile, replacing the data that
   is in ringfile starting at fp and for length packetlength.
   keylen is the number of bytes to copy from keyfile
*/
{	FILE	*f, *g, *h;
	char *tempf;
	int rc;

	setoutdir(ringfile);
	tempf = tempfile(TMP_WIPE);
	/* open file f for reading, binary, as keyring file */
	if ((f = fopen(ringfile,FOPRBIN)) == NULL)
		return(-1);
	/* open file g for writing, binary, as scratch keyring file */
	if ((g = fopen(tempf,FOPWBIN)) == NULL)
	{	fclose(f);
		return(-1);
	}
	/* open file h for reading, binary, as key file to be inserted */
	if ((h = fopen(keyfile,FOPRBIN)) == NULL)
	{	fclose(f);
		fclose(g);
		return(-1);
	}
	/* Copy pre-key keyring data from f to g */
	copyfile(f,g,fp);
	/* Copy temp key data from h to g */
	copyfile(h,g,keylen);
	/* Copy post-key keyring data from f to g */
	copyfilepos(f,g,-1L,fp+packetlength);
	fclose(f);
	rc = write_error(g);
	fclose(g);
	fclose(h);

	if (!rc)
		savetempbak(tempf,ringfile);

	return(rc ? -1 : 0);
}	/* merge_key_to_ringfile */

static int insert_userid(char *keyfile, byte *userid, long fpos)
{	/* insert userid and trust byte at position fpos in file keyfile */
	char *tmpf;
	FILE *f, *g;

	tmpf = tempfile(TMP_TMPDIR);
	if ((f = fopen(keyfile, FOPRBIN)) == NULL)
		return(-1);
	if ((g = fopen(tmpf, FOPWBIN)) == NULL)
	{	fclose(f);
		return(-1);
	}
	copyfile(f, g, fpos);
	putc(CTB_USERID, g);
	fwrite(userid, 1, userid[0]+1, g);
	write_trust(g, KC_LEGIT_COMPLETE);
	copyfile(f, g, -1L);
	fclose(f);
	if (write_error(g))
	{	fclose(g);
		return(-1);
	}
	fclose(g);
	return(savetempbak(tmpf, keyfile));
}

int dokeyedit(char *mcguffin, char *ringfile)
/*	Edit the userid and/or pass phrase for an RSA key pair, and
	put them back into the ring files.
*/
{	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION], d[MAX_UNIT_PRECISION],
		 p[MAX_UNIT_PRECISION], q[MAX_UNIT_PRECISION], u[MAX_UNIT_PRECISION];
	char *fname, secring[MAX_PATH];
	FILE *f;
	word16 iv[4]; /* for IDEA CFB mode, to protect RSA secret key */
	byte userid[256];
	byte userid1[256];
	char passphrase[256];
	word32 tstamp; byte *timestamp = (byte *) &tstamp;	/* key certificate timestamp */
	byte keyID[KEYFRAGSIZE];
	boolean hidekey;	/* TRUE iff secret key is encrypted */
	boolean changed=FALSE, changeID=FALSE;
	byte ctb;
	int status;
	long fpp,fps,trust_pos, keylen;
	int pplength=0, pslength=0;
	byte ideakey[16];
	byte keyctrl;

	if (!ringfile || strlen(ringfile)==0 || !mcguffin || strlen(mcguffin)==0)
		return(-1);	/* Need ringfile name, user name */

	force_extension(ringfile,PGP_EXTENSION);

	strcpy((char *)userid, mcguffin);
	fprintf(pgpout,PSTR("\nEditing userid \"%s\" in key ring: '%s'.\n"),
		LOCAL_CHARSET((char *)userid),ringfile);

	if (!file_exists (ringfile))
	{	fprintf(pgpout,PSTR("\nCan't open public key ring file '%s'\n"),
			ringfile);
		return(-1);
	}

	status = getpublickey(TRUE, TRUE, ringfile, &fpp, &pplength, NULL,
				timestamp, userid, n, e);
	if (status < 0)
	{	fprintf(pgpout,PSTR("\n\007Key not found in key ring '%s'.\n"),
			ringfile);
		return(-1);
	}

	/* Now add to pplength any following key control certificate */
	if ((f = fopen(ringfile,FOPRWBIN)) == NULL)
	{	fprintf(pgpout,PSTR("\n\007Can't open key ring file '%s'\n"),ringfile);
		return(-1);
	}

	fseek(f, fpp, SEEK_SET);
	if (is_compromised(f))
	{	fprintf(pgpout, PSTR("\n\007This key has been revoked by its owner.\n"));
		fclose(f);
		return(-1);
	}
	trust_pos = fpp+pplength;
	fseek(f, trust_pos, SEEK_SET);
	if (read_trust(f, &keyctrl) < 0)
		trust_pos = -1;		/* keyfile: no trust byte */

	extract_keyID(keyID, n);

	/* Now read private key, too */
	strcpy(secring, ringfile);
	strcpy(file_tail(secring), SECRET_KEYRING_FILENAME);

	if (!file_exists (secring))
	{	fprintf(pgpout,PSTR("\nCan't open secret key ring file '%s'\n"),
			secring);
		fclose(f);
		return(-1);
	}

	/* Get position of key in secret key file */
	(void)getpublickey(TRUE, FALSE, secring, &fps, &pslength, keyID,
		timestamp, userid1, n, e);
	/* This was done to get us fps and pslength */
	status = getsecretkey(TRUE, FALSE, secring, keyID, timestamp,
		passphrase, &hidekey, userid1, n, e, d, p, q, u);

	if (status < 0)		/* key not in secret keyring: edit owner trust */
	{	int i;

		fprintf(pgpout, PSTR("\nNo secret key available.  Editing public key trust parameter.\n"));
		if (trust_pos < 0)
		{	fprintf(pgpout,PSTR("\n\007File '%s' is not a public keyring.\n"), ringfile);
			fclose(f);
			return(-1);
		}
		show_key(f, fpp, SHOW_ALL);

		init_trust_lst();
		fprintf(pgpout, PSTR("Current trust for this key's owner is: %s\n"),
				trust_lst[keyctrl & KC_OWNERTRUST_MASK]);

		PascalToC((char *)userid);	/* convert to C string for display */
		i = ask_owntrust((char *) userid, keyctrl);
		if (i == (keyctrl & KC_OWNERTRUST_MASK))
		{	fclose(f);
			return(0);	/* unchanged */
		}

		if (i < 0 || i > KC_OWNERTRUST_ALWAYS)
		{
			fclose(f);
			return(-1);
		}
		keyctrl = (keyctrl & ~KC_OWNERTRUST_MASK) | i;

		fseek(f, trust_pos, SEEK_SET);
		write_trust(f, keyctrl);
		fclose(f);
		fprintf (pgpout, PSTR("Public key ring updated.\n"));
		return(0);
	}
	if (trust_pos > 0 && (keyctrl & (KC_BUCKSTOP|KC_OWNERTRUST_MASK)) !=
			(KC_OWNERTRUST_ULTIMATE|KC_BUCKSTOP))
	{	/* key is in secret keyring but buckstop is not set */
		fprintf(pgpout, PSTR("\nUse this key as an ultimately-trusted introducer (y/N)? "), userid);
		if (getyesno('n'))
		{	fseek(f, trust_pos, SEEK_SET);
			keyctrl = KC_OWNERTRUST_ULTIMATE|KC_BUCKSTOP;
			write_trust(f, keyctrl);
		}
	}

	/* Show user her ID again to be clear */
	PascalToC((char *)userid);
	fprintf(pgpout,PSTR("\nCurrent user ID: %s"),
		LOCAL_CHARSET((char *)userid));
	CToPascal((char *)userid);

	fprintf(pgpout, PSTR("\nDo you want to change your user ID (y/N)? "));
	if (getyesno('n'))	/* user said yes */
	{	fprintf(pgpout,PSTR("\nEnter the new user ID: "));
		getstring((char *)userid,255,TRUE); /* echo keyboard input */
		if (userid[0] == '\0')
		{	fclose(f);
			return(-1);
		}
		CONVERT_TO_CANONICAL_CHARSET((char *)userid);
		fprintf(pgpout, PSTR("\nMake this user ID the primary user ID for this key (y/N)? "));
		if (!getyesno('n'))
		{	/* position file pointer at selected user id */
			int pktlen;
			long fpuser;

			strcpy((char *)userid1, mcguffin);
			if (getpubuserid(ringfile, fpp, userid1, &fpuser, &pktlen, FALSE) < 0)
			{	fclose(f);
				return(-1);
			}
			fseek(f, fpuser, SEEK_SET);
		}
		else	/* possition file pointer at key packet */
			fseek(f, fpp, SEEK_SET);
		nextkeypacket(f, &ctb);	/* skip userid or key packet */
		do	/* new user id will be inserted before next userid or key packet */
		{	fpp = ftell(f);
			if (nextkeypacket(f, &ctb) < 0)
				break;
		} while (ctb != CTB_USERID && !is_key_ctb(ctb));
		CToPascal((char *)userid);	    /* convert to length-prefixed string */
		changeID = TRUE;
		changed = TRUE;
	}
	fclose(f);

	fprintf (pgpout,PSTR("\nDo you want to change your pass phrase (y/N)? "));
	if (getyesno('n'))	/* user said yes */
	{	hidekey = (GetHashedPassPhrase(passphrase, (char *) ideakey, 2) > 0);
		changed = TRUE;
	}
	else
	{	if (hidekey)
			hashpass( passphrase, strlen(passphrase), ideakey );
	}

	if (!changed)
	{	fprintf (pgpout, PSTR("(No changes will be made.)\n"));
		if (hidekey)
			burn((byteptr)passphrase);
		goto done;
	}

	/* init CFB IDEA key */
	if (hidekey)
	{	fill0(iv,8);
		initcfb_idea(iv,ideakey,FALSE);
		burn((byteptr)passphrase);	/* burn sensitive data on stack */
	}

	/* First write secret key data to a file */
	fname = tempfile(TMP_TMPDIR|TMP_WIPE);
	writekeyfile(fname,hidekey,timestamp,userid,n,e,d,p,q,u);
	if (changeID)
		keylen = -1;
	else
	{	/* don't copy userid */
		f = fopen(fname, FOPRBIN);
		if (f == NULL)
			goto err;
		nextkeypacket(f, &ctb);	/* skip key packet */
		keylen = ftell(f);
		fclose(f);
	}
	if (merge_key_to_ringfile(fname,secring,fps,pslength,keylen) < 0)
	{	fprintf (pgpout, PSTR("\n\007Unable to update secret key ring.\n"));
		goto err;
	}
	fprintf (pgpout, PSTR("\nSecret key ring updated...\n"));

	/* Now write public key data to file */
	if (changeID)
	{
		if (insert_userid(ringfile, userid, fpp) < 0)
		{	fprintf (pgpout, PSTR("\n\007Unable to update public key ring.\n"));
			goto err;
		}
		fprintf (pgpout, PSTR("Public key ring updated.\n"));
	}
	else
		fprintf (pgpout, PSTR("(No need to update public key ring)\n"));

	if (hidekey)	/* done with IDEA to protect RSA secret key */
		close_idea();

	rmtemp(fname);

done:
	mp_burn(d);	/* burn sensitive data on stack	*/
	mp_burn(p);	/* 	"		"		"	"	"	*/
	mp_burn(q);	/*	"		"		"	"	"	*/
	mp_burn(u);	/*	"		"		"	"	"	*/
	mp_burn(e);	/*	"		"		"	"	"	*/
	mp_burn(n);	/*	"		"		"	"	"	*/
	burn(iv);	/*	"		"		"	"	"	*/

	return(0);	/* normal return */
err:
	mp_burn(d);	/* burn sensitive data on stack */
	mp_burn(p);	/*	"		"		"	"	"	*/
	mp_burn(q);	/*	"		"		"	"	"	*/
	mp_burn(u);	/*	"		"		"	"	"	*/
	mp_burn(e);	/*	"		"		"	"	"	*/
	mp_burn(n);	/*	"		"		"	"	"	*/
	burn(iv);	/*	"		"		"	"	"	*/

	rmtemp(fname);

	return(-1);	/* error return */

}	/* dokeyedit */


/*======================================================================*/



int dokeygen(char *numstr, char *numstr2)
/*	Do an RSA key pair generation, and write them out to the keyring files.
	numstr is a decimal string, the desired bitcount for the modulus n.
	numstr2 is a decimal string, the desired bitcount for the exponent e.
*/
{	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION], d[MAX_UNIT_PRECISION],
		 p[MAX_UNIT_PRECISION], q[MAX_UNIT_PRECISION], u[MAX_UNIT_PRECISION];
	char *fname;
	char ringfile[MAX_PATH];
	word16 iv[4]; /* for IDEA CFB mode, to protect RSA secret key */
	byte userid[256];
	short keybits,ebits;
	word32 tstamp; byte *timestamp = (byte *) &tstamp;		/* key certificate timestamp */
	boolean hidekey;	/* TRUE iff secret key is encrypted */
	byte ideakey[16];

	if (!numstr || strlen(numstr)==0)
	{	fprintf(pgpout,PSTR("\nPick your RSA key size:\
\n	1)	 384 bits- Casual grade, fast but less secure\
\n	2)	 512 bits- Commercial grade, medium speed, good security\
\n	3)	1024 bits- Military grade, very slow, highest security\
\nChoose 1, 2, or 3, or enter desired number of bits: "));
		numstr = (char *)userid;	/* use userid buffer as scratchpad */
		getstring(numstr,5,TRUE);	/* echo keyboard */
	}

	keybits = 0;
	while ((*numstr>='0') && (*numstr<='9'))
		keybits = keybits*10 + (*numstr++ - '0');

	if (keybits==0)	/* user entered null response */
		return(-1);	/* error return */

	/* Standard default key sizes: */
	if (keybits==1) keybits=384;	/* Casual grade */
	if (keybits==2) keybits=512;	/* Commercial grade */
	if (keybits==3) keybits=1024;	/* Military grade */

#ifndef DEBUG
	/* minimum RSA keysize: */
	if (keybits<384) keybits=384;
	if (keybits>1024) keybits=1024;
#endif

#ifdef notdef	
/*	If we use Merritt's modmult algorithm, the primes p and q's 
	bit length should not be an exact multiple of UNITSIZE, 
	because Merritt's modmult algorithm performs slowest in that 
	case, wasting an extra unit of precision for overflow.
*/
	if ((keybits % (2*UNITSIZE))==0)
		keybits -= 2;	/* make each prime one bit shorter. */
#endif	/* MERRITT */

	ebits = 0;	/* number of bits in e */
	while ((*numstr2>='0') && (*numstr2<='9')) 
		ebits = ebits*10 + (*numstr2++ - '0');

	fprintf(pgpout,PSTR("\nGenerating an RSA key with a %d-bit modulus... "),keybits);

	fprintf(pgpout,
PSTR("\nYou need a user ID for your public key.  The desired form for this\n\
user ID is your name, followed by your E-mail address enclosed in\n\
<angle brackets>, if you have an E-mail address.\n\
For example:  John Q. Smith <12345.6789@compuserve.com>\n"));
	fprintf(pgpout,PSTR("\nEnter a user ID for your public key: \n"));
	getstring((char *)userid,255,TRUE);	/* echo keyboard input */
	if (userid[0]=='\0')	/* user entered null response */
		return(-1);	/* error return */
	CONVERT_TO_CANONICAL_CHARSET((char *)userid);
	CToPascal((char *)userid);	/* convert to length-prefixed string */

	{	char passphrase[256];
		fprintf(pgpout,
PSTR("\nYou need a pass phrase to protect your RSA secret key.\n\
Your pass phrase can be any sentence or phrase and may have many\n\
words, spaces, punctuation, or any other printable characters. "));
		hidekey = (GetHashedPassPhrase(passphrase, (char *) ideakey, 2) > 0);
		/* init CFB IDEA key */
		if (hidekey)
		{	fill0(iv,8);
			initcfb_idea(iv,ideakey,FALSE);
			burn((byteptr)passphrase);	/* burn sensitive data on stack */
		}
	}

	fprintf(pgpout,PSTR("\nNote that key generation is a VERY lengthy process.\n"));

	if (rsa_keygen(n,e,d,p,q,u,keybits,ebits) < 0)
	{	fprintf(pgpout,PSTR("\n\007Keygen failed!\n"));
		return(-1);	/* error return */
	}

	if (verbose)
	{
		fprintf(pgpout,PSTR("Key ID %s\n"), key2IDstring(n));

		mp_display(" modulus n = ",n);
		mp_display("exponent e = ",e);

		mp_display("exponent d = ",d);
		mp_display("   prime p = ",p);
		mp_display("   prime q = ",q);
		mp_display(" inverse u = ",u);
	}

	get_timestamp(timestamp);	/* Timestamp when key was generated */

	fputc('\007',pgpout);  /* sound the bell when done with lengthy process */
	fputc('\n',pgpout);

	/* First, write out the secret key... */
	fname = tempfile(TMP_TMPDIR|TMP_WIPE);
	writekeyfile(fname,hidekey,timestamp,userid,n,e,d,p,q,u); 
	buildfilename(ringfile,SECRET_KEYRING_FILENAME);
	if (file_exists(ringfile))
	{	merge_key_to_ringfile(fname,ringfile,0L,0,-1L);
		rmtemp(fname);
	}
	else
		savetemp(fname, ringfile);

	/* Second, write out the public key... */
	fname = tempfile(TMP_TMPDIR|TMP_WIPE);
	writekeyfile(fname,FALSE,timestamp,userid,n,e,NULL,NULL,NULL,NULL);
	buildfilename(ringfile,PUBLIC_KEYRING_FILENAME);
	if (file_exists(ringfile))
	{	merge_key_to_ringfile(fname,ringfile,0L,0,-1L);
		rmtemp(fname);
	}
	else
		savetemp(fname, ringfile);
	
	if (hidekey)	/* done with IDEA to protect RSA secret key */
		close_idea();

	mp_burn(d);	/* burn sensitive data on stack */
	mp_burn(p);	/* burn sensitive data on stack */
	mp_burn(q);	/* burn sensitive data on stack */
	mp_burn(u);	/* burn sensitive data on stack */
	mp_burn(e);	/* burn sensitive data on stack */
	mp_burn(n);	/* burn sensitive data on stack */
	burn(iv);	/* burn sensitive data on stack */

	fprintf(pgpout,PSTR("\007Key generation completed.\n"));

	/*	Force initialization of cryptographically strong pseudorandom
		number generator seed file for later use...
	*/
	strong_pseudorandom((byte *) iv,1);

	return(0);	/* normal return */
}	/* dokeygen */

