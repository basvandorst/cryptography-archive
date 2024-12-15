/* misc.c - Mark Henderson (markh@wimsey.bc.ca) */
#include "l3.h"
void ferr(char *s)
{
    fprintf(stderr, "%s\n", s);
    exit(123);
}

int 
write_string(FILE *f, unsigned char *string) 
{
    unsigned int length;
    unsigned int i;
    unsigned char clength;

    length = strlen(string);
    if (length > 127)
        return 0;
    clength = (unsigned char)length;

    fputc(length,f);
    for (i=0;i<length;i++)
        fputc(*(string+i),f);
    return 1;
}

int 
read_string(FILE *f, unsigned char *buf) 
{
    int i; int length; int j;
    length = fgetc(f);
    if (length < 0 || length > 127)
        return 0;
    for (i=0;i<length;i++) {
        j = fgetc(f);
        if (j >= 0)
            *(buf+i) = j;
        else
            return 0;
    }
    *(buf+i) = '\0';
    return 1;
}
    
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

#ifndef CBC

/* 
 * use double width triple DES (Outerbridge's D2DES) with IDEA in PCBC mode 
 * operates on 16 byte blocks 
 */

#define cleanup \
zap(mbuf,16); zap(nbuf,16), zap(buf,16); zap(sbuf,16); zap(obuf,16)

int desfile(FILE *in, FILE *out, unsigned char *key, unsigned char *iv, 
int decrypt)
/* key is 40 bytes, iv is 16 bytes */
{
    unsigned char buf[16],nbuf[16];
    unsigned char mbuf[16], sbuf[16], obuf[16];
    unsigned char *t,*s,*n,*o;
    int count;
    int i,b;

    des3key(&key[0],decrypt); /* bytes 0-23 */
    initkey_idea(&key[24],decrypt); /* bytes 24-39 */
    memcpy(mbuf,iv,16);
    for(i=0;i<16;i++)
	obuf[i] = 0;    
    if (decrypt) {
        if (fread(buf,1,16,in) < 16) { 
            cleanup; 
			return 0;
        }
        while (fread(nbuf,1,16,in) == 16) {
            memcpy(sbuf,buf,16);
            D2des(buf,buf);
			idea_ecb((word16 *)&buf[8],(word16 *)&buf[8]);
            for (i=0;i<8;i++)
	        buf[i+8] ^= buf[i];
            idea_ecb((word16 *)buf,(word16 *)buf);
            for (o=obuf,t=buf,s=mbuf, count = 16; count; count--) 
                *(t++) ^= *(s++) ^ *(o++);
            memcpy(mbuf,sbuf,16);
            fwrite(buf,1,16,out);
			memcpy(obuf,buf,16); memcpy(buf,nbuf,16); 
        }
        /* buf contains last 16 bytes of ciphertext */
        memcpy(sbuf,buf,16);
        D2des(buf,buf);
		idea_ecb((word16 *)&buf[8],(word16 *)&buf[8]);
        for (i=0;i<8;i++)
	    buf[i+8] ^= buf[i];
        idea_ecb((word16 *)buf,(word16 *)buf);
        for (o=obuf,t=buf,s=mbuf, count = 16; count; count--) 
            *(t++) ^= *(s++) ^ *(o++);
        memcpy(mbuf,sbuf,16);
        if (buf[15] > 15) {
            fwrite(buf,16,1,out); 
			cleanup;
            return 0;
        }
		if (buf[15]) { 
            for (i=buf[15]; i < 15; i++)
				if (buf[i]) {       
					fwrite(buf,16,1,out); 
					cleanup;
					return 0;
				}
            fwrite(buf,buf[15],1,out);
		}
	}
	else {  /* encrypt */
        while ((b=fread(buf,1,16,in)) == 16) {
            memcpy(nbuf,buf,16);
			for (o=obuf,t=buf,s=mbuf, count = 16; count; count--) 
                *(t++) ^= *(s++) ^ *(o++);
            idea_ecb((word16 *)buf,(word16 *)buf);
            for (i=0;i<8;i++)
				buf[i+8] ^= buf[i];
            idea_ecb((word16 *)&buf[8],(word16 *)&buf[8]);
            D2des(buf,buf);
            memcpy(mbuf,buf,16);
            fwrite(buf,1,16,out);
			memcpy(obuf,nbuf,16);
        }
        /* so b < 16 */
        /* zero elts from b..6 */
        for (i=b;i<15;i++) 
            buf[i] = 0;
        buf[15] = b;
        for (o=obuf,t=buf,s=mbuf, count = 16; count; count--) 
                *(t++) ^= *(s++) ^ *(o++);
		idea_ecb((word16 *)buf,(word16 *)buf);
        for (i=0;i<8;i++)
	    buf[i+8] ^= buf[i];
		idea_ecb((word16 *)&buf[8],(word16 *)&buf[8]);
		D2des(buf,buf);
        memcpy(mbuf,buf,16);
        fwrite(buf,1,16,out);
    }
    close_idea();
    close_des();
    cleanup;
    return 1;
}
#else

