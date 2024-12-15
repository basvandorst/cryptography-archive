/*
 * The author of this software is Matt Blaze.
 *              Copyright (c) 1992, 1993, 1994 by AT&T.
 * Permission to use, copy, and modify this software without fee
 * is hereby granted, provided that this entire notice is included in
 * all copies of any software which is or includes a copy or
 * modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * This software is subject to United States export controls.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */

/*
 * crypto-nfs server functions
 *  rpc handlers
 *  access control policies
 */

#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <rpc/rpc.h>
#include <errno.h>
#ifdef NeXT
#include <sys/dir.h>
#define dirent direct
#else
#include <dirent.h>
#endif
#include "nfsproto.h"
#include "admproto.h"
#include "cfs.h"

typedef struct svc_req *SR;

#define herr(x) (((x)==H_INVALID)?NFSERR_STALE:NFSERR_PERM)

char *
pfh(fh)
     unsigned char *fh;
{
	static char ret[NFS_FHSIZE*2+2];
	unsigned char x[3];
	int i;

	ret[0]='\0';
	for (i=0; i<NFS_FHSIZE; i++) {
		sprintf((char *)x,"%02x",fh[i]);
		strcat(ret,x);
	}
	return ret;
}

void *
nfsproc_null_2(ap,rp)
     void *ap;
     SR rp;
{
	static int ret=0;
	return ((void*) &ret);
}

#define as_attr attrstat_u.attributes

diropres *rootlookup();
readdirres *rootreaddir();

attrstat *
nfsproc_getattr_2(ap,rp)
     nfs_fh *ap;
     SR rp;
{
	static attrstat ret;
	int uid;
	int ht;
	cfs_fileid *h;

#ifdef DEBUG
	printf("getattr:\n");
#endif
	if ((ht=htype(h=geth(ap)))!=H_REG) {
		/* this is an ugly hack to deal w/ lookups in root,
		   but it will do for now */;
		if (ht==H_ROOT) {
			ret.status=NFS_OK;
			rootgetattr(&ret.as_attr);
			return &ret;
		}
		ret.status=NFSERR_STALE;
		return &ret;
	}
	uid=ruid(rp);
	if (!fhuid(h,uid)) {
		ret.status = NFSERR_PERM;
		return &ret;
	}
	become(rp);	/* become the user */
	switch (fhstat(h,&ret.as_attr)) {
	    case 0:	/* ok */
		ret.status=NFS_OK;
		break;
	    case -1:	/* error from syscall */
		ret.status=cfsno(errno);
		break;
	    default:	/* bad handle, probably */
		ret.status=cfserrno;
		break;
	}
	closeout(h);	/* if it was open, close it */
	become(NULL);
	return &ret;
}

attrstat *
nfsproc_setattr_2(ap,rp)
     sattrargs *ap;
     SR rp;
{
	static attrstat ret;
	int uid;
	int ht;
	cfs_fileid *h;

	/* this is non-atomic, and could leave partial results */
	/* also, we vary from std nfs semantics in that failure does
	   not always leave the file in its previous state */

#ifdef DEBUG
	printf("setattr: \n");
#endif
	if ((ht=htype(h=geth(&ap->file)))!=H_REG) {
		ret.status=herr(ht);
		return &ret;
	}
	uid=ruid(rp);
	become(rp);	/* become the user */
	switch (fhsetattr(h,&ap->attributes)) {
	    case 0:	/* ok */
		/* now do a getattr */		
		switch (fhstat(h,&ret.as_attr)) {
		    case 0:
			ret.status=NFS_OK;
			break;
		    case -1:
			ret.status=cfsno(errno);
			break;
		    default:
			ret.status=cfserrno;
			break;
		}
		break;
	    case -1:	/* error from syscall */
		/* but some things may be already set.  should fix this */
		ret.status=cfsno(errno);
		break;
	    default:	/* bad handle, probably */
		ret.status=cfserrno;
		break;
	}
	become(NULL);
	return &ret;
}

void *
nfsproc_root_2(ap,rp)
     void *ap;
     SR rp;
{
	static int ret=0;

	return ((void*)&ret);
}

