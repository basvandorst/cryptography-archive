/*	keymaint.c  - Keyring maintenance pass routines for PGP.
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

	keymaint.c implemented by Branko Lankester.
*/

#include <stdio.h>
#include <time.h>
#include "mpilib.h"
#include "random.h"
#include "crypto.h"
#include "fileio.h"
#include "keymgmt.h"
#include "mpiio.h"
#include "language.h"
#include "pgp.h"


void setup_trust();
int maint_pass1(FILE *f);
int maint_trace_chain(FILE *f);
int maint_cmpfiles(FILE *f, FILE *g);
int trace_sig_chain(FILE *f, byte *fromID, int owner_trust, int depth);
int check_secretkey(FILE *f, long keypos);
int set_legit(FILE *f, long trustpos, int trust_count);
long lookup_by_keyID(FILE *f, byte *srch_keyID);
void show_userid(FILE *f, byte *keyID);
int show_trust(byte, byte);
int show_legit(byte, byte);
void write_trust_pos(FILE *f, byte keyctrl, long pos);

/* returned when trying to do a maintenance pass on a secret keyring or keyfile */
#define	ERR_NOTRUST		-7

#define TRUST_MASK		7	/* mask for userid/signature trust bytes */
#define SET_TRUST(b,v)	(*(b) = (*(b) & ~TRUST_MASK) | (v))
#define TRUST_LEV(b)	((b) & TRUST_MASK)

#define	TRUST_FAC(x)	(trust_tbl[TRUST_LEV(x)])

/*
 * table for tuning user paranoia index.
 * values represent contribution of one signature indexed by the
 * SIGTRUST of a signature
 */
int trust_tbl[8];

static int marginal_min;
static int complete_min;	/* total count needed for a fully legit key */

int marg_min = 2;		/* number of marginally trusted signatures needed for
						   a fully legit key (can be set in config.pgp). */
int compl_min = 1;		/* number of fully trusted signatures needed */

char trust_lst[8][16] = {
	"undefined",		/* PSTR("undefined") */
	"unknown",			/* PSTR("unknown") */
	"untrusted",		/* PSTR("untrusted") */
	"<3>",				/* unused */
	"<4>",				/* unused */
	"marginal",			/* PSTR("marginal") */
	"complete",			/* PSTR("complete") */
	"ultimate",			/* PSTR("ultimate") */
};

char legit_lst[4][16] = {
	"undefined",
	"untrusted",
	"marginal",
	"complete"
};

int trustlst_len = 9;	/* length of longest trust word */
int legitlst_len = 9;	/* length of longest legit word */

char floppyring[MAX_PATH] = "";
int max_cert_depth = 4;		/* maximum nesting of signatures */

static boolean check_only = FALSE;
static boolean mverbose;
static FILE *sec_fp;
static FILE *floppy_fp = NULL;
static int undefined_trust;	/* number of legit keys with undef. trust */

/*
 * do a maintenance pass on keyring "ringfile"
 * options can be:
 *	MAINT_CHECK		check keyring only, "ringfile" will not be changed
 *	MAINT_SILENT	used for implicit maintenance pass, does not print keyring contents
 *	MAINT_VERBOSE	verbose output, shows signature chains
 */
