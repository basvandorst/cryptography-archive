/* pgptools.c */

#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include "usuals.h"
#include "md5.h"
#include "idea.h"
#include "mpilib.h"
#include "pgpmem.h"
#include "fifo.h"
#include "pgptools.h"

/* Bytes in random seed buffer */
#define SEEDSIZE 256

/* Abstract Syntax Notation One (ASN.1) Distinguished Encoding Rules (DER)
   encoding for RSA/MD5, used in PKCS-format signatures. */
static byte asn_array[] = {     /* PKCS 01 block type 01 data */
	0x30,0x20,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,
	0x02,0x05,0x05,0x00,0x04,0x10 };

/* These four globals are used by pgp_initrand and pgp_randombyte */
static word32 randcounter;
static unsigned randpointer;
static byte randseed[SEEDSIZE];
static byte randbuf[16];

/* Returns byte length of an MPI, used by above */
static unsigned pgp_check_mpi(struct fifo *f,word32 pos);

/* Endian swap */
#ifndef HIGHFIRST
void endian(void *data,unsigned length)
{
unsigned i;
byte *datc=data;
byte t;

for(i=0;i<length/2;i++) {
  t=datc[length-i-1];
  datc[length-i-1]=datc[i];
  datc[i]=t;
  }
}
#endif

/* Look at a packet; return its length and type */
void pgp_examine_packet(struct fifo *f,byte *type,
				word32 *length)
{
byte c;
byte *t=(byte *)length;
*length=0;
c=fifo_get(f);
*type=c&0xfc;
switch(c&3) {
  case 2:
    t[0]=fifo_get(f);
    t[1]=fifo_get(f);
  case 1:
    t[2]=fifo_get(f);
  case 0:
    t[3]=fifo_get(f);
  default:
    break;
  }
endian(t,4);
}

/* Initializes the random number generator. Input fifo provides random
   data and is destroyed. */
void pgp_initrand(struct fifo *f)
{
int i;
byte c;
unsigned pos=0;
while((i=fifo_get(f))!=(-1)) {
  c=(byte) i;
  randseed[pos++]^=c;
  if(pos>SEEDSIZE)
    pos=0;
  }
fifo_destroy(f);
time((time_t *)&randcounter);
randpointer=100; /* Make sure it runs the first time */
}

/* Returns a random byte */
byte pgp_randombyte()
{
if(randpointer>15) {
  time_t time;
  MD5_CTX md5ctx;
  MD5Init(&md5ctx);
  MD5Update(&md5ctx,(byte *)&randcounter,4);
  MD5Update(&md5ctx,(byte *)&time,4);
  MD5Update(&md5ctx,randseed,SEEDSIZE);
  MD5Final(randbuf,&md5ctx);
  randpointer=0;
  randcounter++;
  }
return(randbuf[randpointer++]);
}

/* Convert text formats: canonical to produce crlf form, otherwise
   produces lf form (or local if file opened ascii)
   stripspaces true removes trailing spaces after the end of a line
   destroys input fifo */
void pgp_textform(struct fifo *i,struct fifo *o,
		  int canonical,int stripspaces)
{
unsigned spaces=0;
int c=0;
int last;
while(last=c,(c=fifo_get(i))!=(-1)) {
  if(c==' ') {
    spaces++;
    continue;
    }
  if ((c=='\n'&&last!='\r')||(c=='\r'&&last!='\n')) {
    if(!stripspaces) 
      while(spaces) {
	spaces--;
	fifo_put(' ',o);
	}
    spaces=0;
    if(canonical)
      fifo_put('\r',o);
    fifo_put('\n',o);
    continue;
    }
  if(c=='\n'||c=='\r') {
    c=0;
    continue;
    }
  while(spaces) {
    spaces--;
    fifo_put(' ',o);
    }
  fifo_put(c,o);
  }
if(!stripspaces)
  while(spaces) {
    spaces--;
    fifo_put(' ',o);
    }
fifo_destroy(i);
}
       
