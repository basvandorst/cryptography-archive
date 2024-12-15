/*
 * $Author: vince $
 * $Id: sub.c,v 1.8 1992/05/15 13:27:05 vince Exp vince $
 * $Date: 1992/05/15 13:27:05 $
 * $Revision: 1.8 $
 * $Lock:$
 */

#ifndef	lint
static char *rcsid = "@(#) $Id: sub.c,v 1.8 1992/05/15 13:27:05 vince Exp vince $";
#endif	lint

#include <termio.h>
#include <sys/ioctl.h>
#include <syslog.h>
#include "atp.h"

extern char	*pname;		/* Program name. */
extern char	tmpfilename[];
#ifndef MD4
extern long	crc32val;	/* CRC32 value */
#endif

int		nzap;		/* Have we zap()'ed already? */
int		nerror;		/* Number of problems of current file. */
extern int	errors;		/* Number of illegal files. */
extern int	kflg;
extern char	hostname[];

/* Local functions */
int			checkfile(), confirm();
entry			*getentry();
static void		sendmail();
void			add_mail(), zap();
char			*getlongpass(), *pmode(), *askforkey();

/*
 * This function returns the ``entry'' struct of a filename.
 *
 * Returns: struct entry * == 0; NULL == failure.
 */
entry		*getentry(filename)
char		*filename;
{
	static entry		e;
	int			fh;

	if (filename[0] != '/')
		die("%s: not an absolute pathname.\n", filename);
	if ((fh = open(filename, O_RDONLY)) == -1 ||
	    fstat(fh, &(e.st)) == -1 || do_checksum(fh)) {
		close(fh);
		return(NULL);
	}
	close(fh);

	strcpy(e.filename, filename);
#ifdef MD4
	strcpy(&e.digest, getdig(&mdContext.digest));
#else /* Defaults to CRC32 */
	sprintf(&e.CRC32, "%0x", crc32val);
#endif
	return(&e);
}

/*
 * Convert the stat() entry ``st_mode'' into ASCII human readable form.
 * [This routine is by Marco Negri, marco@ghost.dsi.unimi.it. Thanks]
 *
 * Returns: a printable strings.
 */

#include <sys/sysmacros.h>

char	*pmode(mode, rdev, name)
int	mode;
off_t	rdev;
char	*name;
{
	static char	buf[100];
	int		i;
	static	 char	pos[4] = "rwx";

	strcpy(buf, "----------");
	switch (mode & S_IFMT) {
		case	S_IFDIR	:
			buf[0] = 'd'; break;

		case	S_IFCHR	:
		case	S_IFBLK	:
			buf[0] = ((mode &S_IFMT) == S_IFCHR) ? 'c' : 'b';
			sprintf(&buf[11], "(Major, Minor) : (%d, 0x%06x)",
				major(rdev), minor(rdev));
			break;

		case	S_IFIFO	:
			buf[0] = 'p'; break;

		case	S_IFNWK	:
			buf[0] = 'n'; 
			if ((i = open(name, O_RDONLY)) == -1) {
				perror(name);
				break;
			}
			buf[11 + read(i, &buf[11], 100 - 11)] = '\0';
			break;

		case	S_IFLNK	:
			buf[0]  = 'l'; 
			if ((i = readlink(name, &buf[11], 100 - 14)) == -1) {
				perror(name);
				return buf;
			}
			buf[i + 11] = '\0';
			break;

		case	S_IFSOCK:
			buf[0] = 's'; break;
	}

	for (i = 0; i < 9; i++)
		if (mode & (1 << (8 - i)))
			buf[i + 1] = pos[i % 3];

	if (mode & S_ISUID)
		buf[3] = (mode & 0100) ? 's' : 'S';

	if (mode & S_ISGID)
		buf[6] = (mode & 0010) ? 's' : 'S';

	if (mode & S_ISVTX)
		buf[9] = (mode & 0001) ? 't' : 'T';

	return buf;
}


/*
 * Check the entry-line from database againts the real entry status
 * of file. 
 *
 * Returns: boolean.
 */
int		checkfile(buf)
char		*buf;
{
	char		tmp[64];
	entry		*r;
	/* File attributes */
	char		filename[FNL];	/* Absolute path 	*/
	ino_t		st_ino;		/* Inode number		*/
	unsigned	st_mode;	/* Perm bits		*/
	unsigned	st_nlink;	/* Link number		*/
	uid_t		st_uid;		/* UID			*/
	gid_t		st_gid;		/* GID			*/
	off_t		st_size;	/* Filesize		*/
	time_t		st_mtime;	/* Modification time	*/
	time_t		st_ctime;	/* Creation time	*/
	uint		st_acl;		/* ACL			*/
#ifdef MD4
	unsigned char	digest[33];	/* ASCII MD4 		*/
#else /* Defaults to CRC32 */
	char		CRC32[9];	/* CRC32		*/
#endif

	sscanf(buf, FILEPAT,
		filename, &st_ino, &st_mode, &st_nlink, &st_uid,
		&st_gid, &st_size, &st_mtime, &st_ctime, &st_acl,
#ifdef MD4
		digest);
#else /* Defaults to CRC32 */
		CRC32);