/* fix this to deal w/ fs root (instance root should be ok) */
diropres *
nfsproc_lookup_2(ap,rp)
     diropargs *ap;
     SR rp;
{
	static diropres ret;
	diropres *retp;
	nfs_fh *handle;
	int uid;
	cfskey *key;
	char *s;
	int ht;
	cfs_fileid *h;

	/* this, mkdir, create, and the mount protocol are the only ways
	   to create a handle */
#ifdef DEBUG
	printf("lookup: \n");
#endif
	/* this is where most of the security has to lie, since lookup
	   returns handles to files.  so we have to make sure we're coming
	   from the right place and that it's really a priv port, etc */
	if (!goodsrc(rp)) {
		ret.status=NFSERR_PERM;
		return &ret;
	}
	if ((ht=htype(h=geth(&ap->dir)))!=H_REG) {
		if (ht==H_ROOT) {
			become(rp);
			retp=rootlookup(ap->name);
			become(NULL);
			return retp;
		}
		ret.status=herr(ht);
		return &ret;
	}
	uid=ruid(rp);
 	if (!fhuid(h,uid)) {
		ret.status = NFSERR_PERM;
		return &ret;
	}

	become(rp);	/* become the user */
	handle = &ret.diropres_u.diropres.file;
	key=keyof(h);
	if (key==NULL)
		ret.status=cfsno(errno);
	else if ((s=encryptname(key,ap->name))==NULL)
		ret.status=NFSERR_NOENT;
	else switch (fhlook(h,s,handle)) {
	    /* fhlook might update the underlying name associated with the
	       handle, or delete the handle */
	    case 0:	/* found it */
		/* now do a getattr */		
		switch (fhstat(geth(handle),
			       &ret.diropres_u.diropres.attributes)) {
		    case 0:
			ret.status=NFS_OK;
			/* stat is already in attributes field */
			break;
		    case -1:	/* couldn't open file for some reason */
			ret.status=cfsno(errno);
			break;
		    default:	/* shouldn't happen */
			ret.status=cfserrno;
			break;
		}
		break;
	    case -1:	/* some parent dir went away, probably */
		ret.status=cfsno(errno);
		break;
	    default:	/* bad parent handle or notfound, probably */
		ret.status=cfserrno;
		break;
	}
	become(NULL);
	return (&ret);
}

readlinkres *
nfsproc_readlink_2(ap,rp)
     nfs_fh *ap;
     SR rp;
{
	static readlinkres ret;
	int uid;
	char buf[NFS_MAXPATHLEN+1];
	char *s;
	cfskey *key;
	int ht;
	cfs_fileid *h;
	int l;

#ifdef DEBUG
	printf("readlink:\n");
#endif
	if ((ht=htype(h=geth(ap)))!=H_REG) {
		ret.status=herr(ht);
		return &ret;
	}
	uid=ruid(rp);
	become(rp);

	switch (l=fhlinkval(h,buf)) {
	    case -1:
		ret.status=cfsno(errno);
		break;
	    case -2:
		ret.status=cfserrno;
		break;
	    default:
		buf[l]='\0';
		if ((key=keyof(h))==NULL) {
			ret.status=cfserrno;
			break;
		}
		if ((s=decryptname(key,buf)) == NULL) {
			ret.status=NFSERR_NOENT; /* close enough */
			break;
		}
		ret.readlinkres_u.data = s;
		ret.status=NFS_OK;
		break;
	}

	become(NULL);
	return (&ret);
}


