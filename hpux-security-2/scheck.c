/*
 * Copyright (c) 1990 Marco Negri & siLAB Staff
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the authors not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission of the authors.
 * The authors makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Please send any improvements, bug fixes, useful modifications, and comments
 * to security@ghost.unimi.it
 */
/*
 * @(#) scheck.c	: Check the s-bit on root.	(Rel. 2.3)
 * @(#) scheck.c	: By Marco		Milano, 10/89
 * @(#) scheck.c	: By Marco		Milano, 12/90
 *
 * Usage	:
 *	Start first	: scheck -m		# For Make Data Base
 *	& in crontabs 	: scheck		# For checking
 *	
 * Miscellaneous Options:
 *	-f <filename>	# data base file (Default /usr/etc/security/s_perm)
 *	-n		# Don't start find. Get file list from stdin
 *			# Useful to check deamon binary
 *	-u <uid>	# The minimal Interest Uid (Default 170)
 *	-g <uid>	# The minimal Interest Gid (Default 12)
 *	-U <file>	# Update Data Base
 *
 * Crontabs Example :
 * 30 23 * * 1-5		exec /usr/local/bin/scheck
 *
 * Alghoritms	:
 *	Search, with "find", all s-bit, and check it with "/etc/s_perm"
 *	that contains all legal s-bit.
 *
 * Customizations :
 * 	ROOT		Directory where start search
 */
/*
 * $Author: marco $
 * $Id: scheck.c,v 2.6 1991/11/26 12:23:29 marco Exp marco $
 * $Date: 1991/11/26 12:23:29 $
 * $Revision: 2.6 $
 */

#ifndef	lint
static char *rcsid = "@(#) $Id: scheck.c,v 2.6 1991/11/26 12:23:29 marco Exp marco $";
#endif	lint

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "hash.h"
#include "config.h"

#define	MKSTR(b)	strcpy(malloc(strlen(b) + 1), b)

			/*
			 * SPERM (/etc/s_perm)
			 *	File Format (scanf/printf)
			 *	%hu	User  ID
			 *	%hu	Group ID
			 *	%ho	File Mode
			 *	%ld	File Modification Time
			 *	%ld	File size
			 *	%ld	Inode Number
			 *	%ld	Device Number
			 *	%lu	File checksum
			 *	%s	File Path-Name
			 */
#define	SPERM_FMT	"%hu %hu %ho %ld %ld %ld %ld %lu %s"

					/* Command to Send Mail to SysMng */
#define	MAILLOG		"Scheck-Mail"
#define	MAILHDR		"ILLEGAL BIT-S"

#ifndef	DEBUG

					/* Command to find ALL bit-s	 */
#define	ROOT	"/"
#define	PERM	"-fstype hfs -type f -perm -04000 \\( -perm -00010 -o -perm -00001 \\)"
#ifdef	hpux
#define	PRINT	"-print -hidden"
#else	hpux
#define	PRINT	"-print"
#endif	hpux

#else	DEBUG
					/* DEBUG Mode Parameters */
#define	SPERM		"./s_perm"
#define	ROOT		"."
#define	PERM		"-perm -04000 \\( -perm -00010 -o -perm -00001 \\)"
#define	PRINT		"-print"

#endif	DEBUG

	/*
	 * Possible Error (Bit - Oriented)
	 */
#define	NEWFILE		0001	/* New Made File		*/
#define	CUID		0002	/* Changed User  Id		*/
#define	CGID		0004	/* Changed Group Id		*/
#define	CMODE		0010	/* Changed mode			*/
#define	CTIME		0020	/* Changed Modification Time	*/
#define	CSIZE		0040	/* Changed Size Time		*/
#define	CINO		0100	/* Changed Ino or dev		*/
#define	CCHECKSUM	0200	/* Changed Checksum Time	*/
#define	WRITABLE	0400	/* Warning Writeble File	*/


	/*
	 * Flags
	 */
int	testmode	= 1;		/* Test Mode Flag		 */
int	minuid		= MINIMAL_UID;
int	mingid		= MINIMAL_GID;

int	update		= 0;
int	nfault		= 0;

#define	MAXFAULTS	128

#ifdef	MAXFAULTS

static char	*cpstr();
static void	fault_print();

static struct fault {
	char	*name;
	int	mode;
} fault_des[MAXFAULTS];

int	fault_pos	= -1;

#endif	MAXFAULTS

FILE	*popen();

extern int	optind;
extern int	opterr;
extern char	*optarg;

#define	BSIZE		8192		/* Size of checksum buffer	*/

#define	MAXUPDATE	128		/* Max number of update	args	*/

u_long	getchksum();
void	f_test();
char	*tool;
char	*sperm_file	= SPERM;
int	find_file	= 1;

