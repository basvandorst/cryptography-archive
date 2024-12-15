/*	keyadd.c  - Keyring merging routines for PGP.
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

static int ask_to_sign(byte *keyID, char *ringfile);

static int newkeys, newsigs, newids, newrvks;

int mergesigs (FILE *fkey, char *keyfile, long keypos, FILE *fring,
		char *ringfile, long *pringpos, FILE *out)
/* Merge signatures from userid in fkey (which is keyfile) at keypos with
 * userid from fring (which is ringfile) at ringpos, appending result to out.
 */
{
	long ringuseridpos, ringpos;
	int ringpktlen, keypktlen;
	int status;
	byte ctb;
	int copying;
	byte keyID[KEYFRAGSIZE];
	char userid[256];

	/* First, copy the userid packet itself, plus any comments or ctrls */
	ringuseridpos = ringpos = *pringpos;
	fseek (fring, ringpos, SEEK_SET);
	(void) readkeypacket(fring,FALSE,&ctb,NULL,userid,NULL,NULL,
				NULL,NULL,NULL,NULL,NULL,NULL);
	PascalToC(userid);
	ringpktlen = ftell(fring) - ringpos;
	copyfilepos (fring, out, ringpktlen, ringpos);
	for ( ; ; )
	{	ringpos = ftell(fring);
		status = nextkeypacket (fring, &ctb);
		if (status < 0  ||  is_key_ctb(ctb)  ||  ctb==CTB_USERID  ||
						is_ctb_type(ctb,CTB_SKE_TYPE))
			break;
		ringpktlen = ftell(fring) - ringpos;
		copyfilepos (fring, out, ringpktlen, ringpos);
	}
	fseek (fring, ringpos, SEEK_SET);

	/* Now, ringpos points just past userid packet and ctrl packet. */
	/* Advance keypos to the analogous location. */
	fseek (fkey, keypos, SEEK_SET);
	(void) nextkeypacket (fkey, &ctb);
	for ( ; ; )
	{	keypos = ftell(fkey);
		status = nextkeypacket (fkey, &ctb);
		if (status < 0  ||  is_key_ctb(ctb)  ||  ctb==CTB_USERID  ||
						is_ctb_type(ctb,CTB_SKE_TYPE))
			break;
	}
	fseek (fkey, keypos, SEEK_SET);

	/* Second, copy all keyfile signatures that aren't in ringfile.
	 */

	copying = FALSE;
	for ( ; ; )
	{	/* Read next sig from keyfile; see if it is in ringfile;
		 * set copying true/false accordingly.  If copying is true
		 * and it is a signature, copy it.  Loop till hit
		 * a new key or userid in keyfile, or EOF.
		 */
		keypos = ftell(fkey);
		status = readkeypacket(fkey,FALSE,&ctb,NULL,NULL,NULL,NULL,
				NULL,NULL,NULL,NULL,keyID,NULL);
		if (status == -3)	/* unrecoverable error: bad packet length etc. */
			return(status);
		keypktlen = ftell(fkey) - keypos;
		if (status == -1  ||  is_key_ctb (ctb)  ||  ctb==CTB_USERID)
		 	break;	/* EOF or next key/userid */
		if (status < 0)
			continue;	/* bad packet, skip it */
		if (is_ctb_type(ctb,CTB_SKE_TYPE))
		{	long sig_pos;
			int sig_len;
			/* Set copying true if signature is not in the ringfile */
			copying = (getpubusersig (ringfile, ringuseridpos, keyID, &sig_pos,
						&sig_len) < 0);
			if (copying)
			{	fprintf (pgpout, PSTR("New signature from keyID %s on userid \"%s\"\n"), 
					keyIDstring(keyID),LOCAL_CHARSET(userid));
				++newsigs;
			}
		}
		if (copying  &&  is_ctb_type(ctb,CTB_SKE_TYPE))
		{	copyfilepos (fkey, out, keypktlen, keypos);
			write_trust (out, KC_SIGTRUST_UNDEFINED);
		}
	}

	/* Third, for all ring sig's, copy to output */
	fseek (fring, ringpos, SEEK_SET);
	for ( ; ; )
	{	ringpos = ftell(fring);
		status = nextkeypacket (fring, &ctb);
		ringpktlen = ftell(fring) - ringpos;
		if (status < 0  ||  is_key_ctb (ctb)  ||  ctb==CTB_USERID)
		 	break;
		copyfilepos (fring, out, ringpktlen, ringpos);
	}	/* End of loop for each sig in ringfile */
	fseek (fring, ringpos, SEEK_SET);
	*pringpos = ringpos;
	return(0);
} /* mergesigs */


