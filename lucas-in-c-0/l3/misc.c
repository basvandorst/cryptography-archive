/* misc.c - Mark Henderson (markh@wimsey.bc.ca) */
#include <gmp.h>
#include "lucas.h"
#include "d3des.h"
#include "idea.h"
#include <stdio.h>
#include "misc.h"
#include "md5.h"

void write_number(FILE *f ,MP_INT *x)
{
    unsigned char buf[MAXC];
    int i,j;
    unsigned char length;

    /* encode into buf */
    mptcs(buf,MAXC,x);

    /* find first non-zero in buf */
    for (i=0; i<MAXC && !buf[i];i++)
        ;

    if (i == MAXC) {
        /* number is 0 */
        i--;
    }
    length = MAXC - i;

    fputc(length, f);
    for (j=i;j<MAXC;j++)
        fputc(buf[j],f);
}

int read_number(FILE *f ,MP_INT *x)
{
    unsigned char buf[MAXC];
    int length,i,j;
    
    length = fgetc(f);
    if (length < 0 || length > MAXC)
        return 0;
    for (i=0;i<length;i++) {
        j = fgetc(f);
        if (j < 0)
            return 0;
        buf[i] = (unsigned char)j;
    }
    cstmp(x,buf,length);
    return 1;
}

int desfile(FILE *in, FILE *out, unsigned char *key, unsigned char *iv, 
int decrypt)
/* key is 40 bytes, iv is 8 bytes */
{
    unsigned char buf[8],nbuf[8];
    unsigned char mbuf[8], sbuf[8];
    unsigned char *t,*s;
    int count;
    int i,b;

    des3key(&key[0],decrypt); /* bytes 0-23 */
    initkey_idea(&key[24],decrypt); /* bytes 24-39 */
    memcpy(mbuf,iv,8);
    
    if (decrypt) {
        if (fread(buf,1,8,in) < 8) 
            return 0;
        while (fread(nbuf,1,8,in) == 8) {
            memcpy(sbuf,buf,8);
            Ddes(buf,buf);
			idea_ecb((word16 *)buf,(word16 *)buf);
            for (t=buf,s=mbuf, count = 8; count; count--) 
                *(t++) ^= *(s++);
            memcpy(mbuf,sbuf,8);
            fwrite(buf,1,8,out);
            memcpy(buf,nbuf,8);
        }
        /* buf contains last 8 bytes of ciphertext */
        memcpy(sbuf,buf,8);
        Ddes(buf,buf);
		idea_ecb((word16 *)buf,(word16 *)buf);
        for (t=buf,s=mbuf, count = 8; count; count--) 
            *(t++) ^= *(s++);
        memcpy(mbuf,sbuf,8);
        if (buf[7] > 7) {
            fwrite(buf,8,1,out);
            return 0;
        }
        if (buf[7]) 
            fwrite(buf,buf[7],1,out);
    }
    else {  /* encrypt */
        while ((b=fread(buf,1,8,in)) == 8) {
            for (t=buf,s=mbuf, count = 8; count; count--) 
                *(t++) ^= *(s++);
			idea_ecb((word16 *)buf,(word16 *)buf);
            Ddes(buf,buf);
            memcpy(mbuf,buf,8);
            fwrite(buf,1,8,out);
        }
        /* so b < 8 */
        /* zero elts from b..6 */
        for (i=b;i<7;i++) 
            buf[i] = 0;
        buf[7] = b;
        for (t=buf,s=mbuf, count = 8; count; count--) 
            *(t++) ^= *(s++);
		idea_ecb((word16 *)buf,(word16 *)buf);
        Ddes(buf,buf);
        memcpy(mbuf,buf,8);
        fwrite(buf,1,8,out);
    }
	close_idea();
	close_des();
    return 1;
}


void mdfile(FILE *inFile, unsigned char *digest)
{
  MD5_CTX mdContext;
  int bytes;
  unsigned char data[1024];
  MD5Init (&mdContext);
  while ((bytes = fread (data, 1, 1024, inFile)) != 0)
    MD5Update (&mdContext, data, bytes);
  MD5Final (&mdContext);
  memcpy(digest, &mdContext.digest[0],16);
}

