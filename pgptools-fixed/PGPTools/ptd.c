/* PGP Tools Demo */

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
#include "pgpktru.h"

static byte cbuf[256];
static MD5_CTX md5ctx;
static int storekeys=FALSE;
static FILE *pkr=NULL,*skr=NULL;
static FILE *outfp,*infp;
static struct fifo *morekeys=NULL;
/* Keys to search through for key_lookup */

void memchk(void);
void printkeys(struct fifo *f,struct pgp_pubkey *pk,
	       int revoked,struct fifo *listsigs);
void delkey(byte *userid);
void extract(byte *userid,byte *filename,int armor);
void deckey(struct pgp_seckey *sk);
void addkeys(struct fifo *keys);
void key_output(byte keyid[8],byte *userid,
		time_t timestamp,unsigned result);
int key_lookup(byte keyid[8],struct pgp_pubkey *pk,
	       byte *userid,byte *trustbyte);
void gen_output(unsigned status);
void keygen(byte *buf);
byte query_user(byte *keyid,byte *userid);
void trust_update(FILE *keyfile,int keep_ownertrust);
void keymaint(void);
void signkey(struct pgp_pubkey *pkey,struct pgp_seckey *skey,byte *userid);

int main(int argc,char *argv[])
{
unsigned i,armorlines,armorblocks;
byte c;
int ideaonly=FALSE,armor=FALSE,compress=FALSE,text=FALSE;
int revoked,flag,gotsig=FALSE,gotideakey=FALSE,disp=FALSE,showsigs=FALSE;
byte *cryptuid=NULL,*signuid=NULL,*infilename=NULL;
byte *maintuid=NULL;
byte key[16],hash[16],outfilename[256];
byte sigtype;
struct fifo *keyring,*sparekey,*tkey;
struct fifo *inf,*outf,*temp1,*temp2;
struct pgp_pubkey enkey,spkey;
struct pgp_seckey skey;
time_t timestamp,dummyts;
char begin_armor[]="-----BEGIN PGP MESSAGE-----\nVersion: 2.3a\n\n";
char end_armor[]="-----END PGP MESSAGE-----\n";
char begin_fmarmor[]=\
"-----BEGIN PGP MESSAGE, PART 01/%02d-----\nVersion: 2.3a\n\n";
char begin_marmor[]=\
"-----BEGIN PGP MESSAGE, PART %02d/%02d-----\n\n";
char end_marmor[]=\
"-----END PGP MESSAGE, PART %02d/%02d-----\n";

memchk();
#ifdef UNIX
on_exit (memchk);
#else
atexit(&memchk);
#endif

set_precision(MAX_UNIT_PRECISION);

printf("\nPGP Tools Demo\n");
if(argc==1) {
  printf("\nOptions:\n\n\
-e<key> encrypt\n\
-s<key> sign			  -a<lines> ascii armor output\n\
-q use compression                -t input is text\n\
-v view keyring	                  -vs view with sigs\n\
-x examine packets\n\
-kg generate key pair             -kc key maintenance pass\n\
-kd<key> delete key		  -kx<key> extract public key\n\
-ks<key> sign key\n\
\n\
Command format: ptdemo -<opt1> -<opt2> ... -<optn> filename\n\
Any key management command must be the last thing on the line:\n\
ptdemo -a key.asc -kx<key>\n\
ptdemo -s<key to sign with> -ks<key to sign>\n");
  exit(1);
  }

pkr=fopen("pubring.pgp","r+b");
if(!pkr) {
  printf("Unable to open pubring.pgp\n");
  exit(1);
  }

skr=fopen("secring.pgp","rb");
if(!skr) {
  printf("Unable to open secring.pgp\n");
  exit(1);
  }

for(i=1;i<argc;i++) {
  if(*argv[i]!='-') {
    infilename=argv[i];
    continue;
    }
  c=tolower(*(argv[i]+1));
  switch(c) {
    case('e'):
      cryptuid=argv[i]+2;
      keyring=fifo_file_create(pkr);
      temp1=pgpk_findkey(keyring,cryptuid,TRUE);
      fifo_destroy(keyring);
      if(!temp1) {
	printf("Public key for %s not found\n",cryptuid);
	fclose(pkr);
	fclose(skr);
	exit(1);
	}
      printf("Encrypting with:\n");
      pgpk_extract_key(temp1,&enkey,NULL,NULL,&revoked);
      printkeys(temp1,&enkey,revoked,NULL);
      break;
    case('s'):
      signuid=argv[i]+2;
      keyring=fifo_file_create(skr);
      temp1=pgpk_findkey(keyring,signuid,TRUE);
      fifo_destroy(keyring);
      if(!temp1) {
	printf("Secret key for %s not found\n",cryptuid);
	fclose(pkr);
	fclose(skr);
	exit(1);
	}
      printf("Signing with:\n");
      flag=pgpk_extract_key(temp1,&spkey,&skey,NULL,NULL);
      printkeys(temp1,&spkey,2,NULL);
      if(flag)
	deckey(&skey);
      break;
    case('c'):
      ideaonly=TRUE;
      break;
    case('a'):
      armor=TRUE;
      armorlines=0; /* Default if scanf doesn't find anything */
      armorblocks=1;
      sscanf(argv[i]+2,"%u",&armorlines);
      break;
    case('q'):
      compress=TRUE;
      break;
    case('t'):
      text=TRUE;
      break;
    case('x'):
      disp=TRUE;
      break;
    case('v'):
      showsigs=(*(argv[i]+2)=='s');
      if(showsigs)
	pgp_hash_clear();
      keyring=fifo_file_create(pkr);
      sparekey=fifo_copy(keyring);
      printf("\nPublic keys\n\n");
      while(tkey=pgpk_getnextkey(keyring)) {
	pgpk_extract_key(tkey,&enkey,NULL,NULL,&revoked);
	printkeys(tkey,&enkey,revoked,
		  showsigs?fifo_copy(sparekey):NULL);
	}
      fifo_destroy(keyring);
      fifo_destroy(sparekey);
      keyring=fifo_file_create(skr);
      printf("\nSecret keys (* if encrypted)\n\n");
      while(tkey=pgpk_getnextkey(keyring)) {
	if(pgpk_extract_key(tkey,&enkey,&skey,NULL,NULL))
	  printf("*");
	printkeys(tkey,&enkey,2,NULL);
	}
      fifo_destroy(keyring);
      pgp_hash_off();
      fclose(pkr);
      fclose(skr);
      exit(0);
    case('k'):
      maintuid=argv[i]+3;
      c=tolower(*(argv[i]+2));
      switch(c) {
	case('d'):
	  delkey(maintuid);
	case('x'):
	  extract(maintuid,infilename,armor);
	case('g'):
	  keygen(cbuf);
	case('c'):
	  keymaint();
	case('s'):
	  if(!signuid) {
	    printf("You must specify a key to sign with.\n");
	    exit(0);
	    }
	  signkey(&spkey,&skey,maintuid);
	}
      exit(0); /* Did I forget to put in an exit anywhere? */
    }
  }

infp=fopen(infilename,"rb");
if(!infp) {
  printf("Unable to open input file %s\n",infilename);
  exit(1);
  }
inf=fifo_file_create(infp);

if(cryptuid||signuid) {
  sigtype=text?1:0;
  if(text) {
    struct fifo *temp3=fifo_mem_create();
    printf("Converting text format\n");
    pgp_textform(inf,temp3,TRUE,TRUE);
    inf=temp3;
    }
  pgp_initrand(fifo_copy(inf));
  }
if(cryptuid&&!signuid) { /* Encrypt only */
  temp1=fifo_mem_create();
  temp2=fifo_mem_create();
  for(i=0;i<16;i++)
    key[i]=pgp_randombyte();
  printf("Creating RSA packet\n");
  pgp_create_rsa(temp2,key,&enkey);
  printf("Creating literal packet, %s mode\n",text?"text":"binary");
  pgp_create_literal(inf,temp1,text,infilename,time(NULL));
  if(compress) {
    struct fifo *temp3=fifo_mem_create();
    printf("Compressing\n");
    pgp_create_zip(temp1,temp3);
    temp1=temp3;
    }
  printf("IDEA encrypting\n");
  pgp_create_idea(temp1,temp2,key);
  }
else if(signuid&&!cryptuid) { /* Sign only */
  temp1=fifo_copy(inf);
  MD5Init(&md5ctx);
  printf("Taking MD5 of input for signature\n");
  pgp_md5(temp1,&md5ctx);
  time(&timestamp);
  MD5Update(&md5ctx,&sigtype,1);
  endian(&timestamp,4);
  MD5Update(&md5ctx,(byte *)&timestamp,4);
  endian(&timestamp,4);
  MD5Final(key,&md5ctx);
  temp2=fifo_mem_create();
  printf("Creating signature packet\n");
  pgp_create_sig(temp2,key,timestamp,sigtype,&spkey,&skey);
  printf("Creating literal packet, %s mode\n",text?"text":"binary");
  pgp_create_literal(inf,temp2,text,infilename,timestamp);
  if(compress) {
    temp1=fifo_mem_create();
    printf("Compressing\n");
    pgp_create_zip(temp2,temp1);
    temp2=temp1;
    }
  }	
else if(signuid&&cryptuid) {
  temp1=fifo_copy(inf);
  MD5Init(&md5ctx);
  printf("Taking MD5 of input for signature\n");
  pgp_md5(temp1,&md5ctx);
  time(&timestamp);
  MD5Update(&md5ctx,&sigtype,1);
  endian(&timestamp,4);
  MD5Update(&md5ctx,(byte *)&timestamp,4);
  endian(&timestamp,4);
  MD5Final(key,&md5ctx);
  temp1=fifo_mem_create();
  temp2=fifo_mem_create();
  printf("Creating signature packet\n");
  pgp_create_sig(temp1,key,timestamp,sigtype,&spkey,&skey);
  printf("Creating literal packet, %s mode\n",text?"text":"binary");
  pgp_create_literal(inf,temp1,text,infilename,timestamp);
  if(compress) {
    printf("Compressing\n");
    pgp_create_zip(temp1,temp2);
    temp1=temp2;
    temp2=fifo_mem_create();
    }
  for(i=0;i<16;i++)             
    key[i]=pgp_randombyte();
  printf("Creating RSA packet\n");
  pgp_create_rsa(temp2,key,&enkey);
  printf("IDEA encrypting\n");
  pgp_create_idea(temp1,temp2,key);
  }

if(cryptuid||signuid) {
  strcpy(outfilename,infilename);
  if(strchr(outfilename,'.'))
    strcpy(strchr(outfilename,'.'),".pgp");
  else
    strcat(outfilename,".pgp");
  printf("Output file %s\n",outfilename);
  if(armor)
    outfp=fopen(outfilename,"w+a");
  else
    outfp=fopen(outfilename,"w+b");
  if(!outfp) {
    printf("Unable to open output file %s\n",outfilename);
    exit(1);
    }
  outf=fifo_file_create(outfp);
  if(armor) {
    printf("Creating ascii armor");
    if(armorlines) {
      armorblocks=(fifo_length(temp2)+((48*armorlines)-1))/(48*armorlines);
      printf(" with %u lines per section and %u sections",
	     armorlines,armorblocks);
      }
    printf("\n");
    if(armorblocks>1) 
      for(i=1;i<=armorblocks;i++) {
	if(i==1)
	  sprintf(cbuf,begin_fmarmor,armorblocks);
	else
	  sprintf(cbuf,begin_marmor,i,armorblocks);
	fifo_aput(cbuf,strlen(cbuf),outf);
	pgp_create_armor(temp2,outf,armorlines);
	sprintf(cbuf,end_marmor,i,armorblocks);
	fifo_aput(cbuf,strlen(cbuf),outf);
	fifo_put('\n',outf);
	}
    else {
      fifo_aput(begin_armor,strlen(begin_armor),outf);
      pgp_create_armor(temp2,outf,0);
      fifo_aput(end_armor,strlen(end_armor),outf);
      }
    fifo_destroy(temp2);
    }
  else
    fifo_append(outf,temp2);
  }
else { /* Here we are decrypting */
  if(fifo_rget(0,inf)<128) {
    if(!fifo_find("-----BEGIN PGP ",inf)) {
      printf("This is not a PGP message\n");
      exit(1);
      }
    temp1=fifo_mem_create();
    i=0;
    while((cbuf[i]=fifo_get(inf))!='\n') i++;
    cbuf[i]='\0';
    if(sscanf(cbuf,"MESSAGE, PART %02d/%02d",&i,&armorblocks)!=2)
      armorblocks=1;
    printf("Extracting ASCII armor, %u parts\n",armorblocks);
    for(i=1;i<=armorblocks;i++) {
      if(i==1) {
	fifo_find("Version",inf);
	fifo_find("\n",inf);
	}
      while(fifo_rget(0,inf)<33)
	if(fifo_get(inf)==-1) {
	  printf("File ended rudely after part %u\n",i-1);
	  exit(1);
	  }
      if(!pgp_extract_armor(inf,temp1)) {
	printf("ASCII-armored packet %u is corrupted\n",i);
	exit(1);
	}
      if(i<=armorblocks) {
	fifo_find("-----BEGIN PGP ",inf);
	fifo_find("\n",inf);
	}
      }
    fifo_destroy(inf);
    }
  else
    temp1=inf;
  {
    unsigned offset,type,error;
    error=pgp_check_packets(temp1,&offset,&type);
    if(error) {
      printf("Packet error %u in packet type %02X offset %u\n",
	      error,type,offset);
      exit(1); /* The check isn't being used everywhere yet */
    }
  }

if(disp) {
  printf("Listing packets in file %s\n",infilename);
  for(;;) {  
    switch(pgp_nexttype(temp1)) {
      case PGP_PKE:
	printf("RSA encrypted ");
	pgp_get_keyid(temp1,key);
	pgpk_skip_packet(temp1);
	for(i=0;i<8;i++)
	  printf("%02X",key[i]);
	printf("\n");
	break;
      case PGP_SIG:
	printf("Signature ");
	pgp_get_keyid(temp1,key);
	for(i=0;i<8;i++)
	  printf("%02X",key[i]);
	printf(" type %02X",(unsigned int)fifo_rget(5,temp1)); 
	pgpk_skip_packet(temp1);
	printf("\n");
	break;
      case PGP_SK:
	printf("Secret key ");
	goto pub_or_sec_key;
      case PGP_PK:
	printf("Public key ");
	pub_or_sec_key:
	pgp_get_keyid(temp1,key);
	for(i=0;i<8;i++)
	  printf("%02X",key[i]);
	pgpk_skip_packet(temp1);
	printf("\n");
	break;
      case PGP_CMP:
	printf("Compressed data\n");
	exit(1);
      case PGP_CKE:
	printf("IDEA encrypted packet\n");
	pgpk_skip_packet(temp1);
	break;
      case PGP_PT:
	printf("Plaintext\n");
	pgpk_skip_packet(temp1);
	break;
      case PGP_TRU:
	fifo_skipn(temp1,2);
	sigtype=fifo_get(temp1);
	printf("Trust packet %02X\n",sigtype);
	break;
      case PGP_UID:
	printf("Username ");
	fifo_get(temp1);
	i=fifo_get(temp1);
	while(i--)
	  printf("%c",(byte) fifo_get(temp1));
	  printf("\n");
	break;
      case PGP_COM:
	printf("Comment packet - what is this doing here?\n");
	pgpk_skip_packet(temp1);
	break;
      default:
	exit(0);
      }
    }
  }

  if(pgp_nexttype(temp1)==PGP_PK)
    addkeys(temp1);
  gotideakey=FALSE;
  gotsig=FALSE;

  decryptloop:
  switch(fifo_rget(0,temp1)&0xFC) {
    case PGP_CKE:
      printf("Decrypting IDEA packet\n");
      temp2=fifo_mem_create();
      if(!gotideakey) {
	printf("File is conventionally encrypted.\nPassphrase: ");
	gets(cbuf);  
	MD5Init(&md5ctx);
	MD5Update(&md5ctx,cbuf,strlen(cbuf));
	MD5Final(key,&md5ctx);
	}
      if(!pgp_extract_idea(temp1,temp2,key)) {
	printf("Wrong IDEA key\n");
	exit(1);
	}
      fifo_destroy(temp1);
      temp1=temp2;
      goto decryptloop;
    case PGP_PKE:
      pgp_get_keyid(temp1,key);
      keyring=fifo_file_create(skr);
      printf("Decrypting with ");
      for(i=0;i<8;i++)
	printf("%02X",key[i]); printf("\n");
      if(!(tkey=pgpk_findkey(keyring,key,FALSE))) {
	printf("Secret key not found\n");
	exit(1);
	}
      flag=pgpk_extract_key(tkey,&spkey,&skey,NULL,NULL);
      printkeys(tkey,&spkey,2,NULL);
      if(flag)
	deckey(&skey);
      fifo_destroy(keyring);
      if(!pgp_extract_rsa(temp1,key,&spkey,&skey)) {
	printf("RSA-encrypted packet is bad\n");
	exit(1);
	}
      gotideakey=TRUE;
      goto decryptloop;
    case PGP_SIG:
      pgp_get_keyid(temp1,key);
      keyring=fifo_file_create(pkr);
      printf("Signature from ");
      for(i=0;i<8;i++)
	printf("%02X",key[i]); printf("\n");
      if(!(tkey=pgpk_findkey(keyring,key,FALSE))) {
	printf("Public key not found\n");
	exit(1);
	}
      pgpk_extract_key(tkey,&spkey,NULL,NULL,&revoked);
      printkeys(tkey,&spkey,revoked,NULL);
      fifo_destroy(keyring);
      if(!pgp_extract_sig(temp1,hash,&timestamp,&sigtype,&spkey)) {
	printf("Signature packet is bad\n");
	exit(1);
	}
      gotsig=TRUE;
      goto decryptloop;
    case PGP_CMP:
      printf("Decompressing\n");
      temp2=fifo_mem_create();
      pgp_extract_zip(temp1,temp2);
      temp1=temp2;
      goto decryptloop;
    case PGP_PT:
      printf("Extracting plaintext");
      temp2=fifo_mem_create();
      pgp_extract_literal(temp1,temp2,&text,outfilename,&dummyts);
      fifo_destroy(temp1);
      printf(" - original filename %s\n",outfilename);
      if(gotsig) {
	byte hash2[16];
	MD5Init(&md5ctx);
	pgp_md5(fifo_copy(temp2),&md5ctx);
	MD5Update(&md5ctx,&sigtype,1);
	endian(&timestamp,4);
	MD5Update(&md5ctx,(byte *)&timestamp,4);
	MD5Final(hash2,&md5ctx);
	if(!memcmp(hash,hash2,16))
	  printf("Signature is good\n");
	else
	  printf("Signature is bad\n");
	}
	printf("Output file %s\n",outfilename);
	if(text)
	  outfp=fopen(outfilename,"w+a");
	else
	  outfp=fopen(outfilename,"w+b");
	if(!outfp) {
	  printf("Unable to open output file\n");
	  exit(1);
	  }
	outf=fifo_file_create(outfp);
      if(text) {
	printf("Converting from canonical to local text type\n");
	pgp_textform(temp2,outf,FALSE,TRUE);
	}
      else
	fifo_append(outf,temp2);
      goto donedecrypt;
    default:
      printf("Unknown packet type\n");
      exit(1);
    }
  }
donedecrypt:
fifo_destroy(outf);
fclose(pkr);
fclose(skr);
fclose(infp);
fclose(outfp);
return(0);
}

