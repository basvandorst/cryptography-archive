/* fifo.c */

#include <stdio.h>
#include <unistd.h>
#include "usuals.h"
#include "pgpmem.h"
#include "fifo.h"

#define LAC_NONE 0
#define LAC_READ 1
#define LAC_WRITE 2

/* One buffer for all the functions. This way we don't waste stack
   space, or have to allocate and free memory all the time. */
static byte buf[FIFO_BLOCK_SIZE];

/* Create a new fifo block */
static struct fifo_block *fifo_block_create(void);

/* Unlink a fifo block, freeing it if links goes to 0 */
static void fifo_block_unlink(struct fifo_block *f);

/* Create a new fifo block */
static struct fifo_block *fifo_block_create(void)
{
struct fifo_block *fb;
fb=safemalloc(sizeof(struct fifo_block));
fb->size=0;
fb->data=safemalloc(sizeof(struct fifo_data));
fb->data->links=1;
fb->next=NULL;
return(fb);
}

/* Unlink a fifo block, freeing it if links goes to 0 */
static void fifo_block_unlink(struct fifo_block *f)
{
if(!(--f->data->links))
  free(f->data);
free(f);
}

/* Create a new fifo in memory */
struct fifo *fifo_mem_create(void)
{
struct fifo *f;
struct fifo_block *fb;
f=safemalloc(sizeof(struct fifo));
fb=fifo_block_create();
f->first=fb;
f->pos=0;
f->last=fb;
f->fp=NULL;
return(f);
}

/* Create a disk file fifo */
struct fifo *fifo_file_create(FILE *fp)
{
struct fifo *f;
f=safemalloc(sizeof(struct fifo));
f->fp=fp;
f->floc=0;
fseek(fp,0,SEEK_END);
f->flen=ftell(fp);
f->flac=LAC_NONE;
return(f);
}

/* Put a character into a fifo */
void fifo_put(byte c,struct fifo *f)
{
fifo_aput(&c,1,f);
}

/* Get a character from a fifo, returns -1 if empty */
int fifo_get(struct fifo *f)
{
byte c;
if(fifo_aget(&c,1,f)) return(c);
  else return(-1);
}

/* Put an array into a fifo */
void fifo_aput(void *cv,word32 num,struct fifo *f)
{
byte *c=(byte *) cv;
if(f->fp) {
  if(f->flac!=LAC_WRITE) {
    fseek(f->fp,0,SEEK_END);
    f->flac=LAC_WRITE;
    }
  fwrite(c,1,num,f->fp);
  f->flen+=num;
  }
else {
  struct fifo_block *fb,*nfb;
  unsigned blk;
  fb=f->last;
  while(num) {
    if(fb->size==FIFO_BLOCK_SIZE) {
      nfb=fifo_block_create();
      fb->next=nfb;
      fb=nfb;
      }
    blk=FIFO_BLOCK_SIZE-fb->size;
    if(blk>num) blk=num;
    memcpy(&fb->data->array[fb->size],c,blk);
    c+=blk;
    fb->size+=blk;
    num-=blk;
    }   
  f->last=fb; 
  }
}

/* Get num characters from a fifo, returns num actually copied */
word32 fifo_aget(void *cv,word32 num,struct fifo *f)
{
byte *c=(byte *) cv;
if(f->fp) {
  word32 l;
  if(f->flac!=LAC_READ) {
    fseek(f->fp,f->floc,SEEK_SET);
    f->flac=LAC_READ;
    }
  l=fread(c,1,num,f->fp);
  f->floc+=l;
  return(l);
  }
else {
  struct fifo_block *fb,*nfb;
  word32 onum=num;
  unsigned blk;
  fb=f->first;
  while(num) {
    blk=fb->size-f->pos;
    if(blk>num) blk=num;
    memcpy(c,&fb->data->array[f->pos],blk);
    c+=blk;
    f->pos+=blk;
    num-=blk;
    if(f->pos==fb->size) {
      f->pos=0;
      if(fb==f->last) {
	fb->size=0;
	break;
	}
      f->first=fb->next;
      fifo_block_unlink(fb);
      fb=f->first;
      }
    }
  f->first=fb;
  return(onum-num);
  }
}

/* Random-access get a byte from a fifo */
byte fifo_rget(word32 loc,struct fifo *f)
{
byte c;
if(f->fp) {
  f->flac=LAC_NONE;
  fseek(f->fp,f->floc+loc,SEEK_SET);
  if(fread(&c,1,1,f->fp))
    return(c);
  else
    return(0);
  }
else {
  struct fifo_block *fb=f->first;
  loc+=f->pos;
  while(loc>=fb->size) {
    loc-=fb->size;
    fb=fb->next;
    if(!fb)
      return(0);
    }
  c=fb->data->array[loc];
  return(c);
  }
}

