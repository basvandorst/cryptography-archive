/*
 *  chain.c
 *	Chain Cypherpunks remailers for effective encryption
 *	Usage: chain [-r] address remailer [...]
 *	-r means to create an anonymous return address and print it.
 *	No -r means to set up the standard input for sending.
 *	This file is intended to be viewed with tab stop spacing of 4 chars.
 *	This program is released into the public domain.
 *	Version 1.0, May 31, 1993 - Hal Finney
 *		Tested on MSDOS and Sun Unix systems.
 */

#include <stdio.h>
#include <ctype.h>

#ifdef MSDOS
#define PATH_SEP "\\"
#define XPATH 128
#else
#define PATH_SEP "/"
#define XPATH 256
#define SENDMAIL "/usr/lib/sendmail %s"
#endif

#define REMFILE "chain.ini"
#define PGPFLAGCHAR '*'
#define PGPSTARTCHAR '-'

/* Global variables */
char *pname;				/* Program name */
int return_flag = 0;		/* 1 to create return address, else 0 */
int encrypt_flag = 0;		/* 1 to encrypt for destination, else 0 */
int sendmail_flag = 0;		/* 1 to pipe into sendmail.  Only for Unix */
char *subject = NULL;		/* Subject to use on final step in chain */
FILE *frem;					/* Remailer list file */
char buff[2*XPATH + 40];	/* Temp buffer */

/* Forward definitions */
FILE *openremfile();
void swap_cpp ();
void userr();
void filecopy ();
void do_pgp ();
void add_header ();

main (argc, argv)
int	argc;
char	**argv;
{
	int pgpflag, prevpgpflag;
	FILE *f;
	FILE *fout = stdout;
	int i;
	char dest[XPATH], remailer_address[XPATH];
	char infilenm[XPATH], outfilenm[XPATH];
	char *infile=infilenm, *outfile=outfilenm;
	extern char *mktemp();
	extern char *getenv();
	extern FILE *popen();

	/* Parse command-line arguments */
	pname = argv[0];  ++argv;  --argc;
	while (argv[0]  &&  argv[0][0] == '-') {
		char c, *opt = argv[0];
		while (c = *++opt) {
			switch (c) {
			case 'r':	return_flag = 1; break;
#ifdef SENDMAIL
			case 'm':	sendmail_flag = 1; break;
#endif
			case 's':	if (opt[1] || !argv[1])
							userr();
						++argv; --argc;
						subject = argv[0];
						break;
			case 'e':	encrypt_flag = 1; break;
			default:	fprintf (stderr, "Unrecognized option: %c\n", c);
						userr();
						break;
			}
		}
		++argv; --argc;
	}

	if (argc < 2)
		userr();
	
	if ((subject || encrypt_flag || sendmail_flag) && return_flag)
		userr();

	/* Remember destination address */
	strcpy (dest, argv[0]);  ++argv;  --argc;

	/* Open remailer file */
	frem = openremfile();

	/* Name temp files */
	infile[0] = outfile[0] = '\0';
	if (getenv("TMP")) {
		strcpy (infile, getenv("TMP"));
		strcpy (outfile, infile);
		strcat (infile, PATH_SEP);
		strcat (outfile, PATH_SEP);
	}
	strcat (infile, mktemp ("c1XXXXXX"));
	strcat (outfile, mktemp ("c2XXXXXX"));

	if (!(f = fopen(infile, "w"))) {
		fprintf (stderr, "Unable to open temp file %s for output\n", infile);
		exit (2);
	}
	if (!return_flag) {
		/* Copy standard input to temp file */
		filecopy (stdin, f);
	}
	fclose (f);

	/* If encrypting for destination, do that now. */
	if (encrypt_flag) {
		do_pgp (infile, outfile, dest);
		swap_cpp (&infile, &outfile);
	}

	/* Loop for all remailers */
	prevpgpflag = 0;
	for (i=argc-1; i>=0; i--) {
		if (!find_remailer (argv[i], &pgpflag, remailer_address)) {
			fprintf (stderr, "Unable to find remailer matching '%s'\n", argv[i]);
			unlink (infile); unlink (outfile);
			exit (5);
		}
		add_header (infile, outfile, 1, prevpgpflag, dest, subject);
		subject = NULL;			/* Only use it once */
		swap_cpp (&infile, &outfile);
		if (pgpflag) {
			do_pgp (infile, outfile, remailer_address);
			swap_cpp (&infile, &outfile);
			if (i==0) {
				/* Add "Encrypted: PGP" on last pass */
				add_header (infile, outfile, 0, 1, NULL);
				swap_cpp (&infile, &outfile);
			}
		}
		prevpgpflag = pgpflag;
		strcpy (dest, remailer_address);
	}
	if (!(f = fopen(infile, "r"))) {
		fprintf (stderr, "Unable to open temp file %s for input\n", infile);
		unlink (infile); unlink (outfile);
		exit (2);
	}

#ifdef SENDMAIL
	if (sendmail_flag) {
		/* Open pipe to SENDMAIL */
		sprintf (buff, SENDMAIL, remailer_address);
		if (!(fout = popen(buff, "w"))) {
			fprintf (stderr, "Unable to open sendmail pipe: %s\n", buff);
			exit (2);
		}
		fprintf (fout, "To: %s\n", dest);
		fprintf (fout, "From: Chain program\n");
		fprintf (fout, "\n");
	}
#endif /* SENDMAIL */

	if (return_flag) {
		fprintf (fout, "Copy the material between the \"cut here\" lines below and put it\n");
		fprintf (fout, "at the top of your reply message, then send the message to:\n");
		fprintf (fout, "%s\n", remailer_address);
		fprintf (fout, "\n");
		fprintf (fout, "vvvvvvvvvvvvvvvvvvvvvvv cut here vvvvvvvvvvvvvvvvvvvvvvv\n");
	}
	filecopy (f, fout);
	fclose (f);
	if (return_flag)
		fprintf (fout, "^^^^^^^^^^^^^^^^^^^^^^^ cut here ^^^^^^^^^^^^^^^^^^^^^^^\n");
#ifdef SENDMAIL
	if (sendmail_flag)
		pclose (fout);
#endif /* SENDMAIL */
	unlink (infile);  unlink (outfile);
}

