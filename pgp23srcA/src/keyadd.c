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
#include "crypto.h"
#include "fileio.h"
#include "keymgmt.h"
#include "charset.h"
#include "language.h"
#include "pgp.h"
#include "exitpgp.h"
#include "keyadd.h"
#include "keymaint.h"

void gpk_close(void);
int gpk_open(char *keyfile);
int get_publickey(long *file_position, int *pktlen, byte *keyID, byte *timestamp, 
	byte *userid, unitptr n, unitptr e);

static int ask_to_sign(byte *keyID, char *ringfile);
static boolean ask_first;

static boolean publickey;	/* if TRUE, add trust packets */

static int newkeys, newsigs, newids, newrvks;
static byte mykeyID[KEYFRAGSIZE];

static int mergesigs (FILE *fkey, char *keyfile, long keypos, FILE *fring,
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
			{	char *signator;
				if ((signator = user_from_keyID(keyID)) == NULL)
					fprintf(pgpout, PSTR("New signature from keyID %s on userid \"%s\"\n"), 
						keyIDstring(keyID), LOCAL_CHARSET(userid));
				else
				{
					fprintf(pgpout, PSTR("New signature from %s\n"), LOCAL_CHARSET(signator));
					fprintf(pgpout, PSTR("on userid \"%s\"\n"), LOCAL_CHARSET(userid));
				}
				++newsigs;
				if (batchmode)
					show_update(keyIDstring(mykeyID));
			}
		}
		if (copying  &&  is_ctb_type(ctb,CTB_SKE_TYPE))
		{	copyfilepos (fkey, out, keypktlen, keypos);
			if (publickey)
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


static int mergekeys (FILE *fkey, char *keyfile, long keypos, FILE *fring,
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
			PascalToC((char *)userid);
			fprintf(pgpout, PSTR("Key revocation certificate from \"%s\".\n"),
					LOCAL_CHARSET((char *)userid));
			copyfilepos (fkey, out, cert_pktlen, keypos);
			/* Show updates */
			if (batchmode)
				show_key(fring, *pringpos, SHOW_CHANGE);
			++newrvks;
		}
		else
			fprintf(pgpout, PSTR("\n\007WARNING:  File '%s' contains bad revocation certificate.\n"), keyfile);
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
			{	putc('\n', pgpout);
				fprintf (pgpout, PSTR("New userid: \"%s\".\n"), 
					LOCAL_CHARSET((char *)userid));
				fprintf(pgpout, PSTR("\nWill be added to the following key:\n"));
				show_key(fring, *pringpos, 0);
				fprintf(pgpout, PSTR("\nAdd this userid (y/N)? "));
				if (batchmode || getyesno('n')) {
					++newids;
					/* Show an update string */
					if (batchmode) {
						fprintf(pgpout, "\n");
						show_key(fring, *pringpos, SHOW_CHANGE);
					}
				}
				else
					copying = FALSE;
			}
		}
		if (copying)
		{	if (ctb==CTB_USERID  ||  is_ctb_type(ctb,CTB_SKE_TYPE))
			{	copyfilepos (fkey, out, keypktlen, keypos);
				if (publickey) {
					if (is_ctb_type(ctb,CTB_SKE_TYPE))
						write_trust (out, KC_SIGTRUST_UNDEFINED);
					else
						write_trust (out, KC_LEGIT_UNKNOWN);
				}
			}
		}
	}

	/* Third, for all ring userid's, if not in keyfile, copy the userid
	 * plus its dependant signatures.
	 */
	fseek (fring, ringpos, SEEK_SET);
	/* Grab the keyID here */
	readkeypacket(fring,FALSE,&ctb,NULL,(char *)userid,NULL,NULL,
		      NULL,NULL,NULL,NULL,NULL,NULL);
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