/* Random-access put a byte to a fifo - CAUTION do not use with fifo_copy */
void fifo_rput(byte c,word32 loc,struct fifo *f)
{
if(f->fp) {
  f->flac=LAC_NONE;
  fseek(f->fp,f->floc+loc,SEEK_SET);
  fwrite(&c,1,1,f->fp);
  }
else {
  struct fifo_block *fb=f->first;
  loc+=f->pos;
  while(loc>=fb->size) {
    loc-=fb->size;
    fb=fb->next;
    if(!fb)
      return;
    }
  fb->data->array[loc]=c;
  }
}

/* Return length of a fifo */
word32 fifo_length(struct fifo *f)
{
word32 l=0;
if(f->fp)
  return(f->flen-f->floc);
else {
  struct fifo_block *fb=f->first;
  l=fb->size-f->pos;
  while(fb!=f->last) {
    fb=fb->next;
    l+=fb->size;
    }
  return(l);
  }
}

/* Skip length characters in a fifo */
void fifo_skipn(struct fifo *f,word32 length)
{
unsigned j;
while(length) {
  j=FIFO_BLOCK_SIZE;
  if(j>length) j=length;
  fifo_aget(buf,j,f);
  length-=j;
  }
}

/* Move n characters from i to o */
void fifo_moven(struct fifo *i,struct fifo *o,word32 n)
{
unsigned j,k;
while(n) {
  j=FIFO_BLOCK_SIZE;
  if(j>n) j=n;
  k=fifo_aget(buf,j,i);
  if(!k)
    return; /* Prevents hanging if fifo empty */
  n-=k;
  fifo_aput(buf,k,o);
  }
}

/* Clear last-accessed field of a disk fifo. Used when you want to access
   two copies of a disk fifo simultaneously. Will not do any damage if
   done to a memory fifo. */
void fifo_clearlac(struct fifo *f)
{
f->flac=LAC_NONE;
}

/* Create a duplicate of a fifo */
struct fifo *fifo_copy(struct fifo *f)
{
if(f->fp) {
  struct fifo *nf;
  nf=safemalloc(sizeof(struct fifo));
  f->flac=LAC_NONE;
  memcpy(nf,f,sizeof(struct fifo));
  return(nf);
  }
else {
  struct fifo *nf;
  struct fifo_block *fb,*nfb,*pfb;
  nf=safemalloc(sizeof(struct fifo));
  nf->fp=NULL;
  nf->pos=f->pos;
  pfb=NULL;
  fb=f->first;
  nf->first=NULL;
  while(fb->next) {
    nfb=safemalloc(sizeof(struct fifo_block));
    if(nf->first==NULL)
      nf->first=nfb;
    if(pfb)
      pfb->next=nfb;
    memcpy(nfb,fb,sizeof(struct fifo_block));
    nfb->data->links++;
    pfb=nfb;
    fb=fb->next;
    }    
  nfb=safemalloc(sizeof(struct fifo_block));
  if(pfb)
    pfb->next=nfb;
  else
    nf->first=nfb;
  nfb->size=fb->size;
  nfb->next=NULL;
  nfb->data=safemalloc(sizeof(struct fifo_data));
  nf->last=nfb;
  memcpy(nfb->data,fb->data,sizeof(struct fifo_data));  
  return(nf);
  }
}    

/* Find pattern in fifo f, returns true if found, false if not
   Fifo is at end if false, just after pattern if true */
int fifo_find(byte *pattern,struct fifo *f)
{
/* This is the Knuth-Morris-Pratt string matcher, if you're curious */
unsigned i,k,m,q;
unsigned *pf;
byte c;
int ci;

m=strlen(pattern);
pf=safemalloc(m*sizeof(unsigned));
pf[0]=0;
k=0;
for(q=1;q<m;q++) {
  while((k>0)&&(pattern[k]!=pattern[q]))
    k=pf[k-1];
  if(pattern[k]==pattern[q])
    k++;
  pf[q]=k;
  }
q=0;
while((ci=fifo_get(f))!=(-1)) {
  c=ci;
  while((q>0)&&(pattern[q]!=c))
    q=pf[q-1];
  if(pattern[q]==c)
    q++;
  if(q==m) {
    free(pf);
    return(TRUE);
    }
  } 
free(pf);
return(FALSE);
}

/* Append a to b, returning new fifo as a, destroying b */ 
void fifo_append(struct fifo *a,struct fifo *b)
{
word32 num;
while((num=fifo_aget(buf,FIFO_BLOCK_SIZE,b)))
  fifo_aput(buf,num,a);
fifo_destroy(b);
}

/* De-allocate a fifo */
void fifo_destroy(struct fifo *f)
{
if(!f->fp) {
  struct fifo_block *fb,*nfb;
  fb=f->first;
  while(fb) {
    nfb=fb->next;
    fifo_block_unlink(fb);
    fb=nfb;
    }
  }
free(f);
}