/* Add up to two headers to the infile.  If request_flag is true, add
 * "::\nRequest-Remailing-To: <dest>\n\n".  If enc_flag is true, then add
 * "::\nEncrypted: PGP\n\n".  Then copy the rest of the file unless infile
 * is null.
 * If subject is non-NULL then add a Subject line after the
 * Request-Remailing-To line.
 */
void
add_header (infile, outfile, request_flag, enc_flag, dest, subject)
char *infile, *outfile;
int request_flag, enc_flag;
char *dest;
char *subject;
{
	FILE *f1, *f2;

	if (infile && !(f1 = fopen (infile, "r"))) {
		fprintf (stderr, "Unable to open temp file %s for input\n", infile);
		unlink (infile); unlink (outfile);
		exit (3);
	}
	if (!(f2 = fopen (outfile, "w"))) {
		fprintf (stderr, "Unable to open temp file %s for output\n", outfile);
		if (infile) {
			fclose (f1);
			unlink (infile);
		}	
		unlink (outfile);
		exit (3);
	}
	if (request_flag) {
		fprintf (f2, "::\nRequest-Remailing-To: %s\n", dest);
		if (subject)
			fprintf (f2, "Subject: %s\n", subject);
		fprintf (f2, "\n");
	}
	if (enc_flag)
		fprintf (f2, "::\nEncrypted: PGP\n\n");
	
	if (infile) {
		filecopy (f1, f2);
		fclose (f1);
		unlink (infile);
	}
	fclose (f2);
}

