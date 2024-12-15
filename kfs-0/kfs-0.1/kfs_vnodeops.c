/* kfs_vnodeops.c
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


/* *** */


static int
kfs_open(struct vnode **vpp, int flags, struct ucred *uc)
{
kfs_knode	*f;

	f = v2k(*vpp);

	if( kfs_trace )
		printf("kfs_open(\"%s\", flags $%x)\n", f->name, flags);

	VFS_RECORD((*vpp)->v_vfsp, VS_OPEN, VS_CALL);

	return VOP_OPEN(&k2rv(f), flags, uc);
}


/*
 * close - vnode overhead
 */

static int
kfs_close(struct vnode *vp, int flags, int count, struct ucred *uc)
{
kfs_knode	*f;

	f = v2k(vp);

	if( kfs_trace )
		printf("kfs_close(\"%s\")\n", f->name);

	VFS_RECORD(vp->v_vfsp, VS_CLOSE, VS_CALL);

	return VOP_CLOSE(k2rv(f), flags, count, uc);
}


/*
 * rdwr - data OP
 */

static int
kfs_rdwr(struct vnode *vp, struct uio *uiop, enum uio_rw rw, int flags, struct ucred *uc)
{
int		rval;
kfs_knode	*f;

	f = v2k(vp);

	if( kfs_trace )
		printf("kfs_rdwr(\"%s\")\n", f->name);

	if( vp->v_type != VREG )
		return EISDIR;

	if( uiop->uio_resid == 0 )
		return 0;

	if( uiop->uio_offset < 0 || (uiop->uio_offset + uiop->uio_resid) < 0 )		/* %%% boundaries */
		return EINVAL;

	if( rw == UIO_READ ) {
		VFS_RECORD(vp->v_vfsp, VS_READ, VS_CALL);
		rval = kfs_uioread_cb(f, uiop, uc);
	} else {
		VFS_RECORD(vp->v_vfsp, VS_WRITE, VS_CALL);
		rval = kfs_uiowrite_cb(f, uiop, uc);
	}

	return rval;
}


/*
 * ioctl - unimplemented
 */

static int
kfs_ioctl(struct vnode *vp, int cmd, caddr_t data, int flags, struct ucred *uc)
{
aclist		*acl;
kfs_knode	*k;
int		rval;

	k = v2k(vp);

	if( kfs_trace )
		printf("kfs_ioctl(\"%s\", cmd $%x, data $%x, ...)\n", k->name, cmd, (u_int) data);

	VFS_RECORD(vp->v_vfsp, VS_IOCTL, VS_CALL);

	switch( cmd ) {
	case FIO_KFS_GETACL:
		/* %%% must restrict access to ACL */
		if( data == 0 )
			return EINVAL;
		if( (rval = copyout(&k->fhdr.info.aclist, data, sizeof(k->fhdr.info.aclist))) != 0 )
			return rval;
		return 0;
	case FIO_KFS_SETACL:
		if( kfs_acl_access_acl_set(k, uc) < 0 )
			return EACCES;
		if( data == 0 )
			return EINVAL;
		if( (rval = copyin(data, &k->fhdr.info.aclist, sizeof(k->fhdr.info.aclist))) != 0 )
			return rval;
		k->fhdr_dirty = 1;
		return 0;
	case FIO_KFS_ADDACL:
		if( kfs_acl_access_acl_set(k, uc) < 0 )
			return EACCES;
		if( data == 0 )
			return EINVAL;
		acl = kfs_xalloc(sizeof(*acl));
		if( (rval = copyin(data, acl, sizeof(*acl))) != 0 ) {
			kfs_free(acl, sizeof(*acl));
			return rval;
		}
		if( kfs_acl_add(&k->fhdr.info.aclist, acl) < 0 ) {
			kfs_free(acl, sizeof(*acl));
			return EINVAL;
		}
		k->fhdr_dirty = 1;
		kfs_free(acl, sizeof(*acl));
		return 0;
	default:
		return VOP_IOCTL(k2rv(k), cmd, data, flags, uc);
	}
}


/*
 * select - unimplemented
 */