int
maintenance(char *ringfile, int options)
{
	FILE *f, *g;
	int status;
	char *tmpring;
	char secretkeyring[MAX_PATH];
	extern boolean moreflag;

	check_only = (options & MAINT_CHECK) != 0;
	mverbose = (options & MAINT_VERBOSE) != 0;
	undefined_trust = 0;
	if (max_cert_depth > 8)
		max_cert_depth = 8;

	if ((tmpring = tempfile(TMP_TMPDIR)) == NULL)
		return(-1);
	if ((g = fopen(tmpring, FOPWPBIN)) == NULL)
		return(-1);

	if ((f = fopen(ringfile,FOPRBIN)) == NULL)
	{	fprintf(pgpout,PSTR("\n\007Can't open key ring file '%s'\n"),ringfile);
		fclose(g);
		return(-1);
	}

	buildfilename(secretkeyring, SECRET_KEYRING_FILENAME);
	if ((sec_fp = fopen(secretkeyring, FOPRBIN)) == NULL)
		fprintf(pgpout,PSTR("\nCan't open secret key ring file '%s'\n"),
				secretkeyring);

	if (!(options & MAINT_SILENT) && *floppyring != '\0' &&
		(floppy_fp = fopen(floppyring, FOPRBIN)) == NULL)
		fprintf(pgpout,PSTR("\nCan't open backup key ring file '%s'\n"),
				floppyring);

	if ((status = copyfile(f, g, -1L)) < 0)
	{	fprintf(pgpout,PSTR("\n\007Could not read key from file '%s'.\n"), ringfile);
		goto failed;
	}
  
	if (moreflag && !(options & MAINT_SILENT))
		open_more();

	setup_trust();
	if (mverbose)
		fprintf(pgpout, PSTR("\nPass 1: Looking for the \"ultimately-trusted\" keys...\n"));
	rewind(g);
	if ((status = maint_pass1(g)) < 0)
		goto failed;

	if (mverbose)
		fprintf(pgpout, PSTR("\nPass 2: Tracing signature chains...\n"));
	rewind(g);
	if ((status = maint_trace_chain(g)) < 0)
		goto failed;

	if (!(options & MAINT_SILENT))
	{	if (mverbose)
			fprintf(pgpout, PSTR("\nPass 3: Comparing with original keyring...\n"));
		rewind(f);
		rewind(g);
		if ((status = maint_cmpfiles(f, g)) < 0)
			goto failed;
	} else
		status = 1;

	close_more();
	if (write_error(g))
	{	status = -1;
		goto failed;
	}
	fclose(f);
	fclose(g);
	if (sec_fp)
		fclose(sec_fp);
	if (floppy_fp)
	{	fclose(floppy_fp);
		floppy_fp = NULL;
	}

#ifdef DEBUG
	if (check_only)
	{
		if (undefined_trust)
			fprintf(pgpout, "\nKeyring contains %d fully validated key(s) with undefined trust.\n", undefined_trust);
		else
			if (status == 0)
				fprintf(pgpout, "\nMaintenance check OK.\n");
		rmtemp(tmpring);
		return(0);
	}
#endif

	if (status > 0)		/* keyring is changed */
	{
		if (options & MAINT_SILENT)
		{	/* implicit maintenance pass (after pgp -ka, -kr, -ks) */
			remove(ringfile);
			if (savetemp(tmpring, ringfile) == NULL)
				return(-1);
		}
		else
		{	/* explicit maintenance pass */
			fprintf(pgpout, PSTR("Update public keyring '%s' (Y/n)? "), ringfile);
			if (!getyesno('y'))
			{
				rmtemp(tmpring);
				return(0);
			}
			if (savetempbak(tmpring, ringfile))
				return(-1);
		}
	}
	else
		rmtemp(tmpring);

	return(0);

failed:
	close_more();
	if (sec_fp)
		fclose(sec_fp);
	if (floppy_fp)
	{	fclose(floppy_fp);
		floppy_fp = NULL;
	}
	fclose(f);
	fclose(g);
#ifdef DEBUG
	savetempbak(tmpring, "tmpring.pub");
#else
	rmtemp(tmpring);
#endif
	return(status);
}	/* maintenance */


/*
 * check if axiomatic keys are present in the secret keyring and
 * clear userid and signature trust bytes
 */
int
maint_pass1(FILE *f)
{
	int status;
	char userid[256];
	byte keyID[KEYFRAGSIZE];
	byte ctb;
	byte keyctrl;
	boolean buckstop = FALSE, show_user = FALSE;
	boolean compromised = FALSE;
	int buckstopcount = 0;
	int usercount = 0;
	long keypos = 0;

	while ((status = readkpacket(f, &ctb, userid, keyID, NULL)) != -1)
	{
		if (status == -3)
			return(status);
		if (status < 0)
			continue;

		if (is_ctb_type(ctb, CTB_COMMENT_TYPE) || ctb == CTB_KEYCTRL)
			continue;
		if (compromised && is_ctb_type(ctb, CTB_SKE_TYPE) && !usercount)
			continue;	/* compromise certificate */

		/* other packets should have trust byte */
		if (read_trust(f, &keyctrl) < 0)
			return(ERR_NOTRUST);		/* not a public keyring */

		if (is_ctb_type(ctb, CTB_CERT_PUBKEY_TYPE))
		{
			if (compromised = is_compromised(f))
				keyctrl = KC_OWNERTRUST_NEVER;
			if ((keyctrl & KC_BUCKSTOP))
			{
				if (check_secretkey(f, keypos) == 0)
				{
					++buckstopcount;
					buckstop = TRUE;
					if (mverbose)
						fprintf(pgpout, "* %s",keyIDstring(keyID));
				}
				else
				{	/* not in secret keyring */
					keyctrl &= ~KC_BUCKSTOP;
					if (TRUST_LEV(keyctrl) == KC_OWNERTRUST_ULTIMATE)
						keyctrl = KC_OWNERTRUST_ALWAYS;
					if (mverbose)
						fprintf(pgpout, ". %s",keyIDstring(keyID));
				}
				show_user = mverbose;
			}
			else
			{
				buckstop = FALSE;
				show_user = FALSE;
			}
			usercount = 0;
			keyctrl &= ~KC_VISITED;

		}
		else if (ctb == CTB_USERID)
		{
			if (show_user)
			{
				if (usercount)		/* more than one user ID */
					fprintf(pgpout, "        ");
				fprintf(pgpout, "  %s\n", LOCAL_CHARSET(userid));
			}
			keyctrl &= ~KC_LEGIT_MASK;
  			if (buckstop)
  				keyctrl |= KC_LEGIT_COMPLETE;
  			else
  				keyctrl |= KC_LEGIT_UNKNOWN;
			++usercount;
		}
		else if (is_ctb_type(ctb, CTB_SKE_TYPE))
		{
			keyctrl = KC_SIGTRUST_UNDEFINED;
		}
		fseek(f, -1L, SEEK_CUR);
		fwrite(&keyctrl, 1, 1, f);
		fseek(f, 0L, SEEK_CUR);
		keypos = ftell(f);
	}
	if (buckstopcount == 0 && mverbose)
	{
		fprintf(pgpout, PSTR("No ultimately-trusted keys.\n"));
	}
	return(0);
}


