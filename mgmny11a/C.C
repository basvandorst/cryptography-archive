/* c.c */

/* Magic Money V1.1
   Client Main Module */

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

/* Sets up the Magic Money client */
void mmc_setup(void);

/* Deposit coins, generating new proto-coins */
void mmc_deposit(struct fifo *i);

/* Process a message from the bank */
void mmc_bankmsg(struct fifo *i);

/* Extract coins for payment */
void mmc_pay(void);

/* Extract all old coins */
void mmc_xold(void);

/* Look up a protocoin, then remove it from the file */
struct mm_protocoin *mmc_getpcoin(byte *id);

/* Gets the bank's key */
void mmc_get_bank_key(struct pgp_pubkey *pk);

struct coin_list {
  word32 value;
  unsigned count;
  struct coin_list *next;
  };

main(int argc,char *argv[])
{
struct fifo *input,*temp;
FILE *fp;
set_precision(MAX_UNIT_PRECISION);
randomize();
if(argc==1) {
  printf("\
Magic Money Client Module V1.1\n\
-i initialize (generate key)\n\
-r reinitialize (update elist)\n\
-p extract coins for payment\n\
-x exchange old coins for new ones\n\
otherwise provide name of a file containing coins to be processed\n");
  exit(1);
  }
if(*(argv[1])=='-') 
  switch(tolower(*(argv[1]+1))) {
    case('i'):
      mmc_setup();
    case('r'):
      temp=fifo_mem_create();
      mmc_deposit(temp);
    case('p'):
      mmc_pay();
    case('x'):
      mmc_xold();
    default:
      printf("Run with no command line for help\n");
      exit(0);
    }

fp=mm_safeopen(argv[1],"rb");
temp=fifo_file_create(fp);
input=fifo_mem_create();
if(fifo_rget(0,temp)<128) { /* Armored - bank message */
  struct fifo *key;
  MD5_CTX md5ctx;
  byte buf[16],buf2[16];
  struct pgp_pubkey *pk=safemalloc(sizeof(struct pgp_pubkey));
  struct pgp_seckey *sk=safemalloc(sizeof(struct pgp_seckey));
  time_t timestamp;
  byte sigtype;

  if(!mm_dearmor(temp,input)) {
    printf("De-armor failed\n");
    fclose(fp);
    exit(1);
    }
  fclose(fp);
  printf("Input is message from the server ");
  fp=mm_safeopen(USER_SECKEY_FILE,"rb");
  temp=fifo_file_create(fp);
  pgpk_extract_key(temp,pk,sk,NULL,NULL);
  fifo_destroy(temp);
  fclose(fp);
  if(!pgp_extract_rsa(input,buf,pk,sk)) goto bail;
  temp=fifo_mem_create();
  if(!pgp_extract_idea(input,temp,buf)) goto bail;
  fifo_destroy(input);
  input=temp;
  fp=mm_safeopen(BANK_ASCKEY_FILE,"rb");
  temp=fifo_file_create(fp);
  key=fifo_mem_create();
  if(!mm_dearmor(temp,key)) {
    printf("\nCan't get bank's key from %s\n",BANK_ASCKEY_FILE);
    exit(1);
    }
  fclose(fp);
  pgpk_extract_key(key,pk,NULL,NULL,NULL);
  fifo_destroy(key);
  if(!pgp_extract_sig(input,buf,&timestamp,&sigtype,pk)) goto bail;
  endian(&timestamp,4);
  MD5Init(&md5ctx);
  pgp_md5(fifo_copy(input),&md5ctx);
  MD5Update(&md5ctx,&sigtype,1);
  MD5Update(&md5ctx,(byte *)&timestamp,4);
  MD5Final(buf2,&md5ctx);
  if(memcmp(buf,buf2,16)) {
    printf("\nBad signature from the server\n");
    exit(1);
    }
  else {
    free(pk);
    free(sk);
    printf("- good signature\n");
    mmc_bankmsg(input);
    }
  }
else { /* Input is coins */
  fifo_append(input,temp);
  fclose(fp);
  printf("Input is coins for deposit\n");
  mmc_deposit(input);
  }
bail:
printf("\nDecrypt failed\n");
return(1);
}

