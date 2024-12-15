/*
 * $Author: vince $
 * $Header: /staff/vince/src/atp/RCS/atp.c,v 1.15 1992/05/15 13:27:05 vince Exp vince $
 * $Date: 1992/05/15 13:27:05 $
 * $Revision: 1.15 $
 * $Locker: vince $
 */
#ifndef lint
static char *rcsid = "@(#) $Id: atp.c,v 1.15 1992/05/15 13:27:05 vince Exp vince $";
#endif

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>
#include "atp.h"

#define ASTEP		40	/* Allocation step. */
#define BAD_ALLOC(x)	die("Cannot allocate %d bytes!\n", x);
#define BAD_OPTION(x)	die("%s not compatible with previous options.\n", x);
#define WORDSEP		"\n\t \0"
#define MIN(x, y)	(x >= y ? y : x)
#define NO_EDIT_OPT 	(!Sflg && !Eflg && !cflg && !xflg && !kflg && !Dflg)
#define EDIT_OPT	(Sflg || Eflg || cflg || xflg || kflg || Dflg)

char		*pname = "atp";

/* Globals */
static char	*buffer;		/* Data from base */
static long	bufsize;		/* Size of buffer */
static char	**pbuffer;		/* Data pointers */
static int	pbufnum;		/* Number of entries */

static char	*dbfile =  DATABASE;

char		tmpfilename[256];
char		hostname[16];
static int	errflg, dflg, Eflg, cflg, xflg, Sflg, vflg, Dflg;
extern int	nerror, nzap;
int		kflg = 0;	/* -k checking mode flag. */
int		errors = 0;	/* Number of illegal files found. */

/* Crypt stuff */
Key_schedule    schedule;
C_Block         key, ivec, IV;
header		head;

/* Local */
void		die();
static void	insert(), delete(), edit(), sort(), makep(),
		help(), encode(), decode(), cat(), loadbuf(), checkall(),
		find(), destroy(), setcrypt();
void		sigh();
static char	*getstat(), *getbufline(), *isinbuffer(), *interesting();