int _addto_keyring(char *keyfile, char *ringfile)
/*	Adds (prepends) key file to key ring file. */
{	FILE *f, *g, *h;
	long file_position,fp;
	int pktlen;
	byte ctb;
	int status;
	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
	unit n1[MAX_UNIT_PRECISION];
	byte keyID[KEYFRAGSIZE];
	byte userid[256];		/* key certificate userid */
	byte userid1[256];
	word32 tstamp; byte *timestamp = (byte *) &tstamp;		/* key certificate timestamp */
	boolean userid_seen = FALSE;
	int commonkeys = 0;
	int copying;
	struct nkey {
		byte keyID[KEYFRAGSIZE];
		struct nkey *next;
	} *nkey, *nkeys = NULL;
	char *scratchf;

	/* open file f for read, in binary (not text) mode...*/
	if ((f = fopen(keyfile,FOPRBIN)) == NULL)
	{	fprintf(pgpout,PSTR("\n\007Can't open key file '%s'\n"),keyfile);
		return -1;
	}
	ctb = 0;
	if (fread(&ctb, 1, 1, f) != 1 || !is_key_ctb(ctb))
	{
		fclose(f);
		return -1;
	}
	rewind(f);

	setoutdir(ringfile);
	scratchf = tempfile(0);

	/*
	 * get userids from both files, maybe should also use the default public
	 * keyring if ringfile is not the default ring.
	 */
	setkrent(ringfile);
	setkrent(keyfile);
	init_userhash();

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
	fprintf(pgpout, PSTR("\nLooking for new keys...\n"));
	copying = FALSE;
	if (gpk_open(ringfile) < 0)
	{	fclose(f);	/* close key file */
		fclose(g);
		goto err;
	}
	for ( ; ; )
	{	file_position = ftell(f);

		status = readkeypacket(f,FALSE,&ctb,timestamp,(char *)userid,n,e,
				NULL,NULL,NULL,NULL,NULL,NULL);
		/* Note that readkeypacket has called set_precision */
		if (status == -1)	/* EOF */
			break;
		if (status == -2 || status == -3)
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
			publickey = is_ctb_type(ctb, CTB_CERT_PUBKEY_TYPE);
	
			/*	Check for duplicate key in key ring: */
			status = get_publickey(&fp, NULL, keyID, timestamp, userid, n1, e);
			if (status == 0)	/* key in both keyring and keyfile */
			{	if (mp_compare (n, n1) != 0)
				{	fprintf(pgpout, PSTR("\n\007Warning: Key ID %s matches key ID of key already on \n\
key ring '%s', but the keys themselves differ.\n\
This is highly suspicious.  This key will not be added to ring.\n\
Acknowledge by pressing return: "), keyIDstring(keyID), ringfile);
					getyesno('n');
				}
				else
					++commonkeys;
				copying = FALSE;
			}
			else if (status == -1)	/* key NOT in keyring */
			{
				++newkeys;
				if (interactive_add)
				{	show_key(f, file_position, SHOW_ALL);
					fprintf(pgpout, PSTR("\nDo you want to add this key to keyring '%s' (y/N)? "), ringfile);
					copying = getyesno('n');
				}
				else
				{
					show_key(f, file_position, SHOW_LISTFMT);
					copying = TRUE;
				}

				/* If batchmode, output an update message */
				if (batchmode)
					show_key(f, file_position, SHOW_CHANGE);
				if (copying)
				{
					nkey = xmalloc(sizeof(struct nkey));
					memcpy(nkey->keyID, keyID, KEYFRAGSIZE);
					nkey->next = nkeys;
					nkeys = nkey;
				}
			}
			else	/* unknown version or bad key */
				copying = FALSE;
		}
		/* Now, we copy according to the copying flag */
		/*	The key is prepended to the ring to give it search precedence
			over other keys with that same userid. */
	
		if (copying  &&  (is_key_ctb(ctb) || ctb==CTB_USERID ||
								is_ctb_type(ctb,CTB_SKE_TYPE)))
		{	pktlen = (int) (ftell(f) - file_position);
			copyfilepos(f,g,pktlen,file_position);	/* copy packet from f */
			if (publickey)
			{	/* Initialize trust packets after keys and signatures */
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
	}
	gpk_close();

	/* Now copy the remainder of the ringfile, h, to g.  commonkeys tells
		how many keys are common to keyfile and ringfile.  As long as that is
		nonzero we will check each key in ringfile to see if it has a match
		in keyfile.
	*/
	if ((h = fopen(ringfile,FOPRBIN)) != NULL)
	{	
		if (gpk_open(keyfile) < 0)
		{	fclose(f);
			fclose(g);
			fclose(h);
			goto err;
		}
		while (commonkeys)		/* Loop for each key in ringfile */
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
				/* unknown version or bad data: copy (don't remove packets from ringfile) */
				copying = TRUE;
				if (status == 0)
				{	
					/* See if there is a match in keyfile */
					extract_keyID(keyID, n);	/* from ringfile, not keyfile */
					extract_keyID(mykeyID, n); /* save this */
					publickey = is_ctb_type(ctb, CTB_CERT_PUBKEY_TYPE);
					if (get_publickey(&tfp, NULL, keyID, timestamp, userid1, n1, e) >= 0)
					{
						if (verbose)
							fprintf (pgpout, "Merging key ID: %s\n",keyIDstring(keyID));
						if (mergekeys (f,keyfile,tfp, h,ringfile,&file_position, g) < 0)
						{	fclose(f);
							fclose(g);
							fclose(h);
							goto err;
						}
						copying = FALSE;
						--commonkeys;
					}
				}
			}
			if (copying)
			{	/* Copy ringfile key to g, without its sigs */
				copyfilepos (h,g,pktlen,file_position);
				file_position += pktlen;
			}
		}	/* End of loop for each key in ringfile */
		gpk_close();
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
		endkrent();
		return(0);
	}

	if (status = dokeycheck(NULL, scratchf, CHECK_NEW))
	{	if (verbose)
			fprintf(pgpout, "addto_keyring: dokeycheck returned %d\n", status);
		goto err;
	}
	endkrent();

	fprintf(pgpout, PSTR("\nKeyfile contains:\n"));
	if (newkeys)
		fprintf(pgpout, PSTR("%4d new key(s)\n"), newkeys);
	if (newsigs)
		fprintf(pgpout, PSTR("%4d new signatures(s)\n"), newsigs);
	if (newids)
		fprintf(pgpout, PSTR("%4d new user ID(s)\n"), newids);
	if (newrvks)
		fprintf(pgpout, PSTR("%4d new revocation(s)\n"), newrvks);

	ask_first = TRUE;
	if ((status = maint_update(scratchf)) >= 0 && !filter_mode && !batchmode)
		for (nkey = nkeys; nkey; nkey = nkey->next)
			if (ask_to_sign(nkey->keyID, scratchf) != 0)
				break;
	if (status && verbose)
		fprintf(pgpout, "addto_keyring: maint_update returned %d\n", status);

	for (nkey = nkeys; nkey; )
	{	nkey = nkey->next;
		free(nkeys);
		nkeys = nkey;
	}

	savetempbak(scratchf,ringfile);

	return(0);	/* normal return */

