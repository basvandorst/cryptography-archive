/* getacl.c -- get ACL of a file.
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
#include <sys/fcntl.h>

#include "kfs.h"
#include "kfs_acl.h"

extern char *index(char *, char);
extern void perror(const char *msg);
extern int printf(char *fmt, ...);

extern int syscall(int, ...);


static void printacl(const aclist *acl);


char		*pname;
static int	verbose_flag = 0;


int
main(int ac, char *av[])
{
int	a;
aclist	acl;
int	errs = 0;

	if( (pname = index(av[0], '/')) )
		++pname;
	else
		pname = av[0];

	for( a = 1; a < ac ; ++a ) {
		if( verbose_flag )
			printf("\"%s\":\n", av[a]);
		if( syscall(198, av[a], FIO_KFS_GETACL, &acl) < 0 ) {
			perror(av[a]);
			++errs;
			continue;
		}
		printacl(&acl);
	}

	return errs > 0? 1: 0;
}


static void
printacl(const aclist *acl)
{
int	i;
ac_ent	*e;

	printf("\tinuse %d:\n", acl->inuse);
	e = &acl->actab[0];
	for( i = 0; i < acl->inuse ; ++i, ++e ) {
		switch( e->id_type ) {
		case ACL_IDTYPE_UID:
			printf("\tuid %5d, ", e->id.uid);
			break;
		case ACL_IDTYPE_GID:
			printf("\tgid %5d, ", e->id.gid);
			break;
		default:
			printf("\tidtype %d, ", e->id_type);
		}
		switch( e->flags & ACL_FLAG_DOMASK ) {
		case ACL_FLAG_DOMODE:
			printf(" mode 0%03o", e->perm);
			break;
		case ACL_FLAG_DOGROUP:
			printf(" DOGROUP");
			break;
		case ACL_FLAG_DOUSER:
			printf(" DOUSER");
			break;
		default:
			printf(" DOxxx");
		}
		printf(", {");
		if( e->flags & ACL_FLAG_BLACK )
			printf(" BLACK");
		if( e->flags & ACL_FLAG_INHERIT )
			printf(" INHERIT");
		if( e->flags & ACL_FLAG_METAOP_SET )
			printf(" METAOP_SET");
		if( e->flags & ACL_FLAG_ACLOP_SET )
			printf(" ACLOP_SET");
		printf(" } (flags $%04x)\n", e->flags);
	}
}