readres *
nfsproc_read_2(ap,rp)
     readargs *ap;
     SR rp;
{
	static readres ret;
	static char buffer[8192];
	int fd;
	int uid;
	int len;
	cfskey *key;
	int ht;
	cfs_fileid *h;
	
#ifdef DEBUG
	printf("read:\n");
#endif
	if ((ht=htype(h=geth(&ap->file)))!=H_REG) {
		ret.status=herr(ht);
		return &ret;
	}
	uid=ruid(rp);
	if (!fhuid(h,uid)) {
		ret.status = NFSERR_PERM;
		return &ret;
	}
	become(rp);
	switch (fd=fhtofd(h,CFS_READ)) {
	    case -1:	/* syscall error */
		ret.status = cfsno(errno);
		break;
	    case -2:	/* cfs error */
		ret.status = cfserrno;
		break;
	    default:	/* fd should be valid file descriptor */
		/* do the read */
		key=keyof(h);
		if (key==NULL) {
			ret.status=cfserrno;
			break;
		}			
		if ((len=readblock(buffer,fd,ap->offset,ap->count,
				   key,vectof(h)))<0){
			ret.status = cfsno(errno);
			break;
		}
		if (fhstat(h,&ret.readres_u.reply.attributes)!=0) {
			ret.status = cfsno(errno);
			break;
		}
		ret.readres_u.reply.data.data_len = 
			(len > (ap->count)) ? ap->count : len;
		ret.readres_u.reply.data.data_val = buffer;
		ret.status = NFS_OK;
		break;
	}
	become(NULL);
	return (&ret);
}


void *
nfsproc_writecache_2(ap,rp)
     void *ap;
     SR rp;
{
	static int ret;

	return (void *)(&ret);
}


attrstat *
nfsproc_write_2(ap,rp)
     writeargs *ap;
     SR rp;
{
	static attrstat ret;
	int fd;
	int uid;
	int len;
	int offset;
	cfskey *key;
	char *buf;
	int ht;
	cfs_fileid *h;

#ifdef DEBUG
	printf("write:\n");
#endif
	if ((ht=htype(h=geth(&ap->file)))!=H_REG) {
		ret.status=herr(ht);
		return &ret;
	}

	uid=ruid(rp);
	if (!fhuid(h,uid)) {
		ret.status = NFSERR_PERM;
		return &ret;
	}
	become(rp);
	switch (fd=fhtofd(h,CFS_WRITE)) {
	    case -1:	/* syscall error */
		ret.status = cfsno(errno);
		break;
	    case -2:	/* cfs error */
		ret.status = cfserrno;
		break;
	    default:	/* fd should be valid file descriptor */
		len = ap->data.data_len;
		offset = ap->offset;
		buf = ap->data.data_val;
		key=keyof(h);
		if (key==NULL) {
			ret.status=cfserrno;
			break;
		}
		if (writeblock(buf,fd,offset,len,key,vectof(h)) < 0) {
			ret.status = cfsno(errno);
			break;
		}
		if (fhstat(h,&ret.attrstat_u.attributes)!=0) {
			ret.status = cfsno(errno);
			break;
		}
		ret.status = NFS_OK;
		break;
	}
	become(NULL);
	return (&ret);
}


diropres *
nfsproc_create_2(ap,rp)
     createargs *ap;
     SR rp;
{
	static diropres ret;
	static nfs_fh handle;
	int uid;
	int ht;
	char *s;
	cfs_fileid *h, *h2;

#ifdef DEBUG
	printf("create:\n");
#endif
	/* this, lookup, mkdir, and the mount protocol are the only ways
	   to create a handle */
	if (!goodsrc(rp)) {
		ret.status=NFSERR_PERM;
		return &ret;
	}
	if ((ht=htype(h=geth(&ap->where.dir)))!=H_REG) {
		ret.status=herr(ht);
		return &ret;
	}
	uid=ruid(rp);
	if (!fhuid(h,uid)) {
		ret.status = NFSERR_PERM;
		return &ret;
	}
	become(rp);
	
	/* first create the dir entry */
	/* if one already exist, the underlying handle name field gets
	   updated to reflect its new name */
	s=encryptname(keyof(h),ap->where.name);
	switch (fhmkfileent(h,s,&handle)) {
	    case 0: /* didnt exist */
		switch (fhsetattr(h2=geth(&handle),&ap->attributes)) {
		    case 0:	/* ok */
			/* now do a getattr */		
			switch (fhstat(h2,
				       &ret.diropres_u.diropres.attributes)) {
			    case 0:
				bcopy(&handle,&ret.diropres_u.diropres.file,
				      sizeof(nfs_fh));
				ret.status=NFS_OK;
				break;
			    case -1:
				ret.status=cfsno(errno);
				break;
			    default:
				ret.status=cfserrno;
				break;
			}
			break;
		    case -1:	/* error from syscall */
			ret.status=cfsno(errno);
			break;
		    default:	/* bad handle, probably */
			ret.status=cfserrno;
			break;
		}
		break;
	    case 1:	/* did exist; no chmod - KLUDGE */
		switch (fhsetattrprime(h2=geth(&handle),&ap->attributes)) {
		    case 0:	/* ok */
			/* now do a getattr */		
			switch (fhstat(h2,
				       &ret.diropres_u.diropres.attributes)) {
			    case 0:
				bcopy(&handle,&ret.diropres_u.diropres.file,
				      sizeof(nfs_fh));
				ret.status=NFS_OK;
				break;
			    case -1:
				ret.status=cfsno(errno);
				break;
			    default:
				ret.status=cfserrno;
				break;
			}
			break;
		    case -1:	/* error from syscall */
			ret.status=cfsno(errno);
			break;
		    default:	/* bad handle, probably */
			ret.status=cfserrno;
			break;
		}
		break;
	    case -1:
		ret.status=cfsno(errno);
		break;
	    default:
		ret.status=cfserrno;
		break;
	}
	become(NULL);
	return (&ret);
}