extern int	read();

#ifdef	S_IFLNK
#	ifndef	S_ISLNK
#		define	S_ISLNK(mode)	((mode & S_IFMT) == S_IFLNK)
#	endif

extern int	readlink();

#define	READ(st)	((S_ISLNK((st)->st_mode)) ? readlink : read)

#else

#define	READ(st)	read

#endif

main(argc, argv)
int	argc;
char	*argv[];
{
	FILE		*ffind,
			*fperm,
			*fopen();

	char		*fgets();
	char		buf[BUFSIZ];
	char		*strcpy();
	register int	c;
	register int	i;
	char		*upd_list[MAXUPDATE];


	opterr	= 1;
	tool	= argv[0];
	while ((c = getopt(argc, argv, "nf:U:mu:g:")) != EOF)
		switch (c) {
			case	'n':
				find_file	= 0;
				break;

			case	'f' :
				sperm_file	= optarg;
				break;

			case	'U' :
				if (*optarg != '/')
					Usage(tool, "-U : Only Absolute Path");

				upd_list[update++]	= optarg;

				if (update >= MAXUPDATE)
					Usage(tool, "Too many -U Option");

				testmode	= 0;

				break;

			case	'm' :
				testmode = 0;
				break;

			case	'u' :
				minuid	= atoi(optarg);
				break;

			case	'g' :
				mingid	= atoi(optarg);
				break;
			
			default	:
				Usage(tool, "Illegal Option");
		}

	if (optind != argc) 
		Usage(tool, "Too Many Argument");

		/*
		 * Open Data Base.
		 */
	Hinit();
	if ((fperm = fopen(sperm_file, "r")) != NULL) {
		/*
		 * Load DataBase (hashed)
		 */
		Hload(fperm);

		(void) fclose(fperm);
	}

	for (i = 0; i < update; i++)
		f_test(upd_list[i]);

		/*
		 * ReOpen Data Base.
		 */
	if ((fperm = fopen(sperm_file, (testmode) ? "r" : "w")) == NULL) {
		perror(sperm_file);
		exit(1);
	}

		/*
		 * Start Find Command
		 */
	if (update == 0) {
		if (find_file) {
			sprintf(buf, "%s %s %s %s", FIND, ROOT, PERM, PRINT);
			if ((ffind = popen(buf, "r")) == NULL) {
				perror("popen()");
				exit(1);
			}
		} else
			ffind	= stdin;

		/*
		 * Start check/make loop
		 */
		while (fgets(buf, BUFSIZ, ffind) != NULL) {
			buf[strlen(buf) - 1] = '\0';
			f_test(buf);
		}
		fclose(ffind);
	}

	if (! testmode)
		Hprint(fperm);

	fclose(fperm);
	if (nfault) {
#ifdef	MAXFAULTS
		fault_print();
#endif	MAXFAULTS
		a_send(MAILLOG, MAILHDR);
	}

	return	nfault;
}

Hload(fp)
FILE	*fp;
{
	char	buf[BUFSIZ],
		name[BUFSIZ];
	u_short	uid,
		gid,
		mode;
	time_t	mtime;
	u_long	size,
		checksum;
	long	ino,
		dev;

	while (fgets(buf, BUFSIZ, fp) != NULL) {
		sscanf(buf, SPERM_FMT, &uid, &gid, &mode, &mtime, 
					&size, &ino, &dev, &checksum, name);
		Hinsert(MKSTR(name), uid, gid, mode, mtime, size, 
							ino, dev, checksum);
	}
}

test(st, name)
struct stat	*st;
char		*name;
{
	int	err	= 0;
	hash	p, Hfind();


	if ((p = Hfind(name)) == NULL)
		err	|= NEWFILE;
	else {
		if (st->st_uid != p->uid)
			err	|= CUID;

		if (st->st_gid != p->gid)
			err	|= CGID;

		if ((st->st_mode & ~S_IFMT) != p->mode)
			err	|= CMODE;

		if (st->st_mtime != p->mtime)
			err	|= CTIME;

		if (st->st_size != p->size)
			err	|= CSIZE;

		if (st->st_ino != p->ino || st->st_dev != p->dev)
			err	|= CINO;

		if (getchksum(name, READ(st)) != p->checksum)
			err	|= CCHECKSUM;
	}

	if ((st->st_mode & 02) || ((st->st_mode & 020) && st->st_gid >= mingid))
		err	|= WRITABLE;

#ifdef	DEBUG
	printf("Test %s : %o %s\n", name, err, (err) ? "BAD" : "GOOD");
#endif	DEBUG

	return	err;
}

