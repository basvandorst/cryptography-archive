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
 * server adm rpc handlers - ver 1.1
 */
#include <sys/types.h>
#include <stdio.h>
#include <rpc/rpc.h>
#include <sys/time.h>
#include <string.h>
#include "admproto.h"
#include "nfsproto.h"
#include "cfs.h"

typedef struct svc_req *SR;

int topinstance = 0;


cfs_adm()
{
}

void *
admproc_null_1()
{
}

cfsstat *
admproc_attach_1(ap,rp)
     cfs_attachargs *ap;
     SR *rp;
{
	static cfsstat ret;
	int i;
	cfskey tk;
	instance *ins;

#ifdef DEBUG
	printf("attach: %s %s %d\n",ap->dirname, ap->name, ap->anon);
#endif
	if (*ap->dirname != '/') {
		ret = CFSERR_BADNAME;
		return &ret;
	}
	if (index(ap->name,'/') != NULL) {
		ret = CFSERR_BADNAME;
		return &ret;
	}
	if (already(ap->name)) {
		ret=CFSERR_EXIST;
		return &ret;
	}
	become(rp);
	copykey(&ap->key,&tk);
	if ((ret=verify(ap->dirname,&tk)) != CFS_OK) {
		become(NULL);
		return &ret;
	}
	become(NULL);
	for (i=topinstance+1; i<NINSTANCES; i++)
		if (instances[i] == NULL)
			break;
	if (i==NINSTANCES) for (i=1; i<topinstance; i++)
		if (instances[i] == NULL)
			break;
	if (i==topinstance) {
		ret=CFSERR_IFULL;
		return &ret;
	}
	if ((ins=(instance*)malloc(sizeof(instance)))==NULL) {
		ret=CFSERR_IFULL;
		return &ret;
	}
	topinstance=i;
	instances[i]=ins;
	ins->id=i;
	for (i=0; i<HSIZE; i++)
		ins->file[i]=NULL;
	ins->anon=ap->anon;
	sprintf(ins->path,"%s/.",ap->dirname);
	strcpy(ins->name,ap->name);
	copykey(&ap->key,&ins->key);
	genmasks(&ins->key);
	ins->uid=ap->uid;
	ins->highsec=ap->highsec;
	ret=CFS_OK;
	gettimeofday((struct timeval *)&roottime,NULL);
	bzero(ins->check,8);
	bcopy(&roottime,ins->check,sizeof(roottime));
	cipher(&ins->key,ins->check,0);
	return &ret;
}

already(s)
     char *s;
{
	int i;

	for (i=1; i<NINSTANCES; i++)
		if ((instances[i]!=NULL) && !strcmp(instances[i]->name,s))
			return 1;
	return 0;
}

genmasks(k)
     cfskey *k;
{
	unsigned int i;
	char start[9];
	FILE *fp;

	for (i=0; i<SMSIZE; i+=CFSBLOCK) {
		sprintf(start,"0%07x",i/CFSBLOCK);
		bcopy(start,&k->primask[i],CFSBLOCK);
		mask_cipher(k,&k->primask[i],0);
		sprintf(start,"1%07x",i/CFSBLOCK);
		bcopy(start,&k->secmask[i],CFSBLOCK);
		mask_cipher(k,&k->secmask[i],0);
	}
}

cfsstat *
admproc_detach_1(ap,rp)
     cfs_detachargs *ap;
     SR *rp;
{
	static cfsstat ret;
	int i,j;

	if (strncmp(ap->name,".ANON_",6) == 0) {
		i = atoi(&ap->name[6]);
		if ((i>0) && (i<NINSTANCES) && (instances[i]!=NULL))
			goto found;
	}
	for (i=1; i<NINSTANCES; i++)
		if ((instances[i]!=NULL)&&!strcmp(instances[i]->name,ap->name))
			break;
	if (i==NINSTANCES) {
		ret=CFSERR_NOINS;
		return &ret;
	}
found:
	for (j=0; j<HSIZE; j++) {
		freelist(instances[i]->file[j]);
		instances[i]->file[j]=NULL;
	}
	bzero(instances[i],sizeof(instance));
	free(instances[i]);
	instances[i]=NULL;
	ret=CFS_OK;
	gettimeofday((struct timeval *)&roottime,NULL);
	closeall();
	return &ret;
}

freelist(f)
    cfs_fileid *f;
{
	if (f==NULL)
		return;
	freelist(f->next);
	free(f->name);
	free(f);
}

cfs_lsrepl
*admproc_ls_1()
{
	cfs_lsrepl ret;

	ret.status = CFSERR_EXIST;
	return &ret;
}

verify(path,k)
     char *path;
     cfs_admkey *k;
{
	FILE *fp;
	char fn[1024];
	char buf[9];

	sprintf(fn,"%s/...",path);
	if ((fp=fopen(fn,"r"))==NULL)
		return CFSERR_NODIR;
	if (fread(buf,8,1,fp)!=1) {
		fclose (fp);
		return CFSERR_NODIR;
	}
	fclose (fp);
	cipher(k,buf,1);	/* note order here */
	mask_cipher(k,buf,0);
	cipher(k,buf,1);	/* note order here */
	if (bcmp(buf,"qua!",4)!=0)
		return CFSERR_BADKEY;
	return CFS_OK;
}