nfsstat *
nfsproc_remove_2(ap,rp)
     diropargs *ap;
     SR rp;
{
	static nfsstat ret;
	char *s;
	cfskey *key;
	int uid;
	int ht;
	cfs_fileid *h;

#ifdef DEBUG
	printf("remove:\n");
#endif
	if ((ht=htype(h=geth(&ap->dir)))!=H_REG) {
		ret=herr(ht);
		return &ret;
	}
	uid=ruid(rp);
	become(rp);	/* become the user */
	/* first encrypt the name */
	key=keyof(h);
	if (key==NULL) {
		ret=cfserrno;
	} else if ((s=encryptname(key,ap->name)) == NULL) {
		ret=NFSERR_NOENT; /* close enough */
	} else switch (fhdelete(h,s)) {
	    case 0:
		ret=NFS_OK;
		break;
	    case -1:
		ret=cfsno(errno);
		break;
	    default:
		ret=cfserrno;
	}
	become(NULL);
	return (&ret);
}


nfsstat *
nfsproc_rename_2(ap,rp)
     renameargs *ap;
     SR rp;
{
	static nfsstat ret;
	int uid;
	char s1[NFS_MAXPATHLEN+1], s2[NFS_MAXPATHLEN+1];
	char *s;
	cfskey *key;
	int ht;
	cfs_fileid *fromh, *toh;

#ifdef DEBUG
	printf("rename:\n");
#endif
	if ((ht=htype(fromh=geth(&ap->from.dir)))!=H_REG) {
		ret=herr(ht);
		return &ret;
	}
	uid=ruid(rp);
	/* just make sure these are the same fs instance
	   implicitly, this checks for a valid todir */
	if (iid(fromh) != iid(toh=geth(&ap->to.dir))) {
		/* what to return? there's no NFSERR_XDEV */
		ret = NFSERR_EXIST;
		return &ret;
	}
	become(rp);	/* become the user */

	if ((key=keyof(fromh)) == NULL)
		ret=cfserrno;
	else if (((s=encryptname(key,ap->from.name)) == NULL) ||
		 (strcpy(s1,s)==NULL))
		ret=cfserrno;
	else if (((s=encryptname(key,ap->to.name)) == NULL) ||
		 (strcpy(s2,s)==NULL))
		ret=cfserrno;
	/* if s1 already has a handle, rename updates its underlying name */
	else switch (fhrename(fromh,s1,
			      toh,s2)) {
	    case -1:
		ret=cfsno(errno);
		break;
	    case -2:
		ret=cfserrno;
		break;
	    default:
		ret=NFS_OK;
	}

	become(NULL);
	return (&ret);
}


