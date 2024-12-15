#include <stdio.h>
#include <sys/types.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include <utime.h>
#include "l3.h"

unsigned char key[72];
unsigned char k2buf[32];
unsigned char iv[24];
unsigned char eiv[24];
extern unsigned char ver[];

#if defined(sun) || defined(_AIX) || defined(svr4) || defined(hpux) || defined(linux)
#define USE_FSYNC
#endif

/*****************************************************************
 * TAG( main )
 *
 *
 * Usage: mg [-dfc] [file ...]
 * Inputs:
 *  -d:     If given, decompression is done instead.
 *
 *      -c:         Write output on stdout, don't remove original.
 *
 *  -f:     Forces output file to be generated, even if one already
 *          exists, and even if no space is saved by compressing.
 *          If -f is not used, the user will be prompted if stdin is
 *          a tty, otherwise, the output file will not be overwritten.
 *
 *  file ...:   Files to be compressed.  If none specified, stdin
 *          is used.
 * Outputs:
 *  file.U:     Compressed form of file with same mode, owner, and utimes
 *  or stdout   (if stdin used as input)
 *
 */
int suppress_unlink = 0;
int mcat_flg = 0;	/* Write output on stdout, suppress messages */
int precious = 1;	/* Don't unlink output file on interrupt */
int quiet = 1;	/* don't tell me about compression */
int wipe_old_file = 1;
int suppress_kc = 0;
struct timeval init_time;
struct timezone init_zone;
struct stat qq;

int force = 0;
int validate = 0;
char ofname[100];
int bgnd_flag;
int no_header = 0;

unsigned int Pbuffer = 0, Pmask = 128;
unsigned int Gbuffer, Gmask = 0;
FILE *infile, *outfile;
unsigned long int textsize = 0, codesize = 0, printcount = 0;
int do_decomp = 0;
int exit_stat = 0;	/* per-file status */
int perm_stat = 0;	/* permanent status */
int debug = 0;
#ifndef DEBUG
int test_mode = 0;	/* for testing purposes */
#else
int test_mode = 1;
#endif

#ifndef INTSIGRET
void (*oldint) ();
#else
int (*oldint) ();
#endif
int havekey;

void Usage(s)
char *s;
{
	fprintf(stderr, "usage: %s [-cdfnv2DI] [file...]\n", s);
	closerandom();
	exit(1);
}