int		main(argc, argv)
int		argc;
char		*argv[];
{
	DIR		*dir;
	struct dirent	*dent;
	char		*p;
	int		i;

	int		c;
	extern char	*optarg;
	extern int	optind;


	/*
	 * -f <file>	: Set <file> as database (default = DATABASE)
	 * -d		: Delete specified files
	 * -D		: Destroy database
	 * -c		: Cat database
	 * -E		: Edit database
	 * -v		: Verbose
	 * -h		: Help
	 * -x		: Change encryption key
	 * -k <mode>	: ChecK files against database
	 * -S		: Add all SUID/SGID files on / into *buffer
	 * [default]	: Replace existing files
	 */
	while ((c = getopt(argc, argv, "f:dDcEvhxk:S")) != EOF)
		switch (c) {
		case	'f':
			if ((dbfile = optarg) == NULL)
				die("Bad argument for -f option.\n");
			break;
		case	'd':
			if (Eflg || cflg || xflg || kflg || Sflg || Dflg)
				BAD_OPTION("-d");
			dflg++;
			break;
		case	'D':
			if (Eflg || cflg || xflg || kflg || Sflg || dflg)
				BAD_OPTION("-D");
			Dflg++;
			break;
		case	'c':
			if (dflg || Eflg || xflg || kflg || Sflg || Dflg)
				BAD_OPTION("-c");
			cflg++;
			break;
		case	'E':
			if (dflg || cflg || xflg || kflg || Sflg || Dflg)
				BAD_OPTION("-E");
			Eflg++;
			break;
		case	'v':
			if (kflg < 2)
				vflg++;
			break;
		case	'h':
			help();
			break;
		case	'x':
			if (dflg || cflg || Eflg || kflg || Sflg || Dflg)
				BAD_OPTION("-x");
			xflg++;
			break;
		case	'k':
			if (dflg || cflg || xflg || Eflg || Sflg || Dflg)
				BAD_OPTION("-k");
			kflg = atoi(optarg);
			if (kflg < 1 || kflg > 4)
				die("Bad argument for -k option.\n");
			if (kflg > 1)
				vflg = 0;
			break;
		case	'S':
			if (dflg || cflg || xflg || kflg || Eflg || Dflg)
				BAD_OPTION("-S");
			Sflg++;
			break;
		case	'?':
			errflg++;
		}
	
	if (errflg ||
	    (optind == argc && NO_EDIT_OPT) || /* No files and file_opts */
	    (optind != argc && EDIT_OPT)) /* Files and no_files_opts */
		die("Bad options. -h for help.\n");

	for (i = 1; i <= NSIG; i++) /* Set signals. */
		(void) signal(i, sigh);
	(void) signal(SIGCLD, SIG_DFL);
	(void) signal(SIGCHLD, SIG_DFL);
	(void) umask(~(S_IRUSR | S_IWUSR)); /* umask 177 */

	/* Get host name. */
	if (gethostname(hostname, sizeof(hostname)) == -1)
		die("gethostname() error.\n");

	if (Dflg) /* Destroy database. */
		destroy();

	loadbuf();
	if (bufsize == 0L && optind == argc && !Sflg)
		die("Cannot access database %s\n", dbfile);

	/* Set encryption stuff. */
	if ((p = getenv(KEYWORDVAR)) == NULL) {
		if (bufsize)
			p = getlongpass("Key: ");
		else /* Ask twice for key */
			p = askforkey("Encryption key:");
	}
	setcrypt(p);

	decode(); /* Decode database */

	if (xflg) { /* Change encryption key. */
		p = askforkey("New encryption key:");
		setcrypt(p);
		goto savestuff;
	}

	if (Eflg) { /* Direct edit */
		edit();
		goto done;
	}

	if (Sflg) { /* Search for S{UID, GID} files; put'em in the db */
		find(1);
		if (bufsize == NULL)
			exit(0);
		goto done;
	}

	if (cflg) /* Cat */
		cat();

	if (kflg) /* Check files */
		checkall(kflg);

	/* Add & Delete */
	for ( ; optind < argc; optind++) {
		if ((dir = opendir(argv[optind])) == NULL) {
			if (errno == ENOTDIR) { /* It's a file */
				if (dflg)
					delete(argv[optind]);
				else
					insert(argv[optind]);
			}
			else if (dflg) /* Might be in the database. */
				delete(argv[optind]);
			else	
				perror(argv[optind]);
			continue;
		}
		while ((dent = readdir(dir)) != NULL) { /* It's a directory */
			char		*buf;
			struct stat	s;
			int		l;

			l = strlen(argv[optind]) + strlen(dent->d_name);
			if ((buf = malloc(l + 1)) == NULL)
				BAD_ALLOC(l + 1);
			sprintf(buf, "%s/%s", argv[optind], dent->d_name);
			if (stat(buf, &s) == -1)
				die("Cannot stat %s\n", buf);
			if (!(s.st_mode & S_IFREG))
				continue; /* Not an ordinary file */
			if (dflg)
				delete(buf);
			else
				insert(buf);
			free(buf);
		}
	}

done:
	makep();
	sort();

	if (vflg)
		fprintf(stdout, "Database: %d files; %ld bytes\n",
			pbufnum, bufsize);
savestuff:
	if (bufsize) {
		encode();
		if (savebuf(1))
			die("Cannot save to file %s\n", dbfile);
	}
	free(buffer);
	return(0);
}

/*
 * Simple routines to handle signals properly.
 */
void	sigh(s)
int	s;
{
	die("Got signal %d! - Exiting...\n", s);
}

void	insert(filename)
char	*filename;
{
	char	*filestat;
	char	*already_inside = NULL;

	if ((filestat = getstat(filename)) == NULL) {
		fprintf(stderr, "Cannot read file %s\n", filename);
		return;
	}
	already_inside = isinbuffer(filename, 1);
	if (vflg)
		printf("%s: %s\n", 
			already_inside ? "replaced" : "inserted", filename);
	if (addbufline(filestat))
		die("addbufline(): cannot add data.");
}

/*VARARGS*/
void	die(a, b, c, d, e)
char	*a, *b, *c, *d, *e;
{
	fprintf(stderr, "%s: ", pname);
	fprintf(stderr, a, b, c, d, e);
	if (buffer) {
		free(buffer);
		buffer = NULL;
	}
	if (tmpfilename[0] && !access(tmpfilename, F_OK))
		if (unlink(tmpfilename))
			fprintf(stderr, "%s: Cannot unlink %s!", tmpfilename);
	exit(1);
}

/*
 * Get a text line from buffer. At first invocation we must supply
 * the buffer as first argument, then we use a NULL pointer.
 *
 * Returns: a pointer to a null terminated line of text if ok,
 *	    a NULL pointer otherwise.
 */