void addkeys(struct fifo *keys)
{
struct fifo *okr,*nkr,*temp,*ckey;
FILE *newpkr;
storekeys=TRUE;
pgp_hash_clear();
printf("File contains public keys. Adding to keyring.\n");
newpkr=fopen("pubring.new","w+b");
if(!newpkr) {
  printf("Unable to create new keyring\n");
  exit(1);
  }
okr=fifo_file_create(pkr);

morekeys=fifo_copy(keys);
temp=fifo_mem_create();
while(ckey=pgpk_getnextkey(keys))
  pgpk_check_sigs(ckey,temp,&key_lookup,&key_output);
fifo_destroy(keys);
fifo_destroy(morekeys);
keys=temp;

nkr=fifo_file_create(newpkr);
pgpk_keyring_add(okr,nkr,keys);
fifo_destroy(nkr);
trust_update(newpkr,TRUE);
fclose(pkr);
fclose(skr);
fclose(infp);
fclose(newpkr);
unlink("pubring.pgp");
rename("pubring.new","pubring.pgp");
pgp_hash_off();
printf("Done\n");
exit(0);
}

void delkey(byte *userid)
{
FILE *newrp;
struct fifo *oldring,*newring,*key;
struct pgp_pubkey pk;
int revoked;
char c;
printf("Deleting key %s\n",userid);
oldring=fifo_file_create(pkr);
key=pgpk_findkey(oldring,userid,TRUE);
fifo_destroy(oldring);
if(!key) {
  printf("Key %s not found\n",userid);
  exit(0);
  }
pgpk_extract_key(key,&pk,NULL,NULL,&revoked);
printkeys(key,&pk,revoked,NULL);
printf("\nDo you want to delete this key? ");
do c=tolower(getch()); while((c!='y')&&(c!='n'));
printf("%c\n",c);
if(c=='n') {
  printf("Delete cancelled\n");
  fclose(pkr);
  fclose(skr);
  exit(0);
  }
newrp=fopen("pubring.new","w+b");
if(!newrp) {
  printf("Unable to update public key ring\n");
  exit(1);
  }
oldring=fifo_file_create(pkr);
newring=fifo_file_create(newrp);
if(pgpk_change_keyring(oldring,newring,NULL,pk.keyid)) {
  printf("Key deleted from public keyring\n");
  fifo_destroy(newring);
  fclose(pkr);
  trust_update(newrp,TRUE);
  fclose(newrp);
  unlink("pubring.bak");
  rename("pubring.pgp","pubring.bak");
  rename("pubring.new","pubring.pgp");
  }
else {
  fclose(pkr);
  fclose(newrp);
  unlink("pubring.new");
  }
newrp=fopen("secring.new","w+b");
if(!newrp) {
  printf("Unable to update secret key ring\n");
  exit(1);
  }
oldring=fifo_file_create(skr);
newring=fifo_file_create(newrp);
if(pgpk_change_keyring(oldring,newring,NULL,pk.keyid)) {
  printf("Key deleted from secret keyring\n");
  fifo_destroy(newring);
  fclose(skr);
  fclose(newrp);
  unlink("secring.bak");
  rename("secring.pgp","secring.bak");
  rename("secring.new","secring.pgp");
  }
else {
  fifo_destroy(newring);
  fclose(skr);
  fclose(newrp);
  unlink("secring.new");
  }
exit(0);
}