#endif
	/* New file: number of errors = 0, number of zaps = 0 */
	nzap = nerror = 0;

	/* Get real filename entry status */
	if ((r = getentry(filename)) == NULL) {
		add_mail(filename, "ACCESS failed. Cannot access file!");
		printf("\n");
		return(1);
	}

	if (st_ino != r->st.st_ino) {
		add_mail(filename, "INODE has changed from %lu to %lu.\n",
			st_ino, r->st.st_ino);
	}
	if (st_mode != r->st.st_mode) {
		sprintf(tmp, "%s", pmode(st_mode, r->st.st_rdev, filename));
		add_mail(filename, "PERM-BITS has changed from %s to %s.\n",
			tmp, pmode(r->st.st_mode, r->st.st_rdev, filename));
		if (kflg > 1)
			zap(filename);
	}
	if (st_nlink != r->st.st_nlink) { 
		add_mail(filename, "LINK has changed from %u to %u.\n",
			st_nlink, r->st.st_nlink);
	}
	if (st_uid != r->st.st_uid) {
		add_mail(filename, "UID has changed from %ld to %ld.\n",
			st_uid, r->st.st_uid);
		if (kflg > 1)
			zap(filename);
	}
	if (st_gid != r->st.st_gid) {
		add_mail(filename, "GID number has changed from %ld to %ld.\n",
			st_gid, r->st.st_gid);
		if (kflg > 1)
			zap(filename);
	}
	if (st_size != r->st.st_size) {
		add_mail(filename, "SIZE has changed from %ld to %ld.\n",
			st_size, r->st.st_size);
		if (kflg > 1)
			zap(filename);
	}
	if (st_mtime != r->st.st_mtime) {
		sprintf(tmp, "%s", ctime(&st_mtime));
		tmp[strlen(tmp) - 1] = '\0';
		add_mail(filename, "MTIME has changed from %s to %s",
			tmp, ctime(&r->st.st_mtime));
	}
#ifndef hpux /* The file(1) command do modify the c_time of files. */
	if (st_ctime != r->st.st_ctime) {
		sprintf(tmp, "%s", ctime(&st_ctime));
		tmp[strlen(tmp) - 1] = '\0';
		add_mail(filename, "CTIME has changed from %s to %s",
			tmp, ctime(&r->st.st_ctime));
	}
#endif
	/* FIX */
	if (st_acl != r->st.st_acl) {
		add_mail(filename, "ACL has changed: something was %s.\n",
			(st_acl ? "removed" : "added"));
	}
#ifdef MD4
	if (strcmp(digest, r->digest))  {
		add_mail(filename, "MD4 had changed from %s to %s.\n",
			digest, r->digest);
		if (kflg > 1)
			zap(filename);
	}
#else /* Defaults to CRC32 */
	if (strcmp(CRC32, r->CRC32)) {
		add_mail(filename, "CRC32 has changed from %s to %s.\n",
			CRC32, r->CRC32);
		if (kflg > 1)
			zap(filename);
	}
#endif
	if (nerror && kflg == 1) /* stdout */
		printf("\n");

	return(nerror ? 1 : 0);
}

/*
 * If file is SUID or SGID remove s-bits.
 */
void	zap(filename)
char	*filename;
{
	struct	stat	s;
	FILE		*fp;

	if (nzap++)
		return;
	if (stat(filename, &s) == -1)
		die("Cannot stat() %s\n", filename);
	if (!(s.st_mode & (S_ISUID | S_ISGID)))
		return;
	if (chmod(filename, s.st_mode & ~(S_ISUID | S_ISGID)) == -1)
		die("Cannot change mode of %s!\n", filename);
	/* Log to file. */
	if ((fp = fopen(tmpfilename, "a")) == NULL)
		die("Cannot open %s\n", tmpfilename);
	(void) fprintf(fp, "ZAPPED: to restore perms: /bin/chmod %o %s\n",
		s.st_mode & 07777, filename);
	(void) fclose(fp);
	if (openlog(pname, LOG_PID | LOG_CONS, LOG_AUTH) == -1)
		die("openlog() error.\n");
	if (syslog(LOG_ALERT | LOG_AUTH,
	    "Zapped dangerous file: %s (host: %s)", filename, hostname) == -1)
		die("syslog() error.\n");
	if (closelog() == -1)
		die("closelog() error.\n");
}

/*
 * Append output to a file for further mailing via mail(1).
 * Last invocation must have a NULL as first argument to deliver
 * mail and delete temporary file.
 */