int mergekeys (FILE *fkey, char *keyfile, long keypos, FILE *fring,
		char *ringfile, long *pringpos, FILE *out)
/* Merge key from fkey (which is keyfile) at keypos with key from
 * fring (which is ringfile) at ringpos, appending result to out.
 */
{
	long ringkeypos, keykeypos, ringpos;
	int ringpktlen, keypktlen;
	int status;
	byte ctb;
	int copying;
	boolean ring_compromise = FALSE;
	byte userid[256];

	/* First, copy the key packet itself, plus any comments or ctrls */
	ringkeypos = ringpos = *pringpos;
	fseek (fring, ringpos, SEEK_SET);
	(void) nextkeypacket(fring, &ctb);
	ringpktlen = ftell(fring) - ringpos;
	copyfilepos (fring, out, ringpktlen, ringpos);
	for ( ; ; )
	{	ringpos = ftell(fring);
		status = nextkeypacket (fring, &ctb);
		if (status < 0  ||  is_key_ctb(ctb)  ||  ctb==CTB_USERID)
			break;
		if (is_ctb_type(ctb, CTB_SKE_TYPE))
			ring_compromise = TRUE;	/* compromise cert on keyring */
		ringpktlen = ftell(fring) - ringpos;
		copyfilepos (fring, out, ringpktlen, ringpos);
	}
	fseek (fring, ringpos, SEEK_SET);

	/* Now, ringpos points just past key packet and ctrl packet. */
	/* Advance keypos to the analogous location. */
	fseek (fkey, keypos, SEEK_SET);
	keykeypos = keypos;
	(void) nextkeypacket (fkey, &ctb);
	keypktlen = ftell(fkey) - keypos;	/* for check_key_sig() */
	for ( ; ; )
	{	keypos = ftell(fkey);
		status = nextkeypacket (fkey, &ctb);
		if (status < 0 || ctb == CTB_USERID || is_ctb_type(ctb, CTB_SKE_TYPE))
			break;
	}
	if (!ring_compromise && is_ctb_type(ctb, CTB_SKE_TYPE))
	{	/* found a compromise cert on keyfile that is not in ringfile */
		word32 timestamp;
		byte sig_class;
		int cert_pktlen;

		cert_pktlen = ftell(fkey) - keypos;
		if (check_key_sig(fkey, keykeypos, keypktlen, (char *)userid, fkey, keypos,
				ringfile, (char *)userid, (byte *)&timestamp, &sig_class) == 0 &&
				sig_class == KC_SIGNATURE_BYTE)
		{
			fprintf(pgpout, PSTR("Key revocation certificate from \"%s\".\n"),
					LOCAL_CHARSET((char *)userid));
			copyfilepos (fkey, out, cert_pktlen, keypos);
			++newrvks;
		}
		else
			fprintf(pgpout, PSTR("\n\007WARNING:  File '%s' contains bad revocation certificate.\n"));
	}
	fseek (fkey, keypos, SEEK_SET);

	/* Second, copy all keyfile userid's plus signatures that aren't
	 * in ringfile.
	 */

	copying = FALSE;
	for ( ; ; )
	{	/* Read next userid from keyfile; see if it is in ringfile;
		 * set copying true/false accordingly.  If copying is true
		 * and it is a userid or a signature, copy it.  Loop till hit
		 * a new key in keyfile, or EOF.
		 */
		keypos = ftell(fkey);
		status = readkeypacket(fkey,FALSE,&ctb,NULL,(char *)userid,NULL,NULL,
				NULL,NULL,NULL,NULL,NULL,NULL);
		if (status == -3)	/* unrecoverable error: bad packet length etc. */
			return(status);
		keypktlen = ftell(fkey) - keypos;
		if (status == -1  ||  is_key_ctb (ctb))
		 	break;	/* EOF or next key */
		if (status < 0)
			continue;	/* bad packet, skip it */
		if (ctb == CTB_USERID)
		{	long userid_pos;
			int userid_len;
			PascalToC ((char *)userid);
			/* Set copying true if userid is not in the ringfile */
			copying =  (getpubuserid (ringfile, ringkeypos, userid, &userid_pos,
						&userid_len, TRUE) < 0);
			if (copying)
			{	fprintf (pgpout, PSTR("New userid: \"%s\".\n"), 
					LOCAL_CHARSET((char *)userid));
				fprintf(pgpout, PSTR("\nWill be added to the following key:\n"));
				show_key(fring, *pringpos, 0);
				fprintf(pgpout, PSTR("\nAdd this userid (y/N)? "));
				if (getyesno('n'))
					++newids;
				else
					copying = FALSE;
			}
		}
		if (copying)
		{	if (ctb==CTB_USERID  ||  is_ctb_type(ctb,CTB_SKE_TYPE))
			{	copyfilepos (fkey, out, keypktlen, keypos);
				if (is_ctb_type(ctb,CTB_SKE_TYPE))
					write_trust (out, KC_SIGTRUST_UNDEFINED);
				else
					write_trust (out, KC_LEGIT_UNKNOWN);
			}
		}
	}

	/* Third, for all ring userid's, if not in keyfile, copy the userid
	 * plus its dependant signatures.
	 */
	fseek (fring, ringpos, SEEK_SET);
	for ( ; ; )
	{	ringpos = ftell(fring);
		status = readkeypacket(fring,FALSE,&ctb,NULL,(char *)userid,NULL,NULL,
					NULL,NULL,NULL,NULL,NULL,NULL);
		ringpktlen = ftell(fring) - ringpos;
		if (status == -3)
			return(status);
		if (status == -1  ||  is_key_ctb (ctb))
		 	break;
		if (ctb == CTB_USERID)
		{	long userid_pos;
			int userid_len;
			/* See if there is a match in keyfile */
			PascalToC ((char *) userid);
			/* don't use substring match (exact_match = TRUE) */
			if (getpubuserid (keyfile, keykeypos, userid, &userid_pos,
						&userid_len, TRUE) >= 0)
			{	if ((status = mergesigs (fkey,keyfile,userid_pos,fring,ringfile,&ringpos,out)) < 0)
					return(status);
				copying = FALSE;
			}
			else
				copying = TRUE;
		}
		if (copying)
		{	/* Copy ringfile userid and sigs to out */
			copyfilepos (fring, out, ringpktlen, ringpos);
		}
	}	/* End of loop for each key in ringfile */
	fseek (fring, ringpos, SEEK_SET);
	*pringpos = ringpos;
	return(0);
} /* mergekeys */


