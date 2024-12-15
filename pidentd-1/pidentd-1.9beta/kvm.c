/*
** kvm.c            A set of functions emulating KVM for machines without them.
**
** This code is in the public domain and may be used freely by anyone
** who wants to.
**
** Last update: 14 Apr 1992
**
** Author: Peter Eriksson <pen@lysator.liu.se>
*/

#ifndef HAVE_KVM

#include <stdio.h>
#include <errno.h>
#include <nlist.h>
#include "kvm.h"
#include "paths.h"


extern void *malloc();


kvm_t *kvm_open(namelist, corefile, swapfile, flag, errstr)
  char *namelist;
  char *corefile;
  char *swapfile;
  int flag;
  char *errstr;
{
  kvm_t *kd;


  if (!namelist)
    namelist = _PATH_UNIX;
  if (!corefile)
    corefile = _PATH_KMEM;
  
  kd = (kvm_t *) malloc(sizeof(kvm_t));
  if (!kd)
  {
    if (errstr)
      perror(errstr);
    return NULL;
  }

  kd->namelist = (char *) malloc(strlen(namelist)+1);
  if (!kd->namelist)
  {
    if (errstr)
      perror(errstr);
    return NULL;
  }
  
  if (!(kd->fd = open(corefile, flag)) == -1)
  {
    if (errstr)
      perror(errstr);
    free(kd->namelist);
    free(kd);
    return NULL;
  }

  strcpy(kd->namelist, namelist);
  return kd;
}


int kvm_close(kd)
  kvm_t *kd;
{
  int code;
  
  code = close(kd->fd);
  free(kd->namelist);
  free(kd);

  return code;
}


/*
** Extract offsets to the symbols in the 'nl' list. Returns 0 if all found,
** or else the number of variables that was not found.
*/
int kvm_nlist(kd, nl)
  kvm_t *kd;
  struct nlist *nl;
{
  int code;
  int i;


  code = nlist(kd->namelist, nl);
  if (code != 0)
    return code;
  
  /*
  ** Verify that we got all the needed variables. Needed because some
  ** implementations of nlist() returns 0 although it didn't find all
  ** variables.
  */
  if (code == 0)
  {
#ifdef __convex__
    for (i = 0; nl[i].n_name && nl[i].n_name[0]; i++)
#else  
    for (i = 0; nl[i].n_un.n_name && nl[i].n_un.n_name[0]; i++)
#endif
      if (nl[i].n_type == 0)
	code++;
  }
  
  return code;
}


/*
** Get a piece of the kernel memory
*/
int kvm_read(kd, addr, buf, len)
  kvm_t *kd;
  long addr;
  char *buf;
  int len;
{
  if (lseek(kd->fd, addr, 0) == -1 && errno != 0)
    return -1;

  return read(kd->fd, buf, len);
}

#endif
