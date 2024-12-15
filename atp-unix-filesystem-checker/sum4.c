/*
 * $Author: vince $
 * $Header: /staff/vince/src/atp/RCS/sum4.c,v 1.5 1992/02/13 10:55:26 vince Exp vince $
 * $Date: 1992/02/13 10:55:26 $
 * $Revision: 1.5 $
 * $Locker: vince $
 * $Log: sum4.c,v $
 * Revision 1.5  1992/02/13  10:55:26  vince
 * AutoDeposit
 *
 * Revision 1.4  1992/02/12  18:52:11  vince
 * AutoDeposit
 *
 * Revision 1.3  1992/02/12  17:53:10  vince
 * AutoDeposit
 *
 * Revision 1.2  1992/02/12  09:17:12  vince
 * AutoDeposit
 *
 * Revision 1.1  1992/02/11  11:47:54  vince
 * Initial revision
 *
 */

#ifndef	lint
static char *rcsid = "@(#) $Id: sum4.c,v 1.5 1992/02/13 10:55:26 vince Exp vince $";
#endif	lint

#include <dirent.h>
#include <errno.h>
#include "atp.h"

static void	calcfilechecksum();

int		main(argc, argv)
int		argc;
char		*argv[];
{
	int		i;
	DIR		*dir;
	struct dirent	*dent;
	char		buf[_MAXNAMLEN * 2 + 2];
	struct	stat	s;

	if (argc < 2) {
		if (do_checksum(0))
			perror("read()"), exit(1);
#ifdef MD4
			printf("%s\n", getdig(&mdContext.digest));
#else
			printf("%-8lx\n", crc32val);
#endif
		exit(0);
	}
	
	for (i = 1; i < argc; i++) {
		if ((dir = opendir(argv[i])) == NULL) {	
			if (errno == ENOTDIR)
				calcfilechecksum(argv[i]);
			else
				perror(argv[i]);
			continue;
		}
		while ((dent = readdir(dir)) != NULL) {
			sprintf(buf, "%s/%s", argv[i], dent->d_name);
			if (stat(buf, &s) == -1)
				perror(buf), exit(1);
			if (!(s.st_mode & S_IFREG)) /* Skip unordinary files */
				continue;
			calcfilechecksum(buf);
		}
		(void) closedir(dir);
	}
	return(0);
}

static void	calcfilechecksum(filename)
char		*filename;
{
	int		fh;

	if ((fh = open(filename, O_RDONLY)) == -1) {
		perror(filename);
		return;
	}
	if (do_checksum(fh))
		perror(filename);
	(void) close(fh);
#ifdef MD4
	printf("%s  %s\n", getdig(&mdContext.digest), filename);
#else
	printf("%-8lx  %s\n", crc32val, filename);
#endif
}