static int
kfs_select(struct vnode *vp, int which, struct ucred *uc)
{
kfs_knode	*f;

	f = v2k(vp);

	if( kfs_trace )
		printf("kfs_select: call\n");

	VFS_RECORD(vp->v_vfsp, VS_SELECT, VS_CALL);

	return VOP_SELECT(k2rv(f), which, uc);
}


/*
 * getattr - meta-data OP
 */

static int
kfs_getattr(struct vnode *vp, struct vattr *vap, struct ucred *uc)
{
kfs_knode	*f;
int		rval;

	f = v2k(vp);

	if( kfs_trace )
		printf("kfs_getattr(\"%s\", vattr $%x, ucred $%x)\n", f->name, (int) vap, (int) uc);

	if( (rval = VOP_GETATTR(k2rv(f), vap, uc)) != 0 )
		goto done;

	/* might refresh k->uid, k->gid */

	if( f->fhdr.info.type == KFS_CRYPT_1 )
		vap->va_size = f->fhdr.info.file_len.lo;		/* file size in bytes (quad?) */
	vap->va_fsid = makedev(f->me->fixedmajor, f->me->mntid & 0xff);

done:
	return rval;
}


/*
 * setattr - meta-data OP
 *
 * bit patters of all 1's represent "unset" fields
 *
 * %%% trap file-size, uid, gid into fhdr
 */

static int
kfs_setattr(struct vnode *vp, struct vattr *vap, struct ucred *uc)
{
kfs_knode	*k;

	k = v2k(vp);

	if( kfs_trace )
		printf("kfs_setattr(\"%s\", vattr $%x, ucred $%x)\n", k->name, (int) vap, (int) uc);

	VFS_RECORD(vp->v_vfsp, VS_SETATTR, VS_CALL);

	if( kfs_acl_access_meta_set(k, uc) < 0 )
		return EACCES;

	return VOP_SETATTR(k2rv(k), vap, uc);
}


/*
 * access - meta-data OP
 */

static int
kfs_access(struct vnode *vp, int mode, struct ucred *uc)
{
kfs_knode	*f;
int		rval;
struct ucred	*altc;

	f = v2k(vp);

	if( kfs_trace )
		printf("kfs_access(\"%s\", mode %d, ucred $%x)\n", f->name, mode, (int) uc);

	VFS_RECORD(vp->v_vfsp, VS_ACCESS, VS_CALL);

	rval = kfs_acl_access_obj(f, mode, uc, &altc);
	if( rval == 1 )
		return 0;
	if( rval == -1 )
		return EACCES;
	if( altc ) {
		rval = VOP_ACCESS(k2rv(f), mode, altc);
		crfree(altc);
	} else
		rval = VOP_ACCESS(k2rv(f), mode, uc);
	return rval;
}


/*
 * readlink - meta-data OP???
 */

static int
kfs_readlink(struct vnode *vp, struct uio *uiop, struct ucred *uc)
{
kfs_knode	*f;

	f = v2k(vp);

	if( kfs_trace )
		printf("kfs_readlink: call\n");

	VFS_RECORD(vp->v_vfsp, VS_READLINK, VS_CALL);

	return VOP_READLINK(k2rv(f), uiop, uc);
}


/*
 * fsync - not implemented
 */

static int
kfs_fsync(struct vnode *vp, struct ucred *uc)
{
kfs_knode	*k;
int		rval;

	k = v2k(vp);

	if( kfs_trace )
		printf("kfs_fsync(\"%s\")\n", k->name);

	VFS_RECORD(vp->v_vfsp, VS_FSYNC, VS_CALL);

	if( k->fhdr_dirty ) {
		if( (rval = kfs_write_fhdr(k, uc)) != 0 ) {
			printf("kfs_inactive: io error writing file hdr for \"%s\"\n", k->name);
			return rval;
		}
		k->fhdr_dirty = 0;
	}

	return VOP_FSYNC(k2rv(k), uc);
}


/*
 * inactive - vnode overhead
 */