/*VARARGS*/
void	add_mail(filename, a, b, c, d, e, f)
char	*filename;
char	*a, *b, *c, *d, *e, *f;
{
	FILE		*fp;
	char		*p;

	if (kflg == 1)
		goto stdoutmode;
	if (!filename && !tmpfilename[0]) /* should never happen */
		die("Internal error @ add_mail()\n");
	/* It is the first file we get an error. Init temporary file. */
	if (!tmpfilename[0]) {
		int	t;

		sprintf(tmpfilename, "%s/%sXXXXXX", TMPDIR, pname);
		if ((p = mktemp(tmpfilename)) == NULL)
			die("mktemp() failed");
		strcpy(tmpfilename, p);
		if ((fp = fopen(tmpfilename, "a")) == NULL)
			die("Cannot open %s\n", tmpfilename);
		t = time(NULL);
		/* Mail subject. */
		(void) fprintf(fp, "Subject: %s security report @%s\n",
			pname, hostname);
		(void) fprintf(fp, "\n#### `%s' security report @%s on %s\n\n",
			pname, hostname, ctime(&t));
		(void) fclose(fp);
	}
	/* Closing up. Delete temporary file. */
	else if (!filename) {
		sendmail();
		if (unlink(tmpfilename))
			die("Cannot unlink temporay file %s !\n", tmpfilename);
		tmpfilename[0] = '\0';
		return;
	}

	/* Log to file. */
	if ((fp = fopen(tmpfilename, "a")) == NULL)
		die("Cannot open %s\n", tmpfilename);
	if(!nerror)
		(void) fprintf(fp, "\n#### Problems with ``%s'':\n", filename);
	(void) fprintf(fp, a, b, c, d, e, f);
	(void) fclose(fp);
	nerror++;
	errors++;
	return;

stdoutmode:
	/* Log to stdout. */
	if (!nerror)
		printf("\n#### Problems with ``%s'':\n", filename);
	printf(a, b, c, d, e, f);
	nerror++;
	errors++;
	return;
}

/*
 * Ask for confirmation.
 * Returns: boolean
 */
int	confirm()
{
	struct termio	tty, otty;
	int		c;

	printf("Are you sure? ");
	fflush(stdout);
	
	if (ioctl(0, TCGETA, &tty) == -1)
		die("Cannot TCGETA stdin\n");
        otty = tty;
        tty.c_lflag &= ~(ECHO | ECHOK | ECHOE | ICANON);
        tty.c_lflag |= ECHONL;
        tty.c_cc [VMIN] = 1;
        tty.c_cc [VTIME] = 1;
        if (ioctl (0, TCSETA, &tty) == -1)
		die("Cannot TCGETA stdin\n");
	while ((c = getchar()) != 'y' && c != 'n' && c != EOF)
		putchar('\007');
	if (ioctl (0, TCSETA, &otty) == -1)
		die("Cannot TCSETA stdin\n");
	printf("\n");
	if (c == 'y')
		return(1);
	else
		return(0);
}

/*
 * Ask for a key twice, to avoid mistyping.
 * Returns: the keyword
 */
char	*askforkey(prompt)
char	*prompt;
{
	char		*p;
	static char	buf[MAXPWLEN];
	int		i;

	for (i = 0; ; i++) {
		strcpy(buf, getlongpass(prompt));
		if (!strcmp(buf, (p = getlongpass("Retype key:"))))
			break;
		if (i == 2)
			die("Too many failures. Try later.\n");
		fprintf(stderr, "Keys do not match. Try again.\n\n");
	}
	/* Wipe out key from memory */
	memset(p, 0, MAXPWLEN); /* Erase key */
	return(buf);
}

/*
 * Acts like getpass(3), but gets longer strings.
 * 
 * Returns: char pointer to password.
 */
char	*getlongpass(prompt)
char	*prompt;
{
	int		i, fd, len;
	struct termio	t;
	unsigned short	save;
	static char	buf[MAXPWLEN];

	if ((fd = open("/dev/tty", O_RDWR)) == -1)
		die("Cannot open /dev/tty\n");
	(void) write(fd, prompt, strlen(prompt));
	if (ioctl(fd, TCGETA, &t) == -1)
		die("TCGETA /dev/tty");
	save = t.c_lflag;
	t.c_lflag &= ~ECHO;
	if (ioctl(fd, TCSETAW, &t) == -1)
		die("TCSETAW /dev/ttyn");
	for(i = 0; i < sizeof(buf); i++) {
		if((len = read(fd, &buf[i], sizeof(char))) < sizeof(char))
			break;
		if(buf[i] == '\n') {
			if (write(fd, &buf[i], sizeof(char)) == -1)
				die("write to /dev/tty\n");
			break;
		}
	}
	if (len == -1)
		die("read() from /dev/tty\n");
	for(; i < sizeof(buf); i++)
		buf[i] = '\0';

	t.c_lflag = save;
	if (ioctl(fd, TCSETAF, &t) == -1)
		die("TCSETAF /dev/tty\n");
	close(fd);
	return(buf);
}

/*
 * Send security report.
 */
static void	sendmail()
{
	char	buf[256];
	int	rc;

	sprintf(buf, "%s %s < %s", MAILER, RECIPIENTS, tmpfilename);
	rc = system(buf);
	if (rc & 0177 || rc >> 8) /* If got a signal or exit code != 0 */
		die("%s failed!\n", MAILER);
}
