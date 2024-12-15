/* s.c */

/* Magic Money V1.0
   Server Main Module */

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#ifdef MSDOS
# include <mem.h>
#else
# define getch() getc(stdin)
#endif
#include "usuals.h"
#include "md5.h"
#include "mpilib.h"
#include "pgpmem.h"
#include "fifo.h"
#include "pgptools.h"
#include "pgpkmgt.h"
#include "pgpkgen.h"
#include "mm.h"

/* Sets up the Magic Money server */
void mms_setup(void);

/* Initializes or extends the hash file */
void mms_hashsetup(FILE *fp);

/* Computes a new set of e and d pairs, renaming the old one */
void mms_newed(struct pgp_pubkey *pk,struct pgp_seckey *sk);

/* Mint a coin and write it out */
void mms_mint(char *value,FILE *fp,struct pgp_pubkey *pk,
	      struct pgp_seckey *sk);

/* Given a starting e value, looks for a valid e. Then finds d and u. */
void mms_findpair(unit *e,unit *d,struct pgp_pubkey *pk,
		  struct pgp_seckey *sk);
								  
/* Add a coin to the spent list */
void mms_cancel_coin(byte *coinid,byte *filename);

/* Check to see if a coin is on the spent list */
int mms_check_coin(byte *coinid,byte *filename);

/* Processes input message */
void mms_proc(struct fifo *input);

/* Log and print an error, usually before bailing out */
void mm_log(byte *msg);

main(int argc,char *argv[])
{
set_precision(MAX_UNIT_PRECISION);
randomize();

if(argc>1) { /* Something other than process input */
  unsigned i;
  FILE *fp;
  struct pgp_pubkey *pk;
  struct pgp_seckey *sk;
  struct fifo *f;
  switch(tolower(*argv[1])) {
    case('i'):
      mms_setup();
    case('n'):
      unlink(BANK_OLDSPENT_FILE);
      rename(BANK_SPENT_FILE,BANK_OLDSPENT_FILE);
      printf("Creating new hash file\n");
      fp=mm_safeopen(BANK_SPENT_FILE,"wb");
      mms_hashsetup(fp);
      fclose(fp);
      pk=safemalloc(sizeof(struct pgp_pubkey));
      sk=safemalloc(sizeof(struct pgp_seckey));
      fp=mm_safeopen(BANK_SECKEY_FILE,"rb");
      f=fifo_file_create(fp);
      pgpk_extract_key(f,pk,sk,NULL,NULL);
      fifo_destroy(f);
      fclose(fp);
      mms_newed(pk,sk);
      free(pk);
      free(sk);
      exit(0);

    case('m'):
      pk=safemalloc(sizeof(struct pgp_pubkey));
      sk=safemalloc(sizeof(struct pgp_seckey));
      fp=mm_safeopen(BANK_SECKEY_FILE,"rb");
      f=fifo_file_create(fp);
      pgpk_extract_key(f,pk,sk,NULL,NULL);
      fifo_destroy(f);
      fclose(fp);

      fp=mm_safeopen(COIN_OUT_FILE,"wb");
      for(i=2;i<argc;i++)
	mms_mint(argv[i],fp,pk,sk);
      fclose(fp);
      free(pk);
      free(sk);
      exit(0);
    case('d'):
      unlink(ELIST_OLD_FILE);
      unlink(DLIST_OLD_FILE);
      unlink(BANK_OLDSPENT_FILE);
      printf("Files deleted\n");
      exit(0);
    default:
      printf("\
Magic Money Server Module V1.0\n\n\
(i) initialize server\n\
(n) generate new e/d list\n\
(d) delete old e/d list (expire old coins)\n\
(m) (list of values) mint coins of given value\n");
      exit(1);
    }
  }
else {
  struct fifo *ascinput,*input;
  int i;
  ascinput=fifo_mem_create();
  while((i=getc(stdin))!=EOF)
    fifo_put((byte)i,ascinput); /* Snarf from stdin into fifo */
  input=fifo_mem_create();
  if(!mm_dearmor(ascinput,input)) { /* Unable to de-armor */
    fifo_destroy(input);
    mm_log("De-armor failed\n");
    exit(1);
    }
  /* Now we have it - let's see if it's good */
  if(pgp_check_packets(input,NULL,NULL)) {
    mm_log("Bad input packets\n");
    fifo_destroy(input);
    exit(1);
    }
  mms_proc(input);
  }
return(1); /* Shut up the compiler */
}