void mmc_deposit(struct fifo *i)
{
FILE *fp;
word32 value,total=0,total2;
unsigned j;
struct fifo *temp,*key,*depcoins,*protocoins,*valuelist;
struct pgp_pubkey *pk=safemalloc(sizeof(struct pgp_pubkey));
struct mm_coin *c=safemalloc(sizeof(struct mm_coin));
struct mm_elist *el=safemalloc(sizeof(struct mm_elist));
struct mpi *e;
struct mm_protocoin *pc;
unit *u=safemalloc(sizeof(unitarr));
byte ideakey[16];
char *name=mm_getname();
MD5_CTX md5ctx;

/* Get bank's key */

mmc_get_bank_key(pk);

/* Get user's key */

fp=mm_safeopen(USER_PUBKEY_FILE,"rb");
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
  fifo_get(i);
  fifo_aget(c->coinid,16,i);
  pgp_extract_mpi(i,&c->e);
  pgp_extract_mpi(i,&c->c);
  value=mm_getvalue(&c->e,ELIST_FILE);
  if(!value)
    value=mm_getvalue(&c->e,ELIST_OLD_FILE);
  if(!value) {
    printf("Coin has unknown exponent\n"); 
    continue;
    }
  printf("Coin of value %lu %s(s)\n",value,name);
  mp_modexp(u,c->c.value,c->e.value,pk->n.value);
  endian(u,sizeof(unitarr));
  if(!mm_valid_coin(u,&pk->n,c->coinid)) {
    printf("Coin has bad signature\n");
    continue;
    }
  total+=value;
  fifo_put(MM_REGCOIN,depcoins);
  fifo_aput(c->coinid,16,depcoins);
  pgp_create_mpi(depcoins,&c->e);
  pgp_create_mpi(depcoins,&c->c);
  }
fifo_destroy(i);
if(total) 
  printf("Total value to deposit %lu %s(s)\n\n",total,name);
total2=total;

/* Show list of available denominations */
if(total) { /* Skip if initialization */
  fp=mm_safeopen(ELIST_FILE,"rb");
  printf("Denominations: ");
  while(fread(el,sizeof(struct mm_elist),1,fp))
    printf("%lu ",el->value);
  fclose(fp);
  printf("\n\n");  
  }

/* Get list of proto-coins for server to sign */

valuelist=fifo_mem_create();
while(total) {
  printf("%lu %s(s) remaining\n\
Enter value of coin to create: ",total,name);
  fflush(stdout);
  scanf("%lu",&value);
  if(value>total) {
    printf("You do not have %lu %s(s) remaining.\n",value,name);
    continue;
    }
  e=mm_gete(value,ELIST_FILE);
  if(!e) {
    printf("That coin denomination does not exist.\n");
    continue;
    }
  else
    free(e);
  fifo_aput(&value,4,valuelist);
  total-=value;
  }

/* Create proto-coins */

fp=mm_safeopen(USER_PCOIN_FILE,"ab");    
protocoins=fifo_mem_create();
while(fifo_aget(&value,4,valuelist)) {
  printf("Creating new coin of value %lu %s(s)\n",value,name);
  pc=mm_newpcoin(value,&pk->n);
  if(!pc) {
    printf("Proto-coin create failed - load up the debugger\n");
    exit(1);
    }
  fwrite(pc,sizeof(struct mm_protocoin),1,fp);
  fifo_put(MM_PROCOIN,protocoins);
  fifo_aput(pc->tempid,8,protocoins);
  pgp_create_mpi(protocoins,&pc->e);
  pgp_create_mpi(protocoins,&pc->bc);
  free(pc);
  }
fifo_destroy(valuelist);
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
fp=mm_safeopen(USER_OUTPUT_FILE,"wa");
depcoins=fifo_file_create(fp);
mm_armor(temp,depcoins,FALSE);
fifo_destroy(depcoins);
fclose(fp);
printf("Send the file %s to the server to %s.\n",
	USER_OUTPUT_FILE,total2?"deposit coins":"initialize client");
free(pk);
free(c);
free(u);
free(name);
free(el);
exit(0);
}