nfsstat *
nfsproc_link_2(ap,rp)
     linkargs *ap;
     SR rp;
{
	static nfsstat ret;
	int uid;
	char *s;
	cfskey *key;
	int ht;
	cfs_fileid *fromh, *toh;

#ifdef DEBUG
	printf("link:\n");
#endif
	if ((ht=htype(fromh=geth(&ap->from)))!=H_REG) {
		ret=herr(ht);
		return &ret;
	}
	uid=ruid(rp);
	/* again, make sure these are the same fs instance
	   implicitly, this checks for a valid todir, since iip
	   returns -1 on error */
	if (iid(fromh) != iid(toh=geth(&ap->to.dir))) {
		/* what to return? there's no NFSERR_XDEV */
		ret = NFSERR_EXIST;	/* got a better idea? */
		return &ret;
	}
	if ((key=keyof(toh))==NULL) {
		ret=cfserrno;
		return &ret;
	}
	become(rp);
	if ((s=encryptname(key,ap->to.name))==NULL)
		ret=cfserrno;
	else switch (fhdohardlink(fromh,toh,s)) {
	    case -1:
		ret=cfsno(errno);
		break;
	    case -2:
		ret=cfserrno;
		break;
	    default:
		ret=NFS_OK;
		break;
	}
	become(NULL);
	return (&ret);
}


nfsstat *
nfsproc_symlink_2(ap,rp)
     symlinkargs *ap;
     SR rp;
{
	static nfsstat ret;
	int uid;
	char *s;
	char s1[NFS_MAXPATHLEN+1], s2[NFS_MAXPATHLEN+1];
	cfskey *key;
	int ht;
	cfs_fileid *h;

#ifdef DEBUG
	printf("symlink:\n");
#endif
	if ((ht=htype(h=geth(&ap->from.dir)))!=H_REG) {
		ret=herr(ht);
		return &ret;
	}
	uid=ruid(rp);
	if ((key=keyof(h))==NULL) {
		ret=cfserrno;
		return &ret;
	}
	become(rp);
	if ((s=encryptname(key,ap->from.name))==NULL)
		ret=cfserrno;
	else {
		strcpy(s1,s);
		s=encryptname(key,ap->to); /* can't fail */
		strcpy(s2,s);
		/* ignore the attributes stuff */
		switch (fhdosymlink(h,s1,s2)) {
		    case -1:
			ret=cfsno(errno);
			break;
		    case -2:
			ret=cfserrno;
			break;
		    default:
			ret=NFS_OK;
			break;
		}
	}
	become(NULL);
	return (&ret);
}

diropres *
nfsproc_mkdir_2(ap,rp)
     createargs *ap;
     SR rp;
{
	static diropres ret;
	nfs_fh handle;
	char *s;
	int uid;
	int ht;
	cfs_fileid *h, *h2;
	char *k;

#ifdef DEBUG
	printf("mkdir:\n");
#endif
	/* this, lookup, create, and the mount protocol are the only ways
	   to create a handle */
	if (!goodsrc(rp)) {
		ret.status=NFSERR_PERM;
		return &ret;
	}
	if ((ht=htype(h=geth(&ap->where.dir)))!=H_REG) {
		ret.status=herr(ht);
		return &ret;
	}
	uid=ruid(rp);
	become(rp);
	
	/* first create the dir entry */
	/* if one already exists, the underlying handle name field gets
	   updated to reflect its new name */

	s=encryptname(keyof(h),ap->where.name);
	switch (fhmkdirent(h,s,&handle)) {
	    case 0:
		switch (fhsetattr(h2=geth(&handle),&ap->attributes)) {
		    case 0:	/* ok */
			/* now do a getattr */		
			switch (fhstat(h2=geth(&handle),
				       &ret.diropres_u.diropres.attributes)) {
			    case 0:
				bcopy(&handle,&ret.diropres_u.diropres.file,
				      sizeof(nfs_fh));
				ret.status=NFS_OK;
				break;
			    case -1:
				ret.status=cfsno(errno);
				break;
			    default:
				ret.status=cfserrno;
				break;
			}
			break;
		    case -1:	/* error from syscall */
			ret.status=cfsno(errno);
			break;
		    default:	/* bad handle, probably */
			ret.status=cfserrno;
			break;
		}
		break;
	    case -1:
		ret.status=cfsno(errno);
		break;
	    default:
		ret.status=cfserrno;
		break;
	}
	become(NULL);
	return (&ret);
}