/* 
 * more "traditional" triple DES with idea in CBC mode. Also
 * since this is CBC, byte errors are "self correcting"
 * operates on 8 byte blocks
 */

#define cleanup \
zap(mbuf,8); zap(nbuf,8), zap(buf,8); zap(sbuf,8)
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
        if (fread(buf,1,8,in) < 8)  {
            cleanup; return 0;
		}
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
			cleanup;
            return 0;
        }
		if (buf[7]) { 
            for (i=buf[7]; i < 7; i++)
				if (buf[i]) {       
					fwrite(buf,8,1,out); 
					cleanup;
					return 0;
				}
            fwrite(buf,buf[7],1,out);
		}
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
	cleanup;
    return 1;
}
#endif

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

void 
mdpkid(MP_INT *n, MP_INT *e, unsigned char *id, unsigned char *digest)
{
    unsigned char buf[MAXC];
    MD5_CTX mdContext;
    MD5Init (&mdContext);

    mptcs(buf,MAXC,n);
    MD5Update (&mdContext, buf, MAXC);
    mptcs(buf,MAXC,e);
    MD5Update (&mdContext, buf, MAXC);
    MD5Update (&mdContext, id, strlen(id));
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
    unsigned char *pc,*p,*po;
    unsigned char *s;
    unsigned char *path[256];
    unsigned char fbuf[1024];
    unsigned int npath;
    unsigned int i;
    FILE *ret;

    /* if filename contains a /, just try and open it, otherwise
     * work along LPATH trying to open file
     */

    if (strchr(file,'/')) {
        ret = fopen(file, mode);
    }
    else {
        po = p = (unsigned char *)malloc(1024);
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
            if (ret = fopen(fbuf, mode)) {
                break;
            }
        }
        free(po);
    }
    return ret;
}

void zap(void *ptr, unsigned int size)
{
    memset(ptr,0,size);
}

int getalg(void)
{
    char *s;
    int i;
    s=getenv("LALG");
    if (!s) 
        i=PALG;
    else {
        i=atoi(s);
        if (i==0)
            i=PALG;
        if (i>3)
            i=PALG;
    }
#ifdef DEBUG
    fprintf(stderr, "getalg returns %d\n", i);
#endif
    return i;
}

int readalg(FILE *f, int*r)
{
    int j;
    j = fgetc(f);
#ifdef DEBUG
    fprintf(stderr, "readalg -- read %d\n", j);
#endif
    if (j < 0)
        return 0;
    if (j == 0)
        return 0;
    if (j > 3)
        return 0;
    *r=j;
    return 1;
}

/* read a public key and verify signature if there is one 
returns -2 if key corrupt
        -1 if self signed with an invalid signature
         0 if public key is not self signed.
         1 if public key is self signed with a valid signature
         also outputs messages to stderr as a side effect
*/
        

int readpk(FILE *f, MP_INT *n, MP_INT *e)
{
    int salg;
    unsigned char idstring[128];
    char hash[16];
    MP_INT *c;

    if (!read_number(f,n) || !read_number(f,e)) {
        return -2;
    }
    
    if (!read_string(f,idstring)) {
        fprintf(stderr, "warning: public key is not self signed\n");
        return 0;
    }
    else {
        init(c);
        mdpkid(n,e,idstring,hash);
        if (!readalg(f,&salg) || !read_number(f,c) ||  
                !vsign(c,hash,n,e,salg)) {
            fprintf(stderr, 
              "public key is self signed with an INVALID signature\n");
            fprintf(stderr, "public key corruption detected\n");
            clear(c);
            return (-1);
        }
        fprintf(stderr, 
            "key is bound to identifier \n\"%s\"\n with a valid signature\n",
            idstring);
    }
    clear(c);
    return 1;
}