void mmc_bankmsg(struct fifo *i)
{
FILE *fp;
struct pgp_pubkey *pk=safemalloc(sizeof(struct pgp_pubkey));
struct mm_coin *c=safemalloc(sizeof(struct mm_coin));
struct mm_elist *el;
struct mpi *x=safemalloc(sizeof(struct mpi));
struct mm_protocoin *pc;
struct fifo *temp,*key;
byte tempid[8];
byte b;
word32 value;
char *name;

mmc_get_bank_key(pk);

/* Get name of coins */
fp=mm_safeopen(COIN_NAME_FILE,"wb");
while(b=(byte)fifo_get(i))
  fputc(b,fp);
fclose(fp);

name=mm_getname();

/* Display a message, if found */
if(pgp_nexttype(i)==MM_SRVRMSG) {
  struct fifo *msg,*msg2;
  word16 length;
  fifo_get(i);
  msg=fifo_mem_create();
  msg2=fifo_mem_create();
  fifo_aget(&length,2,i);
  endian(&length,2);
  fifo_moven(i,msg,length);
  pgp_textform(msg,msg2,FALSE,TRUE);
  { int y; char z; while((y=fifo_get(msg2))!=(-1)) { z=y; printf("%c",z); } }
  fifo_destroy(msg2);
  getch();
  }

/* Process elist information */
switch(pgp_nexttype(i)) {
  case(MM_NEWELST):
    printf("Updating elist file\n");
    el=safemalloc(sizeof(struct mm_elist));
    fp=mm_safeopen(ELIST_FILE,"wb");
    while(pgp_nexttype(i)==MM_NEWELST) {
      fifo_get(i);
      pgp_extract_mpi(i,&el->e);
      fifo_aget(&el->value,4,i);
      endian(&el->value,4);
      fwrite(el,sizeof(struct mm_elist),1,fp);
      }
    fclose(fp);
    if(pgp_nexttype(i)==MM_OLDELST) {
      fp=mm_safeopen(ELIST_OLD_FILE,"wb");
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
fp=mm_safeopen(USER_COIN_FILE,"ab");
while(pgp_nexttype(i)==MM_BLICOIN) {
  fifo_get(i);
  fifo_aget(tempid,8,i);
  pgp_extract_mpi(i,&c->e);
  pgp_extract_mpi(i,x);
  pc=mmc_getpcoin(tempid);
  if(!pc) {
    printf("Proto-coin not found in file\n");
    exit(1);
    }
  memcpy(c->coinid,pc->coinid,16);
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
  if(!mm_valid_coin(x->value,&pk->n,c->coinid)) {
    printf("Coin from server has bad signature!\n");
    exit(1);
    }
  pgp_fix_mpi(&c->c);
  fwrite(c,sizeof(struct mm_coin),1,fp);
  free(pc);
  value=mm_getvalue(&c->e,ELIST_FILE);
  if(!value)
    value=mm_getvalue(&c->e,ELIST_OLD_FILE);
  printf("Coin of value %lu %s(s)\n",value,name);
  }
fifo_destroy(i);
fclose(fp);
free(pk);
free(c);
free(x);
free(name);
exit(0);
}

/* Look up a protocoin, then remove it from the file */
struct mm_protocoin *mmc_getpcoin(byte *id)
{
FILE *fp;
long floc,flen;
struct mm_protocoin *pc=safemalloc(sizeof(struct mm_protocoin));
struct mm_protocoin *tc=safemalloc(sizeof(struct mm_protocoin));
fp=mm_safeopen(USER_PCOIN_FILE,"r+b");
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

void mmc_pay(void)
{
char *name=mm_getname();
FILE *fp,*fp2;
struct fifo *f;
word32 cvalue,total=0,value=0,value2;
struct coin_list *l=NULL;
struct coin_list *t=NULL;
struct mm_coin *c=safemalloc(sizeof(struct mm_coin));
struct pgp_pubkey *pk=safemalloc(sizeof(struct pgp_pubkey));
long floc,flen;
fp=mm_safeopen(USER_COIN_FILE,"rb");

found: /* Count up coins and values */
while(fread(c,sizeof(struct mm_coin),1,fp)) {
  t=l;
  cvalue=mm_getvalue(&c->e,ELIST_FILE);
  if(!cvalue)
    cvalue=mm_getvalue(&c->e,ELIST_OLD_FILE);
  while(t) {
    if(cvalue==t->value) {
      t->count++;
      goto found;
      }
    t=t->next;
    }
  t=safemalloc(sizeof(struct coin_list));
  t->value=cvalue;
  t->count=1;
  t->next=l;
  l=t;
  }

t=l;
while(t) {
  printf("You have %u coins of value %lu %s(s)\n",t->count,t->value,name);
  total+=(t->value*t->count);
  t=t->next;
  }

fclose(fp);
while(l) {
  t=l->next;
  free(l);
  l=t;
  }

printf("Total balance %lu %s(s)\n\n",total,name);
if(!total) {
  printf("Come back when you have some money.\n");
  goto done;
  }

total=0;

printf("Enter coin values to extract, 0 when done: ");
fp2=mm_safeopen(COIN_OUT_FILE,"wb");
f=fifo_file_create(fp2);
found2:
while(scanf("%lu",&value),value) {
  fp=mm_safeopen(USER_COIN_FILE,"r+b");
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
      total+=value;
      printf("Total of %lu %s(s) transferred: ",total,name);
      fflush(stdout);
      goto found2; /* Spaghetti code */
      }
    }
  printf("Coin of value %lu %s(s) not found in file\n",value,name);
  fclose(fp);
  }
printf("Coins extracted to file %s\n",COIN_OUT_FILE);
mmc_get_bank_key(pk);
fifo_put(MM_BANKID,f); /* Write bank id out */
fifo_aput(pk->keyid,8,f);
fifo_destroy(f);
fclose(fp2);
done:
free(c);
free(name);
free(pk);
exit(0);
}

void mmc_xold(void)
{
FILE *fp;
struct fifo *f;
struct mm_coin *c=safemalloc(sizeof(struct mm_coin));
long floc,flen;
word32 value;

floc=0;
f=fifo_mem_create();
while(fp=mm_safeopen(USER_COIN_FILE,"r+b"),
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
    else
      printf("Expired coin discarded\n");
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
  printf("Exchanging old coins\n");
  mmc_deposit(f);
  }
else {
  printf("No old coins to exchange\n");
  fifo_destroy(f);
  }
exit(0);
}

/* Sets up the Magic Money client */
void mmc_setup(void)
{
unsigned i;
byte *buf=safemalloc(256);
struct pgp_pubkey *pk=safemalloc(sizeof(struct pgp_pubkey));
struct pgp_seckey *sk=safemalloc(sizeof(struct pgp_seckey));
struct fifo *uid,*ff,*temp;
unsigned bits=0;
FILE *fp;

printf("\n\
Client setup will create a PGP key for communication with the server.\n\
Please enter a username for the key.\n");
gets(buf);

while((bits<384)||(bits>1024)) {
  printf("\nEnter number of bits for the key: ");
  fflush(stdout);
  scanf("%u",&bits);
  }

printf("\nGenerating key pair\n");

uid=fifo_mem_create();
fifo_put(PGP_UID,uid);
fifo_put((byte)strlen(buf),uid);
fifo_aput(buf,strlen(buf),uid);

pgpk_keygen(pk,sk,bits,17,time(NULL),0,&gen_output);
printf("\nKey generation completed\n\n");
temp=fifo_mem_create();
pgpk_create_pk(temp,pk);
fifo_append(temp,fifo_copy(uid));

fp=mm_safeopen(USER_PUBKEY_FILE,"wb");
ff=fifo_file_create(fp);
fifo_append(ff,temp);
fifo_destroy(ff);
fclose(fp); /* Write public key */

fp=mm_safeopen(USER_SECKEY_FILE,"wb");
ff=fifo_file_create(fp);
pgpk_create_sk(ff,pk,sk,NULL);
fifo_append(ff,uid);
fifo_destroy(ff);
fclose(fp); /* Write secret key */

free(buf);
free(pk);
free(sk);

ff=fifo_mem_create();
mmc_deposit(ff);
}

void mmc_get_bank_key(struct pgp_pubkey *pk)
{
FILE *fp;
struct fifo *temp,*key;
fp=mm_safeopen(BANK_ASCKEY_FILE,"rb");
temp=fifo_file_create(fp);
key=fifo_mem_create();
if(!mm_dearmor(temp,key)) {
  printf("Can't get bank's key from %s\n",BANK_ASCKEY_FILE);
  exit(1);
  }
fclose(fp);
pgpk_extract_key(key,pk,NULL,NULL,NULL);
fifo_destroy(key);
}