void extract(byte *userid,byte *filename,int armor)
{
unsigned i;
FILE *outfile;
struct fifo *keyring,*key,*temp;
struct pgp_pubkey tpk;
int revoked;
byte keyid[8];
byte beginkey[]=\
"-----BEGIN PGP PUBLIC KEY BLOCK-----\nVersion: 2.3a\n\n";
byte endkey[]=\
"-----END PGP PUBLIC KEY BLOCK-----\n";
				       
printf("Extracting key %s to file %s",userid,filename);
if(armor)
  printf(" with ascii armor");
printf("\n");
keyring=fifo_file_create(pkr);
key=pgpk_findkey(keyring,userid,TRUE);
fifo_destroy(keyring);
fclose(pkr);
fclose(skr);
if(!key) {
  printf("Public key not found\n");
  exit(1);
  }
pgp_get_keyid(key,keyid);
temp=fifo_copy(key);
pgpk_extract_key(temp,&tpk,NULL,NULL,&revoked);
printkeys(temp,&tpk,revoked,NULL);
temp=fifo_mem_create();
pgpk_trustpackets(key,temp,FALSE);
key=temp;
if(armor) {
  temp=fifo_mem_create();
  fifo_aput(beginkey,strlen(beginkey),temp);
  pgp_create_armor(key,temp,0);
  fifo_aput(endkey,strlen(endkey),temp);
  fifo_destroy(key);
  key=temp;
  outfile=fopen(filename,"aa");
  }
else
  outfile=fopen(filename,"ab");
if(!outfile) {
  printf("Unable to open %s for write\n",filename);
  exit(1);
  }
temp=fifo_file_create(outfile);
fifo_append(temp,key);
fifo_destroy(temp);
fclose(outfile);
exit(0);
}

