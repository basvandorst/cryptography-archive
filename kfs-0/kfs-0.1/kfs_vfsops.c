/* kfs_vfsops.c
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

#include "kfs_int.h"


static int kfs_mount(struct vfs *vfsp, const char *newpath, caddr_t data);
static int kfs_remount(kfs_mntent *me, kfs_remntarg *data);
static int kfs_unmount(struct vfs *vfsp);
static int kfs_root(struct vfs *vfsp, struct vnode **vpp);
static int kfs_statfs(struct vfs *vfsp, struct statfs *sbp);
static int kfs_sync(struct vfs *vfsp);
static int kfs_badop(void);
static int kfs_mountroot(struct vfs *vfsp, struct vnode **vpp, char *name);
static int kfs_swapvp(struct vfs *vfsp, struct vnode **vpp, char *name);


struct vfsops	kfs_vfsops = {
	kfs_mount,
	kfs_unmount,
	kfs_root,
	kfs_statfs,
	kfs_sync,
	kfs_badop,
	kfs_mountroot,
	kfs_swapvp
};


/*
 * kfs_mount
 *
 */

static int
kfs_mount(struct vfs *vfsp, const char *newpath, caddr_t data)
{
static u_long	fixed_mntid = 0;
kfs_mntarg	*ma;
kfs_mntent	*me;
int		rval;
struct vnode	*rvp;
kfs_knode	*k;

	if( kfs_trace )
		printf("kfs_mount(..., \"%s\", ...)\n", newpath);

	ma = 0;

	if( vfsp->vfs_flag & VFS_REMOUNT ) {
		rval = kfs_remount((kfs_mntent *) vfsp->vfs_data, (kfs_remntarg *) data);
		goto done;
	}

	ma = kfs_xalloc(sizeof(*ma));

	if( (rval = copyin(data, ma, sizeof(*ma))) != 0 )
		goto done;

	if( (rval = lookupname(ma->remotepath, UIO_SYSSPACE, FOLLOW_LINK, 0, &rvp)) != 0 )
		goto done;

	me = kfs_mntent_new();
	me->remotepath_len = ma->remotepath_len;			/* %%% check bounds */
	me->remotepath = kfs_xalloc(me->remotepath_len);
	bcopy(ma->remotepath, me->remotepath, me->remotepath_len);
	me->vfsp = vfsp;
	me->fixedmajor = vfs_fixedmajor(vfsp);
	me->mntid = ++fixed_mntid;

	kfs_addmnt(me);

	k = kfs_knode_new(me, rvp, me->remotepath, me->remotepath_len, 0, 0);
	k2v(k)->v_flag |= VROOT;

	me->rootknode = k;

	vfsp->vfs_bsize       = rvp->v_vfsp->vfs_bsize;
	vfsp->vfs_fsid.val[0] = me->mntid;
	vfsp->vfs_fsid.val[1] = 0x1234;			/* %%% */
	vfsp->vfs_data = (void *) me;			/* == vfs2me(vfsp) */
	rval = 0;
done:
	if( ma )
		kfs_free(ma, sizeof(*ma));
	return rval;
}


static int
kfs_remount(kfs_mntent *me, kfs_remntarg *data)
{
kfs_remntarg	*ma;
int		rval;
kfs_keyinfo	*k;

	ma = kfs_xalloc(sizeof(*ma));

	if( (rval = copyin(data, ma, sizeof(*ma))) != 0 )
		goto done;

	switch( ma->op ) {
	case KFS_MNT_SETKEY:
		if( u.u_cred->cr_uid != ma->uid && u.u_cred->cr_uid != 0 ) {
			rval = EPERM;
			break;
		}
		if( ma->key_len <= 0 ) {
			rval = EINVAL;
			break;
		}
		for( k = me->keylist; k ; k = k->next ) {
			if( k->uid == ma->uid ) {
				kfs_keyinfo_change(k, ma->key);
				break;
			}
		}
		if( k == 0 ) {
			k = kfs_keyinfo_new(ma->uid, ma->key);
			k->next = me->keylist;
			me->keylist = k;
		}

		if( kfs_trace )
			printf("kfs_remount: loaded (uid %u, key \"%s\")\n", (u_int) k->uid, (char *) k->key_val);
		break;
	case KFS_MNT_TRACE:
		kfs_trace = ma->uid;
		break;
	case KFS_MNT_STATUS:
		ma->node_count = me->node_count;
		rval = copyout(ma, data, sizeof(*ma));
		break;
	default:
		rval = EINVAL;
	}

done:
	kfs_free(ma, sizeof(*ma));
	return rval;
}


static int
kfs_unmount(struct vfs *vfsp)
{
kfs_mntent	*me, **prev;
/* kfs_srvent	*se; */
int		found;

	me = vfs2me(vfsp);
	/* se = me->se; */

	if( me->node_count != 1 || (me->rootknode && me->rootknode->v.v_count != 1) ) {
		if( kfs_trace )
			printf("kfs_unmount: me->node_count %ld, me->root->count %d\n", me->node_count,
								me->rootknode? me->rootknode->v.v_count: -999);
		return EBUSY;
	}

	found = 0;
	for( prev = &kfs_mntlist; *prev ; prev = &(*prev)->next ) {
		if( *prev == me ) {
			found = 1;
			break;
		}
	}
	if( !found ) {
		/* this should not happen */
		printf("kfs_unmount: mntent not in srvent\n");
		return 0;
	}

	*prev = me->next;
	kfs_mntent_free(me);

	return 0;
}


static int
kfs_root(struct vfs *vfsp, struct vnode **vpp)
{
kfs_mntent	*me;

	if( kfs_trace )
		printf("kfs_root: call\n");

	VFS_RECORD(vfsp, VS_ROOT, VS_CALL);

	me = vfs2me(vfsp);

	kfs_knode_hold(me->rootknode);
	*vpp = k2v(me->rootknode);

	if( kfs_trace )
		printf("kfs_root: -> vnode $%x (rvp $%x)\n", (int) *vpp, (int) k2rv(me->rootknode));

	return 0;
}


static int
kfs_statfs(struct vfs *vfsp, struct statfs *sbp)
{
	printf("kfs_statfs: unimplemented\n");
	return ENXIO;
}


static int
kfs_mountroot(struct vfs *vfsp, struct vnode **vpp, char *name)
{
	printf("kfs_mountroot: unimplemented\n");
	return ENXIO;
}


static int
kfs_swapvp(struct vfs *vfsp, struct vnode **vpp, char *name)
{
	printf("kfs_swapvp: unimplemented\n");
	return ENXIO;
}


/*
 * kfs_sync
 *
 * we don't have any buffers to write out, but we may need to write
 * the file headers of open files.
 * %%% do this as root for now.  we should probably do this as the
 * user that created the vnode in the first place.
 */

static int
kfs_sync(struct vfs *vfsp)
{
kfs_mntent	*me;
kfs_knode	*f;
struct ucred	*uc;
struct vfs	*last_vfsp;

	if( vfsp ) {
		me = vfs2me(vfsp);
		if( (f = me->knodelist) ) {
			uc = crget();				/* defaults to root */
			do {
				(void) VOP_FSYNC(k2v(f), uc);
				f = f->knodenext;
			} while( f != me->knodelist );
			crfree(uc);
		}
		last_vfsp = 0;
	}
	return 0;
}


static int
kfs_badop()
{
	panic("kfs_badop");
	return EIO;
}