nfsstat *
nfsproc_rmdir_2(ap,rp)
     diropargs *ap;
     SR rp;
{
	static nfsstat ret;
	char *s;
	cfskey *key;
	int uid;
	int ht;
	cfs_fileid *h;

#ifdef DEBUG
	printf("rmdir:\n");
#endif
	if ((ht=htype(h=geth(&ap->dir)))!=H_REG) {
		ret=herr(ht);
		return &ret;
	}
	uid=ruid(rp);
	become(rp);	/* become the user */
	/* first encrypt the name */
	key=keyof(h);
	if (key==NULL) {
		ret=cfserrno;
	} else if ((s=encryptname(key,ap->name)) == NULL) {
		ret=NFSERR_NOENT; /* close enough */
	} else switch (fhdeletedir(h,s)) {
	    case 0:
		ret=NFS_OK;
		break;
	    case -1:
		ret=cfsno(errno);
		break;
	    default:
		ret=cfserrno;
	}
	become(NULL);
	return (&ret);
}

#define MAXENTRIES 128
#define MAXENTSIZE (NFS_MAXNAMLEN + sizeof(entry) + 32)
#define dentsize(s) (strlen(s) + sizeof(entry) + 32)

/* #define cfsclosedir(x) fhclosedir(x) */

readdirres *
nfsproc_readdir_2(ap,rp)
     readdirargs *ap;
     SR rp;
{
	static readdirres ret;
	int uid;
	int ht;
	static entry entrytab[MAXENTRIES];	/* just deal w/ it staticaly */
	typedef char str[NFS_MAXNAMLEN+1];
	static str names[MAXENTRIES];
	static DIR *dp=NULL;
	static struct dirent *dent;
	entry **prev;
	long loc;
	char s1[NFS_MAXNAMLEN+1];
	char *s;
	cfskey *key;
	static long curcookie=0;
	long cookie;
	int eof;
	int ne;
	int bytes;
	DIR *cfsopendir();
	cfs_fileid *h;
	static int fid= -10;

#ifdef DEBUG
	printf("readdir:\n");
#endif
	if ((ht=htype(h=geth(&ap->dir)))!=H_REG) {
		/* this is an ugly hack to deal w/ readdir of root, */
		if (ht==H_ROOT)
			return (rootreaddir(ap));
		ret.status=herr(ht);
		return &ret;
	}
	uid=ruid(rp);
	become(rp);	/* become the user */

	bytes = ap->count - MAXENTSIZE;
	ne=0;
	prev= &ret.readdirres_u.reply.entries;
	*prev=NULL;
	bcopy(ap->cookie,&cookie,sizeof(cookie));
	eof=TRUE;
	
	key=keyof(h);
	ret.status=NFS_OK;
	if (!((h->fileid==fid)&&(cookie==curcookie))) {
		if (dp!=NULL) {
			cfsclosedir(dp);
			dp=NULL;
		}
		if ((dp=cfsopendir(h,cookie)) == NULL) {
			fid = -10;
			ret.status=cfserrno;
		} else {
			fid=h->fileid;
			curcookie=cookie;
			dent=readdir(dp);
		}
	}
	if (dp!=NULL) while (dent!=NULL) {
		s=decryptname(key,dent->d_name);
		cookie++;
		if (s==NULL) { /* just skip bogus names */
			dent=readdir(dp);
			continue;
		}
		eof=FALSE;
		if (((bytes -= dentsize(s)) < 0) || (ne >= MAXENTRIES))
			break;
		eof=TRUE;
		strcpy(names[ne],s);
		entrytab[ne].name=names[ne];
		if (strcmp(s,".")==0)	/* me */
			entrytab[ne].fileid=fhid(h);
		else if (strcmp(s,"..")==0)	/* parent */
			entrytab[ne].fileid=fhpid(h);
		else entrytab[ne].fileid=dent->d_fileno;
		bcopy(&cookie,entrytab[ne].cookie,sizeof(long));
		*prev = &entrytab[ne];
		prev = &entrytab[ne].nextentry;
		entrytab[ne].nextentry=NULL;
		ne++;
		curcookie++;
		dent=readdir(dp);
	}
	ret.readdirres_u.reply.eof=eof;
	if (dent==NULL) {
		fid= -10;
		cfsclosedir(dp);
		dp=NULL;
	}
	become(NULL);
	return (&ret);
}

