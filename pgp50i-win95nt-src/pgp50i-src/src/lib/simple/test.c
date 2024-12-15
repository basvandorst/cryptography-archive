/*
* test.c -- Test driver for Simple PGP API
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* gcc -g -I../../include -I../.. -DLOOP=0 -DDEBUG=1 -DHAVE_CONFIG_H=1 -o test
* test.c spgp*.o -L.. -lpgp -lbn -lm
*
*/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <assert.h>
#include "spgp.h"
#include "pgpLeaks.h"
#include "pgpRngPub.h"

#ifdef TESTCALLBACK
#include "pgpEnv.h"
#endif

enum {false=0, true};

/* Print an error message and exit */
void
error (const char *s)
{
	fputs (s, stderr);
	fputc ('\n', stderr);
	exit (1);
}

#ifdef TESTCALLBACK
int
progress (size_t numbytes)
{
	static size_t total=0;
	printf ("%3d, %d\n", numbytes, total+=numbytes);
if (total > 1000000)
return -1;
	return 0;
}
#endif

byte *
fileread (char *filename, size_t *bufsize)
{
	byte *buf;
	struct stat statbuf;
	FILE *f;

	if (stat (filename, &statbuf) < 0) {
		perror ("stat");
		exit (2);
	}
	*bufsize = statbuf.st_size;
	buf = (byte *)malloc (statbuf.st_size);
	if (!(f = fopen (filename, "rb"))) {
		perror ("fopen");
		exit (2);
	}
	if (fread (buf, 1, statbuf.st_size, f) != statbuf.st_size) {
		perror ("fread");
		exit (2);
	}
	fclose (f);
	return buf;
}

void
filewrite (char *filename, byte *buf, size_t bufsize)
{
	FILE *f;

	if (!(f = fopen (filename, "wb"))) {
		perror ("fopen");
		exit (3);
	}
	if (fwrite (buf, 1, bufsize, f) != bufsize) {
		perror ("fwrite");
		exit (3);
	}
	fclose (f);
}

void
listkeys ()
{
	char UserID[256];
	char KeyID[11];
	char CreationDate[11];
	char ExpirationDate[11];
	int KeyLength;
	int ValidityDays;
	int KeyType;
	char KeyTypeES[3];
	char KeyState[4];
	int err;

	err = SimplePGPOpenPublicKeyRing (NULL, NULL);
	for ( ; ; ) {
		err = SimplePGPGetNextPublicKey (NULL, UserID, KeyID,
			&KeyLength, CreationDate, ExpirationDate, &ValidityDays,
			&KeyType, KeyTypeES, KeyState);
		if (err)
			break;
		printf ("UserID: %s; KeyID: %s; KeyLength: %d;\n",
			UserID, KeyID, KeyLength);
		printf (" CreationDate: %s; ExpirationDate: %s;\n",
			CreationDate, ExpirationDate);
		printf (
			" ValidityDays: %d; KeyType: %d; KeyTypeES: %s; KeyState: %s\n",
			ValidityDays, KeyType, KeyTypeES, KeyState);
	}
	err = SimplePGPClosePublicKeyRing (NULL);
	printf ("\n");

	err = SimplePGPOpenPrivateKeyRing (NULL, NULL);
	for ( ; ; ) {
		err = SimplePGPGetNextPrivateKey (NULL, UserID, KeyID,
			&KeyLength, CreationDate, ExpirationDate, &ValidityDays,
			&KeyType, KeyTypeES, KeyState);
		if (err)
			break;
		printf ("UserID: %s; KeyID: %s; KeyLength: %d;\n",
			UserID, KeyID, KeyLength);
		printf (" CreationDate: %s; ExpirationDate: %s;\n",
			CreationDate, ExpirationDate);
		printf (
			" ValidityDays: %d; KeyType: %d; KeyTypeES: %s; KeyState: %s\n",
			ValidityDays, KeyType, KeyTypeES, KeyState);
	}
	err = SimplePGPClosePrivateKeyRing (NULL);
}