/* Prints and destroys fifo */
void printkeys(struct fifo *f,struct pgp_pubkey *pk,
	       int revoked,struct fifo *listsigs)
{
byte userid[256],keyid[8];
byte trust;
unsigned i;
char *timestr=ctime(&pk->timestamp);

#ifndef UNIX
timezone=0; /* Times are in GMT */
daylight=0;
#endif
printf("<");
for(i=0;i<8;i++)
  printf("%02X",pk->keyid[i]);
printf("> %4u bits",pk->n.bits);
printf(", %s",timestr);
while(pgpk_extract_username(f,userid,&trust)) {
  if(revoked==2)
    printf("(complete)  ");
  else if(revoked) 
    printf("(revoked)   ");
  else
    switch(trust&3) {
      case 1:
	printf("(untrusted) ");
	break;
      case 0:
	printf("(unknown)   ");
	break;
      case 2:
	printf("(marginal)  ");
	break;
      case 3:
	printf("(complete)  ");
      }
  printf("%s\n",userid);
  if(listsigs) {
    struct pgp_pubkey xpk;
    byte trustbyte;
    while(pgpk_extract_sig(f,keyid,NULL,NULL)) {
      printf("Sig: <");
      for(i=0;i<8;i++)
	printf("%02X",keyid[i]);
      printf(">");
      storekeys=FALSE;
      if(key_lookup(keyid,&xpk,userid,&trustbyte))
	printf(" %s",userid);
      printf("\n");
      }
    }
  }
fifo_destroy(f);
if(listsigs)
  fifo_destroy(listsigs);
}