void pgp_extract_literal(struct fifo *i,struct fifo *o,int *text,
				char *filename,time_t *timestamp)
{
byte type;
word32 length;
unsigned j;
pgp_examine_packet(i,&type,&length);
if(fifo_get(i)=='t')
  *text=TRUE;
else
  *text=FALSE;
j=fifo_get(i);
fifo_aget(filename,j,i);
filename[j]='\0';
fifo_aget(timestamp,4,i);
endian(timestamp,4);
length-=(6+strlen(filename));
fifo_moven(i,o,length);
}

void pgp_create_literal(struct fifo *i,struct fifo *o,int text,
				char *filename,time_t timestamp)
{
word32 length;
unsigned fnlen;
fnlen=strlen(filename);
fifo_put('\x0ae',o);
length=fifo_length(i)+fnlen+6;
endian(&length,4);
fifo_aput(&length,4,o);
if(text)
  fifo_put('t',o);
else
  fifo_put('b',o);
fifo_put((byte) fnlen,o);
fifo_aput(filename,fnlen,o);
endian(&timestamp,4);
fifo_aput(&timestamp,4,o);
fifo_append(o,i);
}

/* Calculate the MD5 hash of a fifo (destroys fifo) */
void pgp_md5(struct fifo *i,MD5_CTX *md5ctx)
{
byte *buf=safemalloc(FIFO_BLOCK_SIZE);
unsigned n;

while((n=fifo_aget(buf,FIFO_BLOCK_SIZE,i)))
  MD5Update(md5ctx,buf,n);
fifo_destroy(i);
free(buf);
}

/* Create a conventionally encrypted (IDEA) data packet */
void pgp_create_idea(struct fifo *i,struct fifo *o,
		     byte key[16])
{
unsigned j;
word32 length;
byte header[10];
byte *buf=safemalloc(FIFO_BLOCK_SIZE);
word16 iv[4]={0,0,0,0};

length=10+fifo_length(i);
endian(&length,4);
fifo_put('\xa6',o);
fifo_aput(&length,4,o);
for(j=0;j<8;j++)
  header[j]=pgp_randombyte();
header[8]=header[6];
header[9]=header[7];
initcfb_idea(iv,key,FALSE);
ideacfb(header,10);
fifo_aput(header,10,o);
while((j=fifo_aget(buf,FIFO_BLOCK_SIZE,i))) {
  ideacfb(buf,j);
  fifo_aput(buf,j,o);
  }
close_idea();
fifo_destroy(i);
free(buf);
}

/* Extract data from a conventionally encrypted (IDEA) data packet
   Returns TRUE if decryption successful, FALSE if wrong key
   Consumes packet in either case, leaving input at next packet */
int pgp_extract_idea(struct fifo *i,struct fifo *o,
		     byte key[16])
{
byte type;
word32 length;
byte header[10];
byte *buf=safemalloc(FIFO_BLOCK_SIZE);
unsigned j;
word16 iv[4]={0,0,0,0};

pgp_examine_packet(i,&type,&length);
fifo_aget(header,10,i);
initcfb_idea(iv,key,TRUE);
ideacfb(header,10);
length-=10;
while(length) {
  j=FIFO_BLOCK_SIZE;
  if(j>length) j=length;
  fifo_aget(buf,j,i);
  ideacfb(buf,j);
  fifo_aput(buf,j,o);
  length-=j;
  }
close_idea();
free(buf);
if((header[6]==header[8])&&(header[7]==header[9]))
  return(TRUE);
else
  return(FALSE);
}

/* Calculates bits and bytes for an mpi */
void pgp_fix_mpi(struct mpi *n)
{
word16 bits,bytes;
bits=countbits(n->value);
bytes=bits2bytes(bits);
n->bits=bits;
n->bytes=bytes;
}

/* Write an MPI to a fifo from an mpi struct */
void pgp_create_mpi(struct fifo *f,struct mpi *n)
{
byte *bp;
word16 bits=n->bits;
unsigned bytes=n->bytes;
bp=(byte *) n->value;
endian(&bits,2);
fifo_aput(&bits,2,f);
endian(bp,sizeof(unitarr));
fifo_aput(bp+(sizeof(unitarr)-bytes),bytes,f);
endian(bp,sizeof(unitarr));
}

