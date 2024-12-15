/*
 * The author of this software is Matt Blaze.
 *              Copyright (c) 1992, 1993, 1994, 1995, 1997 by AT&T.
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
 * client side CFS attach - 1.4.0
 *
 * december 1997
 */
#include <stdio.h>
#include <rpc/rpc.h>
#include <sys/time.h>
#ifdef irix
#include <sys/statfs.h>
#else
#ifndef BSD44
#ifdef	ultrix
#include <sys/param.h>
#include <sys/mount.h>
#else
#include <sys/vfs.h>
#endif	/* ultrix */
#else
#ifndef MAXHOSTNAMELEN
#include <sys/param.h>
#endif
#include <sys/mount.h>
#endif	/* BSD44 */
#endif
#ifdef SOLARIS2X
#include <string.h>
#define rindex strrchr
#else
#include <strings.h>
#endif
#include "nfsproto.h"
#include "admproto.h"
#include "cfs.h"

#ifdef SOLARIS2X
/* NOTE!  delete the #define statfs below if this won't compile on
  your solaris configuration */
#define statfs		statvfs
#endif

#ifndef TMOUT		/* default timeout; override in makefile */
#define TMOUT 0
#endif

#ifndef IDLE		/* default idle timer; override in makefile */
#define IDLE 0
#endif