void deckey(struct pgp_seckey *sk)
{
char pass[256];
byte key[16];
MD5_CTX md5ctx;

printf("Secret key is encrypted.\nPassphrase: ");
gets(pass);
MD5Init(&md5ctx);
MD5Update(&md5ctx,pass,strlen(pass));
MD5Final(key,&md5ctx);
if(!pgp_decrypt_sk(sk,key))
  {
  printf("Bad passphrase\n");
  exit(1);
  }
}

void memchk(void)
{
byte *c;
c=safemalloc(1024);
printf("Memory %p\n",c);
free(c);
}

void key_output(byte keyid[8],byte *userid,
		time_t timestamp,unsigned result)
{
unsigned i;
if(result==5)
  printf("Key <");
else if(result>2)
  printf("Revocation <");
else
  printf("Sig <");
for(i=0;i<8;i++)
  printf("%02X",keyid[i]);
printf("> ");
if(userid)
  printf("%s ",userid);
/* printf("%s\n",ctime(&timestamp)); */
switch(result) {
  case(0):
    printf("Bad sig\n");
    break;
  case(1):
    printf("Unable to check - public key not found\n");
    break;
  case(2):
    printf("Good sig\n");
    break;
  case(3):
    printf("Bad revocation\n");
    break;
  case(4):
    printf("Good revocation\n");
    break;
  case(5):
    printf("\n");
    break;
  }
}