static int
kfs_inactive(struct vnode *vp, struct ucred *uc)
{
kfs_knode	*k;

	k = v2k(vp);

	if( kfs_trace )
		printf("kfs_inactive(\"%s\" vnode $%x v_count %d, ucred $%x)\n", k->name,
								(int) vp, vp->v_count, (int) uc);

	VFS_RECORD(vp->v_vfsp, VS_INACTIVE, VS_CALL);

	if( k->fhdr_dirty ) {
		if( kfs_write_fhdr(k, uc) != 0 ) {
			printf("kfs_inactive: io error writing file hdr for \"%s\"\n", k->name);
		}
	}

	/* ++vp->v_count;			/+ the caller already decremented the ref count to get here */
	kfs_knode_free(k);
	return 0;
}


/*
 * lookup - dir op
 *
 *	resolve a filename into a vnode
 *	creates the shadow vnode (a knode) for the real vnode
 */

static int
kfs_lookup(struct vnode *dvp, char *nm, struct vnode **vpp,
		struct ucred *uc, struct pathname *pnp, int flags)
{
struct vnode	*rvp;
int		rval;
kfs_knode	*d, *f;

	d = v2k(dvp);

	if( kfs_trace )
		printf("kfs_lookup(\"%s\"/\"%s\", ..., flags $%x)\n", d->name, nm, flags);

	VFS_RECORD(dvp->v_vfsp, VS_LOOKUP, VS_CALL);

	if( nm[0] == '\0' ) {
		printf("kfs_lookup of empty string\n");
	}

	if( (rval = VOP_LOOKUP(k2rv(d), nm, &rvp, uc, pnp, flags)) != 0 )
		return rval;

	if( kfs_trace )
		printf("kfs_lookup(\"%s\"/\"%s\") -> (dir rvp $%x) rvp $%x\n", d->name, nm, (int) k2rv(d), (int) rvp);

	switch( rvp->v_type ) {
	case VREG:
		f = kfs_knode_find(d->me, rvp, d->name, d->name_len, nm, strlen(nm) + 1);
		if( (rval = kfs_load_fhdr(f, uc)) != 0 ) {
			kfs_knode_rele(f);
			*vpp = 0;
			if( kfs_trace )
				printf("kfs_lookup() -> err %d\n", rval);
			return rval;
		}
		kfs_knode_pick_key(f, uc);
		*vpp = k2v(f);
		(*vpp)->v_type = rvp->v_type;			/* this is necessary -cj */
		break;
	case VDIR:
	case VLNK:						/* %%% not sure about symlinks */
		f = kfs_knode_find(d->me, rvp, d->name, d->name_len, nm, strlen(nm) + 1);
		kfs_set_fhdr_nocrypt(f);
		*vpp = k2v(f);
		(*vpp)->v_type = rvp->v_type;			/* this is necessary -cj */
		break;
	default:
		/* VNON, VBLK, VCHR, VSOCK, VBAD, VFIFO go right thru */
		if( kfs_trace )
			printf("kfs_lookup: vtype %d\n", rvp->v_type);
		*vpp = rvp;
	}

	return 0;
}


/*
 * create a VREG - dir OP
 */

static int
kfs_create(struct vnode *dvp, const char *nm, const struct vattr *vap,
		enum vcexcl exclusive, int mode, struct vnode **vpp, struct ucred *uc)
{
struct vnode	*rvp;
int		rval;
kfs_knode	*d, *f;

	d = v2k(dvp);

	if( kfs_trace )
		printf("kfs_create(\"%s\"/\"%s\", ..., mode 0%o, ...)\n", d->name, nm, mode);

	VFS_RECORD(dvp->v_vfsp, VS_CREATE, VS_CALL);

	if( (rval = VOP_CREATE(k2rv(d), nm, vap, exclusive, mode, &rvp, uc)) != 0 ) {
		*vpp = 0;
		return rval;
	}

	f = kfs_knode_find(d->me, rvp, d->name, d->name_len, nm, strlen(nm) + 1);
	kfs_set_fhdr_crypt(f);
	kfs_set_acl_owner(f, uc);
	kfs_knode_pick_key(f, uc);

	if( (rval = kfs_fsync(k2v(f), uc)) != 0 ) {
		kfs_knode_rele(f);
		*vpp = 0;
		return rval;
	}

	*vpp = k2v(f);
	return 0;
}


/*
 * remove - dir OP
 */