static char	*getbufline(buf)
char		*buf;
{
	static char	*p;
	static char	*line;
	static int	linesize;
	int		i;

	if (buf) {
		p = buf;
		if (linesize) {
			free(line);
			line = NULL;
			linesize = 0;
		}
		return(NULL);
	}

	for (i = 0; *p && *p != '\n'; i++, p++) {
		if (i >= linesize - 1) {
			if ((line = realloc(line, linesize + ASTEP)) == NULL)
				BAD_ALLOC(ASTEP);
			linesize += ASTEP;
		}
		line[i] = *p;
	}
	if (*p == '\n')
		p++;
	line[i] = '\0';
	return(i ? line : NULL);
}

/*
 * Append a line of text to the db buffer.
 *
 * Returns: boolean.
 */
static int	addbufline(line)
char		*line;
{
	int		linelen;

	linelen = strlen(line);
	if ((buffer = realloc(buffer, linelen + bufsize)) == NULL)
		BAD_ALLOC(linelen);
	memcpy(buffer + bufsize, line, linelen);
	bufsize += (long) linelen;
	return(0);
}

/*
 * Load the database into the *buffer array. Update bufsize.
 * Returns: boolean.
 */
static void	loadbuf()
{
	char	buf[BUFSIZ];
	int	fh;
	int	len;

	/* Load buffer */
	if ((fh = open(dbfile, O_RDONLY)) == -1) {
		perror(dbfile);
		return;
	}
	bufsize = 0L;
	buffer = NULL;
	while (1) {
		if ((len = read(fh, buf, sizeof(buf))) <= 0)
			break;
		if ((buffer = realloc(buffer, bufsize + len)) == NULL)
			BAD_ALLOC(len);
		memcpy(buffer + bufsize, buf, len);
		bufsize += len;
	}
	if (len == -1) {
		perror(dbfile);
		die(NULL);
	}
	/* Null terminate *buffer */
	if ((buffer = realloc(buffer, bufsize + 1)) == NULL)
		BAD_ALLOC(1);
	buffer[bufsize] = '\0';
}

/*
 * Create **pbuffer, by which we can fast find data.
 */
static void	makep()
{
	int	i;
	char	*p;

	if (bufsize == 0L)
		return;
	/* Calc number of lines in *buffer */
	pbufnum = 0;
	for (i = 0; i < bufsize; i++)
		if (buffer[i] == '\n') 
			pbufnum++;
	if (buffer[bufsize - 1] != '\n')
		pbufnum++;
	if (!pbufnum)
		return;

	/* Assign a pointer to each line: fill up **pbuffer */
	if ((pbuffer = calloc(pbufnum, sizeof(char *))) == NULL)
		BAD_ALLOC(pbufnum * sizeof(char *));
	p = buffer;
	i = 0;
	pbuffer[i] = p;
	while ((p = strchr(p, '\n')) != NULL)
		if (++p != NULL)
			pbuffer[++i] = p;
		else
			break;
}

/*
 * Save the database buffer on disk. If renameflg is true the
 * temporary file is moved onto the original one.
 *
 * Returns: boolean.
 */
static int	savebuf(renameflg)
int		renameflg;
{
	int	fh;
	int	i;

	for (i = 1; i <= NSIG; i++) /* Disable signals. */
		(void) signal(i, SIG_IGN);

	sprintf(tmpfilename, "%s/%sXXXXXX", TMPDIR, pname);
	if (mktemp(tmpfilename) == NULL) {
		fprintf(stderr, "mktemp() error.\n");
		return(1);
	}
	if ((fh = open(tmpfilename, O_WRONLY | O_CREAT | O_TRUNC,
	    S_IRUSR | S_IWUSR)) == -1) {
		perror(tmpfilename);
		return(1);
	}
	if (write(fh, buffer, bufsize) != bufsize) {
		perror("write()");
		return(1);
	}
	if (close(fh)) {
		perror("close()");
		return(1);
	}

	if (renameflg && rename(tmpfilename, dbfile)) {
		perror("rename()");
		fprintf(stderr, "%s: Please recover %s !\n",
			pname, tmpfilename);
		return(1);
	}
	for (i = 1; i <= NSIG; i++) /* Re-enable signals. */
		(void) signal(i, sigh);
	(void) signal(SIGCLD, SIG_DFL);
	(void) signal(SIGCHLD, SIG_DFL);

	return(0);
}

/*
 * Calculate a status entry. Collects all valuable security related
 * attributes.
 *
 * Returns: boolean.
 */