int key_lookup(byte keyid[8],struct pgp_pubkey *pk,
	       byte *userid,byte *trustbyte)
{
struct fifo *keyring,*key;
key=NULL;
if(pgp_hash_get(keyid,userid,pk,trustbyte)) 
  return(TRUE);
if(morekeys) {
  keyring=fifo_copy(morekeys);
  key=pgpk_findkey(keyring,keyid,FALSE);
  fifo_destroy(keyring);
  }
if(!key) {
  keyring=fifo_file_create(pkr);
  key=pgpk_findkey(keyring,keyid,FALSE);
  fifo_destroy(keyring);
  }
if(!key)
  return(FALSE);
pgpk_extract_key(key,pk,NULL,trustbyte,NULL);
pgpk_extract_username(key,userid,NULL);
fifo_destroy(key);
pgp_hash_put(keyid,userid,storekeys?pk:NULL,*trustbyte);
return(TRUE);
}

void gen_output(unsigned status)
{
byte outlist[]="pPqQf";
printf("%c",outlist[status]);
fflush(stdout); /* Print it now! */
}

void keygen(byte *buf)
{
FILE *newrp,*newrs;
struct pgp_pubkey pk;
struct pgp_seckey sk;
struct fifo *temp,*oldpr,*newpr,*oldsr,*newsr;
int encrypt=FALSE;
unsigned bits,i;
byte ideakey[16];
MD5_CTX md5ctx;
printf("Key Generation\nEnter some random characters to randomize\n");
gets(buf);

temp=fifo_mem_create();
fifo_aput(buf,256,temp);
pgp_initrand(temp);

printf("384 bits .... fast experimental key\n\
512 bits .... standard length key\n1024 bits ... high security key\n\n\
How many bits? ");
scanf("%u",&bits);
if(bits<384)
  bits=384;
if(bits>1024)
  bits=1024;
printf("Enter for none\nPassphrase: ");
gets(buf);
gets(buf);
if(*buf) {
  MD5Init(&md5ctx);
  MD5Update(&md5ctx,buf,strlen(buf));
  MD5Final(ideakey,&md5ctx);
  encrypt=TRUE;
  printf("Encrypting with passphrase %s\n",buf);
  }
printf("\nGenerating a %u bit key\n",bits);
pgpk_keygen(&pk,&sk,bits,17,time(NULL),0,&gen_output);
printf("\nKeyid <");
for(i=0;i<8;i++)
  printf("%02X",pk.keyid[i]);
printf(">\n");
printf("\nEnter username: ");
gets(buf);
printf("Username %s\n",buf);
temp=fifo_mem_create();
fifo_put(PGP_UID,temp);
fifo_put((byte)strlen(buf),temp);
fifo_aput(buf,strlen(buf),temp);

newrp=fopen("pubring.new","w+b");
if(!newrp) {
  printf("Unable to update public key ring\n");
  exit(1);
  }
newrs=fopen("secring.new","w+b");
if(!newrs) {
  printf("Unable to update secret key ring\n");
  exit(1);
  }
oldpr=fifo_file_create(pkr);
newpr=fifo_file_create(newrp);
oldsr=fifo_file_create(skr);
newsr=fifo_file_create(newrs);
pgpk_create_pk(newpr,&pk);
fifo_aput("\xB0\x1\x87",3,newpr);
fifo_append(newpr,fifo_copy(temp));
fifo_aput("\xB0\x1\x03",3,newpr);
fifo_append(newpr,oldpr);
pgpk_create_sk(newsr,&pk,&sk,encrypt?ideakey:NULL);
fifo_append(newsr,temp);
fifo_append(newsr,oldsr);
fifo_destroy(newpr);
fifo_destroy(newsr);
fclose(pkr);
fclose(skr);
fclose(newrp);
fclose(newrs);
unlink("pubring.bak");
rename("pubring.pgp","pubring.bak");
rename("pubring.new","pubring.pgp");
unlink("secring.bak");
rename("secring.pgp","secring.bak");
rename("secring.new","secring.pgp");
printf("Key added to keyrings\n");
exit(0);
}