/*
 * scan keyring for buckstop keys and start the recursive trace_sig_chain()
 * on them
 */
int
maint_trace_chain(FILE *f)
{
	int status;
	char userid[256];
	byte keyID[KEYFRAGSIZE];
	long trustpos = 0;
	byte ctb;
	byte own_trust;
	boolean buckstop = FALSE;

	while ((status = readkpacket(f, &ctb, userid, keyID, NULL)) != -1)
	{
		if (status == -3)
			return(status);
		if (status < 0)
			continue;

		if (is_ctb_type(ctb, CTB_CERT_PUBKEY_TYPE))
		{
			trustpos = ftell(f);
			if ((status = read_trust(f, &own_trust)) < 0)
				return(status);
			buckstop = (own_trust & KC_BUCKSTOP) != 0;
			userid[0] = '\0';
		}
		else if (ctb == CTB_USERID)
		{
			if (buckstop)
			{
				buckstop = FALSE;	/* only for first user id */
				if (mverbose)
					fprintf(pgpout, "* %s\n", LOCAL_CHARSET(userid));

				if (TRUST_LEV(own_trust) == KC_OWNERTRUST_UNDEFINED)
				{	SET_TRUST(&own_trust, ask_owntrust(userid, own_trust));
					write_trust_pos(f, own_trust, trustpos);
				}
				if (trace_sig_chain(f, keyID, TRUST_LEV(own_trust), 0) < 0)
					return(-1);
			}
		}
	}
	return(0);
}


/* 
 * check if the maintenance pass changed anything
 * returns 0 if files f and g are equal and the number of changed
 * trust bytes if the files are different or a negative value on error
 */
int
maint_cmpfiles(FILE *f, FILE *g)
{
	int status;
	char userid[256];
	byte keyID[KEYFRAGSIZE];
	byte sigkeyID[KEYFRAGSIZE];
	byte ctb;
	byte kc_orig, kc_new;
	int usercount = 0;
	int changed = 0;

	fprintf(pgpout, PSTR("  KeyID   Trust      Validity  User ID\n"));
	while ((status = readkpacket(f, &ctb, userid, keyID, sigkeyID)) != -1)
	{
		if (status == -3)
			break;
		if (status < 0)
			continue;
		if (is_ctb_type(ctb, CTB_COMMENT_TYPE))
			continue;
		 /* unexpected trust packet, probably after version error */
		if (ctb == CTB_KEYCTRL)
			continue;

		/* other packets should have trust byte */
		fseek(g, ftell(f), SEEK_SET);
		if (read_trust(f, &kc_orig) < 0 || read_trust(g, &kc_new) < 0)
		{	status = ERR_NOTRUST;
			break;
		}

		if (is_ctb_type(ctb, CTB_CERT_PUBKEY_TYPE))
		{
			if (is_compromised(f))
			{
				fprintf(pgpout, "# ");
				nextkeypacket(f, &ctb);	/* skip compromise certificate */
				nextkeypacket(g, &ctb);
			}
			else if ((kc_new & KC_BUCKSTOP))
				fprintf(pgpout, "* ");
			else
				fprintf(pgpout, "  ");

			fprintf(pgpout, "%s ",keyIDstring(keyID));
			changed += show_trust(kc_new, kc_orig);
			usercount = 0;
			userid[0] = '\0';
		}
		else if (ctb == CTB_USERID)
		{
  			if (usercount)		/* more than one user ID */
				fprintf(pgpout, "         %*s ", trustlst_len, "");
  			++usercount;
			changed += show_legit(kc_new, kc_orig);
			fprintf(pgpout, "%s\n", LOCAL_CHARSET(userid));
		}
		else if (is_ctb_type(ctb, CTB_SKE_TYPE))
		{
			if (kc_new & KC_CONTIG)
				fprintf(pgpout, "c        ");
			else
				fprintf(pgpout, "         ");
			changed += show_trust(kc_new, kc_orig);
			fprintf(pgpout, "%*s   ", legitlst_len, "");
			show_userid(f, sigkeyID);
		}
	}
	if (status >= -1 && changed)
		fprintf(pgpout, PSTR("\n%d \"trust parameter(s)\" changed.\n"), changed);
	if (status < -1)	/* -1 is OK, EOF */
		return(status);
	return(changed);
}	/* maint_cmpfiles */


