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
 * cfs file handle support - 1.4
 *  local file system interface
 */

#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/errno.h>
#ifdef NeXT
#include <sys/dir.h>
#define dirent direct
#else
#include <dirent.h>
#endif
#include <rpc/rpc.h>
#include <fcntl.h>
#include <stdio.h>
#ifdef hpux
#define NO_UTIMES
#include <time.h>
#endif
#ifdef	ultrix
#include <time.h>
#endif	/* ultrix */
#ifdef NO_UTIMES
#include <utime.h>
#endif

#include "nfsproto.h"
#include "admproto.h"
#include "cfs.h"

#ifdef NO_UTIMES
/* simulate utimes with utime */
int utimes(s,tv)
char *s;
struct timeval tv[2];
{
    struct utimbuf ut;
    ut.actime = tv[0].tv_sec;
    ut.modtime = tv[1].tv_sec;
    return utime(s,&ut);
}
#endif

#define hash(x) ((u_long)((x)&HMASK))
int inst = 0;/* starting point */
instance *instances[NINSTANCES];

#define MAXFDS 12
fdcache *fds=NULL;
int nfds=0;	/* number of open fd's currently in cache */

int cfserrno=NFS_OK;

/*
 * get an encrypted block from fd, which damn well better be valid
 */
int
readblock(blk,fd,offset,len,key,vect)
     char *blk;     /* data */
     int fd;        /* fd */
     u_long offset; /* offset from byte zero */
     u_long len;    /* blk len */
     cfskey *key;     /* des key */
     char *vect;  /* perturbation vector */
{
	u_long begin;
	u_long fronterr, totlen,datalen;
	int iolen;
	int readmore=0;
	int userbytes;
	char buf[8208];	/* big enough, may not even need it */

	/* first, normalize to the proper boundries */
	fronterr = offset&(CFSBLOCK-1);
	datalen=len+fronterr;
	begin=offset-fronterr;
	totlen=vtod(datalen);
	/* now begin and totlen are what we do the actual i/o on */
	if (totlen>8208)
		return -1;	/* shouldn't happen */
	if (lseek(fd,begin,L_SET)<0)
		return -1;
	/* note that we are violating standard unix semantics here - we have
	   no way to distinguish between zeros in the encrypted file and
	   holes in the file, which should be returned to the user as
	   zeros.  This could be fixed by detecting that we're creating
	   holes at write time and filling w/ encrypted zeros, but that
	   violates other semantics. */
	if ((iolen=read(fd,buf,totlen)) < 0)
		return -1;
	userbytes=dtov(iolen)-fronterr;
	if (userbytes < 0) /* empty file */
		return 0;
	dodecrypt(key,buf,iolen,begin,vect);
	/* now copyback to the user's buffer */
	bcopy((char *)(buf+fronterr),(char *)blk,userbytes);
	return(userbytes);
}

/*
 * write an encrypted block to fd
 */
int
writeblock(blk,fd,offset,len,key,vect)
     char *blk;     /* data */
     int fd;        /* fd */
     u_long offset; /* offset from byte zero */
     u_long len;    /* blk len */
     cfskey *key;     /* des key */
     char *vect;    /* perturb. vector */
{
	u_long begin, end;
	u_long fronterr, totlen,datalen;
	int iolen;
	int headlen;
	int writemore=0;
	struct stat sb;
	u_long mask = ~0;	/*tells us whether to expand file */
	char buf[8216];	/* big enough, may not even need it */

	/* first, normalize to the proper boundries */
	fronterr = offset&(CFSBLOCK-1);
	datalen=len+fronterr;
	begin=offset-fronterr;
	totlen=vtod(datalen);
	end=begin+datalen+CFSBLOCK; /* lower bound on end of file */
	if (fstat(fd,&sb)<0)
		return -1;	/* file's not there */
	/* now begin and datalen/totlen are what we do the actual i/o on */
	if (totlen>8208)
		return -1;	/* shouldn't happen */
	bzero((char *)buf,totlen);
	if ((totlen!=len) && (sb.st_size>begin)) {
			/* read begining and ending slop first */
			/* we could reduce the size of this read to just
			   the CFSBLOCKs at the head &/| tail of the file,
			   but that's probably more trouble than it's worth.*/
		if (lseek(fd,begin,L_SET)<0)
			return -1;
		/* the extra CFSBLOCK ensures that next block is ok */
		iolen=isbndry(totlen)?totlen:(totlen+CFSBLOCK);
		if ((read(fd,buf,iolen))<0)
			return -1;
		dodecrypt(key,buf,iolen,begin,vect);
	}

	/* now we can do the actual encrypt and i/o */
	bcopy((char *)blk,(char *)(buf+fronterr),len);
	doencrypt(key,buf,totlen,begin,vect);
	if (lseek(fd,begin,L_SET)<0)
		return -1;
	/* again, holes in the file are a delicate problem.  If the CFSBLOCK
	   bountry is not always on a file boundry, you could even corrupt
	   valid data.  Fortunately, these both are generally powers of 2,
	   so it's not an issue. */
	if ((iolen=write(fd,buf,totlen)) < 0) {
		perror("write");
		return -1;
	}
	/* iolen may contain CFSBLOCK extra chars */
	return(dtov(iolen)-fronterr);
}