main(argc,argv)
     int argc;
     char **argv;
{
	cfs_attachargs ap;
	char *pw;
	char pword[256];
	int n;
	int status;
	cfsstat ret;
	char *getpassword();
	char dir[1024]; char buf[1024];
	char ins[1024];
#ifdef	ultrix
	struct fs_data sfb;
#define f_blocks  fd_req.btot
#else
	struct statfs sfb;
#endif
	char *flg;
	int ciph;
	FILE *fp;
	unsigned char ekey[128];
	char cname[1024];
	char kname[1024]; /* indirect key file */
	char sname[1024];
	char lname[1024]; /* ..data name */
	int smsize;
	int cfmt=0;
	static struct timeval tout = {60,0};
	CLIENT *cln;
	int timeout=TMOUT;
	int idle=IDLE;
	char *dirarg=NULL;
	char *namearg=NULL;
	int keycheck=1;

	ap.highsec=1;
	while (--argc) if (**++argv == '-') {
		for (flg = ++*argv; *flg; ++flg)
			switch (*flg) {
			    case 'l':
				ap.highsec=0;
				break;
			    case 't':	/* absolute timeout */
			    case 'i':	/* idle timer */
				enq(*flg);
				break;
			    case '-':	/* read key from stdin */
				keycheck=0;
				break;
			    default:
				fprintf(stderr,"usage: cattach [-l] [-t timeout] -i [idle] dir name\n");
				exit(1);
			}
	} else {
		switch (deq()) {
		    case -1:
			if (dirarg==NULL)
				dirarg = *argv;
			else if (namearg==NULL)
				namearg = *argv;
			else {
				fprintf(stderr,"usage: cattach [-l] [-t timeout] -i [idle] dir name\n");
				exit(1);
			}
			break;
		    case 't':
			timeout = atoi(*argv);
			break;
		    case 'i':
			idle = atoi(*argv);
			break;
		    default:	/* should never happen */
			fprintf(stderr,"cattach: internal error\n");
			exit(1);
		}
	}
#ifndef	NO_DEFAULT_NAME
	/* From George Sipe */
	/* provide sensible default for name if not specified */
	if ((dirarg!=NULL) && (namearg==NULL)) {
		while ((namearg = rindex(dirarg, '/'))!=NULL) {
			if (dirarg==namearg||*++namearg)
				break;		/* not a trailing slash */
			*--namearg = '\000';	/* remove it, try again */
		}
		if (namearg==NULL)
			namearg = dirarg;
	}
#endif
	if ((dirarg==NULL) || (namearg==NULL)) {
		fprintf(stderr,"usage: cattach [-l] [-t timeout] [-i idle] dir name\n");
		exit(1);
	}
	if (*dirarg!='/') {
		if (getcwd(buf,1024) == NULL) {
			fprintf(stderr,"Can't stat current directory\n");
			exit(1);
		}
		sprintf(dir,"%s/%s",buf,dirarg);
	} else
		strcpy(dir,dirarg);
	sprintf(lname,"%s/..data",dir,1024);
	sprintf(kname,"%s/..k",dir,1024);
	if (chdir(lname) >= 0)
		strcpy(dir,lname);
	else if (chdir(dir)<0) {
		perror(dirarg);
		exit(1);
	}
#ifdef irix
/* or (I hope) more or less any system with the 4 parameter statfs */
    if ((statfs(".",&sfb,sizeof sfb,0)<0) || (sfb.f_blocks==0)) {
        fprintf(stderr,"Sorry, can't attach to a crypted directory\n");
        exit(1);
    }
#else
	if ((statfs(".",&sfb)<0) || (sfb.f_blocks==0)) {
		fprintf(stderr,"Sorry, can't attach to a crypted directory\n");
		exit(1);
	}
#endif
	ap.dirname=dir;
	strcpy(ins,namearg);
	*namearg='\0'; /* weak attempt to hide .instance in ps output */
	ap.name=ins;
	if (keycheck) {
		if ((pw=getpassword("Key:"))==NULL) {
			fprintf(stderr,"Can't get key\n");
			exit(1);
		}
	} else {
		if (fgets(pword,256,stdin) == NULL) {
			perror("cmkdir");
			exit(1);
		}
		pw=pword;
		pw[255]='\0';
		n=strlen(pw);
		if ((n>0) && (pw[n-1] == '\n'))
			pw[n-1] = '\0';
	}
 	sprintf(cname,"%s/..c",dir);
	sprintf(sname,"%s/..s",dir);
	if ((fp=fopen(cname,"r")) == NULL) {
		ciph=CFS_STD_DES;
	} else {
		fscanf(fp,"%d",&ciph);
		fclose(fp);
	}
	if ((fp=fopen(kname,"r")) == NULL) {
		cfmt=0;
	} else {
		cfmt=1;
		if (fread(ekey,32,1,fp) < 0)
			cfmt=0;
		fclose(fp);
	}
	if ((fp=fopen(sname,"r")) == NULL) {
		smsize=LARGESMSIZE;
	} else {
		if (fscanf(fp,"%d",&smsize) != 1)
			smsize=LARGESMSIZE;
		fclose(fp);
		if ((smsize < CFSBLOCK) || (smsize > (LARGESMSIZE*2)))
			smsize=LARGESMSIZE;
	}
	ap.smsize = smsize;
	ap.idle = idle;
	ap.expire = timeout;
	ap.key.cipher=ciph;
	if (smsize != LARGESMSIZE)
		sprintf(pw,"%s%d",pw,smsize);

	if (cfmt) {
		if (new_pwcrunch(pw,&ap.key)!=0) {
			fprintf(stderr,"Invalid key\n");
			exit(1);
		}
		decrypt_key(&ap.key,ekey);
	}
	else {
		if (old_pwcrunch(pw,&ap.key)!=0) {
			fprintf(stderr,"Invalid key\n");
			exit(1);
		}
	}
	ap.anon = ap.name[0]=='.';
	ap.uid=getuid();
	if ((cln=clnt_create("localhost",ADM_PROGRAM,ADM_VERSION,"udp"))
	    == NULL) {
		clnt_pcreateerror("adm");
		exit(1);
	}
	cln->cl_auth = authunix_create_default();
	if ((status = clnt_call(cln,ADMPROC_ATTACH,xdr_cfs_attachargs,&ap,
				xdr_cfsstat,&ret,tout)) != RPC_SUCCESS) {
		clnt_perrno(status);
		exit(1);
	}
	if (ret!=CFS_OK)
		fprintf(stderr,"cattach: %s\n",admmsg(ret));
	exit(ret);
}

#define QS 5
struct {
        int data[QS];
        int head;
        int tail;
} argq = {{0},0,0};

enq(f)
     char f;
{
	argq.tail++;
	argq.tail %= QS;
	if (argq.head==argq.tail) {
		fprintf(stderr,"Can't deal with this\n");
		exit(-2);
	}
	argq.data[argq.tail]=f;
}

deq()
{
	if (argq.head==argq.tail)
		return -1;
	argq.head++;
	argq.head %= QS;
	return(argq.data[argq.head]);
}
