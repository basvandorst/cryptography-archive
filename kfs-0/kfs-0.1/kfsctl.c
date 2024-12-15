/* kfssctl.c -- set a user's password for kfs accesses (and more).
 *
 * Copyright (c) 1993 by William W. Dorsey
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#undef KERNEL

#include <stdio.h>

#include <sys/mount.h>

#include "kfs.h"

extern void bzero(void *addr, int len);
extern int fprintf(FILE *, const char *fmt, ...);
extern int getuid();
extern int mount(const char *type, const char *dir, int flags, void *data);
extern void perror(const char *msg);
extern int printf(char *fmt, ...);
extern char *rindex(char *, char);

char	*pname;


static void usage(void);
char *getphrase(char *prompt);


int
main(int ac, char *av[])
{
kfs_remntarg	kfsa;
char		*dir;
int		a;

	if( (pname = rindex(av[0], '/')) )
		++pname;
	else
		pname = av[0];

	dir = av[1];

	bzero(&kfsa, sizeof(kfsa));

	a = 2;
	if( strcmp(av[a], "key") == 0 ) {
		if (ac == 3) {
			strcpy(kfsa.key, getphrase("Enter pass phrase: "));
			kfsa.key_len = strlen(kfsa.key) + 1;
		}
		else if (ac == 4) {
			strcpy(kfsa.key, av[a+1]);
			kfsa.key_len = strlen(kfsa.key) + 1;
		}
		else {
			usage();
		}
		kfsa.op = KFS_MNT_SETKEY;
		kfsa.uid = getuid();
	} else if( strcmp(av[a], "nokey") == 0 ) {
		if (ac != 3)
			usage();
		kfsa.op = KFS_MNT_SETKEY;
		kfsa.uid = getuid();
		strcpy(kfsa.key, "");		/* XXX */
		kfsa.key_len = 1;
	} else if( strcmp(av[a], "trace") == 0 ) {
		if (ac != 3)
			usage();
		kfsa.op = KFS_MNT_TRACE;
		kfsa.uid = 1;
	} else if( strcmp(av[a], "notrace") == 0 ) {
		if (ac != 3)
			usage();
		kfsa.op = KFS_MNT_TRACE;
		kfsa.uid = 0;
	} else if( strcmp(av[a], "status") == 0 ) {
		if (ac != 3)
			usage();
		kfsa.op = KFS_MNT_STATUS;
		if( mount("kfs", dir, M_NEWTYPE|M_REMOUNT, &kfsa) < 0 ) {
			perror("mount");
			exit(1);
		}
		printf("kfs_nodes busy:  %lu\n", kfsa.node_count);
		return 0;
	} else
		usage();

	if( mount("kfs", dir, M_NEWTYPE|M_REMOUNT, &kfsa) < 0 ) {
		perror("mount");
		exit(1);
	}

	exit(0);
}


static void
usage()
{
	fprintf(stderr, "Usage: %s <KFS dir> [key <keyphrase>] [nokey]\n", pname);
	exit(1);
}
