/*
 * The author of this software is Matt Blaze.
 *              Copyright (c) 1995 by AT&T.
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
 * client cfs cpasswd - 1.3
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
	char buf[1024];
	char *getpassword();
	cfs_admkey oldkey;
	cfs_admkey newkey;
	cfskey kt;
	char nname[1024];
	char oname[1024];
	char cname[1024];
	char kname[1024];
	char sname[1024];
	char lname[1024];
	char dir[1024];
	int smsize;
	FILE *fp;
	char *flg;
	int ciph=CFS_STD_DES;
	int cfmt=1;
	unsigned char ekey[128];
	
	while (--argc && (**++argv == '-')) {
		for (flg= ++*argv; *flg; ++flg)
			switch (*flg) {
			    default:
				fprintf(stderr,"usage: cpasswd dir\n");
				exit(1);
			}
	}
	if (argc!=1) {
		fprintf(stderr,"Usage: cpasswd dir\n");
		exit(1);
	}
	if (*argv[0]!='/') {
		if (getcwd(buf,1024) == NULL) {
			fprintf(stderr,"Can't stat current directory\n");
			exit(1);
		}
		sprintf(dir,"%s/%s",buf,argv[0]);
	} else
		strcpy(dir,argv[0]);
	sprintf(kname,"%s/..k",dir);
	sprintf(nname,"%s/..n",dir);
	sprintf(oname,"%s/..o",dir);
	sprintf(lname,"%s/..data",dir);
	if (chdir(lname) >= 0)
		strcpy(dir,lname);
	else if (chdir(dir)<0) {
		perror(dir);
		exit(1);
	}

	sprintf(cname,"%s/..c",dir);
	sprintf(sname,"%s/..s",dir);
	if ((fp=fopen(cname,"r")) == NULL) {
		fprintf(stderr,"Can only change passphrase on new format CFS directories\n");
		exit(1);
	} else {
		fscanf(fp,"%d",&ciph);
		fclose(fp);
	}
	if (((fp=fopen(kname,"r")) == NULL) || (fread(ekey,1,32,fp)<16)) {
		perror(dir);
		fprintf(stderr,"Can only change passphrase on new format CFS directories\n");
		exit(1);
	}
	fclose(fp);
	if ((fp=fopen(sname,"r")) == NULL) {
		smsize=LARGESMSIZE;
	} else {
		if (fscanf(fp,"%d",&smsize) != 1)
			smsize=LARGESMSIZE;
		fclose(fp);
		if ((smsize < CFSBLOCK) || (smsize > (LARGESMSIZE*2)))
			smsize=LARGESMSIZE;
	}

	oldkey.cipher=ciph;
	if ((pw=getpassword("Old passphrase:"))==NULL) {
		fprintf(stderr,"Can't get key\n");
		exit(1);
	}
	if (smsize != LARGESMSIZE)
		sprintf(pw,"%s%d",pw,smsize);
	if (new_pwcrunch(pw,&oldkey)!=0) {
		fprintf(stderr,"Invalid key\n");
		exit(1);
	}
	decrypt_key(&oldkey,ekey);
	if (!checkkey(dir,&oldkey)) {
		fprintf(stderr,"Incorrect passphrase\n");
		exit(1);
	}

	newkey.cipher=ciph;
	if ((pw=getpassword("New passphrase:"))==NULL) {
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
	if (smsize != LARGESMSIZE)
		sprintf(pw,"%s%d",pw,smsize);
	if (new_pwcrunch(pw,&newkey)!=0) {
		fprintf(stderr,"Invalid key\n");
		exit(1);
	}
	encrypt_key(&newkey,ekey);
	if ((fp=fopen(nname,"w")) == NULL) {
		perror("cmkdir");
		exit(1);
	}
	if (fwrite(ekey,1,32,fp) != 32) {
		perror("can't create new key file");
		exit(1);
	}
	fclose(fp);
	/* do this in 3 phases, ultra paranoid */
	if (rename(kname,oname) < 0) {
		perror("can't rename old key file");
		exit(1);
	}
	if (rename(nname,kname) < 0) {
		perror("can't link new key file");
		exit(1);
	}
	if (unlink(oname)<0)
		perror("warning: old key file not removed");
	exit(0);
}


checkkey(path,ak)
     char *path;
     cfs_admkey *ak;
{
	FILE *fp;
	char fn[1024];
	char buf[9];
	cfskey k;
	
	copykey(ak,&k);
	sprintf(fn,"%s/...",path);
	if ((fp=fopen(fn,"r"))==NULL)
		return 0;
	if (fread(buf,8,1,fp)!=1) {
		fclose (fp);
		return 0;
	}
	fclose (fp);
	cipher(&k,buf,1);        /* note order here */
	mask_cipher(&k,buf,0);
	cipher(&k,buf,1);        /* note order here */
	if (bcmp(buf,"qua!",4)!=0)
		return 0;
	return 1;
}