#define MAXCLEARNAME ((NFS_MAXPATHLEN-7)/2)


/*
 * encrypt a path componenet (no /'s) element
 *  leave "." and ".." unmolested
 */
char *
encryptname(key,s)
     cfskey *key;
     char *s;
{
	static char cryptname[NFS_MAXNAMLEN+1];
	u_char cryptstring[MAXCLEARNAME+CFSBLOCK+1];
	u_char x[3];
	u_long l;
	int i;

	if ((s==NULL) || ((l=strlen(s)+1)>MAXCLEARNAME))
		return NULL;
	if (!strcmp(s,".") || !strcmp(s,".."))
		return s;
	l=(l+(CFSBLOCK-1)) & (~(CFSBLOCK-1));
	bzero((char *)cryptstring,l);
	strcpy(cryptstring,s);
	chksum(cryptstring,l);
	doencrypt(key,cryptstring,l,10241,zerovect);
	cryptname[0]='\0';
	for (i=0; i<l; i++) {
		sprintf((char *)x,"%02x",cryptstring[i]);
		strcat(cryptname,x);
	}
	return cryptname;
}

/*
 * set high order bits
 */
chksum(s,l)
     char *s;
     long l;
{
	u_long acc;
	int i;
	u_char bits[8];

	acc=0;
	for (i=0; s[i]!='\0'; i++)
		acc += s[i]*((i%6)+1);
	for (i++; i<l; i++)	/* fill up the end */
		s[i] = s[i%8];
	for (i=0; i<8; i++)
		bits[i] = (acc<<(i%8))&0x80;
	for (i=0; i<l; i++)
		s[i] |= bits[i%8];
}


/*
 * decrypt path component
 *  leaving "." and ".."
 */
char *
decryptname(key,s)
     cfskey *key;
     char *s;
{
	static char clearstring[MAXCLEARNAME+CFSBLOCK+1];
	char x[3];
	int y;
	char *p, *q;
	int l;
	int i;

	if (s==NULL)
		return NULL;
	if (!strcmp(s,".") || !strcmp(s,".."))
		return s;
	bzero((char *)clearstring,MAXCLEARNAME+CFSBLOCK+1);
	/* unencode the string */
	p=s;
	q=clearstring;
	x[2]='\0';
	l=0;
	while (*p) {
		x[0] = *p++;
		if ((x[1] = *p++) == '\0')
			return NULL;
		if (sscanf(x,"%x",&y)!=1)
			return NULL;
		*q++ = y;
		l++;
	}
	*q='\0';	/* shouldnt need this */
	if (l%CFSBLOCK)
		return NULL;
	dodecrypt(key,clearstring,l,10241,zerovect);
	for (i=0; (clearstring[i]&0x7f) !='\0'; i++)
		clearstring[i] &= 0x7f;
	clearstring[i]='\0';
	return clearstring;
}

doencrypt(k,s,l,salt,vect)
     cfskey *k;
     char *s;
     int l;
     int salt;
     char *vect;
{
	int i,j,smsize;
	
	smsize=k->smsize;
	for (i=0; i<l; i+=8) {
		for (j=0; j<8; j++)
			s[i+j] ^= k->primask[(i+j+salt)%smsize]
				^ vect[j]
				^ (((i+j+salt)/smsize)&0377);
				/* makes big offsets have different masks */
		cipher(k,&s[i],0);
		for (j=0; j<8; j++)
			s[i+j] ^= k->secmask[(i+j+salt)%smsize];
	}
}

