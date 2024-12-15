/* kfs.h
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

#ifndef __kfs_kfs_h__
#define __kfs_kfs_h__

#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioccom.h>


/* %%% these should be renamed/renumbered */

#define FIO_KFS_GETACL		_IO(k, 1)			/* get ACL of object */
#define FIO_KFS_SETACL		_IO(k, 2)			/* set ACL of object */
#define FIO_KFS_ADDACL		_IO(k, 3)			/* add ACL of object */


typedef enum {
	KFS_MNT_VOID = 0,
	KFS_MNT_MOUNT,
	KFS_MNT_SETKEY,
	KFS_MNT_TRACE,
	KFS_MNT_STATUS,
} kfs_mntop;

typedef struct {
	kfs_mntop		op;
	char			remotepath[MAXPATHLEN];		/* remote dir to mount ('\0' terminated) */
	int			remotepath_len;
	struct {
		u_long	hostname	: 1;
		u_long	dummy;
	} flags;
	/* --- */
	
} kfs_mntarg;

typedef struct {
	kfs_mntop	op;
	u_long		node_count;		/* return arg */
	long		uid;
	long		key_len;
	char		key[1024];
} kfs_remntarg;

typedef struct {
	long	hi;
	u_long	lo;
} kfs_quad;

#endif /* __kfs_kfs_h__ */
