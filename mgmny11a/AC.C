/* ac.c */

/* Magic Money V1.1
   Automatic Client Main Module */

#include <stdio.h>
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

/* Enable this to display the error codes
#define exit(result) { printf("Error %u\n",result); exit(result); }
   Enable this to display the error codes */

/* Version of safeopen that outputs error code */
FILE *mmac_safeopen(byte *filename,byte *mode);

/* Gets the bank's key */
void mmac_get_bank_key(struct pgp_pubkey *pk);

/* Deposit function, used for initialize and old coins too */
void mmac_deposit(struct fifo *i,struct fifo *o);

/* Look up a protocoin, then remove it from the file */
struct mm_protocoin *mmac_getpcoin(byte *id);

/* These are functions dispatched from the main menu. All exit when
   done or on error */
void mmac_display_keyid(char *fn1);
void mmac_list_denom(void);
void mmac_initialize(char *fn1);
void mmac_list_coins(char *fn1);
void mmac_process_coins(char *fn1,char *fn2);
void mmac_reinitialize(char *fn1);
void mmac_process_response(char *fn1);
void mmac_withdraw(char *fn1);
void mmac_process_oldcoins(char *fn1);

int main(int argc,char *argv[])
{
char *fn1,*fn2;
set_precision(MAX_UNIT_PRECISION);
randomize();
if(argc==1) {
  printf("\
Magic Money Automatic Client Module V1.1\n\
ac -[option] [inputfile] [outputfile]\n\
Options:\n\
-k display bank's keyid and client's keyid\n\
-k [inputfile] display keyid of bank which made this coins.dat file,\n\
   or the keyid needed to decrypt an ascii-armored pgp message\n\
-d list coin denominations\n\
-i [outputfile] initialize client (key length, key name from stdin)\n\
-l list all coins in coins.dat\n\
-l [inputfile] print total value of a coins.dat (client output) file.\n\
   Checks signatures and rejects if signature bad\n\
-p [inputfile,outputfile] process a coins.dat file for exchange with\n\
   the server. Reads from stdin a list of coin denominations to create,\n\
   terminated by 0. Writes to stdout a 128-bit transaction identifier.\n\
-r [outputfile] Reinitialize (generate init packet)\n\
-s [inputfile] Process a response from the server. Outputs the 128-bit\n\
   transaction identifier, followed by total value, followed by any\n\
   message from the server.\n\
-w [outputfile] Withdraws coins for payment. Accepts a list of coin\n\
   values from stdin, terminated by a zero.\n\
-x Returns the total value of any old coins (in danger of expiration)\n\
-x [outputfile] Exchanges old coins. Accepts a list of denominations\n\
   and generates an identifier like -p\n\
");
  exit(MMACERROR_BADCOMMAND);
  }
if(*(argv[1])=='-') {
  fn1=argv[2];
  if(argc==2) fn2=NULL; else fn2=argv[3];
  switch(tolower(*(argv[1]+1))) {
    case('k'):
      mmac_display_keyid(fn1);
    case('d'):
      mmac_list_denom();
    case('i'):
      mmac_initialize(fn1);
    case('l'):
      mmac_list_coins(fn1);
    case('p'):
      mmac_process_coins(fn1,fn2);
    case('r'):
      mmac_reinitialize(fn1);
    case('s'):
      mmac_process_response(fn1);
    case('w'):
      mmac_withdraw(fn1);
    case('x'):
      mmac_process_oldcoins(fn1);
    default:
      exit(MMACERROR_BADCOMMAND);
    }
  }
else exit(MMACERROR_BADCOMMAND);
}

