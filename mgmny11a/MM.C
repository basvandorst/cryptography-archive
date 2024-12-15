/* mm.c */

/* Magic Money Digital Cash System V1.1 */

#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#ifdef MSDOS
# include <mem.h>
# include <io.h>
#else /* UNIX */
# define getch() getc(stdin)
# include <unistd.h>
#endif
#include "usuals.h"
#include "md5.h"
#include "mpilib.h"
#include "pgpmem.h"
#include "fifo.h"
#include "pgptools.h"
#include "pgparmor.h"
#include "pgpkmgt.h"
#include "mm.h"

static byte armor_data_header[]="\
-----BEGIN PGP MESSAGE-----\n\
Version 2.3a\n\n";

static byte armor_key_header[]="\
-----BEGIN PGP PUBLIC KEY BLOCK-----\n\
Version: 2.3a\n\n";

static byte armor_data_tailer[]="\
-----END PGP MESSAGE-----\n";

static byte armor_key_tailer[]="\
-----END PGP PUBLIC KEY BLOCK-----\n";

/* Abstract Syntax Notation One (ASN.1) Distinguished Encoding Rules (DER)
   encoding for RSA/MD5, used in PKCS-format signatures. */
byte asn_array[] = {     /* PKCS 01 block type 01 data */
	0x30,0x20,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,
	0x02,0x05,0x05,0x00,0x04,0x10 };

/* Get name of coins */
char *mm_getname(void)
{
FILE *fp;
char *name;
long size;
fp=fopen(COIN_NAME_FILE,"rb");
if(fp) {
  fseek(fp,0,SEEK_END);
  size=ftell(fp);
  fseek(fp,0,SEEK_SET);
  name=safemalloc(size+1);
  fread(name,size+1,1,fp);
  fclose(fp);
  name[size]='\0';
  }
else {
  name=safemalloc(1);
  *name='\0';
  }
return(name);
}

/* Simple one-part armor function */
void mm_armor(struct fifo *i,struct fifo *o,int key)
{
byte *header=key?armor_key_header:armor_data_header;
byte *tailer=key?armor_key_tailer:armor_data_tailer;

fifo_aput(header,strlen(header),o);
pgp_create_armor(i,o,0);
fifo_aput(tailer,strlen(tailer),o);
fifo_destroy(i);
}

/* Simple one-part dearmor function */
int mm_dearmor(struct fifo *i,struct fifo *o)
{
int result;
if(!fifo_find("-----BEGIN PGP",i))
  result=FALSE;
else {
  fifo_find("Version",i);
  fifo_find("\n",i);
  result=pgp_extract_armor(i,o);
  }
fifo_destroy(i);
return(result);
}

FILE *mm_safeopen(byte *filename,byte *mode)
{
FILE *fp;
fp=fopen(filename,mode);
if(!fp) {
  printf("Unable to open %s\n",filename);
  exit(1);
  }
return(fp);
}

/* Generate a new proto-coin */
struct mm_protocoin *mm_newpcoin(word32 value,struct mpi *n)
{
FILE *fp;
word16 i;
unsigned bitsleft;
byte *cp,*scp;
struct mpi *e=mm_gete(value,ELIST_FILE);
unit *t;
struct mm_protocoin *p;
byte hash[16];
MD5_CTX md5ctx;
if(!e)
  return(NULL);
p=safemalloc(sizeof(struct mm_protocoin));
t=safemalloc(sizeof(unitarr));
mp_burn(t);
mp_burn(p->e.value);
mp_burn(p->c.value);
mp_burn(p->bc.value);
mp_burn(p->b.value);
mp_move(p->e.value,e->value);

for(i=0;i<8;i++)
  p->tempid[i]=pgp_randombyte();

for(i=0;i<16;i++)
  p->coinid[i]=pgp_randombyte();

MD5Init(&md5ctx);
MD5Update(&md5ctx,p->coinid,16);
MD5Final(hash,&md5ctx);

cp=(byte *)p->b.value;
cp+=sizeof(unitarr); /* Start at the end, assuming big-endianness */
bitsleft=n->bits-8;
/* The blinding factor will be one byte shorter than n. Modexp will not
   work if the input is larger than the modulus. This made for a very
   interesting debugging run... */
while(bitsleft>7) {
  *(--cp)=pgp_randombyte();
  bitsleft-=8;
  }
if(bitsleft) { /* Odd bits */
  *(--cp)=pgp_randombyte();
  *cp >>= (8-bitsleft);
  }
endian(p->b.value,sizeof(unitarr));

cp=(byte *)p->c.value;
cp+=(sizeof(unitarr)-n->bytes);
*cp++=0;
*cp++=1;
for(i=0;i<n->bytes-37;i++)
  *cp++=0xff;
*cp++=0;
memcpy(cp,asn_array,18);
cp+=18;
memcpy(cp,hash,16);
endian(p->c.value,sizeof(unitarr));

mp_modexp(t,p->b.value,p->e.value,n->value);
stage_modulus(n->value);
mp_modmult(p->bc.value,t,p->c.value);

pgp_fix_mpi(&p->e);
pgp_fix_mpi(&p->b);
pgp_fix_mpi(&p->c);
pgp_fix_mpi(&p->bc);
free(e);
free(t);
return(p);
} 

