/* kfs_int.h (kernel private stuff)
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

#ifndef __kfs_kfs_int_h__
#define __kfs_kfs_int_h__

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/vnode.h>
#include <sys/vfs.h>
#include <sys/vfs_stat.h>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/buf.h>
#include <sys/kernel.h>
#include <sys/mman.h>
#include <sys/proc.h>
#include <sys/pathname.h>
#include <sys/dirent.h>
#include <sys/conf.h>
#include <sys/debug.h>
#include <sys/unistd.h>
#include <sys/mount.h>
#include <sys/syslog.h>

#include <vm/seg.h>

#include "kfs.h"

typedef struct _kfs_mntent	kfs_mntent;
typedef struct _kfs_keyinfo	kfs_keyinfo;
typedef struct _kfs_knode	kfs_knode;

#include "kfs_acl.h"


/*
 * all data files get this header attached
 * (was conveniently 32 bytes long -cj)
 */

/*
 * %%% find correct place for these
 */

#define MD5_BUF		(16)
#define KEY_BUF		(256)		/* sizeof DES-table(asc_key) */

typedef struct {
	struct {
		u_long		magic;				/* pick a magic number */
		u_short		header_len;			/* length of this header */
		u_short		type;				/* encryption method */
		kfs_quad	file_len;			/* length of file */
		u_char		iv[4];				/* put partial IV here so that a header
								 * swap will invalidate data */
		u_char		_dummy[12];
		aclist		aclist;
	} info;
	u_char		md5hash[MD5_BUF];
} kfs_file_hdr;

enum kfs_crypt_e { KFS_CRYPT_NONE, KFS_CRYPT_1 };

#define KFS_FMAGIC		((u_long) 0x21222324)


/*
 * a keynode is shared by all active dunodes of a user
 */

struct _kfs_keyinfo {
	kfs_keyinfo	*next;
	u_long		refs;
	uid_t		uid;
	void		*key_val;
	int		key_len;
};


/*
 * client's handle of a remote file
 */

struct _kfs_knode {
	struct _kfs_knode	*knodenext, *knodeprev;
	char			*name;				/* has '\0' terminator (just so printf will work) */
	int			name_len;
	kfs_mntent		*me;				/* == vfs2me(dunode.v.v_vfsp) */
	kfs_keyinfo		*key;
	long			fhdr_dirty;			/* true if fhdr needs to be written out */
	kfs_file_hdr		fhdr;
	uid_t			uid;				/* move these into fhdr */
	gid_t			gid;
	struct vnode		*realvp;
	struct vnode		v;
};

#define k2v(_f_)		(&(_f_)->v)
#define k2rv(_f_)		((_f_)->realvp)
#define v2k(_v_)		((kfs_knode *) (_v_)->v_data)

#define vfs2me(_vfs_)		((kfs_mntent *) (_vfs_)->vfs_data)

/*
 * one of these per local mount point describes the mount point
 */

struct _kfs_mntent {
	struct _kfs_mntent	*next;				/* list of all KFS mounts */
	int			fixedmajor;
	u_long			mntid;				/* should be unique, non repeating */
	char			*remotepath;
	int			remotepath_len;
	kfs_knode		*rootknode;
	/* kfs_knode		*root; */
	struct vfs		*vfsp;
	long			node_count;
	kfs_knode		*knodelist;			/* list of all knodes in this mnt pt */
	kfs_keyinfo		*keylist;			/* linked list of keys
								 * %%% replace with hashtab */
};

extern kfs_mntent	*kfs_mntlist;


extern int		kfs_trace;


/*
 * from kfs_subr.c
 */

extern kfs_knode *kfs_knode_find(kfs_mntent *, struct vnode *vp,
				const char *dname, int dname_len,
				const char *fname, int fname_len);

extern kfs_mntent *kfs_mntent_new(void);
extern void kfs_mntent_free(kfs_mntent *);

extern kfs_keyinfo *kfs_keyinfo_new(uid_t uid, const char *key);
extern void kfs_keyinfo_change(kfs_keyinfo *, const char *key);
extern kfs_keyinfo *kfs_keyinfo_hold(kfs_keyinfo *);
extern void kfs_keyinfo_rele(kfs_keyinfo *);

extern void kfs_addmnt(kfs_mntent *);

extern kfs_knode *kfs_knode_new(kfs_mntent *, struct vnode *vp,
				const char *dname, int dname_len,
				const char *fname, int fname_len);
extern void kfs_knode_pick_key(kfs_knode *, const struct ucred *uc);
extern int kfs_load_fhdr(kfs_knode *, struct ucred *);
extern int kfs_write_fhdr(kfs_knode *, struct ucred *);
extern void kfs_set_fhdr_crypt(kfs_knode *);
extern void kfs_set_fhdr_nocrypt(kfs_knode *);
extern void kfs_knode_free(kfs_knode *);
extern void kfs_knode_rele(kfs_knode *);
extern void kfs_knode_hold(kfs_knode *);

extern void *kfs_xalloc(u_long sz);
extern void *kfs_xzalloc(u_long sz);
extern void kfs_free(void *pt, u_long sz);

/*
 * from kfs_vnodeops.c
 */

extern struct vnodeops	kfs_vnodeops;


extern int kfs_uioread_cb(kfs_knode *f, struct uio *uiop, struct ucred *);
extern int kfs_uiowrite_cb(kfs_knode *f, struct uio *uiop, struct ucred *);

extern int kfs_read_1cb(kfs_knode *f, off_t offset, long *buf, struct ucred *);
extern int kfs_write_1cb(kfs_knode *f, off_t offset, long *buf, struct ucred *);

extern int kfs_vpread(struct vnode *, void *buf, long *resid, off_t loffset, struct ucred *);
extern int kfs_vpwrite(struct vnode *, const void *vbuf, long resid, off_t offset, struct ucred *);


/*
 * other random stuff
 */

extern void bzero(void *, int);
extern int uiomove(void *, int, int, struct uio *);
extern int printf(const char *, ...);
extern int uprintf(const char *, ...);
extern int copyin(const void *, void *, int);
extern int copyout(const void *, void *, int);
extern void bcopy(const void *, void *, int);
extern int bcmp(const void *, const void *, int);
extern void panic(const char *);
extern void kmem_free(void *, int);
extern int lookupname(char *fnamep, int seg, enum symfollow followlink,
		struct vnode **dirvpp, struct vnode **compvpp);
extern int vfs_fixedmajor(struct vfs *);
extern void assfail(char *, char *, int);
extern void uniqtime(struct timeval *);

#endif /* __dufs_dufs_int_h__ */