/* Do PGP encryption to the infile, putting the result into the outfile.
 * rem_address will be used to look up the PGP key to use.
 */
void
do_pgp (infile, outfile, rem_address)
char *infile, *outfile;
char *rem_address;
{
	FILE *f = NULL;
	int stat;

	sprintf (buff, "pgp -feat %s < %s > %s", rem_address, infile, outfile);
	stat = system(buff);
	/* Must check for error rather carefully - output file must start with
	 * '-'.  "system" does not really return a valid status code for us.
	 */
	if (stat != 0 ||
			!(f = fopen(outfile, "r")) || fread(buff, 1, 1, f) != 1  ||
			buff[0] != PGPSTARTCHAR) {
		fprintf (stderr, "PGP abnormal exit, status %d\n", stat);
		if (f)
			fclose (f);
		unlink (infile); unlink (outfile);
		exit (4);
	}
	fclose (f);
	unlink (infile);
}

void
filecopy (f1, f2)
FILE *f1, *f2;
{
	int c;

	while ((c = getc(f1)) != EOF)
		putc (c, f2);
}

/* Print out a usage error message and exit */
void
userr()
{
#ifdef SENDMAIL
	fprintf (stderr, "Usage: %s [-rme] [-s subject] dest remailer [...]\n", pname);
#else
	fprintf (stderr, "Usage: %s [-re] [-s subject] dest remailer [...]\n", pname);
#endif
	fprintf (stderr, "-r means to print an anonymous return address\n");
	fprintf (stderr, "else create message to chain stdin through remailers\n");
#ifdef SENDMAIL
	fprintf (stderr, "-m means to pipe output into sendmail and mail it\n");
#endif
	fprintf (stderr, "-e means to encrypt for the destination\n");
	fprintf (stderr, "-s sets the specified subject for the outgoing message\n");
	exit (1);
}

/* Open the remailer init file.  First try the local directory, and if it
 * is not there and PGPPATH is defined try that directory.
 * Exit with an error if can't find it, else return an open file descriptor.
 */
FILE *
openremfile()
{
	FILE *f;
	char *pp;
	char remfile[XPATH];
	extern char *getenv();

	pp = getenv("PGPPATH");
	if (f = fopen(REMFILE, "r"))
		return f;
	if (pp) {
		strcpy (remfile, pp);
		strcat (remfile, PATH_SEP);
		strcat (remfile, REMFILE);
		if (f = fopen(remfile, "r"))
			return f;
	}
	fprintf (stderr, "Unable to read remailer list file %s\n", REMFILE);
	exit (2);
}

/* Look up remailer based on substring and fill in full_name, pgp flag */
/* File format is a list of remailers, one per line, with a * as the first
 * char if it has PGP.  Return 1 if find it, 0 if no matches.
 */
int
find_remailer (str, ppflag, full_name)
char *str;			/* Substring of name */
int *ppflag;		/* Pointer to flag for whether it has PGP */
char *full_name;	/* Full name */
{
	char buf[XPATH];
	char *p, *p1;
	char c;
	int len;

	fseek (frem, 0L, 0);
	c = *str; len = strlen(str);
	while (fgets (buf, sizeof(buf), frem)) {
		for (p=buf; *p; ++p) {
			if (*p == c) {
				if (strncmp (p, str, len) == 0) {
					/* Found it */
					p = buf;
					if (*p == PGPFLAGCHAR) {
						*ppflag = 1;
						++p;
					} else
						*ppflag = 0;
					/* Skip past blanks, then null-terminate at next blank */
					while (*p && isspace(*p))
						++p;
					p1 = p;
					while (*p && !isspace(*p) && (*p!='\n'))
						++p;
					*p = '\0';
					strcpy (full_name, p1);
					return 1;
				}
			}
		}
	}
	return 0;
}
					

/* Swap two pointers */
void
swap_cpp (p1, p2)
char **p1, **p2;
{
	char *t;
	t = *p1; *p1 = *p2; *p2 = t;
}