void mmac_display_keyid(char *fn1)
{
unsigned i;
if(fn1) { /* Display file key */
  FILE *fp;
  fp=mmac_safeopen(fn1,"rb");
  if(fgetc(fp)<128) { /* Looking at a pgp message */
    struct fifo *f=fifo_file_create(fp);
    struct fifo *m=fifo_mem_create();
    byte keyid[8];
    if(!mm_dearmor(f,m)) 
      exit(MMACERROR_NODATA);
    fifo_destroy(f);
    fclose(fp);
    if(pgp_nexttype(m)!=PGP_PKE)
      exit(MMACERROR_NODATA);
    pgp_get_keyid(m,keyid);
    fifo_destroy(m);
    for(i=0;i<8;i++)
      printf("%02X",keyid[i]);
    printf("\n");
    exit(MMAC_OK);
    }
  else {
    fseek(fp,(long)(-9),SEEK_END);
    if(fgetc(fp)!=MM_BANKID) {
      fclose(fp);
      exit(MMACERROR_NODATA);
      }
    for(i=0;i<8;i++)
      printf("%02X",(unsigned char)fgetc(fp));
    printf("\n");
    fclose(fp);
    exit(MMAC_OK);
    }
  }
else { /* Display bank's key */
  struct pgp_pubkey *pk=safemalloc(sizeof(struct pgp_pubkey));
  struct fifo *f;
  FILE *fp;
  mmac_get_bank_key(pk);
  for(i=0;i<8;i++)
    printf("%02X",pk->keyid[i]);
  printf("\n");
  fp=fopen(USER_PUBKEY_FILE,"rb");
  f=fifo_file_create(fp);
  pgp_get_keyid(f,pk->keyid);
  fifo_destroy(f);
  fclose(fp);
  for(i=0;i<8;i++)
    printf("%02X",pk->keyid[i]);
  printf("\n");
  free(pk);
  exit(MMAC_OK);
  }
}

void mmac_list_denom(void)
{
FILE *fp;
struct mm_elist *el=safemalloc(sizeof(struct mm_elist));
fp=mmac_safeopen(ELIST_FILE,"rb");
while(fread(el,sizeof(struct mm_elist),1,fp))
  printf("%lu\n",el->value);
fclose(fp);
free(el);
exit(MMAC_OK);
}

void mmac_initialize(char *fn1)
{
unsigned i;
byte *buf=safemalloc(256);
struct pgp_pubkey *pk=safemalloc(sizeof(struct pgp_pubkey));
struct pgp_seckey *sk=safemalloc(sizeof(struct pgp_seckey));
struct fifo *uid,*ff,*temp,*o;
unsigned bits=0;
FILE *fp;

scanf("%u",&bits); /* Get key length */
gets(buf); /* Clear out phantom CR from scanf */
gets(buf); /* Get key name */

uid=fifo_mem_create();
fifo_put(PGP_UID,uid);
fifo_put((byte)strlen(buf),uid);
fifo_aput(buf,strlen(buf),uid);

pgpk_keygen(pk,sk,bits,17,time(NULL),0,NULL);
temp=fifo_mem_create();
pgpk_create_pk(temp,pk);
fifo_append(temp,fifo_copy(uid));

fp=mmac_safeopen(USER_PUBKEY_FILE,"wb");
ff=fifo_file_create(fp);
fifo_append(ff,temp);
fifo_destroy(ff);
fclose(fp); /* Write public key */

fp=mmac_safeopen(USER_SECKEY_FILE,"wb");
ff=fifo_file_create(fp);
pgpk_create_sk(ff,pk,sk,NULL);
fifo_append(ff,uid);
fifo_destroy(ff);
fclose(fp); /* Write secret key */

free(buf);
free(pk);
free(sk);

ff=fifo_mem_create();
fp=mmac_safeopen(fn1,"wa");
o=fifo_file_create(fp);
mmac_deposit(ff,o);
fifo_destroy(o);
fclose(fp);
exit(MMAC_OK);
}

void mmac_list_coins(char *fn1)
{
FILE *fp;
word32 cvalue,total;
struct mm_coin *c=safemalloc(sizeof(struct mm_coin));

if(fn1) { /* Check sigs and print coin values from a file */
  struct fifo *i;
  unit *u=safemalloc(sizeof(unitarr));
  struct pgp_pubkey *pk=safemalloc(sizeof(struct pgp_pubkey));

  mmac_get_bank_key(pk);
  fp=mmac_safeopen(fn1,"rb");
  i=fifo_file_create(fp);
  total=0;

  while(pgp_nexttype(i)==MM_REGCOIN) {
    fifo_get(i);
    fifo_aget(c->coinid,16,i);
    pgp_extract_mpi(i,&c->e);
    pgp_extract_mpi(i,&c->c);
    cvalue=mm_getvalue(&c->e,ELIST_FILE);
    if(!cvalue)
      cvalue=mm_getvalue(&c->e,ELIST_OLD_FILE);
    if(!cvalue) 
      exit(MMACERROR_BADEXP);
    mp_modexp(u,c->c.value,c->e.value,pk->n.value);
    endian(u,sizeof(unitarr));
    if(!mm_valid_coin(u,&pk->n,c->coinid))
      exit(MMACERROR_BADSIG);
    total+=cvalue;
    }
  fifo_destroy(i);
  fclose(fp);
  free(pk);
  free(u);
  printf("%lu\n",total);
  }

else { /* Print coin values from allcoins.dat */
  fp=mmac_safeopen(USER_COIN_FILE,"rb");
  while(fread(c,sizeof(struct mm_coin),1,fp)) {
    cvalue=mm_getvalue(&c->e,ELIST_FILE);
    if(!cvalue)
      cvalue=mm_getvalue(&c->e,ELIST_OLD_FILE);
    printf("%lu\n",cvalue);
    }
  }

free(c);
exit(MMAC_OK);
}

