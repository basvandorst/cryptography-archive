/* kfs_subr.c
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
#include "md5.h"
#include "crypt.h"

#define KFS_TRACE		(0)


int		kfs_trace = KFS_TRACE;
kfs_mntent	*kfs_mntlist = 0;


/*
 * thread this mount point on its server's mount list
 */

void
kfs_addmnt(kfs_mntent *me)
{
	me->next = kfs_mntlist;
	kfs_mntlist = me;
}


/*
 * never returns 0
 */

kfs_mntent *
kfs_mntent_new()
{
kfs_mntent	*me;

	me = kfs_xzalloc(sizeof(kfs_mntent));
	return me;
}


void
kfs_mntent_free(kfs_mntent *me)
{
kfs_keyinfo	*k;

	if( me->rootknode )
		kfs_knode_rele(me->rootknode);
	if( me->remotepath )
		kfs_free(me->remotepath, me->remotepath_len);
	while( (k = me->keylist) ) {
		me->keylist = k->next;
		kfs_keyinfo_rele(k);
	}
	kfs_free(me, sizeof(*me));
}


/* *** */


kfs_keyinfo *
kfs_keyinfo_new(uid_t uid, const char *asc_key)
{
kfs_keyinfo	*k;

	if( kfs_trace )
		printf("kfs_keyinfo_new: setting key to <%s>\n", asc_key);
	k = kfs_xzalloc(sizeof(*k));
	k->refs = 1;
	k->uid = uid;
	k->key_len = KEY_BUF;
	k->key_val = kfs_xalloc(k->key_len);
	xfs_init_crypt(k->key_val, asc_key);
	return k;
}


static void
_kfs_keyinfo_free(kfs_keyinfo *k)
{
	if( k->refs != 0 )
		printf("kfs_keyinfo_free: WARNING: refs %lx\n", k->refs);

	bzero(k->key_val, k->key_len);				/* wipe out any clues */
	kfs_free(k->key_val, k->key_len);
	kfs_free(k, sizeof(*k));
}


void
kfs_keyinfo_change(kfs_keyinfo *k, const char *asc_key)
{
	if( kfs_trace )
		printf("kfs_keyinfo_change: changing key to <%s>\n", asc_key);
	xfs_init_crypt(k->key_val, asc_key);
}


kfs_keyinfo *
kfs_keyinfo_hold(kfs_keyinfo *k)
{
	++k->refs;
	return k;
}


void
kfs_keyinfo_rele(kfs_keyinfo *k)
{
	if( --k->refs == 0 )
		_kfs_keyinfo_free(k);
}


/* *** */

/*
 *
 * find a knode with matching (vfsp, rvp)
 */

kfs_knode *
kfs_knode_find(kfs_mntent *me, struct vnode *rvp,
				const char *dname, int dname_len,
				const char *fname, int fname_len)
{
kfs_knode	*f;

	/* %%% use hash table */

	if( (f = me->knodelist) ) {
		do {
			if( k2rv(f)->v_op == rvp->v_op && VOP_CMP(k2rv(f), rvp) ) {
				kfs_knode_hold(f);
				return f;
			}
			f = f->knodenext;
		} while( f != me->knodelist );
	}
	return kfs_knode_new(me, rvp, dname, dname_len, fname, fname_len);
}


/*
 *
 *	- [df]name_len must include '\0' terminator
 *	- only fname may be nil
 *	- vp->v_type needs to have been set
 */

kfs_knode *
kfs_knode_new(kfs_mntent *me, struct vnode *rvp,
				const char *dname, int dname_len,
				const char *fname, int fname_len)
{
kfs_knode	*k;

	k = kfs_xzalloc(sizeof(*k));
	k->realvp = rvp;
	VN_INIT(&k->v, me->vfsp, rvp->v_type, 0);
	k->v.v_data = (void *) k;
	k->v.v_op = &kfs_vnodeops;
	k->name_len = dname_len + fname_len;
	k->name = kfs_xalloc(k->name_len);
	bcopy(dname, k->name, dname_len);
	if( fname ) {
		k->name[dname_len - 1] = '/';
		bcopy(fname, k->name + dname_len, fname_len);
	}
	k->me = me;
	++me->node_count;
	if( me->knodelist ) {
		k->knodenext = me->knodelist;
		k->knodeprev = me->knodelist->knodeprev;
		k->knodeprev->knodenext = k;
		k->knodenext->knodeprev = k;
	} else {
		k->knodeprev = k;
		k->knodenext = k;
	}
	me->knodelist = k;
	if( kfs_trace )
		printf("kfs_knode_new: \"%s\"\n", k->name);
	return k;
}


void
kfs_knode_hold(kfs_knode *f)
{
	if( kfs_trace )
		printf("kfs_knode_hold(\"%s\") -> %d\n", f->name, k2v(f)->v_count + 1);

	VN_HOLD(k2v(f));
}


