/* mount_kfs.c -- mount an encrypted filesystem.
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
#include <mntent.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mount.h>
#include <sys/stat.h>

#include "kfs.h"


extern void bzero(void *addr, int len);
extern int addmntent(FILE *, struct mntent *);
extern int fprintf(FILE *, const char *fmt, ...);
extern int mount(const char *type, const char *dir, int flags, void *data);
extern void perror(const char *msg);
extern char *rindex(char *, char);
extern char *strcat(char *, const char *);


/* static int get_addr(u_long *, u_short *); */


char	*pname;


int
main(int ac, char *av[])
{
char		opts[256], buf[256];
kfs_mntarg	kfsa;
char		*filesys;
char		*dir;
FILE		*mfp;
struct mntent	me;
struct stat	sb;
int		mflags;

	if( (pname = rindex(av[0], '/')) )
		++pname;
	else
		pname = av[0];

	filesys = av[1];
	dir = av[2];

	bzero(&kfsa, sizeof(kfsa));
	sprintf(kfsa.remotepath, "%s", filesys);
	kfsa.remotepath_len = strlen(kfsa.remotepath) + 1;

	/*
	 * bug in sys/os/vfs.c:
	 * can't remount if M_NOSUB set!
	 */

	/* set mount flags & mount filesystem */
	mflags = M_NEWTYPE /*|M_NOSUB*/ /* |M_MULTI */;
	if( mount("kfs", dir, mflags, &kfsa) < 0 ) {
		perror("mount");
		exit(1);
	}
#ifdef DEBUG
	fprintf(stderr, "%s: \"%s\" mounted on \"%s\"\n", pname, kfsa.remotepath, dir);
#endif
	/* update file system descriptor table */
	if( (mfp = setmntent(MOUNTED, "a")) == NULL ) {
		fprintf(stderr, "%s: can't update %s\n", pname, MOUNTED);
		exit(1);
	}
	me.mnt_fsname = filesys;
	me.mnt_dir = dir;
	me.mnt_type = "kfs";
	me.mnt_opts = opts;
	if( mflags & M_RDONLY )
		strcpy(me.mnt_opts, "ro");
	else
		strcpy(me.mnt_opts, "rw");
	if( stat(me.mnt_dir, &sb) < 0 ) {
		fprintf(stderr, "%s: WARNING: can't stat %s\n", pname, me.mnt_dir);
	} else {
		sprintf(buf, ",dev=%04x", sb.st_dev & 0xffff);
		strcat(me.mnt_opts, buf);
	}
	me.mnt_freq = 0;
	me.mnt_passno = 0;
	if( addmntent(mfp, &me) != 0 ) {
		fprintf(stderr, "%s: addmntent on %s failed\n", pname, MOUNTED);
		exit(1);
	}
	endmntent(mfp);

	exit(0);
}
