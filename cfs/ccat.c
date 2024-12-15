/*
 * The author of this software is Matt Blaze.
 *              Copyright (c) 1992, 1994 by AT&T.
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
 * cfs ccat - 1.3
 */
#include <stdio.h>
#include <rpc/rpc.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/stat.h>
#ifdef SOLARIS2X
#include <string.h>
#define rindex strrchr
#else
#include <strings.h>
#endif
#include "nfsproto.h"
#include "admproto.h"
#include "cfs.h"

/* following are never used - just so i can re-use the library */
int validhost;
char zerovect[]={0,0,0,0,0,0,0,0,0};
int cursecs=0;

main(argc,argv)
     int argc;
     char **argv;
{
	char *pw;
	char pword[256];
	char *getpassword();
	cfs_admkey k;
	cfskey kt;
	char *flg;
	char *p;
	char ivfile[1024];
	char base[1024];
	char iv[16];
	int fd;
	int len;
	int siz;
	int offset;
	int i;
	char *buf[8192];
	int ciph=CFS_THREE_DES;

	fprintf(stderr,"WARNING: ccat works only on old format CFS files\n");
	while (--argc && (**++argv == '-')) {
		for (flg= ++*argv; *flg; ++flg)
			switch (*flg) {
			    case '1':
				ciph=CFS_STD_DES;
				break;
			    case '3':
				ciph=CFS_THREE_DES;
				break;
			    case 'b':
				ciph=CFS_BLOWFISH;
				break;
			    case 'm':
				ciph=CFS_MACGUFFIN;
				break;
			    case 's':
				ciph=CFS_SAFER_SK128;
				break;
			    default:
				fprintf(stderr,"usage: ccat [-13bms] file...\n");
				exit(1);
			}
	}
	if (argc<1) {
		fprintf(stderr,"Usage: ccat [-3bms] file...\n");
		exit(1);
	}
	if ((pw=getpassword("Key:"))==NULL) {
		fprintf(stderr,"Can't get key\n");
		exit(1);
	}
	strcpy(pword,pw);
	k.cipher=ciph;
	if (old_pwcrunch(pw,&k)!=0) {
		fprintf(stderr,"Invalid key\n");
		exit(1);
	}
	copykey(&k,&kt);
	kt.smsize=LARGESMSIZE;
	if (((kt.primask=(char*) malloc(kt.smsize)) == NULL)
	    || ((kt.secmask=(char*) malloc(kt.smsize)) == NULL)) {
		fprintf(stderr,"No memory\n");
		exit(2);
	}
	genmasks(&kt);
	for (i=0; i<argc; i++) {
		strcpy(ivfile,argv[i]);
		if ((p=rindex(ivfile,'/'))==NULL)
			sprintf(ivfile,".pvect_%s",argv[i]);
		else {
			*p='\0';
			strcpy(base,++p);
			sprintf(ivfile,"%s/.pvect_%s",ivfile,base);
		}
		if (readlink(ivfile,iv,8) != 8)
			bcopy(zerovect,iv,8);
		fprintf(stderr,"%s %s\n",ivfile,iv);
		if ((fd=open(argv[i],O_RDONLY,0))<0) {
			perror(argv[i]);
			continue;
		}
		len=flen(fd);
		fprintf(stderr,"%s %d\n",argv[i],len);
		for (offset=0; offset<len;){
			siz=len-offset;
			if (siz>8192)
				siz=8192;
			siz=readblock(buf,fd,offset,siz,&kt,iv);
			write(1,buf,siz);
			offset+=siz;
		}
	}
}

flen(fd)
     int fd;
{
	struct stat sb;

	if (fstat(fd,&sb)<0)
		return -1;
	return dtov(sb.st_size);
}