static int
kfs_remove(struct vnode *dvp, char *nm, struct ucred *uc)
{
kfs_knode	*d;

	d = v2k(dvp);

	if( kfs_trace )
		printf("kfs_remove(\"%s\"/\"%s\", ...)\n", d->name, nm);

	VFS_RECORD(dvp->v_vfsp, VS_REMOVE, VS_CALL);

	return VOP_REMOVE(k2rv(d), nm, uc);
}


/*
 * link - dir OP
 */

static int
kfs_link(struct vnode *vp, struct vnode *tdvp, char *tnm, struct ucred *uc)
{
kfs_knode	*f, *td;

	f = v2k(vp);
	td = v2k(tdvp);

	if( kfs_trace )
		printf("kfs_link: call\n");

	VFS_RECORD(vp->v_vfsp, VS_LINK, VS_CALL);

	return VOP_LINK(k2rv(f), k2rv(td), tnm, uc);
}


/*
 * rename - dir OP
 */

static int
kfs_rename(struct vnode *odvp, char *onm, struct vnode *ndvp, char *nnm, struct ucred *uc)
{
kfs_knode	*od, *nd;

	od = v2k(odvp);
	nd = v2k(ndvp);

	if( kfs_trace )
		printf("kfs_rename(\"%s\"/\"%s\" to \"%s\"/\"%s\")\n", od->name, onm, nd->name, nnm);

	VFS_RECORD(odvp->v_vfsp, VS_RENAME, VS_CALL);

	return VOP_RENAME(k2rv(od), onm, k2rv(nd), nnm, uc);
}


/*
 * mkdir - dir OP
 */

static int
kfs_mkdir(struct vnode *dvp, const char *nm, const struct vattr *vap, struct vnode **vpp, struct ucred *uc)
{
struct vnode	*rvp;
kfs_knode	*d, *nd;
int		rval;

	d = v2k(dvp);

	if( kfs_trace )
		printf("kfs_mkdir(\"%s\"/\"%s\", ..., mode 0%o, ...)\n", d->name, nm, vap->va_mode);

	VFS_RECORD(dvp->v_vfsp, VS_MKDIR, VS_CALL);

	if( (rval = VOP_MKDIR(k2rv(d), nm, vap, &rvp, uc)) != 0 ) {
		*vpp = 0;
		return rval;
	}

	nd = kfs_knode_find(d->me, rvp, d->name, d->name_len, nm, strlen(nm) + 1);
	kfs_knode_pick_key(nd, uc);
	*vpp = k2v(nd);
	return rval;
}


/*
 * rmdir - dir OP
 */

static int
kfs_rmdir(struct vnode *dvp, char *nm, struct ucred *uc)
{
kfs_knode	*d;

	d = v2k(dvp);

	if( kfs_trace )
		printf("kfs_rmdir(\"%s\"/\"%s\", ...)\n", d->name, nm);

	VFS_RECORD(dvp->v_vfsp, VS_RMDIR, VS_CALL);

	return VOP_RMDIR(k2rv(d), nm, uc);
}


/*
 * symlink - dir OP
 */

static int
kfs_symlink(struct vnode *dvp, char *lnm, struct vattr *tva, char *tnm, struct ucred *uc)
{
kfs_knode	*d;

	d = v2k(dvp);

	if( kfs_trace )
		printf("kfs_slink(\"%s\"/\"%s\" -> \"%s\"\n", d->name, lnm, tnm);

	VFS_RECORD(dvp->v_vfsp, VS_SYMLINK, VS_CALL);

	return VOP_SYMLINK(k2rv(d), lnm, tva, tnm, uc);
}


/*
 * readdir - dir OP
 */

static int
kfs_readdir(struct vnode *vp, struct uio *uiop, struct ucred *uc)
{
kfs_knode	*d;

	d = v2k(vp);

	if( kfs_trace )
		printf("kfs_readdir(\"%s\")\n", d->name);

	VFS_RECORD(vp->v_vfsp, VS_READDIR, VS_CALL);

	return VOP_READDIR(k2rv(d), uiop, uc);
}


/*
 * lockctl - unimplemented
 */