int addto_keyring(char *keyfile, char *ringfile, boolean query)
/*	Adds (prepends) key file to key ring file.  If query is TRUE, ask
	before doing any actual changes. */
{	FILE *f, *g, *h;
	long file_position,fp;
	int pktlen;	/* unused, just to satisfy getpublickey */
	byte ctb;
	int status;
	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
	unit n1[MAX_UNIT_PRECISION];
	byte keyID[KEYFRAGSIZE];
	byte userid[256];		/* key certificate userid */
	byte userid1[256];
	word32 tstamp; byte *timestamp = (byte *) &tstamp;		/* key certificate timestamp */
	char trans_keyfile[MAX_PATH];
	boolean is_armored = FALSE;
	boolean userid_seen = FALSE;
	int commonkeys = 0;
	int copying;
	struct nkey {
		byte keyID[KEYFRAGSIZE];
		struct nkey *next;
	} *nkey, *nkeys = NULL;
	char *scratchf;

	setoutdir(ringfile);
	scratchf = tempfile(0);

	/* check if the keyfile to be added is armored */
	if ((is_armored = is_armor_file(keyfile, 0L)) == TRUE)
	{   /* decode it into the actual keyfile */
		boolean changed_name;
		strcpy(trans_keyfile,keyfile);
		keyfile = tempfile(TMP_TMPDIR|TMP_WIPE);
		if (de_armor_file(trans_keyfile,keyfile,NULL) < 0)
			return(-1);
	}

	userid[0] = '\0';
	if (dokeycheck((char *) userid, keyfile, NULL) < 0)
	{	fprintf(pgpout, PSTR("\007Keyring check error. ") );
		fprintf(pgpout, PSTR("\nKey(s) will not be added to keyring.\n"));
		goto err;
	}

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(keyfile,FOPRBIN)) == NULL)
	{	fprintf(pgpout,PSTR("\n\007Can't open key file '%s'\n"),keyfile);
		goto err;
	}
	if (!file_exists(ringfile))
	{	/* ringfile does not exist.  Can it be created? */
		/* open file g for writing, in binary (not text) mode...*/
		g = fopen(ringfile,FOPWBIN);
		if (g==NULL)
		{	fprintf(pgpout,PSTR("\nKey ring file '%s' cannot be created.\n"),ringfile);
			fclose(f);
			goto err;
		}
		fclose(g);
	}

	/* Create working output file */
	/* open file g for writing, in binary (not text) mode...*/
	if ((g = fopen(scratchf,FOPWBIN)) == NULL)
	{	fclose(f);
		goto err;
	}
	newkeys = newsigs = newids = newrvks = 0;

	/* Pass 1 - copy all keys from f which aren't in ring file */
	/* Also copy userid and signature packets. */
	copying = FALSE;
	for ( ; ; )
	{	file_position = ftell(f);

		status = readkeypacket(f,FALSE,&ctb,timestamp,(char *)userid,n,e,
				NULL,NULL,NULL,NULL,NULL,NULL);
		/* Note that readkeypacket has called set_precision */
		if (status == -1)	/* EOF */
			break;
		if (status == -3)
		{	fprintf(pgpout,PSTR("\n\007Could not read key from file '%s'.\n"),
				keyfile);
			fclose(f);	/* close key file */
			fclose(g);
			goto err;
		}
		if (status < 0)
		{
			copying = FALSE;
			continue;	/* don't merge keys from unrecognized version */
		}

		/* Check to see if key is already on key ring */
		if (is_key_ctb(ctb))
		{
			extract_keyID(keyID, n);	/* from keyfile, not ringfile */
	
			/*	Check for duplicate key in key ring: */
			if (getpublickey(TRUE, FALSE, ringfile, &fp, &pktlen, keyID, timestamp, userid, n1, e) >= 0)
			{	if (mp_compare (n, n1) != 0)
				{	fprintf(pgpout, PSTR("\n\007Warning: Key ID %s matches key ID of key already on \n\
key ring '%s', but the keys themselves differ.\n\
This is highly suspicious.  This key will not be added to ring.\n\
Acknowledge by pressing return: "), keyIDstring(keyID), ringfile);
					getyesno('n');
					fclose(f);	/* close key file */
					fclose(g);
					goto err;
				}
				else
				{	if (verbose)
						fprintf(pgpout,PSTR("Key ID %s is already included in key ring '%s'.\n"),
							keyIDstring(keyID), ringfile);
					++commonkeys;
				}
				copying = FALSE;
			}
			else
			{
				++newkeys;
#if 0
				if (query)
				{	if (is_ctb_type(ctb, CTB_CERT_SECKEY_TYPE))
						fprintf (pgpout, PSTR ("\nAdd secret key to key ring '%s' (y/N)? "),
								ringfile);
					else
						fprintf (pgpout, PSTR ("\nAdd public key to key ring '%s' (y/N)? "),
								ringfile);
						
					if (! getyesno( 'n' ))
					{	fclose(f);
						fclose(g);
						rmtemp(scratchf);
						return(0);	/* user chose to abort */
					}
					query = FALSE;		/* Don't ask subsequently */
				}
#endif

				if ((nkey = (struct nkey *) malloc(sizeof(struct nkey))) == NULL)
				{
					fprintf(stderr, PSTR("\n\007Out of memory.\n"));
					exitPGP(7);
				}
				memcpy(nkey->keyID, keyID, KEYFRAGSIZE);
				nkey->next = nkeys;
				nkeys = nkey;
	
				fprintf(pgpout, PSTR("New key ID: %s\n"), keyIDstring(keyID));
				copying = TRUE;
			}
		}
		/* Now, we copy according to the copying flag */
		/*	The key is prepended to the ring to give it search precedence
			over other keys with that same userid. */
	
		if (copying  &&  (is_key_ctb(ctb) || ctb==CTB_USERID ||
								is_ctb_type(ctb,CTB_SKE_TYPE)))
		{	pktlen = (int) (ftell(f) - file_position);
			copyfilepos(f,g,pktlen,file_position);	/* copy packet from f */
			/* Initialize trust packets after keys and signatures */
			if (is_key_ctb(ctb))
			{
				write_trust (g, KC_OWNERTRUST_UNDEFINED);
				userid_seen = FALSE;
			}
			else if (is_ctb_type(ctb,CTB_SKE_TYPE))
			{
				if (userid_seen)
					write_trust (g, KC_SIGTRUST_UNDEFINED);
				else
				/* signature certificate before userid must be compromise cert. */
					fprintf(pgpout, PSTR("Key has been revoked.\n"));
			}
			else if (is_ctb_type(ctb,CTB_USERID_TYPE))
			{
				write_trust (g, KC_LEGIT_UNKNOWN);
				userid_seen = TRUE;
			}
		}
	}

	/* Now copy the remainder of the ringfile, h, to g.  commonkeys tells
		how many keys are common to keyfile and ringfile.  As long as that is
		nonzero we will check each key in ringfile to see if it has a match
		in keyfile.
	*/
	if ((h = fopen(ringfile,FOPRBIN)) != NULL)
	{	while (commonkeys)		/* Loop for each key in ringfile */
		{	file_position = ftell(h);
			status = readkeypacket(h,FALSE,&ctb,NULL,(char *)userid,n,e,
						NULL,NULL,NULL,NULL,NULL,NULL);
			if (status == -1 || status == -3)
			{	if (status == -1)	/* hit EOF */
					fprintf(pgpout, PSTR("\n\007Key file contains duplicate keys: cannot be added to keyring\n"));
				else
					fprintf(pgpout,PSTR("\n\007Could not read key from file '%s'.\n"),
						ringfile);
				fclose(f);
				fclose(g);
				fclose(h);
				goto err;
			}
			PascalToC ((char *) userid);
			pktlen = ftell(h) - file_position;
			if (is_key_ctb(ctb))
			{	long	tfp;
				int		tpktlen;
				/* See if there is a match in keyfile */
				extract_keyID(keyID, n);	/* from ringfile, not keyfile */
				/* unknow version or bad data: copy (don't remove packets from ringfile) */
				if (status == 0 && (getpublickey(TRUE, FALSE, keyfile, &tfp, &tpktlen,
					 keyID, timestamp, userid1, n1, e) >= 0)  &&
						(mp_compare(n, n1) == 0))
				{	
					if (verbose)
						fprintf (pgpout, PSTR("Merging key ID: %s\n"),keyIDstring(keyID));
					if (mergekeys (f,keyfile,tfp, h,ringfile,&file_position, g) < 0)
					{	fclose(f);
						fclose(g);
						fclose(h);
						goto err;
					}
					copying = FALSE;
					--commonkeys;
				}
				else
					copying = TRUE;
			}
			if (copying)
			{	/* Copy ringfile key to g, without its sigs */
				copyfilepos (h,g,pktlen,file_position);
				file_position += pktlen;
			}
		}	/* End of loop for each key in ringfile */
		copyfile(h,g,-1L);	/* copy rest of file from file h to g */
		fclose(h);
	}
	fclose(f);
	if (write_error(g))
	{	fclose(g);
		goto err;
	}
	fclose(g);
	if (newsigs == 0 && newkeys == 0 && newids == 0 && newrvks == 0)
	{
		fprintf(pgpout, PSTR("No new keys or signatures in keyfile.\n"));
		rmtemp(scratchf);
		return(0);
	}

	for (nkey = nkeys; nkey; nkey = nkey->next)
	{
		if (dokeycheck(NULL, scratchf, nkey->keyID))
			goto err;
	}
	fprintf(pgpout, PSTR("\nKeyfile contains:\n"));
	if (newkeys)
		fprintf(pgpout, PSTR("%4d new key(s)\n"), newkeys);
	if (newsigs)
		fprintf(pgpout, PSTR("%4d new signatures(s)\n"), newsigs);
	if (newids)
		fprintf(pgpout, PSTR("%4d new user ID(s)\n"), newids);
	if (newrvks)
		fprintf(pgpout, PSTR("%4d new revocation(s)\n"), newrvks);
	if (query)
	{
		fprintf(pgpout, PSTR("\nDo you want to add this keyfile to keyring '%s' (y/N)? "), ringfile);
		if (!getyesno('n'))
		{	rmtemp(scratchf);
			return(1);
		}
	}
	if ((status = maintenance(scratchf, MAINT_SILENT)) == 0)
		for (nkey = nkeys; nkey; nkey = nkey->next)
		{	fprintf(pgpout,PSTR("Adding key ID %s from file '%s' to key ring '%s'.\n"),
				keyIDstring(nkey->keyID),(is_armored?trans_keyfile:keyfile),ringfile);
			ask_to_sign(nkey->keyID, scratchf);
		}

	savetempbak(scratchf,ringfile);

	if (is_armored)
		rmtemp(keyfile);	/* zap decoded keyfile */
	return(0);	/* normal return */