void mmac_process_coins(char *fn1,char *fn2)
{
FILE *fp,*fp2;
struct fifo *i,*o;
fp=mmac_safeopen(fn1,"rb");
fp2=mmac_safeopen(fn2,"wa");
i=fifo_file_create(fp);
o=fifo_file_create(fp2);
mmac_deposit(i,o);
fifo_destroy(o);
fclose(fp);
fclose(fp2);
exit(MMAC_OK);
}

void mmac_reinitialize(char *fn1)
{
FILE *fp;
struct fifo *i,*o;
i=fifo_mem_create();
fp=mmac_safeopen(fn1,"wa");
o=fifo_file_create(fp);
mmac_deposit(i,o);
fifo_destroy(o);
fclose(fp);
exit(MMAC_OK);
}

void mmac_process_response(char *fn1)
{
FILE *fp;
MD5_CTX md5ctx;
byte buf[16],buf2[16],identifier[16];
struct pgp_seckey *sk=safemalloc(sizeof(struct pgp_seckey));
time_t timestamp;
byte sigtype;
struct pgp_pubkey *pk=safemalloc(sizeof(struct pgp_pubkey));
struct mm_coin *c=safemalloc(sizeof(struct mm_coin));
struct mm_elist *el;
struct mpi *x=safemalloc(sizeof(struct mpi));
struct mm_protocoin *pc;
struct fifo *temp,*key,*i,*msg,*msg2;
byte tempid[8];
byte b;
word32 value,total=0;
unsigned j;

/* Open input file */
fp=mmac_safeopen(fn1,"rb");
temp=fifo_file_create(fp);
i=fifo_mem_create();

/* Dearmor input */
if(!mm_dearmor(temp,i)) {
  fclose(fp);
  exit(MMACERROR_NODATA);
  }
fclose(fp);

/* Get user secret key and decrypt file */
fp=mmac_safeopen(USER_SECKEY_FILE,"rb");
temp=fifo_file_create(fp);
pgpk_extract_key(temp,pk,sk,NULL,NULL);
fifo_destroy(temp);
fclose(fp);
if(!pgp_extract_rsa(i,buf,pk,sk)) exit(MMACERROR_NODATA);
temp=fifo_mem_create();
if(!pgp_extract_idea(i,temp,buf)) exit(MMACERROR_NODATA);
fifo_destroy(i);
i=temp;

/* Get bank public key and check signature */
mmac_get_bank_key(pk);
if(!pgp_extract_sig(i,buf,&timestamp,&sigtype,pk))
  exit(MMACERROR_NODATA);
endian(&timestamp,4);
MD5Init(&md5ctx);
pgp_md5(fifo_copy(i),&md5ctx);
MD5Update(&md5ctx,&sigtype,1);
MD5Update(&md5ctx,(byte *)&timestamp,4);
MD5Final(buf2,&md5ctx);
if(memcmp(buf,buf2,16)) exit(MMACERROR_BADSIG);

/* Get name of coins */
fp=mmac_safeopen(COIN_NAME_FILE,"wb");
while(b=(byte)fifo_get(i))
  fputc(b,fp);
fclose(fp);

/* Extract message, if found */
if(pgp_nexttype(i)==MM_SRVRMSG) {
  word16 length;
  fifo_get(i);
  msg=fifo_mem_create();
  msg2=fifo_mem_create();
  fifo_aget(&length,2,i);
  endian(&length,2);
  fifo_moven(i,msg,length);
  pgp_textform(msg,msg2,FALSE,TRUE);
  }
else msg2=NULL;

/* Process elist information */
switch(pgp_nexttype(i)) {
  case(MM_NEWELST):
    el=safemalloc(sizeof(struct mm_elist));
    fp=mmac_safeopen(ELIST_FILE,"wb");
    while(pgp_nexttype(i)==MM_NEWELST) {
      fifo_get(i);
      pgp_extract_mpi(i,&el->e);
      fifo_aget(&el->value,4,i);
      endian(&el->value,4);
      fwrite(el,sizeof(struct mm_elist),1,fp);
      }
    fclose(fp);
    if(pgp_nexttype(i)==MM_OLDELST) {
      fp=mmac_safeopen(ELIST_OLD_FILE,"wb");
      while(pgp_nexttype(i)==MM_OLDELST) {
	fifo_get(i);
	pgp_extract_mpi(i,&el->e);
	fifo_aget(&el->value,4,i);
	endian(&el->value,4);
	fwrite(el,sizeof(struct mm_elist),1,fp);
	}
      fclose(fp);
      }
    free(el);
    break;
  case(MM_DELOLDE):
    unlink(ELIST_OLD_FILE);
  case(MM_NOELIST):
    fifo_get(i);
  }

/* Unblind and store coins */
for(j=0;j<16;j++) identifier[j]=0;
fp=mmac_safeopen(USER_COIN_FILE,"ab");
while(pgp_nexttype(i)==MM_BLICOIN) {
  fifo_get(i);
  fifo_aget(tempid,8,i);
  pgp_extract_mpi(i,&c->e);
  pgp_extract_mpi(i,x);
  pc=mmac_getpcoin(tempid);
  if(!pc)
    exit(MMACERROR_NOPROTO);
  memcpy(c->coinid,pc->coinid,16);
  for(j=0;j<16;j++) identifier[j]^=c->coinid[j];
  mp_burn(c->c.value);
  mp_burn(pc->bc.value);
  set_precision(bits2units(pk->n.bits+SLOP_BITS));
  mp_inv(mm_pofs(pc->bc.value),
	 mm_pofs(pc->b.value),
	 mm_pofs(pk->n.value));
  set_precision(MAX_UNIT_PRECISION);
  stage_modulus(pk->n.value);
  mp_modmult(c->c.value,x->value,pc->bc.value);
  mp_modexp(x->value,c->c.value,c->e.value,pk->n.value);
  endian(x->value,sizeof(unitarr));
  if(!mm_valid_coin(x->value,&pk->n,c->coinid))
    exit(MMACERROR_BADSIG);
  pgp_fix_mpi(&c->c);
  fwrite(c,sizeof(struct mm_coin),1,fp);
  free(pc);
  value=mm_getvalue(&c->e,ELIST_FILE);
  if(!value)
    value=mm_getvalue(&c->e,ELIST_OLD_FILE);
  total+=value;
  }

/* Output transaction ID and total value */
for(j=0;j<16;j++)
  printf("%02X",identifier[j]);
printf("\n%lu\n",total);

/* Output message, if found */
if(msg2) {
  int y; char z;
  while((y=fifo_get(msg2))!=(-1)) 
    { z=y; printf("%c",z); } 
  fifo_destroy(msg2);
  }
fifo_destroy(i);
fclose(fp);
free(pk);
free(sk);
free(c);
free(x);
exit(MMAC_OK);
}