DIR *curdir;
int curdirid=0;
long curcookie;

DIR *
cfsopendir(dir,cookie)
     cfs_fileid *dir;
     long cookie;
{
	DIR *ret;
	DIR *fhopendir();

	if ((ret=fhopendir(dir))==NULL)
		return NULL;	/* fhopendir sets cfserrno */
	/* this is an ugly hack to deal with braindead systems (afs) that can't
	   do seekdir properly.  fortunately, we almost always already have
	   the dir open so we don't have to do this bogosity */
	while (cookie-- > 0)
		readdir(ret);
	return ret;
}

cfsclosedir(dp)
     DIR *dp;
{
	fhclosedir(dp);
}

statfsres *
nfsproc_statfs_2(ap,rp)
     nfs_fh *ap;
     SR rp;
{
	static statfsres ret;

#ifdef DEBUG
	printf("statfs: %s\n",pfh(ap));
#endif
	setstatfsokres(&ret.statfsres_u.reply);
	ret.status=NFS_OK;
	return (&ret);
}

rootgetattr(f)
	struct fattr *f;
{
	f->type=NFDIR;
	f->mode=NFSMODE_DIR|0777;
	f->nlink=4;
	f->uid=0;
	f->gid=0;
	f->size=8192;
	f->blocksize=8192;
	f->rdev= -1;
	f->blocks=1;
	f->fsid=0;
	f->fileid=1;
	bcopy(&roottime,&f->atime,sizeof(nfstime));
	bcopy(&roottime,&f->mtime,sizeof(nfstime));
	bcopy(&roottime,&f->ctime,sizeof(nfstime));
	
}

setstatfsokres(s)
	statfsokres *s;
{
	s->tsize=8192;
	s->bsize=8192;
	s->blocks=0;
	s->bfree=0;
	s->bavail=0;
}

int fhstat(fh,fa)
     cfs_fileid *fh;
     fattr *fa;
{
	struct stat sb;
	int r;
	
	if (fh==NULL) {
		cfserrno=NFSERR_STALE;
		return -2;
	}
	if (fh== &rootnode) {
		rootgetattr(fa);
		return 0;
	}
		
	if ((r=fhgetstat(fh,&sb)) != 0)
		return r;
	fa->type = getftype(sb.st_mode);
	fa->mode = fhmode(fh,sb.st_mode);
	fa->nlink = sb.st_nlink;
	fa->uid = fhowner(fh,sb.st_uid);
	fa->gid = sb.st_gid;
	fa->size = sb.st_size; /* already tweeked */
#ifdef irix
    fa->blocksize = 512;
    fa->blocks = (sb.st_size + 511)/512;
#else
	fa->blocksize = sb.st_blksize;
	fa->blocks = sb.st_blocks;
#endif
	fa->rdev = sb.st_rdev;
	fa->fsid = iid(fh); /* we return the instance id */
	fa->fileid = fhid(fh); /* inode or 0-i for iroots */
	bcopy(&sb.st_atime,&fa->atime,sizeof(time_t));
	bcopy(&sb.st_mtime,&fa->mtime,sizeof(time_t));
	bcopy(&sb.st_ctime,&fa->ctime,sizeof(time_t));
	return 0;
}

/* return uid associated w/ rpc call - unix style only for now */
/* note that this really doesnt belong here */
int
ruid(rp)
     SR rp;
{
	struct authunix_parms *cred;

	if (rp->rq_cred.oa_flavor != AUTH_UNIX)
		return -1;
	cred=(struct authunix_parms *) rp->rq_clntcred;
	return cred->aup_uid;
}