/*
 * translate the messages in the arrays trust_lst and legit_lst.
 * trustlst_len and legitlst_len will be set to the length of
 * the longest translated string.
 */
void
init_trust_lst()
{
	static int initialized = 0;
	int i, len;
	char *s;

	if (initialized)
		return;
	for (i = 0; i < 8; ++i)
	{	if (trust_lst[i][0])
		{	s = PSTR (trust_lst[i]);
			if (s != trust_lst[i])
				strncpy(trust_lst[i], s, sizeof(trust_lst[0]) - 1);
			len = strlen(s);
			if (len > trustlst_len)
				trustlst_len = len;
		}
	}
	for (i = 0; i < 4; ++i)
	{	s = PSTR (legit_lst[i]);
		if (s != legit_lst[i])
			strncpy(legit_lst[i], s, sizeof(legit_lst[0]) - 1);
		len = strlen(s);
		if (len > legitlst_len)
			legitlst_len = len;
	}
	++trustlst_len;
	++legitlst_len;
	initialized = 1;
}	/* init_trust_lst */

/*
 * show the trust level of trust byte kc_new.  If it is
 * different from the old level (kc_orig) the print a '!' before
 * the level string
 */
int
show_trust(byte kc_new, byte kc_orig)
{
#ifdef DEBUG
	fprintf(pgpout, " %02x", kc_new);
	if (kc_new != kc_orig)
	{
		fprintf(pgpout, " (%02x)    ", kc_orig);
		return(1);
	}
	fprintf(pgpout, "        ");
	return(0);
#else
	init_trust_lst();
	fprintf(pgpout, "%c%-*s", (kc_new == kc_orig ? ' ' : '!'),
			trustlst_len, trust_lst[TRUST_LEV(kc_new)]);
	return(kc_new != kc_orig);
#endif
}	/* show_trust */

/*
 * show_legit is the same as show_trust, only it uses legit_lst
 * for messages
 */
int
show_legit(byte kc_new, byte kc_orig)
{
#ifdef DEBUG
	return(show_trust(kc_new, kc_orig));
#else
	init_trust_lst();
	kc_new &= KC_LEGIT_MASK;
	kc_orig &= KC_LEGIT_MASK;
	fprintf(pgpout, "%c%-*s", (kc_new == kc_orig ? ' ' : '!'),
			legitlst_len, legit_lst[kc_new]);
	return(kc_new != kc_orig);
#endif
}	/* show_legit */


/*
 * Find all signatures made with a key, fromID is the keyID of this key
 * owner_trust is the trust level of this key.
 * If a trusted signature makes a key fully legit then signatures made
 * with this key are also recursively traced on down the tree.
 *
 * The file f is the public keyring, it must be open for update.
 * depth is the level of recursion, it is used to indent the userIDs
 * and to check if we don't exceed the limit "max_cert_depth"
 *
 * fromID should be the keyID of a fully validated key.
 */