void mmac_withdraw(char *fn1)
{
FILE *fp,*fp2;
struct fifo *f;
word32 value,value2;
long floc,flen;
struct mm_coin *c=safemalloc(sizeof(struct mm_coin));
struct pgp_pubkey *pk=safemalloc(sizeof(struct pgp_pubkey));
int error_code=MMAC_OK;

fp2=mmac_safeopen(fn1,"wb");

f=fifo_file_create(fp2);
found:
while(scanf("%lu",&value),value) {
  fp=mmac_safeopen(USER_COIN_FILE,"r+b");
  while(fread(c,sizeof(struct mm_coin),1,fp)) {
    value2=mm_getvalue(&c->e,ELIST_FILE);
    if(!value2)
      value2=mm_getvalue(&c->e,ELIST_OLD_FILE);
    if(value==value2) { 
      fifo_put(MM_REGCOIN,f);
      fifo_aput(c->coinid,16,f);
      pgp_create_mpi(f,&c->e);
      pgp_create_mpi(f,&c->c);
      floc=ftell(fp);
      fseek(fp,0,SEEK_END);
      flen=ftell(fp);
      if(floc!=flen) {
	fseek(fp,flen-sizeof(struct mm_coin),SEEK_SET);
	fread(c,sizeof(struct mm_coin),1,fp);
	fseek(fp,floc-sizeof(struct mm_coin),SEEK_SET);
	fwrite(c,sizeof(struct mm_coin),1,fp);
	}
      fclose(fp);
      mm_truncate_file(USER_COIN_FILE,flen-sizeof(struct mm_coin));
      goto found; /* Spaghetti code ugly goto loop */
      }
    }
  error_code=MMACERROR_NOCOIN;
  break;
  }
mmac_get_bank_key(pk);
fifo_put(MM_BANKID,f); /* Write bank id out */
fifo_aput(pk->keyid,8,f);
fifo_destroy(f);
fclose(fp2);
free(c);
free(pk);
exit(error_code);
}