void
kfs_knode_rele(kfs_knode *k)
{
	VN_RELE(k2v(k));		/* calls kfs_inactive(), calls kfs_knode_free() */
}


/*
 * %%% what about k2rv()???
 */

void
kfs_knode_free(kfs_knode *k)
{
kfs_mntent	*me;

	me = k->me;
	if( k->v.v_count != 0 ) {
		printf("kfs_knode_free: v_count %d\n", k->v.v_count);
		return;
	}
	--me->node_count;

	if( k == me->knodelist ) {
		if( k == k->knodenext )
			me->knodelist = 0;
		else
			me->knodelist = k->knodenext;
	}
	k->knodeprev->knodenext = k->knodenext;
	k->knodenext->knodeprev = k->knodeprev;

	VN_RELE(k2rv(k));
	kfs_free(k->name, k->name_len);
	if( k->key )
		kfs_keyinfo_rele(k->key);
	kfs_free(k, sizeof(*k));
}


void
kfs_knode_pick_key(kfs_knode *f, const struct ucred *uc)
{
kfs_keyinfo	*k;

	for( k = f->me->keylist; k ; k = k->next ) {
		if( k->uid == uc->cr_ruid ) {
			if( !f->key )
				f->key = kfs_keyinfo_hold(k);
			return;
		}
	}
	if( kfs_trace )
		printf("kfs_knode_pick_key(\"%s\"):  no key found for uid %u\n", f->name, uc->cr_ruid);
}


/*
 *
 * returns an errno
 *
 * for now, just do a VOP_GETATTR to find out real owner; later, we should
 * probably put the (uid, gid) into fhdr.
 */

int
kfs_load_fhdr(kfs_knode *f, struct ucred *uc)
{
u_char	md5_hash[MD5_BUF];
MD5_CTX	ctx;
long	cc;
int	rval;
struct vattr	va;

	if( kfs_trace )
		printf("kfs_load_fhdr(\"%s\")\n", f->name);

	cc = sizeof(f->fhdr);
	if( (rval = kfs_vpread(k2rv(f), &f->fhdr, &cc, (off_t) 0, uc)) != 0 ) {
		return rval;
	}
	/* %%% maybe allow cc == 0 */
	if( cc != sizeof(f->fhdr) ) {
		uprintf("\nKFS: partial meta data \"%s\"\n", f->name);
		return EIO;
	}
	if( f->fhdr.info.magic != KFS_FMAGIC ) {
		uprintf("\nKFS: bad magic number in meta data \"%s\"\n", f->name);
		return EIO;
	}

	MD5Init(&ctx);
	MD5Update(&ctx, (void *) &f->fhdr.info, sizeof(f->fhdr.info));
	MD5Final(md5_hash, &ctx);

	/* %%% de-crypt header here? */

	if( bcmp(f->fhdr.md5hash, md5_hash, sizeof(md5_hash)) != 0 ) {
		uprintf("\nKFS: corrupt meta data \"%s\"\n", f->name);
		return EACCES;
	}

	if( (rval = VOP_GETATTR(k2rv(f), &va, uc)) != 0 )
		return rval;
	f->uid = va.va_uid;
	f->gid = va.va_gid;

	f->fhdr_dirty = 0;

	return 0;
}


/*
 * write file header
 *
 * %%% maybe use root ucred instead?
 */

int
kfs_write_fhdr(kfs_knode *f, struct ucred *uc)
{
MD5_CTX	ctx;
int	rval;

	MD5Init(&ctx);
	MD5Update(&ctx, (void *) &f->fhdr.info, sizeof(f->fhdr.info));
	MD5Final(f->fhdr.md5hash, &ctx);

	/* %%% crypt header here? */

	if( (rval = kfs_vpwrite(k2rv(f), &f->fhdr, sizeof(f->fhdr), (off_t) 0, uc)) != 0 ) {
		/* printf("kfs_inactive: io error writing file hdr for \"%s\"\n", f->name); */
		return rval;
	}

	f->fhdr_dirty = 0;

	return 0;
}


/*
 *
 * - assumes f->fhdr is zero'd out
 */

void
kfs_set_fhdr_crypt(kfs_knode *f)
{
	f->fhdr.info.magic = KFS_FMAGIC;
	f->fhdr.info.type = KFS_CRYPT_1;
	f->fhdr_dirty = 1;
}


/*
 *
 * - assumes f->fhdr is zero'd out
 */

void
kfs_set_fhdr_nocrypt(kfs_knode *f)
{
	f->fhdr.info.type = KFS_CRYPT_NONE;
}


/* *** */

void *
kfs_xzalloc(u_long sz)
{
	return new_kmem_zalloc(sz, KMEM_SLEEP);
}


void *
kfs_xalloc(u_long sz)
{
	return new_kmem_zalloc(sz, KMEM_SLEEP);			/* or just ..._alloc() */
}


void
kfs_free(void *pt, u_long sz)
{
	kmem_free(pt, sz);
}