int
trace_sig_chain(FILE *f, byte *fromID, int owner_trust, int depth)
{
	long filepos;
	int status;
	int trust_count = 0;
	byte keyID[KEYFRAGSIZE];
	byte sigkeyID[KEYFRAGSIZE];
	byte ctb;
	byte own_trust, sig_trust;
	char userid[256];
	int sig_count = 0;
	int compromised = 0;
	long usertrustpos = 0;
	long ownt_pos = 0;
	boolean fromsig = FALSE;

#ifdef DEBUG
	if (mverbose)
		fprintf(pgpout, "%*s--- %s\n", 2*depth, "", keyIDstring(fromID));
#endif
	filepos = ftell(f);
	rewind(f);
	userid[0] = '\0';
	while ((status = readkpacket(f, &ctb, userid, keyID, sigkeyID)) != -1)
	{
		if (status == -3)
			return(status);
		if (status < 0)
			continue;

		if (is_ctb_type(ctb, CTB_CERT_PUBKEY_TYPE))
		{
			compromised = is_compromised(f);
			ownt_pos = ftell(f);
			if (read_trust(f, &own_trust))
				return(ERR_NOTRUST);
			userid[0] = '\0';
		}
		else if (ctb == CTB_USERID)
		{
			usertrustpos = ftell(f);
			trust_count = 0;
			fromsig = FALSE;
		}
		else if (is_ctb_type(ctb, CTB_SKE_TYPE))
		{	/* signature packet */
			if (compromised)
				continue;
			read_trust(f, &sig_trust);
			/* we're looking for signatures made by "fromID" */
			if (memcmp(sigkeyID, fromID, sizeof(sigkeyID)) == 0)
			{
				++sig_count;
				if (mverbose)
					fprintf(pgpout, "%*s  > %s\n", 2*depth, "", 
						LOCAL_CHARSET(userid));
				if (TRUST_LEV(sig_trust) != owner_trust)
				{
					SET_TRUST(&sig_trust, owner_trust);
				}
				sig_trust |= KC_CONTIG;
				fseek(f, -1L, SEEK_CUR);
				fwrite(&sig_trust, 1, 1, f);
				fseek(f, 0L, SEEK_CUR);
				fromsig = TRUE;
			}

			if (sig_trust & KC_CONTIG)
			{
				trust_count += TRUST_FAC(sig_trust);

				if (fromsig)	/* fromID has signed this key */
				{
					switch (set_legit(f, usertrustpos, trust_count))
					{
					case -1: return(-1);
					case  1:	/* this signature made the key fully valid */
						if ((own_trust & (KC_BUCKSTOP|KC_VISITED)) == 0)
						{
							if (TRUST_LEV(own_trust) == KC_OWNERTRUST_UNDEFINED)
								SET_TRUST(&own_trust, ask_owntrust(userid, own_trust));
							if (depth < max_cert_depth)
								own_trust |= KC_VISITED;
							write_trust_pos(f, own_trust, ownt_pos);
							if (depth < max_cert_depth)
								trace_sig_chain(f, keyID, TRUST_LEV(own_trust), depth+1);
						}
					}
				}
			}
		}
	}
#ifdef DEBUG
	if (mverbose && sig_count == 0)	/* no signatures from this user */
		fprintf(pgpout, PSTR("%*s  (No signatures)\n"), 2*depth, "");
	if (mverbose)
		fprintf(pgpout, "%*s--- %s\n", 2*depth, "", keyIDstring(fromID));
#endif
	fseek(f, filepos, SEEK_SET);
	return(0);
}	/* trace_sig_chain */


/*
 * compare the key in file f at keypos with the matching key in the
 * secret keyring, the global variable sec_fp must contain the file pointer
 * for of the secret keyring.
 *
 * returns 1 if the key was not found, -2 if the keys were different
 * and 0 if the keys compared OK
 */
int
check_secretkey(FILE *f, long keypos)
{
	int status = -1;
	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
	unit nsec[MAX_UNIT_PRECISION], esec[MAX_UNIT_PRECISION];
	char userid[256];
	byte keyID[KEYFRAGSIZE];
	long savepos, pktlen;
	byte ctb;

	if (sec_fp == NULL)
		return(-1);

	savepos = ftell(f);
	fseek(f, keypos, SEEK_SET);
	if (readkeypacket(f, FALSE, &ctb, NULL, NULL, n, e,
					NULL, NULL, NULL, NULL, NULL, NULL) < 0)
		goto ex;
	extract_keyID(keyID, n);

	do	/* get userid */
	{	status = readkpacket(f, &ctb, userid, NULL, NULL);
		if (status == -1 || status == -3)
			goto ex;
	} while (ctb != CTB_USERID);

	if (lookup_by_keyID(sec_fp, keyID) < 0)
	{
#if 0
		if (!check_only)
		{
			fprintf(pgpout, PSTR (
"\nAn \"axiomatic\" key is one which does not need certifying by\n\
anyone else.  Usually this special status is reserved only for your\n\
own keys, which should also appear on your secret keyring.  The owner\n\
of an axiomatic key (who is typically yourself) is \"ultimately trusted\"\n\
by you to certify any or all other keys.\n"));
			fprintf(pgpout, PSTR ("\nKey for user ID: \"%s\"\n\
is designated as an \"ultimately-trusted\" introducer, but the key\n\
does not appear in the secret keyring.\n\
Use this key as an ultimately-trusted introducer (y/N)? "), 
				LOCAL_CHARSET(userid));
			status = (getyesno('n') ? 0 : 1);
		}
#else
		status = 1;
#endif
	}
	else
	{
		long kpos = ftell(sec_fp);
		if (readkeypacket(sec_fp, FALSE, &ctb, NULL, NULL, nsec, esec,
						NULL, NULL, NULL, NULL, NULL, NULL) < 0)
		{
			fprintf(pgpout, PSTR("\n\007Cannot read from secret keyring.\n"));
			status = -3;
			goto ex;
		}
		if (mp_compare(n, nsec) || mp_compare(e, esec))
		{	/* Red Alert! */
			fprintf(pgpout, PSTR("\n\007WARNING: Public key for user ID: \"%s\"\n\
does not match the corresponding key in the secret keyring.\n"), 
				LOCAL_CHARSET(userid));
			fprintf(pgpout, PSTR("This is a serious condition, indicating possible keyring tampering.\n"));
			status = -2;
		}
		else
			status = 0;

		if (floppy_fp)
		{
			if (lookup_by_keyID(floppy_fp, keyID) < 0)
			{
				fprintf(pgpout, PSTR("Public key for: \"%s\"\n\
is not present in the backup keyring '%s'.\n"),
					LOCAL_CHARSET(userid), floppyring);
			}
			else
			{
				pktlen = ftell(sec_fp) - kpos;
				fseek(sec_fp, kpos, SEEK_SET);
				while (--pktlen >= 0 && getc(sec_fp) == getc(floppy_fp)) ;
				if (pktlen != -1)
				{
					fprintf(pgpout, PSTR("\n\007WARNING: Secret key for: \"%s\"\n\
does not match the key in the backup keyring '%s'.\n"),
						LOCAL_CHARSET(userid), floppyring);
					fprintf(pgpout, PSTR("This is a serious condition, indicating possible keyring tampering.\n"));
					status = -2;
				}
			}
		}
	}
ex:
	fseek(f, savepos, SEEK_SET);
	return(status);
}	/* check_secretkey */