int
rgid(rp)
     SR rp;
{
	struct authunix_parms *cred;

	if (rp->rq_cred.oa_flavor != AUTH_UNIX)
		return -1;
	cred=(struct authunix_parms *) rp->rq_clntcred;
	return cred->aup_gid;
}

/* this is ugly, since it duplicates much of the code
   above in lookup, but it does the job */
diropres *
rootlookup(s)
     char *s;
{
	static diropres ret;
	nfs_fh *handle;
	
	handle = &ret.diropres_u.diropres.file;
	switch (fhrootlook(s,handle)) {
	    case 0:	/* found it */
		/* now do a getattr - handle is an instance root, which
		   is treated normally */
		switch (fhstat(geth(handle),
			       &ret.diropres_u.diropres.attributes)) {
		    case 0:
			ret.status=NFS_OK;
			/* stat is already in attributes field */
			break;
		    case -1:	/* couldn't open file for some reason */
			ret.status=cfsno(errno);
			break;
		    default:	/* shouldn't happen */
			ret.status=cfserrno;
			break;
		}
		break;
	    case -1:	/* some parent dir went away, probably */
		ret.status=cfsno(errno);
		break;
	    default:	/* bad parent handle or notfound, probably */
		ret.status=cfserrno;
		break;
	}
	return (&ret);
}

/* ugly ugly ugly - duplicates fair bit of code from readdir, but at least
   it works */
readdirres *
rootreaddir(ap)
    readdirargs *ap;

{
	static readdirres ret;
	static entry entrytab[MAXENTRIES];	/* just deal w/ it staticaly */
	typedef char str[NFS_MAXNAMLEN+1];
	static str names[MAXENTRIES];
	entry **prev;
	long cookie;
	int eof;
	int ne;
	int bytes;
	cfs_fileid *h;
	struct dirent *dent;
	struct dirent *rootrd();

	bytes = ap->count - MAXENTSIZE;
	ne=0;
	prev= &ret.readdirres_u.reply.entries;
	*prev=NULL;
	bcopy(ap->cookie,&cookie,sizeof(long));
	eof=1;
	
	ret.status=NFS_OK;
	while ((dent=rootrd(cookie))!=NULL) {
		eof=0;
		if (((bytes -= dentsize(dent->d_name))<0)||(ne >= MAXENTRIES)){
			break;
		}
		eof=1;
		strcpy(names[ne],dent->d_name);
		entrytab[ne].name=names[ne];
		if (strcmp(dent->d_name,".")==0)	/* me */
			entrytab[ne].fileid=1;
		else if (strcmp(dent->d_name,"..")==0) /* parent, also me */
			entrytab[ne].fileid=1;
		else entrytab[ne].fileid=dent->d_fileno;
		cookie=dent->d_reclen;	/* may not work everywhere */
		*prev = &entrytab[ne];
		bcopy(&cookie,entrytab[ne].cookie,sizeof(long));
		prev = &entrytab[ne].nextentry;
		entrytab[ne].nextentry=NULL;
		ne++;
	}
	ret.readdirres_u.reply.eof=eof;
	return (&ret);
}

extern struct in_addr validhost;

goodsrc(rp)
     SR rp;
{
	struct in_addr ia;
	struct in_addr *ip;

	if (svc_getcaller(rp->rq_xprt)->sin_addr.s_addr != validhost.s_addr) {
		fprintf(stderr,"bad addr! %x !- %x\n",
		  svc_getcaller(rp->rq_xprt)->sin_addr.s_addr,validhost.s_addr);
		return 0;
	}
	if (rp->rq_cred.oa_flavor != AUTH_UNIX)
		return 0;	/* bolster defense against portmap bug */
#ifndef ANYPORT
	if (ntohs(svc_getcaller(rp->rq_xprt)->sin_port) > 1023) {
		fprintf(stderr,"bad port! %d\n", ntohs(svc_getcaller(rp->rq_xprt)->sin_port));
		return 0;
	}
#endif
	return 1;
}