void mms_mint(char *value,FILE *fp,struct pgp_pubkey *pk,
	      struct pgp_seckey *sk)
{
word32 val;
struct mm_protocoin *pc;
unit *temp,*dp,*dq;
struct mpi *d;
struct mm_coin *c;
struct fifo *f;

if(sscanf(value,"%lu",&val)!=1) {
  printf("Bad coin value\n");
  return;
  }
pc=mm_newpcoin(val,&pk->n);
if(!pc) {
  printf("Coin value %lu does not exist\n",val);
  return;
  }
printf("Minting coin of value %lu\n",val);
c=safemalloc(sizeof(struct mm_coin));
mp_burn(c->c.value);
memcpy(&c->e,&pc->e,sizeof(struct mpi));

temp=pc->b.value; /* Use these for scratch */
dp=pc->bc.value;
dq=pc->e.value;
d=mm_getd(&pc->e,ELIST_FILE,DLIST_FILE);
if(!d)
  d=mm_getd(&pc->e,ELIST_OLD_FILE,DLIST_OLD_FILE);
if(!d) {
  mm_log("No secret exponent - fix files\n");
  exit(1);
  }
mp_move(temp,sk->p.value);
mp_dec(temp);
mp_mod(dp,d->value,temp);
mp_move(temp,sk->q.value);
mp_dec(temp);
mp_mod(dq,d->value,temp);
mp_modexp_crt(c->c.value,pc->c.value,sk->p.value,sk->q.value,
	      dp,dq,sk->u.value);
pgp_fix_mpi(&c->c);
f=fifo_file_create(fp);
fifo_put(MM_REGCOIN,f);
fifo_aput(pc->coinid,16,f);
pgp_create_mpi(f,&c->e);
pgp_create_mpi(f,&c->c);
fifo_destroy(f);
free(pc);
free(c);
free(d);
}