/*
 * setup tables for trust scoring.
 */
void
setup_trust()
{	/* initialize trust table */
	if (marg_min == 0)	/* marginally trusted signatures are ignored */
	{
		trust_tbl[5] = 0;
		trust_tbl[6] = 1;
		complete_min = compl_min;
	}
	else
	{
		if (marg_min < compl_min)
			marg_min = compl_min;
		trust_tbl[5] = compl_min;
		trust_tbl[6] = marg_min;
		complete_min = compl_min * marg_min;
	}
	trust_tbl[7] = complete_min;	/* ultimate trust */
	marginal_min = complete_min / 2;
}	/* setup_trust */


/*
 * set valid level of a userid, computed from trust_count.
 * returns 1 if the key/userid pair is fully legit.
 */
int
set_legit(FILE *f, long trustpos, int trust_count)
{
	long filepos;
	byte keyctrl;

	filepos = ftell(f);
	fseek(f, trustpos, SEEK_SET);	/* user id trust byte */
	if (read_trust(f, &keyctrl) < 0)
		return(-1);
	
	keyctrl &= ~KC_LEGIT_MASK;
	if (trust_count < marginal_min)
		keyctrl |= KC_LEGIT_UNTRUSTED;
	else if (trust_count < complete_min)
		keyctrl |= KC_LEGIT_MARGINAL;
	else
		keyctrl |= KC_LEGIT_COMPLETE;
	
	fseek(f, trustpos, SEEK_SET);
	write_trust(f, keyctrl);

	fseek(f, filepos, SEEK_SET);

	if ((keyctrl & KC_LEGIT_MASK) == KC_LEGIT_COMPLETE)
		return(1);
	else
		return(0);
}	/* set_legit */


int ask_owntrust(char *userid, byte cur_trust)
/* Ask for a wetware decision from the human on how much to trust 
this key's owner to certify other keys.  Returns trust value. */
{
	char buf[8];

	if (check_only)
	{	++undefined_trust;
		return(KC_OWNERTRUST_UNDEFINED);
	}

	fprintf(pgpout, 
PSTR("\nMake a determination in your own mind whether this key actually\n\
belongs to the person whom you think it belongs to, based on available\n\
evidence.  If you think it does, then based on your estimate of\n\
that person's integrity and competence in key management, answer\n\
the following question:\n"));  
	fprintf(pgpout, PSTR("\nWould you trust \"%s\"\n\
to act as an introducer and certify other people's public keys to you?\n\
(1=I don't know. 2=No. 3=Usually. 4=Yes, always.) ? "), 
		LOCAL_CHARSET(userid));
	fflush(pgpout);
	getstring(buf, sizeof(buf)-1, TRUE);
	switch (buf[0])
	{	case '1': return KC_OWNERTRUST_UNKNOWN;
		case '2': return KC_OWNERTRUST_NEVER;
		case '3': return KC_OWNERTRUST_USUALLY;
		case '4': return KC_OWNERTRUST_ALWAYS;
		default: return(TRUST_LEV(cur_trust));
	}
}	/* ask_owntrust */


/* 
 * compare all key packets in keyring file f with file g
 * XXX: untested
 */