static char	*getstat(filename)
char		*filename;
{
	static char	buf[512];
	entry		*e;

	if ((e = getentry(filename)) == NULL)
		return(NULL);
	if (sprintf(buf, FILEPAT,
		e->filename,
		e->st.st_ino,
		e->st.st_mode,
		e->st.st_nlink,
		e->st.st_uid,
		e->st.st_gid,
		e->st.st_size,
		e->st.st_mtime,
		e->st.st_ctime,
		e->st.st_acl,
#ifdef MD4
		e->digest)
#else /* Defaults to CRC32 */
		e->CRC32)
#endif
			< 1) {
			perror("sprintf()");
			return(NULL);
	}
	return(buf);
}

static void	delete(buf)
char		*buf;
{
	if (isinbuffer(buf, 1) == NULL)
		printf("%s: ``%s'': no such filename in database\n",
			pname, buf);
	if (vflg)
		printf("deleted: %s\n", buf);
}

/*
 * Direct editing of the database file. This is risky: a plaintext copy
 * of database is written down, so any superuser can modify it.
 */
static void	edit()
{
	char	*tmp;
	int	status;
	int	i;

	if (bufsize == 0L)
		die("Database is empty. Nothing to edit.\n");

	printf("Warning: an unencrypted temporary file will created.\n");
	if (!confirm())
		die("Edit aborted.\n");

	/* Save *buffer to tmpfilename (which is created). Do not rename() */
	if (savebuf(0))
		die("Cannot save buffer to %s", tmpfilename);
	free(buffer);

	for (i = 1; i <= NSIG; i++)
		(void) signal(i, SIG_IGN);
	(void) signal(SIGCLD, SIG_DFL);
	(void) signal(SIGCHLD, SIG_DFL);
	/* Run editor on unencrypted temporary file. */
	switch (fork()) {
		case	-1:
			die("Cannot fork()!\n");
		case	0:
			for (i = 1; i <= NSIG; i++)
				(void) signal(i, SIG_DFL);
			execl(EDITOR, EDITOR, tmpfilename, NULL);
			die("Cannot exec %s\n", EDITOR);
		default	:
			if (wait(&status) == -1)
				die("Wait() error\n");
			if (status & 0177 || status >> 8)
				die("Editor %s failed.\n", EDITOR);
			break;
	}
	for (i = 1; i <= NSIG; i++)
		(void) signal(i, sigh);

	/* Reload temporary file then delete it. */
	tmp = dbfile;
	dbfile = tmpfilename;
	loadbuf();
	if (unlink(dbfile))
		die("Cannot unlink UNENCRYPTED temporary file %s !",
			tmpfilename);
	dbfile = tmp;
}

/*
 * Cat database to stdout. This way we can view data without worring
 * about temporary files (see edit()).
 */
static void	cat()
{
	if (!bufsize)
		exit(0);
	if (write(1, buffer, bufsize) != bufsize)
		die("Cannot write() to stdout.\n");
	free(buffer);
	exit(0);
}

static void	destroy()
{
	if (access(dbfile, F_OK))
		die("Database %s doesn't exist.\n", dbfile);
	printf("Warning: database %s will be destroyed!\n",
		dbfile);
	if (!confirm())
		die("Destruction aborted.\n");
	if (unlink(dbfile))
		die("Cannot unlink %s!\n", dbfile);
	exit(0);
}

/*
 * Recursive tree search. If ``recordflg'' is true, found files are added
 * to *buffer; else all found files are checked against *buffer,
 * looking for illegal new SUID/SGID files.
 */
static void	find(recordflg)
int		recordflg;
{
	FILE		*fp;
	char		buf[1024];	/* Filename. */
	struct stat	s;
	char		*p;

	if (!recordflg && vflg)
		printf("Looking for S'ed files...\n\n");
	if (vflg)
		printf("Executing: %s\n", FIND);
	if ((fp = popen(FIND, "r")) == NULL)
		die("Cannot exec %s\n", FIND);
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		buf[strlen(buf) - 1] = '\0';

		nerror = nzap = 0; /* New file */

		if (stat(buf, &s) == -1) {
			printf("%s: Warning: cannot stat() %s\n", pname, buf);
			continue;
		}

		/* Insert-mode and S'ed file: put it in the database. */
		if (recordflg && ISREMARKABLE()) {
			insert(buf);
			continue;
		}

		/* This is a new S'ed file. */

		/* Check for interesting string patterns. */
		if (kflg > 3 && (p = interesting(buf)) != NULL)
			add_mail(buf, "STRINGS pattern: `%s'.\n", p);

		/* If file is in database skip it for now. */
		if (isinbuffer(buf, 0))
			continue;

		/* * We have an unexpected illegal file. */
		if (ISREMARKABLE()) {
			add_mail(buf, "ILLEGAL SUID/SGID permissions: %s\n",
				pmode(s.st_mode, s.st_rdev, buf));
			zap(buf);
		}
	}
	(void) fclose(fp);
}