void trust_update(FILE *keyfile,int keep_ownertrust)
{
struct fifo *buckstop;
struct fifo *keyring;
struct fifo *sk;
unsigned i;
byte keyid[8];
printf("Keyring trust update - %s old ownertrust values\n",
       keep_ownertrust?"keeping":"clearing");
i=0;
keyring=fifo_file_create(skr);
buckstop=fifo_mem_create();
while(sk=pgpk_getnextkey(keyring)) {
  pgp_get_keyid(sk,keyid);
  fifo_destroy(sk);
  fifo_aput(keyid,8,buckstop);  
  i++;
  }
fifo_destroy(keyring);
printf("%u buckstop keys found in secret keyring\n",i);
keyring=fifo_file_create(keyfile);
fifo_destroy(pgpt_update_trust(keyring,buckstop,&query_user,
	     2,1,8,keep_ownertrust));
printf("Keyring trust update completed\n");
}

byte query_user(byte *keyid,byte *userid)
{
unsigned t,i;
printf("\nKeyid <");
for(i=0;i<8;i++)
  printf("%02X",keyid[i]);
printf("> %s\n",userid);
printf("Do you trust this person to certify keys? Enter:\n\
1 ..... don't know\n\
2 ..... do not trust person\n\
5 ..... marginally trust person\n\
6 ..... always trust person\n\nEnter trust value: ");
scanf("%u",&t);
return( (byte) t);
}