dodecrypt(k,s,l,salt,vect)
     cfskey *k;
     char *s;
     int l;
     int salt;
     char *vect;
{
	int i,j,smsize;
	
	smsize=k->smsize;
	for (i=0; i<l; i+=8) {
		for (j=0; j<8; j++)
			s[i+j] ^= k->secmask[(i+j+salt)%smsize];
		cipher(k,&s[i],1);
		for (j=0; j<8; j++)
			s[i+j] ^= k->primask[(i+j+salt)%smsize]
				^ vect[j]
				^ (((i+j+salt)/smsize) & 0377);
	}
		
}

/*
 * convert size of data to size of block to be written
 */
vtod(d)
     int d;
{
	return isbndry(d)?d:(d+CFSBLOCK);
}


/*
 * convert file size into #of valid bits
 */
dtov(d)
     int d;
{
	return isbndry(d)?d:(d-CFSBLOCK);
}

/*
 * is block size a CFS boundry?
 */
isbndry(d)
     int d;
{
	return ((d&(CFSBLOCK-1))==0);
}


static fh_u roothandle;
cfs_fileid rootnode={1,0,"\0\0\0\0\0\0\0\0","/NOWHERE/null",NULL,NULL,0,NULL,NULL};/* fileid=1; should be unique */
nfstime roottime={0,0};

/* Had to change this - someone else was using the same magic number! */
/* PLEASE change this value if you're going to re-use this code for
/* something else!  Otherwise your FS and CFS can't both run... */
/* static u_char magictest[8]={0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef}; */
static u_char magictest[8]="M.Blaze";

static cfs_fileid *
findh(id,ins
#ifdef SHORTLINKS
	    ,linkid)
     int linkid;
#else /* SHORTLINKS */
     )
#endif /* SHORTLINKS */
     int id;
     int ins;
{
	u_int bucket;
	cfs_fileid *f;
	
	bucket=hash(id);
	f=instances[ins]->file[bucket];
	while (f!=NULL) {
		if (f->fileid == id)	/* note recast */
#ifdef SHORTLINKS
		    if (f->linkid == linkid)
#endif /* SHORTLINKS */
			return f;
		f=f->next;
	}
	return NULL;
}


cfs_fileid *
geth(h)
     fhdata *h;	/* note recasting here - may give trouble with strict ansi */
{
	int i;
	cfs_fileid *r;

	if (h==NULL)
		return &rootnode;
	/* check for rootness */
	for (i=0; i<8; i++)
		if (h->magic[i] != magictest[i]) {
			/* update the root handle, so we return the
			   latest thing it gave out for lookups of /.  */
			bcopy((char *)h,(char*)&roothandle,sizeof(roothandle));
			return &rootnode;
		}
	if (instances[h->instance] == NULL)
		return NULL;	/* invalid/stale instance */
	r=findh(h->fileid,h->instance
#ifdef SHORTLINKS
				     ,h->linkid
#endif /* SHORTLINKS */
				     );
	/* instance may have timed out */
	if ((r != NULL) && (r->ins->dead != 0)) {
		/* if so, we GC it first */
		freeinstance(h->instance);
		r=NULL;
	}
	if ((r==NULL) || (bcmp(r->ins->check,h->check,8)!=0))
		return NULL;	/* cheater */
	return r;
}


htype(f)
     cfs_fileid *f;
{
	if (f == &rootnode)
		return H_ROOT;
	else if (f == NULL)
		return H_INVALID;
	else
		return H_REG;
}

iid(f)
     cfs_fileid *f;
{
	if (f == &rootnode)
		return -1;
	else if (f == NULL)
		return -2;
	else
		return f->ins->id;
}

fhowner(f,uid)
     cfs_fileid *f;
     int uid;
{
	if (f->fileid == 0)	/* an instance root */
		return (f->ins->uid);	/* return the instance owner */
	return (uid);	/* normal case */
}

fhmode(f,mode)
     cfs_fileid *f;
     int mode;
{
	if (f->fileid == 0)	/* an instance root */
		return (mode&(~077));	/* set a umask of 0077 */
	return (mode);
}

