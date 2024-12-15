/* kfs_acl.h
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

#ifndef __kfs_kfs_acl_h__
#define __kfs_kfs_acl_h__

#define ACL_NUSERS		(16)

#define ACL_FLAG_BLACK		(0x01)				/* hidden entry (must have ACLOP rights to see) */
#define ACL_FLAG_INHERIT	(0x02)				/* new files inherit dir's ACL (default?) */
#define ACL_FLAG_METAOP_SET	(0x04)				/* subject may act on meta-data (as if owner) */
#define ACL_FLAG_ACLOP_SET	(0x08)				/* subject may modify ACL (as if ACL owner) */
#define ACL_FLAG_DOMASK		(0x30)
#define ACL_FLAG_DOMODE		(0x10)				/* subject follows mode field */
#define ACL_FLAG_DOGROUP	(0x20)				/* !MODE -> subject makes group access */
#define ACL_FLAG_DOUSER		(0x00)				/* (pseudo flag) !MODE -> subject makes user access */


#define ACL_IDTYPE_UID		(0)
#define ACL_IDTYPE_GID		(1)

typedef struct {
	u_short		id_type;			/* specify user or group */
	union {
		uid_t	uid;
		gid_t	gid;
	} id;
	/* u_short		how;				/+ access: user|group-member|apply-mode */
	u_short		flags;				/* _OWN|... */
	u_short		perm;				/* (apply-mode only) VWRITE|... */
} ac_ent;

typedef struct {
	u_int		inuse;				/* # entries in use */
	ac_ent		actab[ACL_NUSERS];
} aclist;

#ifdef KERNEL
extern int kfs_acl_add(aclist *acl1, const aclist *acl2);
extern int kfs_acl_access_obj(const kfs_knode *k, int mode, const struct ucred *uc, struct ucred **altc);
extern int kfs_acl_access_acl_set(const kfs_knode *k, const struct ucred *uc);
extern int kfs_acl_access_meta_set(const kfs_knode *k, const struct ucred *uc);
extern void kfs_set_acl_owner(kfs_knode *k, const struct ucred *uc);
#endif /* KERNEL */

#endif /* __kfs_kfs_acl_h__ */
