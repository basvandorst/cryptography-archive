/*
 * $Author: vince $
 * $Header: /staff/vince/src/atp/RCS/atp.h,v 1.13 1992/05/15 13:27:05 vince Exp vince $
 * $Date: 1992/05/15 13:27:05 $
 * $Revision: 1.13 $
 * $Locker: vince $
 */

#ifndef	lint
static char *rcsid = "@(#) $Id: atp.h,v 1.13 1992/05/15 13:27:05 vince Exp vince $";
#endif	lint

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "md4.h"
#include "libdes/des.h"

/**************************/
/*** CUSTOMIZABLE STUFF ***/
/**************************/

/* Locate all SUID/SGID files. */
#define FIND		"/bin/find / -hidden -type f"
#define EDITOR		"/usr/bin/vi"
#define VALUABLE	"crack fuck pass login key vince root opr"
#define STRINGS		"/usr/bin/strings"

/*
 * DATABASE and TMPDIR must be in the same filesystem in order
 * to use the rename(2) system call. TMPDIR should be a directory
 * not writeable by others. Also, both files must be `vieable' by
 * all the hosts involved.
 */
#define DATABASE	"/staff/vince/etc/database"
#define TMPDIR		"/staff/vince/etc"

/*
 * The MAILER command should support multiple recipients as
 * argument and should understand the `subject: xxx' syntax.
 */
#define RECIPIENTS	"vince"
#define MAILER		"/bin/rmail"	/* Equivalent to /bin/mail */

/*
 * SUID files with uid > UIDLIMIT or SGID files with gid > GIDLIMIT
 * are not considered dangerous and therefore discarted.
 */
#define UIDLIMIT	170
#define GIDLIMIT	13

#define KEYWORDVAR	"KEYWORD"	/* Environment keyword-var name */

/*********************************/
/*** END OF CUSTOMIZABLE STUFF ***/
/*********************************/

#define ISREMARKABLE()	((s.st_mode & S_ISUID && s.st_uid <= UIDLIMIT) || \
			(s.st_mode & S_ISGID && s.st_gid <= GIDLIMIT))
/* Used by fprintf() for database creation */
#define FILEPAT		"%s %lu %u %u %ld %ld %ld %ld %ld %u %s\n"
#define FNL		256 		/* Max File Name Lenght */
#define ENTRYLEN	512		/* Max Lenght of an entry line */
#define MAXPWLEN	128
#define STDOUTMODE	1			/* Stadout mode */

typedef struct {	/* File stat information */
	char		filename[FNL];
	struct stat	st;
#ifdef MD4
	unsigned char	digest[33];
#else /* Defaults to CRC32 */
	char		CRC32[9];
#endif
} entry;

typedef struct {
	long		size;
	long		datestamp;
	unsigned char 	digest[16];
	unsigned char	unused[8];
} header;

entry		*getentry();
char		*getlongpass(), *pmode(), *askforkey();
unsigned char	*getdig(), *getIV();
extern int	do_checksum();

/* Crypt stuff */
MD4_CTX         mdContext;	/* RSA MD4 */
#ifndef MD4
long		crc32val;
#endif