/* return the inode of me */
fhid(f)
     cfs_fileid *f;
{
	if (f==NULL)
		return 1;
	if (f->fileid == 0)	/* an instance root */
		return (0-f->ins->id);	/* should be unique enough */
	return (f->fileid);
}

/* return the inode of .. (directory only) */
fhpid(f)
     cfs_fileid *f;
{
	if (f==NULL)	/* this is main root */
		return 1;
	if (f->parent<=0) /* child of instance root */
		if (f->fileid==0)
			return 1;
		else
			return (0-f->ins->id);
	return f->parent;	/* this is already 2 for main root  */
}

/* is uid authorized for this? */
fhuid(f,u)
     cfs_fileid *f;
     int u;
{
	if (f==NULL)
		return 1;
	if (f->fileid == 1)	/* special case */
		return 1;
	if (f->ins->idle && f->fileid) /* update idle timer */
		f->ins->access=cursecs;
	return (f->ins->uid==u);
}

cfsno(err)
     int err;
{
/* 	if (err==ENOENT)
		return NFSERR_STALE;
	else */
		return err;
}


static cfs_fileid *openfd=NULL;
static int curmode=0700;
static int curfd= -1;

/* open file descriptor for handle.  maintains cache of one fd */
int
fhtofd(f,mode)
     cfs_fileid *f;
     int mode;
{
	int fd;

#ifdef DEBUG
	fprintf(stderr,"Translating %s\n",f->name);
#endif
	if (f==NULL) {
		cfserrno=NFSERR_STALE;
		return -2;
	}
	if (curfd>=0) { /* it's open already */
		if ((openfd==f) && (curmode==mode)) /* yay */
			return curfd;
		close(curfd);	/* hope it was valid... */
		openfd=NULL;
	}

	if (mode==0) { mode=CFS_WRITE; }
	/* Phil Karn's hack for R/O file systems */
	if ((fd=open(f->name,mode,0))<0 && errno == EROFS) {
		mode = CFS_READ;        /* Force read and try again */
		fd = open(f->name,mode,0);
	}

	if(fd<0) {
		cfserrno=cfsno(errno);
		if ((errno == EACCES) && (mode==CFS_WRITE)) { /* ugly hack */
			int omode;

			omode=modeof(f->name);
			if (omode<0)
				return -2;
			chmod(f->name,0700);
			fd=open(f->name,mode,0);
			chmod(f->name,omode);
			if (fd<0)
				return -2;
		} else return -2;
	}
	/* now we have to make sure the vect didn't slide out from under us */
	/* check for a linkfile */
	if (readlink(f->vectname,f->vect,9) != 8)
		bcopy((char *)zerovect,(char *)f->vect,8);
	curfd=fd;
	curmode=mode;
	openfd=f;
	return fd;
}

int
modeof(s)
     char *s;
{
	struct stat sb;

	if (lstat(s,&sb)<0)
		return -1;
	return((int)sb.st_mode);
}

int
fhgetstat(h,sb)
     cfs_fileid *h;
     struct stat *sb;
{
	if (h == NULL) {
		cfserrno=NFSERR_STALE;
		return -2;
	}
	/* lstat is correct even for instance roots, since their names
	   end in /. */
	if (lstat(h->name,sb)<0)
		return -1;
	sb->st_size=dtov(sb->st_size); /* tweek size */
	return 0;
}

closeout(f)
     cfs_fileid *f;
{
	if (openfd==f) {
		if (curfd>=0)
			close(curfd);
		openfd=NULL;
		curfd = -1;
	}
}

closeall()
{
	if (openfd!=NULL) {
		if (curfd>=0)
			close(curfd);
		openfd=NULL;
		curfd = -1;
	}
}