err:
	gpk_close();	/* save to call if not opened */
	endkrent();
	/* make sure we remove any garbage files we may have created */
	rmtemp(scratchf);
	return(-1);
}	/* addto_keyring */


int addto_keyring(char *keyfile, char *ringfile)
{
	long armorline = 0;
	char *tempf;
	int addflag = 0;

	if (_addto_keyring(keyfile, ringfile) == 0)
		return 0;
	/* check if the keyfile to be added is armored */
	while (is_armor_file(keyfile,armorline))
	{
		tempf = tempfile(TMP_TMPDIR|TMP_WIPE);
		if (de_armor_file(keyfile,tempf,&armorline))
		{	rmtemp(tempf);
			return -1;
		}
		if (_addto_keyring(tempf, ringfile) == 0)
			addflag = 1;
		rmtemp(tempf);
	}
	if (!addflag)
	{
		fprintf(pgpout, PSTR("\nNo keys found in '%s'.\n"), keyfile);
		return -1;
	}
	else
		return 0;
}


static int ask_to_sign(byte *keyID, char *ringfile)
{
	FILE *f;
	word32 timestamp;
	byte ctb, trust;
	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
	byte userid[256];
	long fpos;
	int status;
	extern char my_name[];

	if (getpublickey(GPK_GIVEUP, ringfile, &fpos, NULL, keyID,
			(byte *)&timestamp, userid, n, e) < 0)
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
	if (ask_first)	/* shortcut for adding big keyfile */
	{	fprintf(pgpout, PSTR("\nOne or more of the new keys are not fully certified.\n\
Do you want to certify any of these keys yourself (y/N)? "));
		if (!getyesno('n'))
		{	
			fclose(f);
			return 1;
		}
	}
	ask_first = FALSE;
	show_key(f, fpos, SHOW_ALL|SHOW_HASH);
	fclose(f);
	PascalToC((char *)userid);
	fprintf(pgpout, PSTR("\nDo you want to certify this key yourself (y/N)? "));
	if (getyesno('n'))
	{
		if (signkey((char *)userid, my_name, ringfile) == 0)
			maint_update(ringfile);
	}
	return(0);
}



/**** faster version of getpublickey() ****/

static long find_keyID(byte *keyID);

static FILE *gpkf = NULL;

/*
 * speedup replacement for getpublickey(), does not have the arguments
 * giveup, showkey and keyfile (giveup = TRUE, showkey = FALSE, keyfile
 * is set with gpk_open().
 * only searches on keyID
 */