/* Retrieve an MPI from a packet and put it in an mpi struct */
void pgp_extract_mpi(struct fifo *f,struct mpi *n)
{
byte *bp;
word16 bits;
unsigned bytes;
fifo_aget(&bits,2,f);
endian(&bits,2);
n->bits=bits;
bytes=bits2bytes(bits);
n->bytes=bytes;
if(bytes>sizeof(unitarr)) {
  n->bytes=0;
  n->bits=0;
  return;
  }
bp=(byte *) n->value;
mp_burn((unit *)bp);
fifo_aget(bp+(sizeof(unitarr)-bytes),bytes,f);
endian(bp,sizeof(unitarr));
}

/* Look for specific packet types */
/* Searches for up to 3 types; returns type or 0 if end of fifo */
byte pgp_find_packet(struct fifo *f,byte a,
			      byte b,byte c)
{
byte t;
word32 length;
while(t=pgp_nexttype(f)) {
  if((t==a)||(t==b)||(t==c))
    return(t);
  pgp_examine_packet(f,&t,&length);
  fifo_skipn(f,length);
  }
return(0);
}

/* Returns a 16 bit checksum; used to check secret keys */
void pgp_checksum(void *buf,unsigned length,word16 *checksum)
{
byte *bbuf=buf;
while(length--)
  *checksum+=*(bbuf++);
}

/* Decrypts an encrypted secret key, returns TRUE if successful */
int pgp_decrypt_sk(struct pgp_seckey *sk,byte key[16])
{
byte *cp;
word16 iv[4]={0,0,0,0};
word16 checksum=0;
word16 bits;
unsigned i;
struct mpi *mpis[4];
struct mpi *cmpi;

initcfb_idea(iv,key,TRUE);
ideacfb(sk->iv,8);

mpis[0]=&sk->d;
mpis[1]=&sk->p;
mpis[2]=&sk->q;
mpis[3]=&sk->u;

for(i=0;i<4;i++) {
  cmpi=mpis[i];
  cp=(byte *)cmpi->value;
  endian(cp,sizeof(unitarr));
  cp+=(sizeof(unitarr)-cmpi->bytes);
  ideacfb(cp,cmpi->bytes);
  bits=cmpi->bits;
  endian(&bits,2);
  pgp_checksum(&bits,2,&checksum);
  pgp_checksum(cp,cmpi->bytes,&checksum);
  endian(cmpi->value,sizeof(unitarr));
  }
close_idea();
endian(&sk->checksum,2);
if(checksum==sk->checksum)
  return(TRUE);
else
  return(FALSE);
}

/* Create an RSA-encrypted packet */
void pgp_create_rsa(struct fifo *f,byte ideakey[16],
		    struct pgp_pubkey *pk)
{       
word16 i;
struct mpi *p=safemalloc(sizeof(struct mpi));
struct mpi *c=safemalloc(sizeof(struct mpi));
word16 checksum=0;
byte *cp,*scp;
set_precision(MAX_UNIT_PRECISION); /* modexp will take care of it */
mp_burn(p->value);
cp=(byte *)p->value;
cp+=(sizeof(unitarr)-pk->n.bytes);
*cp++=0;
*cp++=2;
for(i=0;i<(pk->n.bytes-22);)
  if((*cp=pgp_randombyte())) {
    cp++; i++;
    }
*cp++=0;
*cp++=1;
memcpy(cp,ideakey,16);
cp+=16;
pgp_checksum(ideakey,16,&checksum);
endian(&checksum,2);
memcpy(cp,&checksum,2);
endian(p->value,sizeof(unitarr));
mp_modexp(c->value,p->value,pk->e.value,pk->n.value);
pgp_fix_mpi(c);
i=12+c->bytes;
endian(&i,2);
fifo_put(PGP_PKE|1,f);
fifo_aput(&i,2,f);
fifo_put(2,f);
fifo_aput(pk->keyid,8,f);
fifo_put(1,f);
pgp_create_mpi(f,c);
free(p);
free(c);
}