fhsetattr(f,a)
     cfs_fileid *f;
     sattr *a;
{
	struct timeval tv[2];
	struct stat sb;
	
	if (f==NULL) {
		cfserrno=NFSERR_STALE;
		return -2;
	}
	if (f->fileid==0) { /* disallow on instance roots */
		cfserrno=NFSERR_PERM;
		return -2;
	}
	if (a==NULL) /* i dunno */
		return 0;
	if ((a->mode&0177777) != 0177777) {
		if (chmod(f->name,a->mode&0177777)<0)
			return -1;
	}
	if (((int)a->atime.seconds>=0) || ((int)a->mtime.seconds>=0)) {
		if ((int)a->atime.seconds<0) {
			lstat(f->name,&sb);
			tv[0].tv_sec=sb.st_atime;
			tv[0].tv_usec=0;
		} else {
			tv[0].tv_sec=a->atime.seconds;
			tv[0].tv_usec=a->atime.useconds;
		}
		if ((int)a->mtime.seconds<0) {
			lstat(f->name,&sb);
			tv[1].tv_sec=sb.st_mtime;
			tv[1].tv_usec=0;
		} else {
			tv[1].tv_sec=a->mtime.seconds;
			tv[1].tv_usec=a->mtime.useconds;
		}
		if (utimes(f->name,tv)<0)
			return -1;
	}
	if (((int)a->size) >= 0)
		if (truncate(f->name,vtod(a->size)))
			if (errno != EISDIR)
				return 0;
/*	printf("fhsetattr: atime=%x,%x  mtime=%x,%x\n",
	       a->atime.seconds, a->atime.useconds,
	       a->mtime.seconds, a->mtime.useconds); */
	return 0;
}

int
fhsetattrprime(f,a)
     cfs_fileid *f;
     sattr *a;
{
	struct timeval tv[2];
	struct stat sb;
	
	if (f==NULL) {
		cfserrno=NFSERR_STALE;
		return -2;
	}
	if (f->fileid==0) { /* disallow on instance roots */
		cfserrno=NFSERR_PERM;
		return -2;
	}
	if (a==NULL) /* i dunno */
		return 0;
	/* same as above, modulo the chmod */
	if (((int)a->atime.seconds>=0) || ((int)a->mtime.seconds>=0)) {
		if ((int)a->atime.seconds<0) {
			lstat(f->name,&sb);
			tv[0].tv_sec=sb.st_atime;
			tv[0].tv_usec=0;
		} else {
			tv[0].tv_sec=a->atime.seconds;
			tv[0].tv_usec=a->atime.useconds;
		}
		if ((int)a->mtime.seconds<0) {
			lstat(f->name,&sb);
			tv[1].tv_sec=sb.st_mtime;
			tv[1].tv_usec=0;
		} else {
			tv[1].tv_sec=a->mtime.seconds;
			tv[1].tv_usec=a->mtime.useconds;
		}
		if (utimes(f->name,tv)<0)
			return -1;
	}
	if (((int)a->size) >= 0)
		if (truncate(f->name,vtod(a->size)))
			if (errno != EISDIR)
				return 0;
	return 0;
}

fhmkdirent(p,comp,h)
     cfs_fileid *p;
     char *comp;
     fhdata *h;
{
	char path[NFS_MAXPATHLEN+1];
	struct stat sb;
	
	if (p==NULL) {
		cfserrno=NFSERR_STALE;
		return -2;
	}
	sprintf(path,"%s/%s",p->name,comp);
	if (mkdir(path,0)<0)
		return -1;
	if (stat(path,&sb)<0)
		return -1;
	return (mkhandle(p,path,comp,sb.st_ino,h,zerovect,"/NOWHERE/null"
#ifdef SHORTLINKS
									 ,FALSE
#endif /* SHORTLINKS */
									 ));
}