/*
 * Examine a file looking for interesting internal patterns.
 * 
 * Returns: NULL if nothing is found, a char pointer is we got it.
 */
static char	*interesting(filename)
char		*filename;
{
	FILE		*fp;
	char		strings[256], *valuable, *p;
	static char	buf[128];
	int		len;

	sprintf(strings, "%s %s", STRINGS, filename);
	valuable = (valuable = getenv("VALUABLE")) ? valuable : VALUABLE;
	if ((fp = popen(strings, "r")) == NULL)
		die("Cannot exec: %s\n", strings);
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		buf[strlen(buf) - 1] = '\0';
		for (p = valuable; *p; p++) {
			len = strlen(buf);
			if (len > 4 && !strncasecmp(p, buf, len)) {
				(void) pclose(fp);
				return(buf);
			}
		}
	}
	(void) pclose(fp);
	return(NULL);
}

/*
 * Compare database files with disk files.
 */
static void	checkall()
{
	char		*p;

	if (kflg < 1 || kflg > 4)
		die("Internal error.\n");
	if (kflg == 1 && vflg)
		printf("Comparing database with real files...\n\n");
	/*
	 * Find, mail, zap and eventually do strings. find() already
	 * updates the ``errors'' flag.
	 */
	else if (kflg >= 3)
		find(0);

	/* For each entry in the database... */
	(void) getbufline(buffer);
	while ((p = getbufline(NULL)) != NULL) {
#ifdef DEBUG
		printf("Checking file: %s\n", p);
#endif
		if (checkfile(p))
			errors++;
	}

	if (errors && kflg > 1) /* Mail results */
		add_mail(NULL);
	free(buffer);
	exit(0);
}

/*
 * Look in *buffer for a *filename entry. If ``flag'' is set
 * the fist letter of the filename in *buffer is changed to a tilde (~),
 * which is considered a deletion mark by sort().
 *
 * Returns: boolean
 */
static char	*isinbuffer(filename, flag)
char		*filename;
int		flag;
{
	char		*p;
	int		len;
	static char	buf[ENTRYLEN];

	/* FIX - search should be binary for speed */
	len = strlen(filename);
	/* First entry? */
	if (!strncmp(buffer, filename, len) && *(buffer + len) == ' ') {
		if (flag)
			*buffer = '~';
		strncpy(buf, buffer, strcspn(buffer, "\n\0"));
		return(buf);
	}
	/* Look trought all the buffer. */
	p = buffer;
	while ((p = strchr(p, '\n')) != NULL) {
		p++;
		if (!strncmp(p, filename, len) && *(p + len) == ' ') {
			if (flag)
				*p = '~';
			strncpy(buf, p, strcspn(p, "\n\0"));
			return(buf);
		}
	}
	return(NULL);
}

/*
 * Sort data pointers **pbuffer, then copy arrays into a temporary
 * **buffer2, then swap **buffer and **buffer2.
 */
static void	sort()
{
	int	compare();
	char	*buffer2, *tmpbuf;
	long	bufsize2;
	int	i, len, pbufnum2;

	if (bufsize == 0L)
		return;
	qsort(pbuffer, pbufnum, sizeof(char *), compare);
	if ((buffer2 = malloc(bufsize)) == NULL)
		BAD_ALLOC(bufsize);

	bufsize2 = 0L;
	pbufnum2 = 0;
	for (i = 0; i < pbufnum; i++) {
		if (*pbuffer[i] == '~')
			continue;
		len = strcspn(pbuffer[i], "\n\0") + 1;
		(void) memcpy(buffer2 + bufsize2, pbuffer[i], len);
		bufsize2 += (long) len;
		pbufnum2++;
	}
	if ((buffer2 = realloc(buffer2, bufsize2)) == NULL)
		BAD_ALLOC(bufsize2);
	tmpbuf = buffer2;
	buffer = buffer2;
	(void) free(tmpbuf);
	bufsize = bufsize2;
	pbufnum = pbufnum2;
}