/* Create an RSA signature packet */
void pgp_create_sig(struct fifo *f,byte hash[16],
     time_t timestamp,byte sigtype,
     struct pgp_pubkey *pk,struct pgp_seckey *sk)
{
word16 i;
struct mpi *p=safemalloc(sizeof(struct mpi));
struct mpi *c=safemalloc(sizeof(struct mpi));
unit *dp=safemalloc(sizeof(unitarr));
unit *dq=safemalloc(sizeof(unitarr));
unit *temp=safemalloc(sizeof(unitarr));
byte *cp,*scp;
mp_burn(p->value);
set_precision(bits2units(pk->n.bits+SLOP_BITS));
cp=(byte *)p->value;
cp+=(sizeof(unitarr)-pk->n.bytes);
*cp++=0;
*cp++=1;
for(i=0;i<pk->n.bytes-37;i++)
  *cp++=0xff;
*cp++=0;
memcpy(cp,asn_array,18);
cp+=18;
memcpy(cp,hash,16);
endian(p->value,sizeof(unitarr));
mp_move(temp,sk->p.value);
mp_dec(temp);
mp_mod(dp,sk->d.value,temp);
mp_move(temp,sk->q.value);
mp_dec(temp);
mp_mod(dq,sk->d.value,temp);
mp_modexp_crt(c->value,p->value,sk->p.value,sk->q.value,
	      dp,dq,sk->u.value);
pgp_fix_mpi(c);
fifo_put(PGP_SIG|1,f);
i=21+c->bytes;
endian(&i,2);
fifo_aput(&i,2,f);
fifo_put(2,f);
fifo_put(5,f);
fifo_put(sigtype,f);
endian(&timestamp,4);
fifo_aput(&timestamp,4,f);
fifo_aput(pk->keyid,8,f);
fifo_put(1,f);
fifo_put(1,f);
fifo_aput(hash,2,f);
pgp_create_mpi(f,c);
set_precision(MAX_UNIT_PRECISION);
free(p);
free(c);
free(dp);
free(dq);
free(temp);
}

/* Returns the 64-bit keyid needed to process an RSA-encrypted or
   signature block, or the keyid of a public or secret key. Does not
   consume the input fifo */
void pgp_get_keyid(struct fifo *f,byte keyid[8])
{
unsigned type;
type=pgp_nexttype(f);
if((type==PGP_PK)||(type==PGP_SK)) {
  unsigned modbits,modbytes,uidloc,i;
  modbits=256*fifo_rget(11,f)+fifo_rget(12,f);
  modbytes=bits2bytes(modbits);
  uidloc=modbytes+5; /* modbytes + 13 - 8 */
  for(i=0;i<8;i++)
    *(keyid++)=fifo_rget(uidloc++,f);
  }
else {
  unsigned i,j,k;
  j=k=fifo_rget(0,f);
  j&=3;
  if(j==0)
    j=1;
  else if (j==1)
    j=2;
  else
    j=4;
  switch(k&0xFC) {
    case PGP_PKE:
      for(i=0;i<8;i++)
	keyid[i]=fifo_rget(2+i+j,f);
      return;
    case PGP_SIG:
      j+=fifo_rget(2+j,f);
      for(i=0;i<8;i++)
	keyid[i]=fifo_rget(3+i+j,f);
    default:
      return;
    }
  }
}

/* Decrypts and extracts the key from an RSA-encrypted block */
/* Returns true if successful, false if not */
int pgp_extract_rsa(struct fifo *f,byte ideakey[16],
     struct pgp_pubkey *pk,struct pgp_seckey *sk)
{
struct mpi *p=safemalloc(sizeof(struct mpi));
struct mpi *c=safemalloc(sizeof(struct mpi));
unit *dp=safemalloc(sizeof(unitarr));
unit *dq=safemalloc(sizeof(unitarr));
unit *temp=safemalloc(sizeof(unitarr));
byte result;

word16 checksum=0;
byte *pp;
byte type;
word32 length;
set_precision(bits2units(pk->n.bits+SLOP_BITS));
pgp_examine_packet(f,&type,&length);
fifo_skipn(f,10);
pgp_extract_mpi(f,c);
mp_move(temp,sk->p.value);
mp_dec(temp);
mp_mod(dp,sk->d.value,temp);
mp_move(temp,sk->q.value);
mp_dec(temp);
mp_mod(dq,sk->d.value,temp);
mp_modexp_crt(p->value,c->value,sk->p.value,sk->q.value,
	      dp,dq,sk->u.value);
set_precision(MAX_UNIT_PRECISION);
pgp_fix_mpi(p);
endian(p->value,sizeof(unitarr));
pp=(byte*)p->value;
if(pp[sizeof(unitarr)-p->bytes]==1) { /* Version 2.2 or earlier */
  memcpy(ideakey,pp+sizeof(unitarr)-p->bytes+1,16);
  pgp_checksum(ideakey,16,&checksum);
  endian(&checksum,2);
  if(memcmp(&checksum,pp+sizeof(unitarr)-p->bytes+17,2))
    result=FALSE;
  else
    result=TRUE;
  }
else { /* Version 2.3 or later */
  memcpy(ideakey,pp+sizeof(unitarr)-18,16);
  pgp_checksum(ideakey,16,&checksum);
  endian(&checksum,2);
  if(memcmp(&checksum,pp+sizeof(unitarr)-2,2))
    result=FALSE;
  else
    result=TRUE;
  }    
free(p);
free(c);
free(dp);
free(dq);
free(temp);
return(result);
}