static int
kfs_lockctl(struct vnode *vp, struct flock *ld, int cmd, struct ucred *uc, int clid)
{
kfs_knode	*f;

	if( kfs_trace )
		printf("kfs_lockctl: call\n");

	VFS_RECORD(vp->v_vfsp, VS_LOCKCTL, VS_CALL);

	return VOP_LOCKCTL(k2rv(f), ld, cmd, uc, clid);
}


/*
 * getpage - unimplemented
 */

static int
kfs_getpage(struct vnode *vp, u_int off, u_int len, struct page *pl[],
		u_int plsz, struct seg *seg, addr_t addr, enum seg_rw rw, struct ucred *uc)
{
int		rval;
kfs_knode	*f;

	f = v2k(vp);

	if( kfs_trace )
		printf("kfs_getpage(\"%s\", off %d, len %d, ...)\n", f->name, off, len);

	rval = ENXIO;

	return rval;
}


/*
 * putpage - unimplemented
 */

static int
kfs_putpage(struct vnode *vp, u_int off, u_int len, int flags, struct ucred *uc)
{
int		rval;
kfs_knode	*f;

	f = v2k(vp);

	if( kfs_trace )
		printf("kfs_putpage(\"%s\", off %d, len %d, flags $%x...)\n", f->name, off, len, flags);

	rval = ENXIO;

	return rval;
}


static int
kfs_map(struct vnode *vp, u_int off, struct as *as, addr_t *addrp, u_int len,
		u_int prot, u_int maxprot, u_int flags, struct ucred *uc)
{
int	rval;

	if( kfs_trace )
		printf("kfs_map: call\n");

	rval = ENXIO;

	return rval;
}


/*
 * cmp - vnode overhead
 */

static int
kfs_cmp(struct vnode *vp1, struct vnode *vp2)
{
	if( kfs_trace )
		printf("kfs_cmp: call\n");

	VFS_RECORD(vp1->v_vfsp, VS_CMP, VS_CALL);

	return vp1 == vp2;
}


/*
 * cmp - vnode overhead
 */

static int
kfs_realvp(struct vnode *vp, struct vnode **vpp)
{
	if( kfs_trace )
		printf("kfs_realvp: call\n");

	VFS_RECORD(vp->v_vfsp, VS_REALVP, VS_CALL);

	return EINVAL;
}


/*
 * cntl - unimplemented
 */

static int
kfs_cntl(struct vnode *vp, int cmd, caddr_t idata, caddr_t odata, int iflag, int oflag)
{
kfs_knode	*k;

	k = v2k(vp);

	if( kfs_trace )
		printf("kfs_cntl(\"%s\", cmd $%x, idata $%x, ...)\n", k->name, cmd, (u_int) idata);

	VFS_RECORD(vp->v_vfsp, VS_CNTL, VS_CALL);

	return ENOSYS;
}


/*
 * dump - unimplemented
 */

static int
kfs_dump(struct vnode *vp, caddr_t addr, int bn, int count)
{
int	rval;

	if( kfs_trace )
		printf("kfs_dump: call\n");

	rval = ENXIO;

	return rval;
}


/*
 * fid - used by getfh(2)
 *
 *	we don't allow this FS type to be exported for security reasons.
 */

static int
kfs_fid(struct vnode *vp, void *fidpp)			/* %%% last arg */
{
	if( kfs_trace )
		printf("kfs_fid: call\n");

	return ENXIO;
}


struct vnodeops kfs_vnodeops = {
	kfs_open,
	kfs_close,
	kfs_rdwr,
	kfs_ioctl,
	kfs_select,
	kfs_getattr,
	kfs_setattr,
	kfs_access,
	kfs_lookup,
	kfs_create,
	kfs_remove,
	kfs_link,
	kfs_rename,
	kfs_mkdir,
	kfs_rmdir,
	kfs_readdir,
	kfs_symlink,
	kfs_readlink,
	kfs_fsync,
	kfs_inactive,
	kfs_lockctl,
	kfs_fid,
	kfs_getpage,
	kfs_putpage,
	kfs_map,
	kfs_dump,
	kfs_cmp,
	kfs_realvp,
	kfs_cntl
};