/* Get e given value */
struct mpi *mm_gete(word32 value,char *filename)
{
FILE *fp=fopen(filename,"rb");
struct mm_elist *el;
struct mpi *e;
if(!fp)
  return(NULL);
el=safemalloc(sizeof(struct mm_elist));
e=safemalloc(sizeof(struct mpi));
while(fread(el,sizeof(struct mm_elist),1,fp)) {
  if(el->value==value) {
    memcpy(e->value,el->e.value,sizeof(unitarr));
    pgp_fix_mpi(e);
    free(el);
    fclose(fp);
    return(e);
    }
  }
free(e);
free(el);
fclose(fp);
return(NULL);
}

/* Get value given e */
word32 mm_getvalue(struct mpi *e,char *filename)
{
FILE *fp=fopen(filename,"rb");
struct mm_elist *el;
word32 value;
if(!fp)
  return(0);
el=safemalloc(sizeof(struct mm_elist));
while(fread(el,sizeof(struct mm_elist),1,fp)) {
  if(mp_compare(e->value,el->e.value)==0) {
    value=el->value;
    free(el);
    fclose(fp);
    return(value);
    }
  }
free(el);
fclose(fp);
return(0);
}

/* Given e, get corresponding d */
struct mpi *mm_getd(struct mpi *e,char *efile,char *dfile)
{
struct mpi *d;
FILE *ef,*df;
struct mm_elist *el;

ef=fopen(efile,"rb");
if(!ef)
  return(NULL);
df=fopen(dfile,"rb");
if(!df)
  return(NULL);
el=safemalloc(sizeof(struct mm_elist));
while(fread(el,sizeof(struct mm_elist),1,ef)) {
  if(mp_compare(e->value,el->e.value)==0) {
    d=safemalloc(sizeof(unitarr));
    fread(d,sizeof(unitarr),1,df);
    free(el);
    fclose(ef);
    fclose(df);
    return(d);
    }
  fseek(df,(long)sizeof(unitarr),SEEK_CUR);
  }
free(el);
fclose(ef);
fclose(df);
return(NULL);
}
				  
void gen_output(unsigned status)
{
byte outlist[]="pPqQf";
printf("%c",outlist[status]);
fflush(stdout); /* Print it now! */
}

/* Truncate a file at a given point */
void mm_truncate_file(byte *filename,word32 pos)
{
#ifdef MSDOS
int f;
f=open(filename,O_RDWR|O_BINARY);
if(f==(-1))
  return; /* Do something better here? */
chsize(f,pos);
close(f);
#else /* UNIX */
truncate(filename,pos);
#endif
}

void randomize(void)
{
FILE *fp;
struct fifo *temp;
fp=mm_safeopen(RANDOM_FILE,"rb");
temp=fifo_file_create(fp);
pgp_initrand(temp);
fclose(fp);
}

/* Check a coin to see if it is properly formed and 
   as long as the modulus n */
int mm_valid_coin(unit *c,struct mpi *n,byte *coinid)
{
byte *p=(byte *) c;
unsigned l=countbytes(n->value);
MD5_CTX md5ctx;
byte hash[16];

MD5Init(&md5ctx);
MD5Update(&md5ctx,coinid,16);
MD5Final(hash,&md5ctx);

p+=(units2bytes(MAX_UNIT_PRECISION)-l);
if(*p++!=0)
  return(FALSE);
if(*p++!=1)
  return(FALSE);
l-=37;
while(l--)
  if(*p++!=0xff)
    return(FALSE);
if(*p++!=0)
  return(FALSE);
if(memcmp(p,asn_array,18))
  return(FALSE);
p+=18;
if(memcmp(p,hash,16))
  return(FALSE);
else
  return(TRUE);
}