void mmac_process_oldcoins(char *fn1)
{
FILE *fp;
struct mm_coin *c=safemalloc(sizeof(struct mm_coin));
if(fn1) { /* Extract old coins for exchange */
  struct fifo *f,*o;
  long floc,flen;
  word32 value;
  floc=0;
  f=fifo_mem_create();
  while(fp=mmac_safeopen(USER_COIN_FILE,"r+b"),
	fseek(fp,floc,SEEK_SET),
	fread(c,sizeof(struct mm_coin),1,fp)) {
    floc=ftell(fp);
    if(!mm_getvalue(&c->e,ELIST_FILE)) {
      if(mm_getvalue(&c->e,ELIST_OLD_FILE)) {
	fifo_put(MM_REGCOIN,f);
	fifo_aput(c->coinid,16,f);
	pgp_create_mpi(f,&c->e);
	pgp_create_mpi(f,&c->c);
	}
      fseek(fp,0,SEEK_END);
      flen=ftell(fp);
      if(floc!=flen) {
	fseek(fp,flen-sizeof(struct mm_coin),SEEK_SET);
	fread(c,sizeof(struct mm_coin),1,fp);
	fseek(fp,floc-sizeof(struct mm_coin),SEEK_SET);
	fwrite(c,sizeof(struct mm_coin),1,fp);
	}
      fclose(fp);
      mm_truncate_file(USER_COIN_FILE,flen-sizeof(struct mm_coin));
      floc=0;
      }
    else
      fclose(fp);
    }
  fclose(fp);
  free(c);
  if(fifo_length(f)) {
    fp=mmac_safeopen(fn1,"wa");
    o=fifo_file_create(fp);
    mmac_deposit(f,o);
    fifo_destroy(o);
    fclose(fp);
    }
  else /* No coins to exchange */
    fifo_destroy(f);
  }
else { /* List total value of old coins */
  word32 cvalue,total=0;
  fp=mmac_safeopen(USER_COIN_FILE,"rb");
  while(fread(c,sizeof(struct mm_coin),1,fp)) {
    cvalue=mm_getvalue(&c->e,ELIST_OLD_FILE);
    total+=cvalue;
    }
  printf("%lu\n",total);
  fclose(fp);
  }
free(c);
exit(MMAC_OK);
}

FILE *mmac_safeopen(byte *filename,byte *mode)
{
FILE *fp;
fp=fopen(filename,mode);
if(!fp) 
  exit(MMACERROR_NOFILE);
return(fp);
}

void mmac_get_bank_key(struct pgp_pubkey *pk)
{
FILE *fp;
struct fifo *temp,*key;
fp=mmac_safeopen(BANK_ASCKEY_FILE,"rb");
temp=fifo_file_create(fp);
key=fifo_mem_create();
if(!mm_dearmor(temp,key))
  exit(MMACERROR_NODATA);
fclose(fp);
pgpk_extract_key(key,pk,NULL,NULL,NULL);
fifo_destroy(key);
}