int
keyring_cmp(FILE *f, FILE *g)
{
	int status, error = 0, badkeys = 0;
	byte ctb;
	byte keyID[KEYFRAGSIZE];
	char userid[256];
	long pktlen, keypos = 0;

	rewind(f);
	while ((status = readkpacket(f, &ctb, userid, keyID, NULL)) != -1)
	{
		if (status == -3)
			return(status);
		if (status < 0)
			continue;
		if (is_key_ctb(ctb))
		{
			pktlen = ftell(f) - keypos;
			fseek(f, keypos, SEEK_SET);
			if (lookup_by_keyID(g, keyID) < 0)
			{
				error = 1;
				continue;
			}
			while (--pktlen >= 0 && getc(f) == getc(g)) ;
			if (pktlen != -1)
			{
				error = 2;
				++badkeys;
			}

		}
		if (error && ctb == CTB_USERID)
		{
			switch (error) {
				case 1: fprintf(pgpout, PSTR("\n\007Key for user ID \"%s\"\n\
is not present in backup keyring\n"), LOCAL_CHARSET(userid));
				case 2: fprintf(pgpout, PSTR("\n\007Key for user ID \"%s\"\n\
does not match key in backup keyring\n"), LOCAL_CHARSET(userid));
			}
			error = 0;
		}
		keypos = ftell(f);
	}
	if (error)
		status = -5;
	return(status < 0 ? status : badkeys);
}	/* keyring_cmp */


/*
 * stripped down version of readkeypacket(), the output userid
 * is a null terminated string.
 */
int
readkpacket(FILE *f, byte *ctb, char *userid, byte *keyID, byte *sigkeyID)
{
	int status;
	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];

	status = readkeypacket(f, FALSE, ctb, NULL, userid, n, e,
					NULL, NULL, NULL, NULL, sigkeyID, NULL);

	if (status < 0)
	{
#ifdef DEBUG
		if (status < -1)
			fprintf(stderr, "readkeypacket returned %d\n", status);
#endif
		return(status);
	}

	if (keyID && is_key_ctb(*ctb))
		extract_keyID(keyID, n);

	if (userid && *ctb == CTB_USERID)
		PascalToC(userid);

	return(0);
}	/* readkpacket */


/*
 * scan keyfile f for keyID srch_keyID.
 * returns the file position of the key if it is found, and sets the
 * file pointer to the start of the key packet.
 * returns -1 if the key was not found or < -1 if there was an error
 */
long
lookup_by_keyID(FILE *f, byte *srch_keyID)
{
	int status;
	long keypos = 0;
	byte keyID[KEYFRAGSIZE];
	byte ctb;

	rewind(f);
	while ((status = readkpacket(f, &ctb, NULL, keyID, NULL)) != -1)
	{
		if (status == -3)
			break;
		if (status < 0)
			continue;
		if (is_key_ctb(ctb) && memcmp(keyID, srch_keyID, KEYFRAGSIZE) == 0)
		{
			fseek(f, keypos, SEEK_SET);
			return(keypos);
		}
		keypos = ftell(f);
	}
	return(status);
}	/* lookup_by_keyID */


/*
 * look up the key matching "keyID" and print the first userID
 * of this key.  File position of f is saved.
 */
void
show_userid(FILE *f, byte *keyID)
{
	int status;
	long filepos;
	char userid[256];
	byte ctb;

	filepos = ftell(f);
	if (lookup_by_keyID(f, keyID) >= 0)
		while ((status = readkpacket(f, &ctb, userid, NULL, NULL)) != -1 && status != -3)
			if (ctb == CTB_USERID)
			{
				fprintf(pgpout, "%s\n", LOCAL_CHARSET(userid));
				fseek(f, filepos, SEEK_SET);
				return;
			}

	fprintf(pgpout, "(KeyID: %s)\n",keyIDstring(keyID));
	fseek(f, filepos, SEEK_SET);
}	/* show_userid */

/*
 * show the key in file f at file position keypos.
 * 'what' controls the info that will be shown:
 *   SHOW_TRUST: show trust byte info
 *   SHOW_SIGS:  show signatures
 * these constants can be or'ed to get both
 */