int
fhmkfileent(p,comp,h)
     cfs_fileid *p;
     char *comp;
     fhdata *h;
{
	char path[NFS_MAXPATHLEN+1];
	int fd;
	struct stat sb;
	int rs=0;
	int m;
	char vect[9];
	union{
		u_char ch[9];
		u_long i[2];
	} buf;
	char linkname[NFS_MAXPATHLEN+1];
	
	if (p==NULL) {
		cfserrno=NFSERR_STALE;
		return -2;
	}
	sprintf(path,"%s/%s",p->name,comp);

	if ((fd=open(path,O_CREAT|O_WRONLY|O_EXCL,0))<0) {
		if (errno==EEXIST) {
			if ((fd=open(path,O_WRONLY,0))<0)
				return -1;
			else
				rs=1;
		} else
			return -1;
	}
	if (fstat(fd,&sb)<0)
		return -1;
	close(fd);
	sprintf(linkname,"%s/.pvect_%s",p->name,comp);
	if (!rs) {
	   if (p->ins->highsec) {	/* create new pert file iff highsec */
		/* note that there's a race condition here until the simlink */
		/*
		sprintf((char *)buf,"%08x",(u_long)sb.st_ino+(u_long)sb.st_ctime);
		*/
		buf.i[0]=(u_long)sb.st_ino;
		buf.i[1]=(u_long)sb.st_ctime;
		q_block_cipher("fixedkey",&buf,1);
		/* des is just used here as a hash fn to spread the bits */
		/* since we only use 32 bits of the result, its a nonperfect */
		/* hash. but this doesn't really matter since collisions */
		/* are rare.  we could save all 64 bits, but the encoding */
		/* of the link would get messy and large */
		sprintf(vect,"%02x%02x%02x%02x",
			buf.ch[0],buf.ch[1],buf.ch[2],buf.ch[3]);
		if (symlink(vect,linkname) != 0) {
			strcpy(linkname,"/NOWHERE/null");
			bcopy((char *)zerovect,(char *)vect,8);
		}
	   } else {
		unlink(linkname);
		bcopy((char *)zerovect,(char *)vect,8);
		strcpy(linkname,"/NOWHERE/null");
	   }
	} else {
		if (readlink(linkname,vect,9) != 8) {
			bcopy((char *)zerovect,(char *)vect,8);
			strcpy(linkname,"/NOWHERE/null");
		}
		else vect[8]='\0';
	}
	m=mkhandle(p,path,comp,sb.st_ino,h,vect,linkname
#ifdef SHORTLINKS
							,FALSE
#endif /* SHORTLINKS */
							);
	if (m!=0)
		return m;
	return rs;
}


int
fhlook(p,comp,h)
     cfs_fileid *p;
     char *comp;
     fhdata *h;
{
	char path[NFS_MAXPATHLEN+1];
	char linkname[NFS_MAXPATHLEN+1];
	char vect[NFS_MAXPATHLEN+1];
	struct stat sb;
	
	if (p==NULL) {
		cfserrno=NFSERR_STALE;
		return -2;
	}
	if (p->fileid==0) {
		if (!strcmp(comp,"."))
			return fhrootlook(p->ins->name,h);
		if (!strcmp(comp,".."))
			return fhrootlook(".",h);
	}
	if ((p->parent==0) && (!strcmp(comp,"..")))
		return fhrootlook(p->ins->name,h);
	sprintf(path,"%s/%s",p->name,comp);
	if (lstat(path,&sb)<0) {
		return -1;	/* just need the inode */
	}
	/* check for a linkfile */
	sprintf(linkname,"%s/.pvect_%s",p->name,comp);
	if (readlink(linkname,vect,9) != 8) {
		bcopy((char *)zerovect,(char *)vect,8);
		strcpy(linkname,"/NOWHERE/null");
	}
	else vect[8]='\0';
#ifdef SHORTLINKS
	{
		struct stat sp;
		ino_t pino = p->fileid;
		if (pino == 0) {
			if (lstat(p->name, &sp) == 0)
				pino = sp.st_ino;
		}
		return (mkhandle(p,path,comp,sb.st_ino,h,vect,linkname,
				 (S_ISLNK(sb.st_mode)&&(pino == sb.st_ino))));
	}
#else /* SHORTLINKS */
	return (mkhandle(p,path,comp,sb.st_ino,h,vect,linkname));
#endif /* SHORTLINKS */
}


int
mkhandle(p,n,comp,ino,h,vect,vectname
#ifdef SHORTLINKS
				     ,isl)
     int isl;	/* is short link */
#else /* SHORTLINKS */
				     )
