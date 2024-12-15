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
 * client cfs mkdir - 1.4.0
 */
#include <stdio.h>
#include <rpc/rpc.h>
#include <sys/time.h>
#include "nfsproto.h"
#include "admproto.h"
#include "cfs.h"
#include "shs.h"

main(argc,argv)
     int argc;
     char **argv;
{
	char *pw;
	char pword[256];
	char *getpassword();
	int n;
	cfs_admkey k;
	cfskey kt;
	char path[1024];
	char str[8];
	FILE *fp;
	char *flg;
	struct timeval tv;
	u_long r;
	int i;
	int ciph=CFS_THREE_DES;
	int cfmt=1;
	int smsize=LARGESMSIZE;
	unsigned char ekey[128];
	unsigned char ek1[128];
	unsigned int l;
	int keycheck=1;
	
	while (--argc && (**++argv == '-')) {
		for (flg= ++*argv; *flg; ++flg)
			switch (*flg) {
			    case '1':
			    case 'd':
				ciph=CFS_STD_DES;
				break;
			    case '2':
				ciph=CFS_THREE_DES;
				break;
			    case '3':
				ciph=CFS_TRUE_THREE_DES;
				break;
			    case 'b':
				ciph=CFS_BLOWFISH;
				fprintf(stderr,
				      "Warning: Blowfish is a new cipher.\n");
				break;
			    case 'm':
				ciph=CFS_MACGUFFIN;
				fprintf(stderr,
				      "Warning: MacGuffin is a weak cipher\n");
				break;
			    case 's':
				ciph=CFS_SAFER_SK128;
				fprintf(stderr,
				      "Warning: SAFER is a new cipher.\n");
				break;
			    case 'o':
				cfmt=0;
				break;
			    case 'p':	/* puny memory option */
				smsize=SMALLSMSIZE;
				break;
			    case '-':
				keycheck=0;
				break;
			    default:
				fprintf(stderr,"usage: cmkdir [-123bdmosp-] dir\n");
				exit(1);
			}
	}
	if (argc!=1) {
		fprintf(stderr,"Usage: cmkdir [-123bdmosp-] dir\n");
		exit(1);
	}
	if (keycheck) {
		if ((pw=getpassword("Key:"))==NULL) {
			fprintf(stderr,"Can't get key\n");
			exit(1);
		}
		strcpy(pword,pw);
		if (strlen(pw)<16) {
			fprintf(stderr,"Key must be at least 16 chars.\n");
			exit(1);
		}
		if ((pw=getpassword("Again:"))==NULL) {
			fprintf(stderr,"Can't get key\n");
			exit(1);
		}
		if (strcmp(pword,pw)!=0) {
		     fprintf(stderr,
			"Keys don't match; drink some coffee and try again\n");
		     exit(1);
		}
	}
	else { /* just accept key from stdio */
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
	if (smsize != LARGESMSIZE)
		sprintf(pw,"%s%d",pword,smsize);
	k.cipher=ciph;
	if (cfmt==0) { 
		if (old_pwcrunch(pw,&k)!=0) {
			fprintf(stderr,"Invalid key\n");
			exit(1);
		}
	} else {
		/* this is very ugly and will be replaced but it works */
		if (new_pwcrunch(pw,&k)!=0) {
			fprintf(stderr,"Invalid key\n");
			exit(1);
		}
		/* now we xor in some truerand bytes for good measure */
		bcopy(&k,ekey,32);  /* assumes key material < 32 bytes */
		for (i=0; i<32; i++) {
			ekey[i] ^= randbyte(); 
		}
		encrypt_key(&k,ekey);
		bcopy(ekey,ek1,32);
		decrypt_key(&k,ek1);
		/* new &k is our real key */
	}
	if (mkdir(argv[0],0777)<0) {
		perror("cmkdir");
		exit(1);
	}
	sprintf(path,"%s/...",argv[0]);
	strcpy(str,"qua!");
	/* now randomize the end of str.. */
	r = trand32();
	for (i=0; i<4; i++)
		str[i+4]=(r<<(i*8))&0377;
	copykey(&k,&kt);
	cipher(&kt,str,0);
	mask_cipher(&kt,str,1);
	cipher(&kt,str,0);
	if ((fp=fopen(path,"w")) == NULL) {
		perror("cmkdir");
		exit(1);
	}
	fwrite(str,8,1,fp);
	fclose(fp);
	sprintf(path,"%s/..c",argv[0]);
	if ((fp=fopen(path,"w")) == NULL) {
		perror("cmkdir");
		exit(1);
	}
	fprintf(fp,"%d",k.cipher);
	fclose(fp);
	sprintf(path,"%s/..s",argv[0]);
	if ((fp=fopen(path,"w")) == NULL) {
		perror("cmkdir");
		exit(1);
	}
	fprintf(fp,"%d\n",smsize);
	fclose(fp);
	if (cfmt) {
		sprintf(path,"%s/..k",argv[0]);
		if ((fp=fopen(path,"w")) == NULL) {
			perror("cmkdir");
			exit(1);
		}
		fwrite(ekey,32,1,fp);
		fclose(fp);
	}
	exit(0);
}