void
usage (char *pname)
{
	fprintf (stderr,
"Usage: %s [-m] [-d -c <passphrase> -u -z <passphrase> |\n"
"\t[-e <username> | -c <passphrase>] -s<signerid> -a -t -u \n"
"\t  -z <passphrase>] infile outfile\n", pname);
	fprintf (stderr, "\tOne of -d, -c, or -e is mandatory.\n");
}

void
userr (char *pname)
{
	usage (pname);
	exit (1);
}

/*
* Options:
* -d				Decrypt
* -s<opt name>		 Sign it with default or specified user name
* -b				Break off as separate signature
* -a				Ascii armor the output
* -t				Treat as text mode
* -c <passphrase>	Use pass phrase for encrypting/decrypting
* -u				Use untrusted keys
* -z <passphrase>	Key passphrase for signing/decrypting
* -e <username>	 One encryption target
* -m				Use memory buffer functions rather than file functions
* -v				Do verify, infile is sig, outfile is text (no output)
* -y				AnalYze to see what the file is. Output file not used.
* -l				List keyring contents. Filenames not used.
* -k Add keys from infile into outfile
* -x Extract key ("infile" param) to outfile
*/

main (int argc, char *argv[])
{
	int rslt;
	char *infile, *outfile;
	byte *reciplist;
	char *pname = argv[0];
	byte fdecrypt=0, farmor=0, ftextmode=0, funtrusted=0, fconf=0, fkeypass=0,
		fsign=0, fencrypt=0, fwipe=0, fconv=0, fbreak=0, fclearsig=0,
		fmem=0, fverify=0, fanalyze=0, faddkey=0, flistkeys=0, fextkey=0;
	char *sconv=0, *skeypass=0, *ssignid=0, *sencryptid=0;
	int sencryptlen;
	int sstat;
	char signer[256];
	char signdate[40];
	int c;
	byte *inbuf, *outbuf;
	size_t inbufsize, outbufsize;
	extern char *optarg;
	extern int optind;
	int nsets, nfiles;
	extern PGPKDBExport RingPool *pgpRingPool;
#if LOOP
	static char buf[256];
	char *p;
	static char *xargv[32];
	extern char *strtok();
#endif

#if LOOP
	for ( ; ; ) {
		int lastlen;
		fputs("Command -> ", stdout);
		fgets (buf, sizeof(buf), stdin);
		argc = 1;
		p = buf;
		while ((unsigned)argc < sizeof(xargv) / sizeof(xargv[0])
		&& (xargv[argc] = strtok(p, " "))) {
			p = 0;
			argc++;
		}
		argv = xargv;
		lastlen = strlen (argv[argc-1]);
		if (argv[argc-1][lastlen-1] == '\n')
			argv[argc-1][lastlen-1] = '\0';
		if (argc==2 && argv[1][0]=='\0')
			break;
	fdecrypt=0, farmor=0, ftextmode=0, funtrusted=0, fconf=0, fkeypass=0,
		fsign=0, fencrypt=0, fwipe=0, fconv=0, fbreak=0, fclearsig=0,
		fmem=0, fverify=0, fanalyze=0, faddkey=0, flistkeys=0, fextkey=0;
	sconv=0, skeypass=0, ssignid=0, sencryptid=0;
#endif

#if DEBUG
	pgpLeaksBeginSession ("main");
#endif

	sencryptid = (char *)malloc(2);
	sencryptid[0] = INCLUDE_ONLYUSERIDS;
	sencryptid[1] = '\0';
	sencryptlen = 1;

	optind = 0;
	while ((c = getopt (argc, argv, "daykxls::btc:uz:e:mv")) != EOF) {
		switch (c) {
		case 'd':	fdecrypt = true; break;
		case 'a':	farmor = true; break;
		case 'b':	fbreak = true; break;
		case 't':	ftextmode = true; break;
		case 'u':	funtrusted = true; break;
		case 'm':	fmem = true; break;
		case 'c':	fconv = true; sconv = optarg; break;
		case 'z':	fkeypass = true; skeypass = optarg; break;
		case 's':	fsign = true; ssignid = optarg; break;
		case 'e':	if (SimplePGPCheckRecipient (NULL, optarg, 1, NULL)) {
						fprintf (stderr, "Unable to find a key for %s\n",
							optarg);
					} else {
						int newlen = strlen(optarg);
						sencryptid = realloc (sencryptid,
										sencryptlen+newlen+1);
						strcpy (sencryptid+sencryptlen, optarg);
						strcpy (sencryptid+sencryptlen+newlen, "\n");
						sencryptlen += newlen + 1;
						fencrypt = true;
					}
					break;
		case 'v':	fverify = true; break;
		case 'y':	fanalyze = true; break;
		case 'l':	flistkeys = true; break;
		case 'k':	faddkey = true; break;
		case 'x':	fextkey = true; break;
		case ':':
		case '?':	userr (pname);
		default:	userr (pname);
		}
	}

	if (flistkeys) {
		listkeys();
#if DEBUG
printf ("(Checking for leaks...)\n");
pgpLeaksEndSession ();
#endif
		exit (0);
	}
	
	if (fdecrypt + fencrypt + fconv + fverify + fanalyze
		+ faddkey + fextkey > 1)
		userr (pname);
	if (fdecrypt + fencrypt + fconv + fverify + fsign
		+ fanalyze + faddkey + fextkey < 1)
		userr (pname);
	if ((fdecrypt || fverify || fanalyze || faddkey || fextkey)
			&& (farmor || ftextmode || fsign))
		userr (pname);

	if (!(infile = argv[optind++]))
		userr (pname);
	if (!fanalyze) {
		if (!(outfile = argv[optind++]))
			userr (pname);
	}
	if (argv[optind++])
		userr (pname);


	if (fencrypt || fconv) {
		reciplist = (byte *)sencryptid;
		if (fconv)
			reciplist[0] = 0;
#ifdef TESTCALLBACK
		{ extern PGPKDBExport PgpEnv *pgpEnv;
		pgpLibInit();
		pgpenvSetPointer (pgpEnv, PGPENV_ENCRYPTIONCALLBACK, &progress,
							sizeof (&progress));
		}
#endif
		if (fmem) {
			inbuf = fileread (infile, &inbufsize);
			outbufsize = inbufsize * 2;
			outbuf = (byte *)malloc (outbufsize);
			rslt = SimplePGPEncryptBuffer (NULL, inbuf, inbufsize, "test",
				outbuf, &outbufsize,
				fsign,
				farmor, ftextmode, fconv, funtrusted, reciplist,
				(ssignid?ssignid:""), 0, (skeypass?skeypass:""), 0,
				(sconv?sconv:""), 0,
				"pubring.pkr", "secring.skr");
			if (!rslt)
				filewrite (outfile, outbuf, outbufsize);
		} else {
			rslt = SimplePGPEncryptFile (NULL, infile, outfile,
				fsign, fwipe,
				farmor, ftextmode, fconv, funtrusted, reciplist,
				(ssignid?ssignid:""), 0, (skeypass?skeypass:""), 0,
				(sconv?sconv:""), 0, NULL, NULL);
		}
#ifdef TESTCALLBACK
		{ extern PGPKDBExport PgpEnv *pgpEnv;
		pgpenvSetPointer (pgpEnv, PGPENV_ENCRYPTIONCALLBACK, NULL, 0);
		pgpLibCleanup();
		}
#endif
	} else if (fsign) {
		if (fbreak && farmor && ftextmode) {
			fbreak = false;
			fclearsig = true;
		}
		if (fmem) {
			inbuf = fileread (infile, &inbufsize);
			outbufsize = inbufsize * 2;
			outbuf = (byte *)malloc (outbufsize);
			rslt = SimplePGPSignBuffer (NULL, inbuf, inbufsize, "test",
				outbuf, &outbufsize,
				farmor, ftextmode,
				fbreak, fclearsig, (ssignid?ssignid:""), 0,
				(skeypass?skeypass:""), 0, "secring.skr");
			if (!rslt)
				filewrite (outfile, outbuf, outbufsize);
		} else {
			rslt = SimplePGPSignFile (NULL, infile, outfile, farmor, ftextmode,
				fbreak, fclearsig, (ssignid?ssignid:""), 0,
				(skeypass?skeypass:""), 0, NULL);
		}
	} else if (fdecrypt || fverify) {
		if (fverify) {
			char *sigfile = infile;
			char *datafile = outfile;
			byte *sigbuf, *databuf;
			size_t sigbufsize, databufsize;
			if (fmem) {
				sigbuf = fileread (sigfile, &sigbufsize);
				databuf = fileread (datafile, &databufsize);
				rslt = SimplePGPVerifyDetachedSignatureBuffer (NULL,
					databuf, databufsize, sigbuf, sigbufsize,
					&sstat, signer, sizeof(signer),
					signdate, sizeof(signdate), NULL);
			} else {
				rslt = SimplePGPVerifyDetachedSignatureFile (NULL,
					datafile, sigfile,
					&sstat, signer, sizeof(signer),
					signdate, sizeof(signdate), "pubring.pkr");
			}
		} else {
			if (fmem) {
				inbuf = fileread (infile, &inbufsize);
				outbufsize = inbufsize * 10;
				outbuf = (byte *)malloc (outbufsize);
				rslt = SimplePGPReceiveBuffer (NULL, inbuf, inbufsize, "test",
						outbuf, &outbufsize, (skeypass?skeypass:""), 0,
						&sstat, signer, sizeof(signer),
						signdate, sizeof(signdate),
						NULL, NULL);
				if (!rslt)
					filewrite (outfile, outbuf, outbufsize);
			} else {
				rslt = SimplePGPReceiveFile (NULL, infile, outfile,
						(skeypass?skeypass:""), 0,
						&sstat, signer, sizeof(signer),
						signdate, sizeof(signdate),
						NULL, NULL);
			}
		}
		if (!rslt) {
			printf ("Signature status: 0x%x\n", sstat);
			printf ("Signer: %s\n", signer);
			printf ("Signature timestamp: %s\n", signdate);
		}
	} else if (fanalyze) {
		if (fmem) {
			inbuf = fileread (infile, &inbufsize);
			rslt = SimplePGPAnalyzeBuffer (inbuf, inbufsize);
		} else {
			rslt = SimplePGPAnalyzeFile (infile);
		}
	} else if (faddkey) {
		char *keyfile = outfile;
		if (fmem) {
			inbuf = fileread (infile, &inbufsize);
			rslt = SimplePGPAddKeyBuffer (NULL, inbuf, inbufsize, keyfile);
		} else {
			rslt = SimplePGPAddKey (NULL, infile, keyfile);
		}
	} else if (fextkey) {
		char *userid = infile;
		if (fmem) {
			outbufsize = 1;
			outbuf = (byte *)malloc(outbufsize);
			rslt = SimplePGPExtractKeyBuffer (NULL, userid, (char *)outbuf,
				&outbufsize, NULL);
			outbuf = (byte *)malloc(outbufsize);
			rslt = SimplePGPExtractKeyBuffer (NULL, userid, outbuf,
				&outbufsize, NULL);
			if (!rslt)
				filewrite (outfile, outbuf, outbufsize);
		} else {
			rslt = SimplePGPExtractKey (NULL, userid, outfile, NULL);
		}
	}
#if DEBUG
	printf ("(Checking for leaks...)\n");
	pgpLeaksEndSession ();
#endif
	printf ("Finished, result = %d\n", rslt);
#if defined(WIN32) && !defined(DEBUG)
	/* Only in win32 non-debug mode is the ringpool kept open */
	ringPoolConsistent (pgpRingPool, &nsets, &nfiles);
	assert (nsets==0);
	assert (nfiles==1); /* MEMRING */
#endif
#if LOOP
	}
#endif
	exit (rslt);
}


/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
