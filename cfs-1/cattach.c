/*
 * The author of this software is Matt Blaze.
 *              Copyright (c) 1992, 1993, 1994 by AT&T.
 * Permission to use, copy, and modify this software without fee
 * is hereby granted, provided that this entire notice is included in
 * all copies of any software which is or includes a copy or
 * modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * This software is subject to United States export controls.  You may
 * not export it, in whole or in part, or cause or allow such export,
 * through act or omission, without prior authorization from the United
 * States government and written permission from AT&T.  In particular,
 * you may not make any part of this software available for general or
 * unrestricted distribution to others, nor may you disclose this software
 * to persons other than citizens and permanent residents of the United
 * States and Canada. 
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */

/*
 * client side attach - 1.1
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
#include <sys/mount.h>
#endif	/* BSD44 */
#endif
#include "nfsproto.h"
#include "admproto.h"
#include "cfs.h"

#ifdef SOLARIS2X
#define statfs		statvfs
#define getwd(b)	getcwd(b, 1024)
#endif

main(argc,argv)
     int argc;
     char **argv;
{
	cfs_attachargs ap;
	char *pw;
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
	char cname[1024];
	static struct timeval tout = {60,0};
	CLIENT *cln;

	ap.highsec=1;
	while (--argc && (**++argv == '-')) {
		for (flg= ++*argv; *flg; ++flg)
			switch (*flg) {
			    case 'l':
				ap.highsec=0;
				break;
			    default:
				fprintf(stderr,"usage: cattach [-l] dir name\n");
				exit(1);
			}
	}
	if (argc!=2) {
		fprintf(stderr,"Usage: cattach [-l] dir name\n");
		exit(1);
	}
	if (*argv[0]!='/') {
		if (getwd(buf) == NULL){
			fprintf(stderr,"Can't stat current directory\n");
			exit(1);
		}
		sprintf(dir,"%s/%s",buf,argv[0]);
	} else
		strcpy(dir,argv[0]);
	if (chdir(dir)<0) {
		perror(argv[0]);
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
	strcpy(ins,argv[1]);
	*argv[1]='\0'; /* weak attempt to hide .instance in ps output */
	ap.name=ins;
	if ((pw=getpassword("Key:"))==NULL) {
		fprintf(stderr,"Can't get key\n");
		exit(1);
	}
	sprintf(cname,"%s/..c",dir);
	if ((fp=fopen(cname,"r")) == NULL) {
		ciph=CFS_STD_DES;
	} else {
		fscanf(fp,"%d",&ciph);
		fclose(fp);
	}
	ap.key.cipher=ciph;
	if (pwcrunch(pw,&ap.key)!=0) {
		fprintf(stderr,"Invalid key\n");
		exit(1);
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