void mmac_deposit(struct fifo *i,struct fifo *o)
{
FILE *fp;
word32 value;
unsigned j;
struct fifo *temp,*key,*depcoins,*protocoins;
struct pgp_pubkey *pk=safemalloc(sizeof(struct pgp_pubkey));
struct mm_coin *c=safemalloc(sizeof(struct mm_coin));
struct mm_elist *el=safemalloc(sizeof(struct mm_elist));
struct mpi *e;
struct mm_protocoin *pc;
unit *u=safemalloc(sizeof(unitarr));
byte ideakey[16];
byte identifier[16];
MD5_CTX md5ctx;
int coins=FALSE; /* Are there any coins? */

/* Get bank's key */

mmac_get_bank_key(pk);

/* Get user's key */

fp=mmac_safeopen(USER_PUBKEY_FILE,"rb");
temp=fifo_file_create(fp);
key=fifo_mem_create();
pgpk_copy_packet(temp,key);
fifo_destroy(temp);
fclose(fp);

/* Add in e value so server can decide whether to update */

depcoins=key;
mp_burn(el->e.value);
fp=fopen(ELIST_FILE,"rb");
if(fp) { /* Get last (highest) e value */
  fseek(fp,(long) 0-sizeof(struct mm_elist),SEEK_END);
  fread(el,sizeof(struct mm_elist),1,fp);
  fclose(fp);
  }
pgp_fix_mpi(&el->e);
pgp_create_mpi(depcoins,&el->e);

/* Process coins for deposit */

while(pgp_nexttype(i)==MM_REGCOIN) {
  coins=TRUE;
  fifo_get(i);
  fifo_aget(c->coinid,16,i);
  pgp_extract_mpi(i,&c->e);
  pgp_extract_mpi(i,&c->c);
  value=mm_getvalue(&c->e,ELIST_FILE);
  if(!value)
    value=mm_getvalue(&c->e,ELIST_OLD_FILE);
  if(!value)
    exit(MMACERROR_BADEXP);
  mp_modexp(u,c->c.value,c->e.value,pk->n.value);
  endian(u,sizeof(unitarr));
  if(!mm_valid_coin(u,&pk->n,c->coinid))
    exit(MMACERROR_BADSIG);
  fifo_put(MM_REGCOIN,depcoins);
  fifo_aput(c->coinid,16,depcoins);
  pgp_create_mpi(depcoins,&c->e);
  pgp_create_mpi(depcoins,&c->c);
  }
fifo_destroy(i);

/* Create proto-coins */
for(j=0;j<16;j++)
  identifier[j]=0;
fp=mmac_safeopen(USER_PCOIN_FILE,"ab");    
protocoins=fifo_mem_create();
while(coins) {
  scanf("%lu",&value);
  if(!value) break;
  pc=mm_newpcoin(value,&pk->n);
  if(!pc)
    exit(MMACERROR_NOCOIN);
  for(j=0;j<16;j++)
    identifier[j]^=pc->coinid[j];
  fwrite(pc,sizeof(struct mm_protocoin),1,fp);
  fifo_put(MM_PROCOIN,protocoins);
  fifo_aput(pc->tempid,8,protocoins);
  pgp_create_mpi(protocoins,&pc->e);
  pgp_create_mpi(protocoins,&pc->bc);
  free(pc);
  }
fclose(fp);

/* Assemble message to server */

fifo_append(depcoins,protocoins);
temp=depcoins;
depcoins=fifo_mem_create();
MD5Init(&md5ctx);
pgp_md5(fifo_copy(temp),&md5ctx);
MD5Final(ideakey,&md5ctx);
fifo_aput(ideakey,16,depcoins);
fifo_append(depcoins,temp);

for(j=0;j<16;j++)
  ideakey[j]=pgp_randombyte();
temp=fifo_mem_create();
pgp_create_rsa(temp,ideakey,pk);
pgp_create_idea(depcoins,temp,ideakey);
mm_armor(temp,o,FALSE);
if(coins) {
  for(j=0;j<16;j++)
    printf("%02X",identifier[j]);
  printf("\n");
  }
free(pk);
free(c);
free(u);
free(el);
}

/* Look up a protocoin, then remove it from the file */
struct mm_protocoin *mmac_getpcoin(byte *id)
{
FILE *fp;
long floc,flen;
struct mm_protocoin *pc=safemalloc(sizeof(struct mm_protocoin));
struct mm_protocoin *tc=safemalloc(sizeof(struct mm_protocoin));
fp=mmac_safeopen(USER_PCOIN_FILE,"r+b");
while(fread(pc,sizeof(struct mm_protocoin),1,fp)) {
  if(!memcmp(id,pc->tempid,8)) {
    floc=ftell(fp);
    fseek(fp,0,SEEK_END);
    flen=ftell(fp);
    if(floc!=flen) {
      fseek(fp,flen-sizeof(struct mm_protocoin),SEEK_SET);
      fread(tc,sizeof(struct mm_protocoin),1,fp);
      fseek(fp,floc-sizeof(struct mm_protocoin),SEEK_SET);
      fwrite(tc,sizeof(struct mm_protocoin),1,fp);
      }
    fclose(fp);
    free(tc);
    mm_truncate_file(USER_PCOIN_FILE,flen-sizeof(struct mm_protocoin));
    return(pc);
    }
  }
fclose(fp);
free(pc);
free(tc);
return(NULL);
}