#endif /* SHORTLINKS */
     cfs_fileid *p;
     char *n;
     char *comp;
     int ino;
     fhdata *h;
     char *vect;
     char *vectname;
{
	cfs_fileid *f;

#ifdef SHORTLINKS
	if (!isl && ((f=findh(ino,p->ins->id,0)) != NULL)) {
#else /* SHORTLINKS */
	if ((f=findh(ino,p->ins->id)) != NULL) {
#endif /* SHORTLINKS */
		if (strcmp(comp,".") && strcmp(comp,"..")) {
			free(f->name);
			f->name=NULL;
		}
	} else {
		if ((f=(cfs_fileid *)malloc(sizeof(cfs_fileid)))==NULL) {
			fprintf(stderr,"cfsd: out of memory\n");
			cfserrno=NFSERR_STALE;	/* bad news */
			return -2;
		}
		f->fileid=ino;
		f->parent=p->fileid;	/* only need for directory */
#ifdef SHORTLINKS
		f->link_count = 0;	/* Always start at zero */
		if (isl)
			f->linkid = ++p->link_count;
		else
			f->linkid = 0;
#endif /* SHORTLINKS */
		f->ins=p->ins;
		f->name=NULL;
		inserth(f);
	}
	if (f->name==NULL) {
		if ((f->name=(char *)malloc(strlen(n)+1)) ==NULL) {
			fprintf(stderr,"cfsd: out of memory\n");
			cfserrno=NFSERR_STALE;
			return -2;
		}
		strcpy(f->name,n);
		strcpy(f->vectname,vectname);
		bcopy((char *)vect,(char *)f->vect,8);
		f->vect[8]='\0';
	}
	if (h!=NULL) {
		fillinh(h,p->ins->id,ino,p->ins->check);
#ifdef SHORTLINKS
		h->linkid = f->linkid;
		if (isl)
			h->htype = H_SLNK;
#endif /* SHORTLINKS */
	}
	return 0;
}

inserth(f)
     cfs_fileid *f;
{
	u_int bucket;
	
	bucket=hash(f->fileid);
	f->next=instances[f->ins->id]->file[bucket];
	instances[f->ins->id]->file[bucket]=f;
}

fillinh(h,iid,ino,check)
     fhdata *h;
     int iid;
     int ino;
     char *check;
{
	bzero((char *)h,sizeof(fhdata));
	bcopy((char *)magictest,(char *)h->magic,sizeof(magictest));
	bcopy((char *)check,(char *)h->check,8);
	h->htype=H_REG;
	h->instance=iid;
	h->fileid=ino;
}

int
fhdelete(f,s)
     cfs_fileid *f;
     char *s;
{
	char path[NFS_MAXPATHLEN+1];
	char linkname[NFS_MAXPATHLEN+1];
	int ret;

	if (f==NULL) {
		cfserrno=NFSERR_STALE;
		return -2;
	}
	sprintf(linkname,"%s/.pvect_%s",f->name,s);
	sprintf(path,"%s/%s",f->name,s);
	/* note that we don't bother to check and see if there's a
	   handle allocated for this - just wait for the inode to be
	   reclaimed */
	if ((ret=unlink(path)) == 0)
		unlink(linkname); /* doesn't matter if link isn't there */
	return (ret);
}

int
fhdeletedir(f,s)
     cfs_fileid *f;
     char *s;
{
	char path[NFS_MAXPATHLEN+1];

	if (f==NULL) {
		cfserrno=NFSERR_STALE;
		return -2;
	}
	sprintf(path,"%s/%s",f->name,s);
	return (rmdir(path));
}

DIR *
fhopendir(d)
     cfs_fileid *d;
{
	DIR *dp;
	
	if (d==NULL) {
		cfserrno=NFSERR_STALE;
		return NULL;
	}
	if ((dp=opendir(d->name))==NULL)
		cfserrno=cfsno(errno);
	return dp;
}

fhclosedir(dp)
     DIR *dp;
{
	closedir(dp);
}


int
fhlinkval(f,buf)
     cfs_fileid *f;
     char *buf;
{
	if (f==NULL) {
		cfserrno=NFSERR_STALE;
		return -2;
	}
	return(readlink(f->name,buf,NFS_MAXNAMLEN));
}


int
fhdohardlink(f,t,n)
     cfs_fileid *f;
     cfs_fileid *t;
     char *n;
{
	char buf[NFS_MAXPATHLEN+1];
	char linkname[NFS_MAXPATHLEN+1];
	char vectval[9];
	int ret;
	
	if ((f==NULL)||(t==NULL)) {
		cfserrno=NFSERR_STALE;
		return -2;
	}
	sprintf(buf,"%s/%s",t->name,n);
	if ((ret=link(f->name,buf))!=0)
			return ret;
	sprintf(linkname,"%s/.pvect_%s",t->name,n);
	unlink(linkname);
	if (readlink(f->vectname,vectval,9) == 8) {
		vectval[8]='\0';
		symlink(vectval,linkname);
	}
	return ret;
}

int
fhdosymlink(f,n,t)
     cfs_fileid *f;
     char *n;
     char *t;
{
	char buf[NFS_MAXPATHLEN+1];
	
	if (f==NULL) {
		cfserrno=NFSERR_STALE;
		return -2;
	}
	sprintf(buf,"%s/%s",f->name,n);
	return (symlink(t,buf));
}

int
fhrename(f,fn,t,tn)
     cfs_fileid *f;
     char *fn;
     cfs_fileid *t;
     char *tn;
{
	char fb[NFS_MAXPATHLEN+1];
	char tb[NFS_MAXPATHLEN+1];
	char fblink[NFS_MAXPATHLEN+1];
	char tblink[NFS_MAXPATHLEN+1];
	char vectval[9];
	
	if ((f==NULL)||(t==NULL)) {
		cfserrno=NFSERR_STALE;
		return -2;
	}
	sprintf(fb,"%s/%s",f->name,fn);
	sprintf(tb,"%s/%s",t->name,tn);
	sprintf(fblink,"%s/.pvect_%s",f->name,fn);
	sprintf(tblink,"%s/.pvect_%s",t->name,tn);
	if (rename(fb,tb)==0) { /* now we have to do a lookup */
		unlink(tblink);	/* may be a quick race cndtn here */
		if (readlink(fblink,vectval,9)==8) {
			vectval[8]='\0';
			symlink(vectval,tblink);
		}
		unlink(fblink);
		/* rename(fblink,tblink); */
		fhlook(t,tn,NULL);
		return 0;
	}
	return -1;
}

int
getftype(m)
     int m;
{
	if ((m&S_IFMT) == S_IFREG)
		return NFREG;
	if ((m&S_IFMT) == S_IFDIR)
		return NFDIR;
	if ((m&S_IFMT) == S_IFLNK)
		return NFLNK;
	if ((m&S_IFMT) == S_IFBLK)
		return NFBLK;
	if ((m&S_IFMT) == S_IFCHR)
		return NFCHR;
	if ((m&S_IFMT) == S_IFSOCK)
		return NFSOCK;
	return NFBAD;
}

/* should make . and .. the first entries, but this probably doesn't
   break anything */
struct dirent *
rootrd(cookie)
     long cookie;
{
	static struct dirent d;

	/* note that cookie should never == NINSTANCES */
	if (cookie == 0) {
		strcpy(d.d_name,".");
		d.d_fileno=1;
	} else if (cookie == 1) {
		strcpy(d.d_name,"..");
		d.d_fileno=1;
	} else while (cookie<(NINSTANCES+2)) {
		if (instances[cookie-2] != NULL) {
			if (instances[cookie-2]->anon)
				sprintf(d.d_name,".ANON_%d",cookie-2);
			else
				strcpy(d.d_name,
				       instances[cookie-2]->name);
			d.d_fileno=(0-(cookie-2));
			break;
		}
		++cookie;
	}
	if (cookie>=(NINSTANCES+2))
		return NULL;
	d.d_reclen = ++cookie;
	return &d;
}


int
fhrootlook(n,h)
     char *n;
     nfs_fh *h;
{
	int i;
	cfs_fileid *f;
	
	if (!strcmp(n,".") || !strcmp(n,"..")) {	/* self */
		bcopy((char *)&roothandle,(char *)h,sizeof(roothandle));
		return 0;
	}
	for (i=0; i<NINSTANCES; i++) {
		if (instances[i]==NULL)
			continue;
		if (!strcmp(instances[i]->name,n)) {
			if ((f=findh(0,i
#ifdef SHORTLINKS
					,0
#endif /* SHORTLINKS */
					)) == NULL) {
				if ((f=(cfs_fileid *)
				     malloc(sizeof(cfs_fileid)))==NULL) {
					fprintf(stderr,"cfsd: no memory\n");
					cfserrno=NFSERR_STALE;	/* bad news */
					return -2;
				}
				f->fileid = 0;
				f->parent = 0;
				f->ins=instances[i];
				inserth(f);
				if ((f->name=(char *)
				     malloc(NFS_MAXPATHLEN+1))==NULL) {
					fprintf(stderr,
						"cfsd: out of memory\n");
					cfserrno=NFSERR_STALE;
					return -2;
				}
				strcpy(f->name,instances[i]->path);
			}
			fillinh(h,i,0,f->ins->check);
			return 0;
		}
	}
	cfserrno=NFSERR_NOENT;
	return -2;
}