/* Decrypts and extracts the MD from a signature packet */
/* Returns true if successful, false if not */
int pgp_extract_sig(struct fifo *f,byte hash[16],
    time_t *timestamp,byte *sigtype,
    struct pgp_pubkey *pk)
{
struct mpi *p=safemalloc(sizeof(struct mpi));
struct mpi *c=safemalloc(sizeof(struct mpi));
byte *pp;
byte type;
word32 length;
byte check[2];
set_precision(MAX_UNIT_PRECISION); /* modexp will take care of it */
pgp_examine_packet(f,&type,&length);
fifo_skipn(f,2);
*sigtype=fifo_get(f);
fifo_aget(timestamp,4,f);
endian(timestamp,4);
fifo_skipn(f,10);
fifo_aget(check,2,f);
pgp_extract_mpi(f,c);
mp_modexp(p->value,c->value,pk->e.value,pk->n.value);
pgp_fix_mpi(p);
endian(p->value,sizeof(unitarr));
pp=(byte*)p->value;
if(!memcmp(pp+sizeof(unitarr)-34,asn_array,18))
  memcpy(hash,pp+sizeof(unitarr)-16,16); /* Version 2.3 or later */
else /* Version 2.2 or earlier */
  memcpy(hash,pp+sizeof(unitarr)-p->bytes+1,16);
free(p);
free(c);
if(memcmp(hash,check,2))
  return(FALSE);
else
  return(TRUE);
}

/* This checks a series of packets for any problems. If there aren't
   any, it returns FALSE (0). If there are problems, it returns
   one of the PGP_BAD_* errors. If offset is a null pointer, it will
   be ignored; if not, it will return the offset (first packet is 0,
   next is 1, etc) of the bad packet. If type is non-null, it will
   return the type of the bad packet. This function uses random reads
   and does not alter the input fifo */