int	compare(a, b)
char	**a, **b;
{
	int	len1 = strcspn(*a, WORDSEP);
	int	len2 = strcspn(*b, WORDSEP);
	return(strncmp(*a, *b, MIN(len1, len2)));
}

static void	help()
{
	printf("Usage: %s [options] [files...]\n\
 -f <file>: Specify database File (default: %s)\n\
	-E: Direct Editing of database\n\
	-c: Cat database to stdout\n\
	-d: Delete specified files\n\
	-D: Destroy database\n\
	-x: Change database encryption key\n\
	-S: Add SUID/SGID files in the database using find(1)\n\
 -k<value>: Check file integrity. Possible values are:\n\
	    `1' - Security report is sent to stdout\n\
	    `2' - Reports using mail(1); use syslogd(1M), zap illegal files;\n\
	    `3' - As above plus look for SUID/SGID files using find(1)\n\
	    `4' - As ab. plus inspect with strings(1) for $VALUABLE items\n\
	-v: Be Verbose\n\
	-h: This Help\n\
 [default]: add or replace specified files.\n",
		pname, DATABASE);
	exit(0);
}

/*
 * Encrypt data buffer. Data lenght is made a multiple of eight,
 * some ``random'' padding is added.
 */
static void	encode()
{
	int	i;

	memset(&head, 0, sizeof(head));

	/* Calc MD4 checksum */
	MD4Init(&mdContext);
	MD4Update(&mdContext, buffer, bufsize);
	MD4Final(&mdContext);

	/* Padding */
	srand48(time(NULL) * getpid());
	for (i = 0; i < sizeof(head.unused); i++)
		head.unused[i] = (lrand48() >> 20) & 0xff;

	/* Fill header */
	head.size = bufsize;
	memcpy(&head.digest, &mdContext.digest, 16);
	head.datestamp = time(NULL);

	/* Append header at the end of *buffer. */
	if ((buffer = realloc(buffer, bufsize + sizeof(head))) == NULL)
		BAD_ALLOC(bufsize);
	memcpy(buffer + bufsize, &head, sizeof(head));
	bufsize += sizeof(head);

	/* Crypt all */
	memcpy(&ivec, &IV, sizeof(C_Block)); /* Set IV */
	des_cbc_encrypt(buffer, buffer, bufsize, &schedule, &ivec, DES_ENCRYPT);
}

static void	decode()
{
	int	i;

	if (bufsize == 0L) /* Nothing was loaded */
		return;
	else if (bufsize < sizeof(head) + 1)
		die("Encrypted file is corrupted: file appears truncated.\n");

	/* Decrypt all */
	memcpy(&ivec, &IV, sizeof(C_Block)); /* Set IV. */
	des_cbc_encrypt(buffer, buffer, bufsize, &schedule, &ivec, DES_DECRYPT);

	bufsize -= sizeof(head);
	memcpy(&head, buffer + bufsize, sizeof(head));
	if (head.size != bufsize)
		die("Databse corrupted or wrong keyword.\n");

	/* Check MD4 */
	MD4Init(&mdContext);
	MD4Update(&mdContext, buffer, bufsize);
	MD4Final(&mdContext);
        for (i = 0; i < 16; i++)
		if (head.digest[i] != mdContext.digest[i])
			die("Database is corrupted (probabily tampered).\n");

	if (vflg) {
		printf("Database %s (%ld bytes) verified.\n", dbfile,
			head.size);
		printf("Database's MD4		: %s\n",
			getdig(&head.digest));
		printf("Checksum algorithm	: %s\n",	
#ifdef MD4
			"RSA MD4"
#else
			"CRC32"
#endif
			);
		printf("Database's datestamp	: %s",
			ctime(&(head.datestamp)));
		printf("Key dependent IV	: %s\n\n", getIV(&IV));
	}
}

/*
 * Create an Initialization Variable which is key dependent.
 * After fourty rounds it should be reasonably ``random''. Feel free
 * to send comments about that!
 */
static void	setcrypt(p)
char		*p;
{
	int	i;

	string_to_key(p, &key);
	memset(p, 0, strlen(p));
	des_set_key(&key, &schedule);
	memcpy(&IV, &key, sizeof(C_Block));
	for (i = 0; i < 40; i++)
		des_ecb_encrypt(&IV, &IV, schedule, DES_ENCRYPT);
}