err_print(err, name)
int	err;
char	*name;
{
	a_msg("Problems : ");
	if (err & NEWFILE)
		a_msg("NewFile, ");

	if (err & CUID)
		a_msg("UID, ");

	if (err & CGID)
		a_msg("GID, ");

	if (err & CMODE)
		a_msg("MODE, ");

	if (err & CTIME)
		a_msg("Mod. Time, ");

	if (err & CSIZE)
		a_msg("Mod. Size, ");

	if (err & CINO)
		a_msg("Mod. inumber or dev, ");

	if (err & CCHECKSUM)
		a_msg("Mod. Checksum, ");

	if (err & WRITABLE)
		a_msg("WRITABLE, ");

	return a_msg("%s\n", name);
}

Usage(prg, err)
char	*prg;
char	*err;
{
	fprintf(stderr, "%s	: %s\n", prg, err);
	fprintf(stderr, 
	"Usage	: %s [-f db_file] [-mn] [-u <num>] [-g <num>] [-U <file>]\n",
						prg);
	exit(1);
}

u_long	getchksum(name, readfun)
char	*name;
int	(*readfun)();
{
	int		fd;
	register int	r;
	register u_char	*p;
	register u_long	chksum	= 0;
	u_char		buf[BSIZE];

	if ((fd = open(name, O_RDONLY)) == -1) {
		perror(name);
		return -1;
	}

	while ((r = (*readfun)(fd, buf, sizeof(buf))) > 0)
		for (p = buf; r-- > 0; chksum += *p++);

	(void) close(fd);

	return chksum;
}

void	f_test(name)
char	*name;
{
	struct stat	st;
	int		err;

#ifdef	S_IFLNK
	if (lstat(name, &st) == -1)
#else
	if (stat(name, &st) == -1)
#endif
	{
		perror(name);
		exit(1);
	}

	if (find_file && testmode) {
			/*
			 * Test UID
			 */
		if (st.st_uid >= minuid)
			return;
	
			/*
			 * Test GID
			 */
		if (! (st.st_mode & 00001) && st.st_gid >= mingid)
			return;
	}

	if (! testmode) {
		/*
		 * Make Data Base
		 */
		Hinsert(MKSTR(name), st.st_uid, st.st_gid,
				 st.st_mode & ~S_IFMT,
				 st.st_mtime, st.st_size, 
				 st.st_ino, st.st_dev,
				 getchksum(name, READ(&st)));
		return;
	}

	/*
	 * Test Legality of file
	 */
	if ((err = test(&st, name)) == 0)
		return;

	/*
	 * Advise Sys Mng
	 */
	nfault++;
	if (err_print(err, name) <= 0) {
		a_msg("%s : Old mode %ho, uid %hd, gid %hd\n", name,
				st.st_mode, st.st_uid, st.st_gid);
		if (chmod(name, st.st_mode & ~(S_ISUID | S_ISGID)) == -1)
			a_msg("%s : Change permission failed\n", name);
		else {
			a_msg("%s : Changed permission.\n", name);
#ifdef	MAXFAULTS
			if (fault_pos == MAXFAULTS)
				fault_print();
			fault_pos++;
			fault_des[fault_pos].name	= cpstr(name);
			fault_des[fault_pos].mode	= st.st_mode & 07777;
#else	MAXFAULTS
			a_msg("chmod %ho %s; scheck -U %s\n", 
					(st.st_mode & 07777), name, name);
#endif	MAXFAULTS
		}
	}
}

Hprint(fp)
FILE	*fp;
{
	extern hash	Htable[HASHSIZE];
	register hash	*p, l;

	for (p = Htable; p < &Htable[HASHSIZE]; p++)
		for (l = *p; l; l = l->next) {
			fprintf(fp, SPERM_FMT,	l->uid, l->gid, l->mode,
						l->mtime, l->size, l->ino,
						l->dev, l->checksum,
						l->name);
			putc('\n', fp);
		}
}

#ifdef	MAXFAULTS
static char	*cpstr(s)
register char	*s;
{
	register char	*p	= (char *) malloc(strlen(s) + 1);
	extern char	*strcpy();

	if (p == NULL) {
		perror("malloc()");
		a_msg("PANIC	: Malloc\n");
		a_send(MAILLOG, MAILHDR);
		exit(1);
	}

	return strcpy(p, s);
}

static void	fault_print()
{
	register struct fault	*f	= fault_des;

	if (fault_pos == -1)
		return;

	a_msg("Changed permission. To restore exec :\n");
	a_msg("#!/bin/sh\n\n");

	while (fault_pos-- > 0) {
		a_msg("chmod 0%ho %s\nscheck -U %s\n\n", 
						f->mode, f->name, f->name);
		free(f->name);
		f++;
	}
	fault_pos	= -1;
}

#endif	MAXFAULTS