void mdstring(char *s, unsigned int len, unsigned char *digest)
{
  MD5_CTX mdContext;
  int bytes;
  MD5Init (&mdContext);
  MD5Update (&mdContext, s, len);
  MD5Final (&mdContext);
  memcpy(digest, &mdContext.digest[0],16);
}

/* 16 byte key - for private keys */
static void
desbuf(int mode, unsigned char *buf, unsigned int len,
    unsigned char *key, unsigned char *iv)
{
    int count;
    unsigned char miv[8], siv[8];
    unsigned char *s, *t;
    unsigned int block_cnt = (len+8-1)/8;

    des2key(key,mode);
    memcpy(miv,iv,8);

    if(!mode) {  /* encrypt == EN0 */
        while(block_cnt--) {
            for(t=buf,s=miv,count=8; count; count--) 
                *(t++) ^= *(s++);
            Ddes(buf,buf);
            memcpy(miv,buf,8);
            buf += 8;
        }
    } else {
        while(block_cnt--) {
            memcpy(siv,buf,8);
            Ddes(buf,buf);
            for(t=buf,s=miv,count=8; count; count--) 
                *(t++) ^= *(s++);
            memcpy(miv,siv,8);
            buf += 8;
        }
    }
	close_des();
}

void write_numberx(FILE *f ,MP_INT *x, unsigned char *key, unsigned char *iv)
{
    unsigned char buf[MAXC];
    int i,j,k;
    unsigned char length;

    /* encode into buf */
    mptcs(buf,MAXC,x);

    /* find first non-zero in buf */
    for (i=0; i<MAXC && !buf[i];i++)
        ;

    if (i == MAXC) {
        /* number is 0 */
        i--;
    }
    length = MAXC - i;
    k = length % 8;

    i -= (8 - k);
    length += (8 - k);

    fputc(length, f);
    desbuf(EN0,&buf[i],length,key,iv);
    for (j=i;j<MAXC;j++)
        fputc(buf[j],f);
}

int read_numberx(FILE *f ,MP_INT *x, unsigned char *key, unsigned char *iv)
{
    unsigned char buf[MAXC];
    int length,i,j;
    
    length = fgetc(f);
    if (length < 0 || length > MAXC || length % 8)
        return 0;
    for (i=0;i<length;i++) {
        j = fgetc(f);
        if (j < 0)
            return 0;
        buf[i] = (unsigned char)j;
    }
    desbuf(DE1,buf,length,key,iv);
    if (buf[0] != 0)
        return 0;
    cstmp(x,buf,length);
    return 1;
}

FILE *mfopen(unsigned char *file, unsigned char *mode) {
    unsigned char *pc,*p;
    unsigned char *s;
    unsigned char *path[256];
    unsigned char fbuf[1024];
    unsigned int npath;
    unsigned int i;
    FILE *ret;

	p = (unsigned char *)malloc(1024);
    npath = 0; ret = NULL;
    if (pc=(unsigned char *)getenv("LPATH")) {
		strncpy(p,pc,1024); p[1023] = '\0';
        for (s=p; *p  && (npath < 256); p++) {
            if (*p == ':') {
                *p = '\0';
                if (s!=p)
                    path[npath] = s;
                else
                    path[npath] = ".";
                s=p+1;
                npath++;
            }
        }
        if (npath < 256) {
            /* need to deal with end */
            *p = '\0';
            if (s!=p)
                path[npath] = s;
            else
                path[npath] = ".";
            npath++;
        }
    }
    else 
    {
        npath = 1;
        path[0] = ".";
    }
    for (i=0;i<npath;i++) {
        strcpy(fbuf,path[i]);
        strcat(fbuf, "/");
        strcat(fbuf,file);
#ifdef DEBUG
        fprintf(stderr, "trying to open %s\n", fbuf);
#endif
        if (ret = fopen(fbuf, mode)) {
#ifdef DEBUG
            fprintf(stderr, "successful open of %s\n", fbuf);
#endif
            break;
        }
    }
	free(p);
    return ret;
}

void zap(void *ptr, unsigned int size)
{
	memset(ptr,0,size);
}