int get_publickey(long *file_position, int *pktlen, byte *keyID, byte *timestamp, 
	byte *userid, unitptr n, unitptr e)
{
	byte ctb;	/* returned by readkeypacket */
	int status, keystatus = -1;
	long fpos;

	if ((fpos = find_keyID(keyID)) == -1)
		return -1;
	fseek(gpkf, fpos, SEEK_SET);

	while (TRUE) 
	{
		fpos = ftell(gpkf);
		status = readkeypacket(gpkf,FALSE,&ctb,timestamp,(char *)userid,n,e,
				NULL,NULL,NULL,NULL,NULL,NULL);
		/* Note that readkeypacket has called set_precision */

		if (status < 0 && status != -4 && status != -6)
			return(status);

		/* Remember packet position and size for last key packet */
		if (is_key_ctb(ctb))
		{	if (file_position)
				*file_position = fpos;
			if (pktlen)
				*pktlen = (int)(ftell(gpkf) - fpos);
			if (keystatus != -1)
				return -3;	/* should not happen, probably missing userid pkt */
			keystatus = status;
		}
		if (ctb == CTB_USERID)
			return keystatus;
	}	/* while TRUE */
	return -1;
}

#define	PK_HASHSIZE	256		/* must be power of 2 */
#define	PK_HASH(x)		(*(byte *) (x) & (PK_HASHSIZE - 1))
#define	HASH_ALLOC	400

static VOID * allocbuf(int size);
static void freebufpool(void);

static struct hashent {
	struct hashent *next;
	byte keyID[KEYFRAGSIZE];
	long offset;
} **hashtbl = NULL, *hashptr;

static int hashleft = 0;

int
gpk_open(char *keyfile)
{
	int status;
	long fpos = 0;
	byte keyID[KEYFRAGSIZE];
	byte ctb;

	if (gpkf) {
		fprintf(pgpout, "gpk_open: already open\n");
		return -1;
	}
	default_extension(keyfile,PGP_EXTENSION);
	if ((gpkf = fopen(keyfile,FOPRBIN)) == NULL)
		return(-1);	/* error return */
	hashtbl = allocbuf(PK_HASHSIZE * sizeof(struct hashent *));
	memset(hashtbl, 0, PK_HASHSIZE * sizeof(struct hashent *));
	while ((status = readkpacket(gpkf, &ctb, NULL, keyID, NULL)) != -1) {
		if (status == -2 || status == -3)
		{	fprintf(pgpout,PSTR("\n\007Could not read key from file '%s'.\n"),
				keyfile);
			fclose(gpkf);	/* close key file */
			return -1;
		}
		if (is_key_ctb(ctb)) {
			if (find_keyID(keyID) != -1)
				fprintf(pgpout, "warning: duplicate key in keyring '%s'\n", keyfile);
			if (!hashleft) {
				hashptr = allocbuf(HASH_ALLOC * sizeof(struct hashent));
				hashleft = HASH_ALLOC;
			}
			memcpy(hashptr->keyID, keyID, KEYFRAGSIZE);
			hashptr->offset = fpos;
			hashptr->next = hashtbl[PK_HASH(keyID)];
			hashtbl[PK_HASH(keyID)] = hashptr;
			++hashptr;
			--hashleft;
		}
		fpos = ftell(gpkf);
	}
	return 0;
}

void
gpk_close(void)
{
	if (!gpkf)
		return;
	hashleft = 0;
	hashtbl = NULL;
	freebufpool();
	fclose(gpkf);	/* close key file */
	gpkf = NULL;
}

/*
 * Lookup file position in hash table by keyID, returns -1 if not found
 */
static long
find_keyID(byte *keyID)
{
	struct hashent *p;

	for (p = hashtbl[PK_HASH(keyID)]; p; p = p->next)
		if (memcmp(keyID, p->keyID, KEYFRAGSIZE) == 0)
			return p->offset;
	return -1;
}


static struct bufpool {
	struct bufpool *next;
	char buf[1];		/* variable size */
} *bufpool = NULL;

/*
 * allocate buffer, all buffers allocated with this function can be
 * freed with one call to freebufpool()
 */
static VOID *
allocbuf(int size)
{
	struct bufpool *p;

	p = xmalloc(size + sizeof(struct bufpool *));
	p->next = bufpool;
	bufpool = p;
	return p->buf;
}

/*
 * free all memory obtained with allocbuf()
 */
static void
freebufpool(void)
{
	struct bufpool *p;

	while (bufpool) {
		p = bufpool;
		bufpool = bufpool->next;
		free(p);
	}
}