main(argc, argv)
int argc;
char **argv;
{
	int overwrite = 0;	/* Do not overwrite unless given -f
                     * flag */
	char *cp;

	char tempname[100];
	char **filelist, **fileptr;
	int length;
	struct stat statbuf;
	extern onintr();
	gettimeofday(&init_time, &init_zone);
	initrandom();

	/* This bg check only works for sh. */
	if ((oldint = signal(SIGINT, SIG_IGN)) != SIG_IGN) {
		signal(SIGINT, onintr);
	}
	bgnd_flag = oldint != SIG_DFL;

	filelist = fileptr = (char **) (malloc(argc * sizeof(*argv)));
	*filelist = NULL;

	if ((cp = rindex(argv[0], '/')) != 0) {
		cp++;
	}
	else {
		cp = argv[0];
	}
	if (strcmp(cp, "unmg") == 0) {
		do_decomp = 1;
	}
	else if (strcmp(cp, "ucat") == 0) {
		do_decomp = 1;
		mcat_flg = 1;
	}

	/*
     * Argument Processing All flags are optional. -d => do_decomp -f =>
     * force overwrite of output file -c => cat all output to stdout
     */
	for (argc--, argv++; argc > 0; argc--, argv++) {
		int j;
		if (**argv == '-') {	/* A flag argument */
			while (*++(*argv)) {	/* Process all flags in this
                         * arg */
				switch (**argv) {
				case 'k':	/* check file on ENCRYPT */
					validate = 1;
					break;

				case 't':
					test_mode = 1;
					break;
				case 'D':
					debug = 1;
					break;
				case 'w':
					wipe_old_file = 0;
					break;
				case 'W':	/* triple wipe */
					wipe_old_file = (-1);
					break;

				case 's':	/* really for testing only */
					suppress_unlink = 1;
					break;

				case 'v':
					quiet = 0;
					fprintf(stderr, "mg -- %s (", ver);
#ifdef O_SYNC
					fprintf(stderr, " O_SYNC");
#endif
#ifdef USE_FSYNC
					fprintf(stderr, " USE_FSYNC");
#endif
					fprintf(stderr, " )\n");

					break;
				case 'd':
					do_decomp = 1;
					break;
				case 'f':
				case 'F':
					overwrite = 1;
					force = 1;
					break;
				case 'c':
					mcat_flg = 1;
					break;
				default:
					fprintf(stderr, "Unknown flag: '%c'; ", **argv);
					Usage(cp);
					exit(1);
				}
			}
		}
		else {	/* Input file name */
			*fileptr++ = *argv;	/* Build input file list */
			*fileptr = NULL;
			/* process nextarg; */
		}
	  nextarg:continue;
	}


	if (*filelist != NULL) {
		for (fileptr = filelist; *fileptr; fileptr++) {
			exit_stat = 0;
			if (do_decomp) {	/* DECRYPTION */
				/* Check for suffix */
				if (strcmp(*fileptr + strlen(*fileptr) - 2, ".U") != 0) {
					/* No .U: tack one on */
					strcpy(tempname, *fileptr);
					strcat(tempname, ".U");
					*fileptr = tempname;
				}
				/* Open input file */
				if ((freopen(*fileptr, "r", stdin)) == NULL) {
					perror(*fileptr);
					perm_stat = 1;
					continue;
				}
				/* Generate output filename */
				strcpy(ofname, *fileptr);
				ofname[strlen(*fileptr) - 2] = '\0';	/* Strip off .U */
			}
			else {	/* ENCRYPTION */
				if (strcmp(*fileptr + strlen(*fileptr) - 2, ".U") == 0) {
					fprintf(stderr, "%s: already has .U suffix -- no change\n",
							*fileptr);
					continue;
				}
				/* Open input file */
				if ((freopen(*fileptr, "r", stdin)) == NULL) {
					perror(*fileptr);
					perm_stat = 1;
					continue;
				}
				/* Generate output filename */
				strcpy(ofname, *fileptr);
#ifdef SHORTNAMES
				if ((cp = rindex(ofname, '/')) != NULL)
					cp++;
				else
					cp = ofname;
				if (strlen(cp) > 12) {
					fprintf(stderr, "%s: filename too long to tack on .U\n", cp);
					continue;
				}
#endif							/* UNIX     Long filenames allowed */
				strcat(ofname, ".U");
			}
			/* Check for overwrite of existing file */
			if (overwrite == 0 && mcat_flg == 0) {
				if (stat(ofname, &statbuf) == 0) {
					char response[2];
					response[0] = 'n';
					fprintf(stderr, "%s already exists;", ofname);
					if (bgnd_flag == 0 && isatty(2)) {
						fprintf(stderr, " do you wish to overwrite %s (y or n)? ",
								ofname);
						fflush(stderr);
						read(2, response, 2);
						while (response[1] != '\n') {
							if (read(2, response + 1, 1) < 0) {	/* Ack! */
								perror("stderr");
								break;
							}
						}
					}
					if (response[0] != 'y') {
						fprintf(stderr, "\tnot overwritten\n");
						continue;
					}
				}
			}
			if (mcat_flg == 0) {	/* Open output file */
				if (freopen(ofname, "w", stdout) == NULL) {
					perror(ofname);
					perm_stat = 1;
					continue;
				}
				precious = 0;
				if (!quiet)
					fprintf(stderr, "%s: \n", *fileptr);
			}
			/* Actually do the encryption/decryption */
			if (do_decomp == 0)
				do_it(0);
			else
				do_it(1);
			if (mcat_flg == 0) {
				copystat(*fileptr, ofname);	/* Copy stats */
				precious = 1;
				if ((exit_stat == 1) || (!quiet))
					putc('\n', stderr);
			}
		}
	}
	else {	/* Standard input */
		if (do_decomp == 0) {
			do_it(0);
			if (!quiet)
				putc('\n', stderr);
		}
		else {
			do_it(1);
		}
	}
	zap(eiv, sizeof(eiv));
	zap(key, sizeof(key));
	zap(k2buf, sizeof(k2buf));
	zap(iv, sizeof(iv));
	closerandom();
	exit(perm_stat ? perm_stat : exit_stat);
}