void keymaint(void)
{
FILE *newrp;
struct fifo *keyring,*key,*newring;
storekeys=TRUE;
pgp_hash_clear();
newrp=fopen("pubring.new","w+b");
if(!newrp) {
  printf("Unable to update public key ring\n");
  exit(1);
  }
keyring=fifo_file_create(pkr);
newring=fifo_file_create(newrp);
morekeys=NULL; /* for key_lookup */
while(key=pgpk_getnextkey(keyring)) {
  fifo_clearlac(keyring); /* key_lookup accesses same file */
  pgpk_check_sigs(key,newring,&key_lookup,&key_output);
  }
fifo_destroy(keyring);
fifo_destroy(newring);

trust_update(newrp,FALSE);
fclose(pkr);
fclose(skr);
fclose(newrp);

unlink("pubring.bak");
rename("pubring.pgp","pubring.bak");
rename("pubring.new","pubring.pgp");
pgp_hash_off();
exit(0);
}

void signkey(struct pgp_pubkey *pkey,
	     struct pgp_seckey *skey,byte *userid)
{
FILE *newrp;
struct fifo *oldring,*newring,*key,*oldkey,*newkey,*pk,*uid;
byte hash[16];
struct pgp_pubkey tpk;
int revoked;
time_t timestamp;

printf("\nSigning key %s\n\n",userid);
oldring=fifo_file_create(pkr);
key=pgpk_findkey(oldring,userid,TRUE);
fifo_destroy(oldring);
if(!key) {
  printf("Key %s not found\n",userid);
  exit(0);
  }
oldkey=fifo_copy(key);
pgpk_extract_key(key,&tpk,NULL,NULL,&revoked);
printkeys(key,&tpk,revoked,NULL);
newkey=fifo_mem_create();
pk=fifo_mem_create();
pgpk_copy_packet(oldkey,pk);
fifo_append(newkey,fifo_copy(pk));
while(pgp_nexttype(oldkey)!=PGP_UID)
  pgpk_copy_packet(oldkey,newkey);
uid=fifo_mem_create();
pgpk_copy_packet(oldkey,uid);
/* Isn't dynamic allocation wonderful? */
fifo_append(newkey,fifo_copy(uid));
pgpk_copy_packet(oldkey,newkey); /* Trust packet */
MD5Init(&md5ctx);
pgp_md5(pk,&md5ctx);
fifo_skipn(uid,2);
pgp_md5(uid,&md5ctx);
MD5Update(&md5ctx,"\x10",1);
time(&timestamp);
endian(&timestamp,4);
MD5Update(&md5ctx,(byte *)&timestamp,4);
endian(&timestamp,4);
MD5Final(hash,&md5ctx);
pgp_create_sig(newkey,hash,timestamp,'\x10',pkey,skey);
fifo_aput("\xB0\x1\x0",3,newkey);
fifo_append(newkey,oldkey);
newrp=fopen("pubring.new","w+b");
if(!newrp) {
  printf("Unable to update public key ring\n");
  exit(1);
  }
oldring=fifo_file_create(pkr);
newring=fifo_file_create(newrp);
if(pgpk_change_keyring(oldring,newring,newkey,tpk.keyid)) {
  printf("Signature added to keyring\n\n");
  fifo_destroy(newring);
  fclose(pkr);
  trust_update(newrp,TRUE);
  fclose(newrp);
  fclose(skr);
  unlink("pubring.bak");
  rename("pubring.pgp","pubring.bak");
  rename("pubring.new","pubring.pgp");
  }
else {
  printf("Didn't find key in ring (bug!)\n");
  fclose(pkr);
  fclose(newrp);
  fclose(skr);
  unlink("pubring.new");
  }
exit(0);
}