err:
	/* make sure we remove any garbage files we may have created */
	if (is_armored)
		rmtemp(keyfile);	/* zap decoded keyfile */
	rmtemp(scratchf);
	return(-1);
}	/* addto_keyring */


static int ask_to_sign(byte *keyID, char *ringfile)
{
	FILE *f;
	word32 timestamp;
	byte ctb, trust;
	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
	byte userid[256];
	long fpos;
	int pktlen;	/* unused, just to satisfy getpublickey */
	int status;
	extern char my_name[];

	if (getpublickey(TRUE, FALSE, ringfile, &fpos, &pktlen, 
			keyID, (byte *)&timestamp, userid, n, e) < 0)
		return(-1);

	if ((f = fopen(ringfile, FOPRBIN)) == NULL)
		return(-1);

	fseek(f, fpos, SEEK_SET);
	if (is_compromised(f))
	{	fclose(f);
		return(0);
	}
	if (nextkeypacket(f, &ctb) < 0)
	{	fclose(f);
		return(-1);
	}
	if (ctb != CTB_CERT_PUBKEY)
	{	fclose(f);
		return(0);	/* don't ask to sign secret key */
	}
	while (nextkeypacket(f, &ctb) == 0 && !is_key_ctb(ctb))
		if (ctb == CTB_USERID)	/* check first userid */
			break;
	if (ctb != CTB_USERID)
	{
		fclose(f);
		return(-1);
	}

	if ((status = read_trust(f, &trust)) < 0)
	{
		fclose(f);
		return(status);
	}
	if ((trust & KC_LEGIT_MASK) == KC_LEGIT_COMPLETE)
	{
		fclose(f);
		return(0);
	}
	show_key(f, fpos, SHOW_ALL);
	fclose(f);
	PascalToC((char *)userid);
	fprintf(pgpout, PSTR("\nDo you want to certify this key yourself (y/N)? "));
	if (getyesno('n'))
	{
		if (signkey((char *)userid, my_name, ringfile) == 0)
			maintenance(ringfile, MAINT_SILENT);
	}
	return(0);
}