void mms_proc(struct fifo *input)
{
FILE *fp;
byte ideakey[16],hash[16];
struct pgp_pubkey *pk=safemalloc(sizeof(struct pgp_pubkey));
struct pgp_seckey *sk=safemalloc(sizeof(struct pgp_seckey));
struct mm_coin *c=safemalloc(sizeof(struct mm_coin));
struct mm_elist *el;
unit *t=safemalloc(sizeof(unitarr));
unit *dp=safemalloc(sizeof(unitarr));
unit *dq=safemalloc(sizeof(unitarr));
struct mpi *d;
struct mpi *x=safemalloc(sizeof(struct mpi));
unit *u=safemalloc(sizeof(unitarr));
byte *cp;
struct fifo *temp,*key,*new;
word32 value,vdep=0,vproto=0;
byte *name=mm_getname();
time_t timestamp;
MD5_CTX md5ctx;
unsigned i;
int oldlist;

/* Get server keys */

fp=mm_safeopen(BANK_SECKEY_FILE,"rb");
temp=fifo_file_create(fp);
pgpk_extract_key(temp,pk,sk,NULL,NULL);
fifo_destroy(temp);
fclose(fp);

/* Decrypt message */

if(pgp_nexttype(input)!=PGP_PKE) {
  mm_log("No RSA packet in message\n");
  exit(1);
  }
if(!pgp_extract_rsa(input,ideakey,pk,sk)) {
  mm_log("Bad RSA packet in message\n");
  exit(1);
  }
if(pgp_nexttype(input)!=PGP_CKE) {
  mm_log("No IDEA packet in message\n");
  exit(1);
  }
temp=fifo_mem_create();
if(!pgp_extract_idea(input,temp,ideakey)) {
  mm_log("Decryption of IDEA packet failed\n");
  exit(1);
  }
fifo_destroy(input);
fifo_aget(hash,16,temp);
MD5Init(&md5ctx);
pgp_md5(fifo_copy(temp),&md5ctx);
MD5Final(ideakey,&md5ctx);
if(memcmp(hash,ideakey,16)) {
  mm_log("Bad MD5 check in message\n");
  exit(1);
  }

new=fifo_mem_create();

/* Send name of coins */

fifo_aput(name,strlen(name),new);
fifo_put('\0',new);
		       
/* Send server message, if found */

fp=fopen(BANK_MSG_FILE,"rb");
if(fp) {
  word16 length;
  struct fifo *msg,*fmsg;
  msg=fifo_file_create(fp);
  fmsg=fifo_mem_create();
  pgp_textform(msg,fmsg,TRUE,TRUE);
  fclose(fp);
  length=(word16) fifo_length(fmsg);
  endian(&length,2);
  fifo_put(MM_SRVRMSG,new);
  fifo_aput(&length,2,new);
  fifo_append(new,fmsg);
  }

/* Get user reply key */

if(pgp_nexttype(temp)!=PGP_PK) {
  mm_log("No PGP key found in message\n");
  exit(1);
  }
key=fifo_mem_create();
pgpk_copy_packet(temp,key);

/* Send elist information */

el=safemalloc(sizeof(struct mm_elist));
fp=mm_safeopen(ELIST_FILE,"rb");
fread(el,sizeof(struct mm_elist),1,fp);
pgp_extract_mpi(temp,x);
if(mp_compare(el->e.value,x->value)==1) { /* Send new elist */
  rewind(fp);
  while(fread(el,sizeof(struct mm_elist),1,fp)) {
    fifo_put(MM_NEWELST,new);
    pgp_create_mpi(new,&el->e);
    endian(&el->value,4);
    fifo_aput(&el->value,4,new);
    }
  fclose(fp);
  fp=fopen(ELIST_OLD_FILE,"rb");
  if(fp) {
    while(fread(el,sizeof(struct mm_elist),1,fp)) {
      fifo_put(MM_OLDELST,new);
      pgp_create_mpi(new,&el->e);
      endian(&el->value,4);
      fifo_aput(&el->value,4,new);
      }
    fclose(fp);
    }
  }
else { /* No new elist */
  fclose(fp);
  fp=fopen(ELIST_OLD_FILE,"rb");
  if(fp) {
    fclose(fp);
    fifo_put(MM_NOELIST,new);
    }
  else
    fifo_put(MM_DELOLDE,new);
  }
free(el);

/* Check and count up deposited coins */

while(pgp_nexttype(temp)==MM_REGCOIN) {
  fifo_get(temp);
  fifo_aget(c->coinid,16,temp);
  pgp_extract_mpi(temp,&c->e);
  pgp_extract_mpi(temp,&c->c);
  value=mm_getvalue(&c->e,ELIST_FILE);
  oldlist=value?FALSE:TRUE;
  if(!value)
    value=mm_getvalue(&c->e,ELIST_OLD_FILE);
  if(!value) {
    mm_log("Coin has invalid exponent (expired?)\n");
    exit(1);
    }
  vdep+=value;
  mp_modexp(u,c->c.value,c->e.value,pk->n.value);
  endian(u,sizeof(unitarr));
  cp=(byte *)u;
  if(!mm_valid_coin(u,&pk->n,c->coinid)) {
    mm_log("Coin has bad signature\n");
    exit(1);
    }
  if(mms_check_coin(cp+sizeof(unitarr)-16,
	oldlist?BANK_OLDSPENT_FILE:BANK_SPENT_FILE)) {
    mm_log("Coin has already been spent\n");
    exit(1);
    }
  mms_cancel_coin(cp+sizeof(unitarr)-16,
	oldlist?BANK_OLDSPENT_FILE:BANK_SPENT_FILE);
  }

/* Sign new proto-coins from user */
vproto=vdep;
while(pgp_nexttype(temp)==MM_PROCOIN) {
  fifo_get(temp);
  fifo_aget(ideakey,8,temp);
  pgp_extract_mpi(temp,&c->e);
  pgp_extract_mpi(temp,&c->c);
  value=mm_getvalue(&c->e,ELIST_FILE);
  if(!value)
    value=mm_getvalue(&c->e,ELIST_OLD_FILE);
  if(!value||(value>vproto)) {
    mm_log(value?"Protocoins worth more than deposited coins\n" :
		 "Protocoin has unknown exponent (expired?)\n");
    exit(1);
    }
  vproto-=value;
  d=mm_getd(&c->e,ELIST_FILE,DLIST_FILE);
  if(!d)
    d=mm_getd(&c->e,ELIST_OLD_FILE,DLIST_OLD_FILE);
  if(!d) {
    mm_log("No secret exponent - fix files\n");
    exit(1);
    }
  mp_burn(x->value);
  mp_move(t,sk->p.value);
  mp_dec(t);
  mp_mod(dp,d->value,t);
  mp_move(t,sk->q.value);
  mp_dec(t);
  mp_mod(dq,d->value,t);
  free(d);
  mp_modexp_crt(x->value,c->c.value,sk->p.value,sk->q.value,
		dp,dq,sk->u.value);
  pgp_fix_mpi(x);
  fifo_put(MM_BLICOIN,new);
  fifo_aput(ideakey,8,new);
  pgp_create_mpi(new,&c->e);  
  pgp_create_mpi(new,x);
  }
fifo_destroy(temp);

/* Sign, encrypt, and armor output message */

temp=fifo_mem_create();
time(&timestamp);
endian(&timestamp,4);
MD5Init(&md5ctx);
pgp_md5(fifo_copy(new),&md5ctx);
MD5Update(&md5ctx,"\0",1);
MD5Update(&md5ctx,(byte *)&timestamp,4);
MD5Final(hash,&md5ctx);
endian(&timestamp,4);
pgp_create_sig(temp,hash,timestamp,'\0',pk,sk);
fifo_append(temp,new);
new=fifo_mem_create();
for(i=0;i<16;i++)
  ideakey[i]=pgp_randombyte();
pgpk_extract_key(key,pk,NULL,NULL,NULL);
pgp_create_rsa(new,ideakey,pk);
pgp_create_idea(temp,new,ideakey);
temp=fifo_mem_create();
mm_armor(new,temp,FALSE);
{ int y; char z; while((y=fifo_get(temp))!=(-1)) { z=y; printf("%c",z); } }
fifo_destroy(temp);
fifo_destroy(key); 
free(pk); free(sk); free(c); free(t);
free(dp); free(dq); free(x); free(u);
free(name);
exit(0);
}