do_it(decrypt)
int decrypt;
{
	int ret;
	int i;
	infile = stdin;
	outfile = stdout;
	if (fstat(fileno(infile), &qq) < 0) {
		fprintf(stderr, "cannot stat %s\n", infile);
		exit(1);
	}
	if (!havekey) {
		unsigned char *ss = NULL, *tt = NULL;
		do {
			if (ss) {
				zap(ss, strlen(ss));
				free(ss);
			}
			if (tt) {
				zap(tt, strlen(tt));
				free(tt);
			}
			fprintf(stderr, "enter password: ");
			ss = getstring(1);
			fprintf(stderr, "enter password again: ");
			tt = getstring(1);
		} while (strcmp(ss, tt));
		zap(tt, strlen(tt));
		mdstring(ss, strlen(ss), key);
		sn512string(ss, strlen(ss), &key[16]);
		haval_string(ss, k2buf);
		zap(ss, strlen(tt));
		for (i=0; i<24; i++)
			key[48+i] = k2buf[i];
		zap(k2buf,sizeof(k2buf));
		
	}
	havekey = 1;

	if (debug) {
		fprintf(stderr, "\nraw key:\n   ");
		for (i = 0; i < 36; i++)
			fprintf(stderr, "%02x", key[i]);
		fprintf(stderr, "\n   ");
		for (i = 36; i < 72; i++)
			fprintf(stderr, "%02x", key[i]);
		fprintf(stderr, "\n");
	}
	if (!decrypt) {
		if (!test_mode)
			for (i = 0; i < 24; i++)
				iv[i] = randombyte();
		else
			for (i = 0; i < 24; i++)
				iv[i] = 0;

		fwrite(iv, 1, 24, stdout);
	}
	else {
		i = fread(iv, 1, 24, stdin);
		if (i < 24) {
			fprintf(stderr, "short file\n");
			oops();
		}
	}
	if (debug) {
		fprintf(stderr, "iv : ");
		for (i = 0; i < 24; i++)
			fprintf(stderr, "%02x", iv[i]);
		fprintf(stderr, "\n");
	}
	ret = scramble(stdin, stdout, key, iv, decrypt);
	if (!ret) {
		fprintf(stderr, "file corruption detected\n");
		oops();
	}
}

void wipe_file(fname, val, flength)
char *fname;
int val;
{
	FILE *fle;
	long count = 0;
	int qpqp;
	struct stat qq;
	int i;
	char tbuf[8192];
	unsigned char c = (unsigned char) val;
	fle = fopen(fname, "r+");
	if (!fle) {	/* couldn't open */
		/* attempt to alter perms */
		chown(fname, getuid(), getgid());
		chmod(fname, 0777);
		/* try again */
		fle = fopen(fname, "r+");
		if (!fle) {
			fprintf(stderr, "warning: cannot open %s for write -- not wiped\n", fname);
			return;
		}	/* still couldn't open file */
	}
#if defined(O_SYNC)
	/* after fflush we'll know it is on disk */
	if ((qpqp = fcntl(fileno(fle), F_GETFL, 0)) < 0)
		fprintf(stderr, "warning: fcntl(F_GETFL) failed in wipe_file\n");
	fcntl(fileno(fle), F_SETFL, qpqp | O_SYNC);
#endif
	bzero(tbuf, sizeof(tbuf));

	while (flength > 0) {
		if (flength < 8192)
			count = flength;
		else
			count = 8192;
		fwrite(tbuf, 1, count, fle);
		flength -= count;
	}
	fflush(fle);
#ifdef USE_FSYNC
	/* probably redundant if O_SYNC works */
	if (fsync(fileno(fle)) < 0)
		fprintf(stderr, "fsync failed!\n");
#endif
	fclose(fle);
#ifdef oldlinux
/*best we can do-although linux has O_SYNC defined it isn't implemented yet*/
	sync();
#endif
	if (test_mode)
		fprintf(stderr, "wipe %s -- val = %d, c=%02x\n", fname, val, c);
}