int pgp_check_packets(struct fifo *f,unsigned *offsetp,unsigned *typep)
{
word32 len=fifo_length(f);
word32 pos=0; /* Start of current packet */
word32 ppos;  /* Position within packet */
word32 plen;  /* Packet length */
unsigned offset=0;
unsigned type;
byte *cp;
byte c;
unsigned ll; /* Length of length field + type byte */

while(pos<len) {
  if(offsetp)
    *offsetp=offset;
  ppos=1;
  plen=0;
  type=fifo_rget(pos,f);
  if(typep)
    *typep=type&0xFC;
  cp=(byte*)&plen;
  switch(type&3) {
    case 2:
      cp[0]=fifo_rget(pos+ppos++,f);
      cp[1]=fifo_rget(pos+ppos++,f);
    case 1:
      cp[2]=fifo_rget(pos+ppos++,f);
    case 0:
      cp[3]=fifo_rget(pos+ppos++,f);
      break;
    default:
      if((type&0xFC)!=PGP_CMP)
	return(PGP_BAD_LENGTH);
      break;
    }
  endian(&plen,4);
  ll=ppos;
  switch(type&0xFC) {
    default:
      return(PGP_BAD_TYPE);

    case PGP_PKE:
      if(fifo_rget(pos+ppos,f)!=2)
	return(PGP_BAD_VERSION); /* Version byte */	 
      ppos+=9;
      if(fifo_rget(pos+ppos++,f)!=1)
	return(PGP_BAD_VERSION); /* Algorithm byte */
      ppos+=pgp_check_mpi(f,pos+ppos);
      break;

    case PGP_CKE:
      ppos+=plen;	
      break;

    case PGP_PT:
      c=fifo_rget(pos+ppos,f);
      if(c!='t'&&c!='b')         /* Assume any unknown packet types */
	return(PGP_BAD_VERSION); /* are unknown versions */
      ppos=plen+ll; /* Nothing else to check */   
      break;

    case PGP_SIG:
      if(fifo_rget(pos+ppos++,f)!=2)
	return(PGP_BAD_VERSION); /* Version byte */	 
      if(fifo_rget(pos+ppos,f)!=5)
	return(PGP_BAD_VERSION); /* Appended data (validity not present) */	 
      ppos+=14;
      if(fifo_rget(pos+ppos++,f)!=1)
	return(PGP_BAD_VERSION); /* RSA Algorithm byte */
      if(fifo_rget(pos+ppos,f)!=1)
	return(PGP_BAD_VERSION); /* MD5 Algorithm byte */
      ppos+=3;
      ppos+=pgp_check_mpi(f,pos+ppos);
      break;

    case PGP_PK:
      if(fifo_rget(pos+ppos,f)!=2) /* Version byte */
	return(PGP_BAD_VERSION);
      ppos+=7;
      if(fifo_rget(pos+ppos++,f)!=1) /* Algorithm byte */
	return(PGP_BAD_VERSION);
      ppos+=pgp_check_mpi(f,pos+ppos); /* Modulus n */
      ppos+=pgp_check_mpi(f,pos+ppos); /* Exponent e */
      break;

    case PGP_SK:
      if(fifo_rget(pos+ppos,f)!=2) /* Version byte */
	return(PGP_BAD_VERSION);
      ppos+=7;
      if(fifo_rget(pos+ppos++,f)!=1) /* Algorithm byte */
	return(PGP_BAD_VERSION);
      ppos+=pgp_check_mpi(f,pos+ppos); /* Modulus n */
      ppos+=pgp_check_mpi(f,pos+ppos); /* Exponent e */
      switch(fifo_rget(pos+ppos++,f)) {
	case(0):
	  break; /* Unencrypted */
	case(1):
	  ppos+=8; /* Encrypted, jump over IV */
	  break;
	default:
	  return(PGP_BAD_VERSION); /* Later version, different cipher */
	}
      ppos+=pgp_check_mpi(f,pos+ppos); /* Exponent d */
      ppos+=pgp_check_mpi(f,pos+ppos); /* Prime p */
      ppos+=pgp_check_mpi(f,pos+ppos); /* Prime q */
      ppos+=pgp_check_mpi(f,pos+ppos); /* Inverse u */
      ppos+=2; /* We're not actually going to check the checksum. If it's
		  bad, you will find out when you try to use the key. */
      break;

    case PGP_UID:
      ppos=plen+ll; /* Not much to check here */
      break;

    case PGP_TRU:
      ppos=1+ll; /* Must be one byte long */
      break;

    case PGP_CMP:
      return(FALSE); /* A compressed packet has no length field, so we
		     can't check it, and it has to be the last thing 
		     in the fifo. We're done. */
    }
  if(ppos!=plen+ll)
    return(PGP_BAD_LENGTH);
  if(ppos+pos>len)
    return(PGP_BAD_LENGTH);
  pos+=ppos;
  offset++;
  }
return(FALSE);
}    
	       
/* Returns byte length of an MPI, used by above */
static unsigned pgp_check_mpi(struct fifo *f,word32 pos)
{
word16 l,len;
byte *cp=(byte *)&l;
cp[0]=fifo_rget(pos,f);
cp[1]=fifo_rget(pos+1,f);
endian(&l,2);
len=bits2bytes(l);
if(len>sizeof(unitarr))
  return(0); /* Make sure length is wrong if mpi too long */
else
  return(len+2); /* Account for length field */
}