int
show_key(FILE *f, long keypos, int what)
{
	int status;
	long filepos;
	char userid[256];
	unit n[MAX_UNIT_PRECISION], e[MAX_UNIT_PRECISION];
	byte sigkeyID[KEYFRAGSIZE];
	word32 timestamp;
	byte ctb, keyctrl;
	int userids = 0;
	boolean print_trust = FALSE;
	int precision = global_precision;

	filepos = ftell(f);
	fseek(f, keypos, SEEK_SET);
	while ((status = readkeypacket(f, FALSE, &ctb, (byte *)&timestamp, userid,
					n, e, NULL, NULL, NULL, NULL, sigkeyID, &keyctrl)) == 0)
	{
		if (is_key_ctb(ctb))
		{	if (userids)
				break;
		}
		else if (ctb == CTB_KEYCTRL)  /* trust bytes only in public keyrings */
		{
			if (what & SHOW_TRUST)
				print_trust = TRUE;
		}
		else if (ctb == CTB_USERID)
		{	if (userids == 0)
			{	PascalToC(userid);	/* for display */
				fprintf(pgpout,PSTR("\nKey for user ID: %s\n"),
					LOCAL_CHARSET(userid));
				fprintf(pgpout,PSTR("%d-bit key, Key ID %s, created %s\n"),
					countbits(n), key2IDstring(n), cdate(&timestamp) );
				if (print_trust)
				{
					switch (TRUST_LEV(keyctrl))
					{
						case KC_OWNERTRUST_UNDEFINED:
						case KC_OWNERTRUST_UNKNOWN:
							/* Just don't say anything in this case */
							break;
						case KC_OWNERTRUST_NEVER: 	/* untrusted */
							fprintf(pgpout, PSTR("This user is untrusted to certify other keys.\n"));
							break;
						case KC_OWNERTRUST_USUALLY: 	/* marginal trust */
							fprintf(pgpout, PSTR("This user is generally trusted to certify other keys.\n"));
							break;
						case KC_OWNERTRUST_ALWAYS: 	/* complete trust */
							fprintf(pgpout, PSTR("This user is completely trusted to certify other keys.\n"));
							break;
						case KC_OWNERTRUST_ULTIMATE: 	/* axiomatic, ultimate trust */
							fprintf(pgpout, PSTR("This axiomatic key is ultimately trusted to certify other keys.\n"));
							break;
						default:
							break;
					}	/* switch */
				}
				++userids;
			}
			else
			{	PascalToC(userid);
				if (what != 0)
					fprintf(pgpout, "\n");
				fprintf(pgpout,PSTR("Also known as: %s\n"), 
					LOCAL_CHARSET(userid));
			}
			if (print_trust)
			{
				read_trust(f, &keyctrl);
				switch (keyctrl & KC_LEGIT_MASK)
				{
					case 0: 	/* undefined certification level */
					case 1: 	/* uncertified */
						fprintf(pgpout, PSTR("This key/userID association is not certified.\n"));
						break;
					case 2: 	/* marginal certification */
						fprintf(pgpout, PSTR("This key/userID association is marginally certified.\n"));
						break;
					case 3: 	/* complete certification */
						fprintf(pgpout, PSTR("This key/userID association is fully certified.\n"));
						break;
					default:	/* can't get here, right? */
						break;
				}	/* switch */
			}	/* print_trust */
		}
		else if ((what & SHOW_SIGS) && is_ctb_type(ctb, CTB_SKE_TYPE))
		{
			if (print_trust)
			{
				read_trust(f, &keyctrl);
				switch (TRUST_LEV(keyctrl))
				{
					case KC_SIGTRUST_UNDEFINED: 	/* undefined trust level */
					case KC_SIGTRUST_UNKNOWN:
						/* Just don't say anything in this case */
						fprintf(pgpout, PSTR("  Questionable certification from:\n  "));
						break;
					case KC_SIGTRUST_UNTRUSTED: 	/* untrusted */
						fprintf(pgpout, PSTR("  Untrusted certification from:\n  "));
						break;
					case KC_SIGTRUST_MARGINAL: 	/* marginal trust */
						fprintf(pgpout, PSTR("  Generally trusted certification from:\n  "));
						break;
					case KC_SIGTRUST_COMPLETE: 	/* complete trust */
						fprintf(pgpout, PSTR("  Completely trusted certification from:\n  "));
						break;
					case KC_SIGTRUST_ULTIMATE: 	/* axiomatic, ultimate trust */
						fprintf(pgpout, PSTR("  Axiomatically trusted certification from:\n  "));
						break;
					default:	/* can't get here, right? */
						break;
				}	/* switch */
			}
			else
				fprintf(pgpout, PSTR("  Certified by: "));
			show_userid(f, sigkeyID);
		}
	}
	if (status == -1 && userids)
		status = 0;
	set_precision(precision);
	fseek(f, filepos, SEEK_SET);
	return(status);
}	/* show_key */


/*
 * write trust byte "keyctrl" to file f at file position "pos"
 */
void
write_trust_pos(FILE *f, byte keyctrl, long pos)
{
	long fpos;

	fpos = ftell(f);
	fseek(f, pos, SEEK_SET);
	write_trust(f, keyctrl);
	fseek(f, fpos, SEEK_SET);
}	/* write_trust_pos */


/*
 * read a trust byte packet from file f, the trust byte will be
 * stored in "keyctrl".
 * returns -1 on EOF, -3 on corrupt input, and ERR_NOTRUST if
 * the packet was not a trust byte (this can be used to check if
 * a file is a keyring (with trust bytes) or a keyfile).
 */
int
read_trust(FILE *f, byte *keyctrl)
{
	unsigned char buf[3];

	if (fread(buf, 1, 3, f) != 3)
		return -1;
	if (buf[0] != CTB_KEYCTRL)
	{
		if (is_ctb(buf[0]))
			return(ERR_NOTRUST);
		 else
			return(-3);		/* bad data */
	}
	if (buf[1] != 1)		/* length must be 1 */
		return(-3);
	if (keyctrl)
		*keyctrl = buf[2];
	return(0);
}	/* read_trust */