copystat(ifname, ofname)
char *ifname, *ofname;
{
	struct stat statbuf;
	int mode;
#ifdef OLD_UTIME
	time_t timep[2];
#else
	struct utimbuf timep;
#endif

	fclose(stdout);
	if (debug)
		fprintf(stderr, "copystat(%s,%s)\n",ifname,ofname);
	if (validate && !do_decomp && !mcat_flg) {
		struct stat qq;
		FILE *chkfile;
		int ret;
		if (!(chkfile = fopen(ofname, "r"))) {
			fprintf(stderr, "validation, cannot open %s for read\n", ofname);
			exit(-1);
		}
		if (fstat(fileno(chkfile), &qq) < 0) {
			fprintf(stderr, "cannot stat %s\n", infile);
			exit(1);
		}
		ret = scramble(stdin, stdout, key, iv, 1);
	    if (debug)
		   fprintf(stderr, "validation scramble returns %d\n", ret);
		if (ret != 0 && !quiet)
			fprintf(stderr, "validation of %s successful\n", ofname);
		if (ret == 0) {
			fprintf(stderr, "validation of %s failed\n", ofname);
			fprintf(stderr, "warning: neither %s nor %s will be removed\n", ifname, ofname);
			exit_stat = 1;
			perm_stat = 1;
			return;
		}
	}

	if (stat(ifname, &statbuf)) {	/* Get stat on input file */
		perror(ifname);
		return;
	}
	if ((statbuf.st_mode & S_IFMT /* 0170000 */ ) != S_IFREG /* 0100000 */ ) {
		if (quiet)
			fprintf(stderr, "%s: ", ifname);
		fprintf(stderr, " -- not a regular file: unchanged");
		exit_stat = 1;
		perm_stat = 1;
	}
	else if (statbuf.st_nlink > 1) {
		if (quiet)
			fprintf(stderr, "%s: ", ifname);
		fprintf(stderr, " -- has %d other links: unchanged",
				statbuf.st_nlink - 1);
		exit_stat = 1;
		perm_stat = 1;
	}
	else {
		exit_stat = 0;
		mode = statbuf.st_mode & 07777;
		if (chmod(ofname, mode))	/* Copy modes */
			perror(ofname);
		chown(ofname, statbuf.st_uid, statbuf.st_gid);	/* Copy ownership */
#ifdef OLD_UTIME
		timep[0] = statbuf.st_atime;
		timep[1] = statbuf.st_mtime;
		utime(ofname, timep);	/* Update last accessed and modified
                     * times */
#else
		timep.actime = statbuf.st_atime;
		timep.modtime = statbuf.st_mtime;
		utime(ofname, &timep);
#endif
		if (!quiet && wipe_old_file)
			fprintf(stderr, "wipe %s\n", ifname);
		if (wipe_old_file)
			wipe_file(ifname, 0xff, statbuf.st_size);
		if (wipe_old_file < 0) {
			wipe_file(ifname, 0, statbuf.st_size);
			wipe_file(ifname, -1, statbuf.st_size);
		}
		if (!suppress_unlink)
			if (unlink(ifname))	/* Remove input file */
				perror(ifname);
		if (!quiet)
			fprintf(stderr, " -- replaced with %s", ofname);
		return;	/* Successful return */
	}

	/* Unsuccessful return -- one of the tests failed */
	if (unlink(ofname))
		perror(ofname);
}

onintr()
{
	if (!quiet)
		fprintf(stderr, "\nSIGINT -- exiting\n");
	if (!precious)
		unlink(ofname);
	closerandom();
	exit(1);
}

oops()
{
	zap(eiv, sizeof(eiv));
	zap(key, sizeof(key));
	zap(k2buf, sizeof(k2buf));
	zap(iv, sizeof(iv));
	if (do_decomp)
		fprintf(stderr, "corrupt input\n");
	if (!mcat_flg) {
		fprintf(stderr, "%s not written\n", ofname);
		unlink(ofname);
	}
	closerandom();
	exit(1);
}
