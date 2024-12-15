/* kfs_acl.c
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


static ac_ent *actab_search(const aclist *acl, const struct ucred *uc);


int
kfs_acl_add(aclist *acl, const aclist *acl_new)
{
	if( acl->inuse + acl_new->inuse > ACL_NUSERS )
		return -1;
	bcopy(&acl_new->actab[0], &acl->actab[acl->inuse], acl_new->inuse * sizeof(acl_new->actab[0]));
	acl->inuse += acl_new->inuse;
	return 0;
}


/*
 * do permission checking for object ops (R/W/X)
 *
 * returns:
 *	0	access not specified (may have new cred in *altc)
 *	1	access granted
 *	-1	access denied
 */

int
kfs_acl_access_obj(const kfs_knode *k, int mode, const struct ucred *uc, struct ucred **altc)
{
ac_ent	*e;

	*altc = 0;
	if( (e = actab_search(&k->fhdr.info.aclist, uc)) == 0 )
		return 0;

	if( (e->flags & ACL_FLAG_DOMASK) == ACL_FLAG_DOMODE ) {
		if( (mode & e->perm) == mode )
			return 1;
		else
			return -1;
	}

	/* do the impersonation */

	*altc = crdup(uc);
	if( (e->flags & ACL_FLAG_DOMASK) == ACL_FLAG_DOGROUP ) {
		(*altc)->cr_gid = k->gid;
	} else {
		(*altc)->cr_uid = k->uid;
	}
	return 0;
}


/*
 * this is for setattr/chmod, etc...
 *
 * only protects files that have a crypt header (eg. not dirs)
 *
 * returns:
 *	-1	NO
 *	0	OK
 *	(maybe add trinary value 1)
 */

int
kfs_acl_access_meta_set(const kfs_knode *k, const struct ucred *uc)
{
ac_ent	*e;

	if( k->fhdr.info.type == KFS_CRYPT_NONE )
		return 0;

	if( (e = actab_search(&k->fhdr.info.aclist, uc)) == 0 )
		return -1;

	if( e->flags & ACL_FLAG_METAOP_SET )
		return 0;
	else
		return -1;
}


/*
 * this is for acledit
 *
 * returns:
 *	-1	NO
 *	0	OK
 */

int
kfs_acl_access_acl_set(const kfs_knode *k, const struct ucred *uc)
{
ac_ent	*e;

	if( (e = actab_search(&k->fhdr.info.aclist, uc)) == 0 )
		return -1;

	if( e->flags & ACL_FLAG_ACLOP_SET )
		return 0;
	else
		return -1;
}


/* *** */



/*
 * make first ACL entry for owner of object
 */

void
kfs_set_acl_owner(kfs_knode *k, const struct ucred *uc)
{
ac_ent	*e;

	e = &k->fhdr.info.aclist.actab[0];
	e->id_type = ACL_IDTYPE_UID;
	e->id.uid = uc->cr_uid;
	e->flags = ACL_FLAG_DOUSER|ACL_FLAG_ACLOP_SET|ACL_FLAG_METAOP_SET;

	k->fhdr.info.aclist.inuse = 1;
}


/* *** */


static ac_ent *
actab_search(const aclist *acl, const struct ucred *uc)
{
int	i, g;
ac_ent	*e;

	e = &acl->actab[0];
	for( i = 0; i < acl->inuse ; ++i, ++e ) {
		switch( e->id_type ) {
		case ACL_IDTYPE_UID:
			if( e->id.uid == uc->cr_uid )
				goto found;
			break;
		case ACL_IDTYPE_GID:
			if( e->id.gid == uc->cr_gid )
				goto found;
			for( g = 0; g < NGROUPS && uc->cr_groups[g] != 0 ; ++g )
				if( e->id.gid == uc->cr_groups[g] )
					goto found;
			break;
		}
	}
	return 0;
found:
	return e;
}