#define RECORDSIZE BUCKETSIZE*16L
#define bucket(i) (record+(16*i))

/* Add a coin to the spent list */
void mms_cancel_coin(byte *coinid,byte *filename)
{
FILE *fp;
word32 floc,oloc,flen;
unsigned i;
byte *record=safemalloc(RECORDSIZE);
byte zeroes[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
fp=mm_safeopen(filename,"r+b");
floc=coinid[0]+256*coinid[1];
floc%=BUCKETS;
floc*=RECORDSIZE;
fseek(fp,0,SEEK_END);
flen=ftell(fp);
oloc=floc;
while(floc<flen) {
  fseek(fp,floc,SEEK_SET);
  fread(record,RECORDSIZE,1,fp);
  for(i=0;i<BUCKETSIZE;i++)
    if(!memcmp(zeroes,bucket(i),16)) {
      memcpy(bucket(i),coinid,16);
      fseek(fp,floc,SEEK_SET);
      fwrite(record,RECORDSIZE,1,fp);
      fclose(fp);
      free(record);
      return;
      }
  floc+=BUCKETS*RECORDSIZE;
  }
fseek(fp,0,SEEK_END);
mms_hashsetup(fp);
memset(record,0,RECORDSIZE);
memcpy(bucket(0),coinid,16);
fseek(fp,flen+oloc,SEEK_SET);
fwrite(record,RECORDSIZE,1,fp);
fclose(fp);
free(record);
}

/* Check to see if a coin is on the spent list */
int mms_check_coin(byte *coinid,byte *filename)
{
FILE *fp;
word32 floc,flen;
unsigned i;
byte *record=safemalloc(RECORDSIZE);
fp=mm_safeopen(filename,"rb");
floc=coinid[0]+256*coinid[1];
floc%=BUCKETS;
floc*=RECORDSIZE;
fseek(fp,0,SEEK_END);
flen=ftell(fp);
while(floc<flen) {
  fseek(fp,floc,SEEK_SET);
  fread(record,RECORDSIZE,1,fp);
  for(i=0;i<BUCKETSIZE;i++)
    if(!memcmp(coinid,bucket(i),16)) {
      fclose(fp);
      free(record);
      return(TRUE);
      }
  floc+=BUCKETS*RECORDSIZE;
  }
fclose(fp);
free(record);
return(FALSE);
}

/* Sets up the Magic Money server */
void mms_setup(void)
{
unsigned i;
byte *buf=safemalloc(256);
byte *coinname=safemalloc(256);
struct pgp_pubkey *pk=safemalloc(sizeof(struct pgp_pubkey));
struct pgp_seckey *sk=safemalloc(sizeof(struct pgp_seckey));
struct mm_elist *el=safemalloc(sizeof(struct mm_elist));
struct fifo *uid,*ff,*temp,*valuelist;
word32 value;
unsigned bits=0;
FILE *fp;

printf("\nServer setup will create a PGP key for your server, ask you for\
 a list of\n\coin values, and create an e-d pair for each coin.\n\nEnter\
 a name for your server (this will be the PGP key userid)\n");
gets(buf);

while((bits<384)||(bits>1024)) {
  printf("\nEnter number of bits for the key: ");
  scanf("%u",&bits);
  }

gets(coinname); /* Eat pending cr */
printf("\nEnter a name for your digital coins.\n");
gets(coinname);
				  
printf("\n\
Enter values for your digital coins. Each value can be between 1 and \n\
2^32 %s(s). Press return after each one, enter 0 to end.\n",coinname);
valuelist=fifo_mem_create();
while(scanf("%lu",&value),value)
  fifo_aput(&value,4,valuelist);

printf("\nGenerating key pair\n");

fp=mm_safeopen(COIN_NAME_FILE,"wb");
fwrite(coinname,strlen(coinname),1,fp);
fclose(fp); /* Write coin name */

uid=fifo_mem_create();
fifo_put(PGP_UID,uid);
fifo_put((byte)strlen(buf),uid);
fifo_aput(buf,strlen(buf),uid);

pgpk_keygen(pk,sk,bits,17,time(NULL),0,&gen_output);
printf("\nKey generation completed\n\n");
temp=fifo_mem_create();
pgpk_create_pk(temp,pk);
fifo_append(temp,fifo_copy(uid));

fp=mm_safeopen(BANK_PUBKEY_FILE,"wb");
ff=fifo_file_create(fp);
fifo_append(ff,fifo_copy(temp));
fifo_destroy(ff);
fclose(fp); /* Write public key */

fp=mm_safeopen(BANK_ASCKEY_FILE,"wa");
ff=fifo_file_create(fp);
mm_armor(temp,ff,TRUE);
fifo_destroy(ff);
fclose(fp); /* Write ascii-armored copy of public key */

fp=mm_safeopen(BANK_SECKEY_FILE,"wb");
ff=fifo_file_create(fp);
pgpk_create_sk(ff,pk,sk,NULL);
fifo_append(ff,uid);
fifo_destroy(ff);
fclose(fp); /* Write secret key */

printf("Creating value list\n");
fp=mm_safeopen(ELIST_FILE,"wb");
mp_burn(el->e.value);
el->e.bits=0;
el->e.bytes=0;
while(fifo_aget(&value,4,valuelist)) {
  printf("%lu ",value);
  el->value=value;
  fwrite(el,sizeof(struct mm_elist),1,fp);
  }
fclose(fp);
fifo_destroy(valuelist);
printf("\n");

printf("\nGenerating e and d lists\n");
mms_newed(pk,sk);
unlink(ELIST_OLD_FILE);

printf("\nInitializing hash file\n");
fp=mm_safeopen(BANK_SPENT_FILE,"wb");
mms_hashsetup(fp);
fclose(fp);

printf("\n\
Your server is initialized. An ASCII-armored copy of your public key has\n\
been placed in file (%s).\n",BANK_ASCKEY_FILE);

free(coinname);
free(buf);
free(pk);
free(sk);
free(el);

exit(0);
}

/* Computes a new set of e and d pairs, renaming the old one */
void mms_newed(struct pgp_pubkey *pk,struct pgp_seckey *sk)
{
struct mm_elist *el=safemalloc(sizeof(struct mm_elist));
unit *e=safemalloc(sizeof(unitarr));
unit *d=safemalloc(sizeof(unitarr));
struct fifo *ff,*valuelist;
word32 value;
FILE *fp,*fp2;

unlink(ELIST_OLD_FILE);
rename(ELIST_FILE,ELIST_OLD_FILE);
unlink(DLIST_OLD_FILE);
rename(DLIST_FILE,DLIST_OLD_FILE);

mp_burn(e);
mp_move(e,pk->e.value);
fp=mm_safeopen(ELIST_OLD_FILE,"rb");
valuelist=fifo_mem_create();
/* Find biggest e so far, and get value list */
while(fread(el,sizeof(struct mm_elist),1,fp)) {
  value=el->value;
  fifo_aput(&value,4,valuelist);
  if(mp_compare(e,el->e.value)==(-1))
    mp_move(e,el->e.value);
  }
fclose(fp);

fp=mm_safeopen(ELIST_FILE,"wb");
fp2=mm_safeopen(DLIST_FILE,"wb");

mp_move(el->e.value,e);
while(fifo_aget(&value,4,valuelist)) {
  printf("Finding e and d for value %lu\n",value);
  mp_burn(d);
  mms_findpair(el->e.value,d,pk,sk);
  el->value=value;
  pgp_fix_mpi(&el->e);
  fwrite(el,sizeof(struct mm_elist),1,fp);
  fwrite(d,sizeof(unitarr),1,fp2);
  }
fifo_destroy(valuelist);
fclose(fp);
fclose(fp2);
free(el);
free(e);
free(d);
}

void mms_findpair(unit *e,unit *d,struct pgp_pubkey *pk,
		  struct pgp_seckey *sk)
{
unit *ptemp=safemalloc(sizeof(unitarr));
unit *qtemp=safemalloc(sizeof(unitarr));
unit *phi=safemalloc(sizeof(unitarr));
unit *F=safemalloc(sizeof(unitarr));
unit *G=safemalloc(sizeof(unitarr));
unit *p=sk->p.value;
unit *q=sk->q.value;

/* This code was taken from the key generation routines */

/* phi(n) is the Euler totient function of n, or the number of
   positive integers less than n that are relatively prime to n.
   G is the number of "spare key sets" for a given modulus n. 
   The smaller G is, the better.  The smallest G can get is 2. 
*/

mp_burn(d);
set_precision(bits2units(pk->n.bits+SLOP_BITS));
p=mm_pofs(p); q=mm_pofs(q); e=mm_pofs(e); d=mm_pofs(d);

mp_move(ptemp,p); mp_move(qtemp,q);
mp_dec(ptemp); mp_dec(qtemp);
mp_mult(phi,ptemp,qtemp);	/*  phi(n) = (p-1)*(q-1)  */
mp_gcd(G,ptemp,qtemp);		/*  G(n) = gcd(p-1,q-1)  */
mp_udiv(ptemp,qtemp,phi,G);	/* F(n) = phi(n)/G(n)  */
mp_move(F,qtemp);

/* We now have phi and F.  Next, compute e... */

do /* look for e such that gcd(e,phi(n)) = 1 */
{	mp_inc(e); mp_inc(e);
	/* try odd e's until we get it. */
	mp_gcd(ptemp,e,phi); 
} while (testne(ptemp,1));

/* Now we have e.  Next, compute d.
   d is the multiplicative inverse of e, mod F(n).
*/
mp_inv(d,e,F);	/* compute d such that (e*d) mod F(n) = 1 */
set_precision(MAX_UNIT_PRECISION);
free(ptemp);
free(qtemp);
free(phi);
free(F);
free(G);
}

/* Log and print an error, usually before bailing out */
void mm_log(byte *msg)
{
FILE *fp;
printf("ERROR: %s",msg);
fp=fopen(BANK_LOG_FILE,"aa");
if(!fp)
  return;
fprintf(fp,"%s",msg);
fclose(fp);
}

/* Initializes or extends the hash file */
void mms_hashsetup(FILE *fp)
{
unsigned i;
byte *record=safemalloc(RECORDSIZE);
memset(record,0,RECORDSIZE);
for(i=0;i<BUCKETS;i++)
  fwrite(record,RECORDSIZE,1,fp);
free(record);
}
