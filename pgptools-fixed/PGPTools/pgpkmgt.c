/* pgpkmgt.c */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include "usuals.h"
#include "mpilib.h"
#include "md5.h"
#include "pgpmem.h"
#include "fifo.h"
#include "pgptools.h"
#include "pgpkmgt.h"

/* A blank trust packet */
static byte blanktrust[]={ '\xB0','\x1','\x0' };

/* If key is present (same keyid) in list, return key as found in list,
   and remove it from list. If not, return null and leave list alone.
   Key is not altered. List is a double pointer so a new one can be
   returned to the caller */
static struct fifo *pgpk_key_compare(struct fifo *key,struct fifo **list);

/* Merges the uids and signatures from two keys, destroys input keys */
static struct fifo *pgpk_merge_uids(struct fifo *key1,struct fifo *key2);

/* Merge sigs from l1 and l2, copying into new with no duplicates.
   Uses up sigs in l1 and l2 until reaching the first non-sig. */
static void pgpk_merge_sigs(struct fifo *l1,struct fifo *l2,
			    struct fifo *new);

/* Compares two uids without changing them, returning TRUE if they
   are the same and FALSE if not */
static int pgpk_compare_uids(struct fifo *uid1,struct fifo *uid2);

/* Copy one packet from i to o and return its type. */
byte pgpk_copy_packet(struct fifo *i,struct fifo *o)
{
byte type;
word32 length;
byte *cp=(byte *) &length;

length=0;
if(!pgp_nexttype(i))
  return(0);
fifo_put(type=fifo_get(i),o);
switch(type&3) {
  case 2:
    fifo_put(cp[0]=fifo_get(i),o);
    fifo_put(cp[1]=fifo_get(i),o);
  case 1:
    fifo_put(cp[2]=fifo_get(i),o);
  case 0:
    fifo_put(cp[3]=fifo_get(i),o);
  }
endian(&length,4);
fifo_moven(i,o,length);
return(type&0xFC);
}

void pgpk_skip_packet(struct fifo *f)
{
word32 length;
byte type;

pgp_examine_packet(f,&type,&length);
fifo_skipn(f,length);
}

/* Get next key from a key ring, returns a memory FIFO containing the key
   if TRUE, or NULL if at the end of the keyring */
struct fifo *pgpk_getnextkey(struct fifo *i)
{
byte type;
struct fifo *o;

if(!pgp_find_packet(i,PGP_PK,PGP_SK,0))
  return(NULL);
o=fifo_mem_create();
do {
  pgpk_copy_packet(i,o);
  type=pgp_nexttype(i);
  } while(type&&(type!=PGP_PK)&&(type!=PGP_SK));
return(o);
}

/* Search a keyring for a particular key. If userid is true, the function
   does a case-insensitive substring search for a particular user id.
   If userid is false, it looks for an 8-byte key id. Returns a memory
   FIFO of the key if found, NULL if not. Keyring is at end if not found,
   or immediately after key if found. */ 
struct fifo *pgpk_findkey(struct fifo *keyring,
			  byte *key,int userid)
{

#define STRING_SIZE 256

struct fifo *k,*kc;
byte type;
word32 length;
word16 bits,bytes;
unsigned j;
byte *id=safemalloc(STRING_SIZE); /* To save stack space */
byte *k2=safemalloc(STRING_SIZE); 

if(userid) {
  for(j=0;key[j];j++)
    k2[j]=toupper(key[j]);
  k2[j]='\0';
  }

while(k=pgpk_getnextkey(keyring)) {
  kc=fifo_copy(k);
  if(userid) {
    if(!pgp_find_packet(k,PGP_UID,0,0))
      goto not_it;
    pgp_examine_packet(k,&type,&length);
    fifo_aget(id,length,k);
    id[length]='\0';
    for(j=0;j<length;j++)
      id[j]=toupper(id[j]);
    if(strstr(id,k2))
      goto found_it;
    else
      goto not_it;
  }
  else { /* Keyid */
    if(!pgp_find_packet(k,PGP_PK,PGP_SK,0))
      goto not_it;
    pgp_examine_packet(k,&type,&length);
    fifo_skipn(k,8);
    fifo_aget(&bits,2,k);
    endian(&bits,2);
    bytes=bits2bytes(bits);
    fifo_skipn(k,bytes-8);             
    fifo_aget(id,8,k);
    if(memcmp(id,key,8))
      goto not_it;
    else
      goto found_it;
    }
  found_it:
    fifo_destroy(k);

    bzero (id, STRING_SIZE);
    bzero (k2, STRING_SIZE);

    free(id);
    free(k2);
    return(kc);
  not_it:
    fifo_destroy(k);
    fifo_destroy(kc);
  }

bzero (id, STRING_SIZE);
bzero (k2, STRING_SIZE);

free(id);
free(k2);
return(NULL);
}

/* Extract the key material from a key.
   If the input is a secret key and sk is non-null, the secret key
   components will also be returned. In this case, the function will
   return TRUE if the secret key components are encrypted. It returns
   FALSE in all other cases. Trustbyte and compromise are ignored if
   the pointers are NULL */
int pgpk_extract_key(struct fifo *f,struct pgp_pubkey *pk,
		     struct pgp_seckey *sk,byte *trustbyte,
		     int *compromise)
{
int encrypted=FALSE;
int comp=FALSE;
byte type,trust;
word32 length;

if((pgp_nexttype(f)==PGP_SK)&&(!sk))
  return(FALSE); /* Secret key and no secret key structure provided */
comp=FALSE;
pgp_get_keyid(f,pk->keyid);
pgp_examine_packet(f,&type,&length);
fifo_get(f); /* Skip version byte */
fifo_aget(&pk->timestamp,4,f);
endian(&pk->timestamp,4);
fifo_aget(&pk->validity,2,f);
endian(&pk->validity,2);
fifo_get(f); /* Skip algorithm byte */   
pgp_extract_mpi(f,&pk->n);
pgp_extract_mpi(f,&pk->e);

if(type==PGP_SK) {
  encrypted=fifo_get(f);
  if(encrypted)
    fifo_aget(sk->iv,8,f);
  pgp_extract_mpi(f,&sk->d);
  pgp_extract_mpi(f,&sk->p);
  pgp_extract_mpi(f,&sk->q);
  pgp_extract_mpi(f,&sk->u);
  fifo_aget(&sk->checksum,2,f);
  }
trust=0;
gettrust:
switch(pgp_nexttype(f)) {
  case(PGP_TRU):
    fifo_skipn(f,2);
    trust=fifo_get(f);
    goto gettrust;
  case(PGP_SIG):
    comp=TRUE;
  default:
    break;
  }
if(trustbyte)
  *trustbyte=trust;
if(compromise)
  *compromise=comp;
return(encrypted);
}

/* Gets a username and corresponding trust byte from a key. Returns
   TRUE if it found one, FALSE if done. This can be called multiple times
   to get all the names associated with the key. Username string should
   be at least 256 bytes, and trustbyte can be null to ignore */
int pgpk_extract_username(struct fifo *f,byte *username,
			  byte *trustbyte)
{
byte type,trust;
word32 length;
switch(pgp_find_packet(f,PGP_UID,0,0)) {
  case(0):
    return(FALSE);
  case(PGP_UID):
    pgp_examine_packet(f,&type,&length);
    fifo_aget(username,length,f);
    username[length]='\0';
  }
trust=0;
if(pgp_nexttype(f)==PGP_TRU) {
  fifo_skipn(f,2);
  trust=fifo_get(f);
  }
if(trustbyte)
  *trustbyte=trust;
return(TRUE);
}

/* Extracts a key signature packet, returning TRUE if it found one
   (next type was a signature) and FALSE if not. Keyid, if non-null,
   receives the 8-byte key id. Trust, if non-null, receives the
   signature trust byte, and sig, if non-null, receives a copy
   of the signature packet (for checking) */
int pgpk_extract_sig(struct fifo *f,byte *keyid,
		     byte *trust,struct fifo *sig)
{
if(pgp_nexttype(f)==PGP_TRU)
  fifo_skipn(f,3);
if(pgp_nexttype(f)!=PGP_SIG)
  return(FALSE);
if(keyid)
  pgp_get_keyid(f,keyid);
if(sig)
  pgpk_copy_packet(f,sig);
else {
  byte type;
  word32 length;
  pgp_examine_packet(f,&type,&length);
  fifo_skipn(f,length);
  }
if(trust&&(pgp_nexttype(f)==PGP_TRU)) {
  fifo_skipn(f,2);
  *trust=fifo_get(f);
  }
return(TRUE);
}

/* Keyring add. Merges a set of keys with the keyring, combining the
   userids and signatures of those found, and adding the rest.
   Inputs i and keys are destroyed */
void pgpk_keyring_add(struct fifo *i,struct fifo *o,struct fifo *keys)
{
struct fifo *i2,*key,*key2,*modkeys;
modkeys=fifo_mem_create();
i2=fifo_copy(i);
while(key=pgpk_getnextkey(i2)) { /* Separate keys into new and mod */
  key2=pgpk_key_compare(key,&keys);
  if(!key2) { /* Not in list. Ignore. */
    fifo_destroy(key);
    continue; } 
  fifo_append(modkeys,key2); /* In list, put in mod */
  fifo_destroy(key);
  }
pgpk_trustpackets(keys,o,TRUE); /* Prepend new keys, putting in
				   blank trust packets along the way */
while(key=pgpk_getnextkey(i)) { /* Make changes to mod keys */
  key2=pgpk_key_compare(key,&modkeys);
  if(!key2) { /* Not in list. Pass it through. */
    fifo_append(o,key);
    continue; } 
  pgpk_merge_keys(key,key2,o);
  }
fifo_destroy(i);
fifo_destroy(i2);
fifo_destroy(modkeys);
}

/* If key is present (same keyid) in list, return key as found in list,
   and remove it from list. If not, return null and leave list alone.
   Key is not altered. List is a double pointer so a new one can be
   returned to the caller */
static struct fifo *pgpk_key_compare(struct fifo *key,struct fifo **list)
{
struct fifo *newlist,*key2;
byte keyid1[8],keyid2[8];
pgp_get_keyid(key,keyid1);
newlist=fifo_mem_create();
while(key2=pgpk_getnextkey(*list)) {
  pgp_get_keyid(key2,keyid2);
  if(!memcmp(keyid1,keyid2,8)) {
    fifo_append(newlist,*list);
    *list=newlist;
    return(key2); /* Found it */
    }
  else /* Keep looking */
    fifo_append(newlist,key2);
  }
fifo_destroy(*list);
*list=newlist;
return(NULL);
}

/* Merges two copies of a key, returning the new one. Combines their
   userids and signatures. Ownertrust byte of key1 is retained.
   All other trust packets are cleared to zero. Inputs are destroyed. */
void pgpk_merge_keys(struct fifo *key1,struct fifo *key2,
			     struct fifo *newkey)
{
int revoked=FALSE;
struct fifo *newuids,*temp;
pgpk_copy_packet(key1,newkey); /* Copy key packet */
if(pgp_nexttype(key1)==PGP_TRU)
  pgpk_copy_packet(key1,newkey);  /* Copy ownertrust packet if found */
if(pgp_nexttype(key1)==PGP_SIG) {  /* Copy a revocation if found */
  revoked=TRUE;
  pgpk_copy_packet(key1,newkey);
  }
if(pgp_find_packet(key2,PGP_UID,revoked?0:PGP_SIG,0)==PGP_SIG) {
  /* Get revocation from key2 if there is one and not in key1 */
  /* Then fast-forward key2 to the first UID */
  pgpk_copy_packet(key2,newkey);
  pgp_find_packet(key2,PGP_UID,0,0);
  }
temp=key1;
key1=fifo_mem_create();
pgpk_trustpackets(temp,key1,FALSE); /* Strip trust packets from key1 */
temp=key2;
key2=fifo_mem_create();
pgpk_trustpackets(temp,key2,FALSE); /* And from key2 */
newuids=pgpk_merge_uids(key1,key2); /* Merge the uids and signatures */
pgpk_trustpackets(newuids,newkey,TRUE); /* Put in zero trust packets */
}

/* Merges the uids and signatures from two keys, destroys input keys */
static struct fifo *pgpk_merge_uids(struct fifo *key1,struct fifo *key2)
{
struct fifo *uid1,*uid2,*newkey,*newkey2;
newkey=fifo_mem_create();
while(pgp_nexttype(key1)==PGP_UID) {
  uid1=fifo_mem_create();
  pgpk_copy_packet(key1,uid1); /* Get a uid from key1 */
  newkey2=fifo_mem_create();
  while(pgp_nexttype(key2)==PGP_UID) {
    uid2=fifo_mem_create();
    pgpk_copy_packet(key2,uid2); /* Get a uid from key2 */
    if(pgpk_compare_uids(uid1,uid2)) { /* If they match... */
      fifo_append(newkey,uid1); /* Output the new uid */
      pgpk_merge_sigs(key1,key2,newkey); /* Merge their signatures */
      fifo_destroy(uid2); /* Get rid of extra copy */
      fifo_append(newkey2,key2); /* Keep rest of uids */
      goto match; /* Jump over some stuff we don't need to do */
      }
    fifo_append(newkey2,uid2); /* Otherwise, return uid to the list */
    while(pgp_nexttype(key2)==PGP_SIG)
      pgpk_copy_packet(key2,newkey2); /* Copy sigs to new list */
    }
  fifo_append(newkey,uid1); /* Copy uid1 back to new key */
  while(pgp_nexttype(key1)==PGP_SIG)
    pgpk_copy_packet(key1,newkey); /* And all its sigs */
  fifo_destroy(key2); /* Key2 is empty, everything is in newkey2 */
  match:
  key2=newkey2;
  }
fifo_append(newkey,key2); /* Dump all new uids in at the end */
fifo_destroy(key1);
return(newkey);
}

/* Merge sigs from l1 and l2, copying into new with no duplicates.
   Uses up sigs in l1 and l2 until reaching the first non-sig. */
static void pgpk_merge_sigs(struct fifo *l1,struct fifo *l2,
			    struct fifo *new)
{
byte keyid1[8],keyid2[8];
struct fifo *sl1,*sl2,*nsl2;
sl1=fifo_mem_create();
sl2=fifo_mem_create();
while(pgp_nexttype(l1)==PGP_SIG) /* Copy all sigs from l1 to sl1 */
  pgpk_copy_packet(l1,sl1);      /* Stopping at the first non-sig */
while(pgp_nexttype(l2)==PGP_SIG) 
  pgpk_copy_packet(l2,sl2);      /* Repeat for l2/sl2 */
while(pgp_nexttype(sl1)==PGP_SIG) {
  pgp_get_keyid(sl1,keyid1); /* Get keyid for sl1 */
  nsl2=fifo_mem_create();
  while(pgp_nexttype(sl2)==PGP_SIG) {
    pgp_get_keyid(sl2,keyid2); /* Get keyid for sl2 */
    if(!memcmp(keyid1,keyid2,8)) { /* Signatures are the same */
      pgpk_copy_packet(sl1,new); /* Output signature */
      pgpk_skip_packet(sl2); /* Discard copy */
      fifo_append(nsl2,sl2); /* Put rest of signatures */
      goto match; /* Bail out */
      }
    pgpk_copy_packet(sl2,nsl2); /* Move it to the end of the line */
    }
  pgpk_copy_packet(sl1,new); /* Copy signature */
  fifo_destroy(sl2); /* Get rid of empty list */
  match:
  sl2=nsl2;
  }
fifo_destroy(sl1);
fifo_append(new,sl2); /* Put all new signatures in at the end */
}

/* Compares two uids without changing them, returning TRUE if they
   are the same and FALSE if not */
static int pgpk_compare_uids(struct fifo *uid1,struct fifo *uid2)
{
word32 i,length1,length2;
length1=fifo_length(uid1);
length2=fifo_length(uid2);
if(length1!=length2)
  return(FALSE);
for(i=0;i<length1;i++)
  if(fifo_rget(i,uid1)!=fifo_rget(i,uid2))
    return(FALSE);
return(TRUE);
}

/* Do something with this piece of shit! */

/* Keyring add/delete/replace function. This function is used to add
   keys, delete keys, or update a key. Input i is copied to o, with
   changes: if keyid is NULL, o <= i with key prepended. If keyid is
   valid and key is present, o <= i with key replacing the previous
   version of the same key (used to add sigs, change userid, etc.)
   If keyid is valid and key is NULL, o <= i with the key matching
   keyid deleted. Function returns TRUE if it successfully performed
   its operation, or FALSE if it couldn't find keyid. Inputs i and key
   are destroyed */
int pgpk_change_keyring(struct fifo *i,struct fifo *o,struct fifo *key,
			byte *keyid)
{
unsigned j;
byte checkkeyid[8];
struct fifo *k;
if(keyid)
  for(;;) {
    if(!(k=pgpk_getnextkey(i))) {
      fifo_destroy(i);
      if(key)
	fifo_destroy(key);
      return(FALSE);
      }
    pgp_get_keyid(k,checkkeyid);
    if(!memcmp(keyid,checkkeyid,8))
      goto rightkey;
    fifo_append(o,k);
    }
rightkey:
fifo_destroy(k);
if(key)
  fifo_append(o,key);
fifo_append(o,i);
return(TRUE);
}

/* Puts in or takes out trust packets. If putin is TRUE, zeroed trust
   packets are put in everywhere that one belongs. If putin is FALSE,
   all trust packets are removed. Normally, you would take out trust
   packets when extracting a key, and put them in when preparing a key
   for signature checking and adding to the keyring. */
void pgpk_trustpackets(struct fifo *i,struct fifo *o,int putin)
{
uidsigs:
if(!fifo_length(i))
  goto endofinput;
  switch(pgp_nexttype(i)) {
    case(PGP_PK):
      goto pubkey;
    case(PGP_TRU):
      fifo_skipn(i,3);
      goto uidsigs;
    default:
      pgpk_copy_packet(i,o);
      if(putin)
	fifo_aput(blanktrust,3,o);
      goto uidsigs;
    }

pubkey:
  pgpk_copy_packet(i,o); /* Copy the public key */
  if(putin)
    fifo_aput(blanktrust,3,o); /* Put in an ownertrust packet */
  afterpk:
    if(!fifo_length(i))
      goto endofinput;
    switch(pgp_nexttype(i)) {
      case(PGP_TRU):
	fifo_skipn(i,3);
	goto afterpk;
      case(PGP_SIG):
	pgpk_copy_packet(i,o); /* No trust packet after compromise */
	goto uidsigs;
      default:
	goto uidsigs;
      }

endofinput:
fifo_destroy(i);
}

/* Checks all of the signatures on a key, requesting public keys as needed.
   Destroys its input and returns a new memory FIFO containing the same
   key with all bad signatures removed. Receives function pointer lookup()
   and calls it to look up public keys. Lookup's prototype is:
int lookup(byte keyid[8],struct pgp_pubkey *pk,
	   byte *userid,byte *trustbyte);
   Lookup should return TRUE if found and FALSE if not, and should write
   the public key and first userid to the pointers provided. Function
   pointer output() allows you to print results as each key is checked.
   Output can be NULL to avoid calling it. Output's prototype is:
void output(byte keyid[8],byte *userid,
	    time_t timestamp,unsigned result);
   where userid is NULL if not available, and result is:
   0=bad signature 1=key not available to check 2=good signature
   3=bad revocation certificate 4=good revocation certificate
   5=userid we are about to check keys for
   A signature in the wrong place on the ring will be bad. */
void pgpk_check_sigs(struct fifo *key,struct fifo *out,
		     int (*lookup)(), void (*output)() )
{
MD5_CTX md5ctx;
struct fifo *pkp,*cpkt,*cpkt2,*temp;
struct pgp_pubkey *pk=safemalloc(sizeof(struct pgp_pubkey));
time_t timestamp;
byte type,sigtype,trustbyte;
byte keyid[8],hash1[16],hash2[16];
byte *userid=safemalloc(STRING_SIZE); /* To save stack space, which  */
byte *ck_uid=safemalloc(STRING_SIZE); /* DOS has way to little of... */

pkp=fifo_copy(key); /* Make a disposable copy */
pgpk_extract_key(pkp,pk,NULL,NULL,NULL); /* Extract the key */
fifo_destroy(pkp); /* Take out the garbage */
pkp=fifo_mem_create();
pgpk_copy_packet(key,pkp); /* Make a copy of the public key packet */
				    
fifo_append(out,fifo_copy(pkp)); /* Copy the public key to output */
if(pgp_nexttype(key)==PGP_TRU)
  pgpk_copy_packet(key,out);  /* Copy ownertrust packet if found */

if(pgp_nexttype(key)==PGP_SIG) { /* Found a revocation */
  cpkt=fifo_mem_create();
  pgpk_copy_packet(key,cpkt);
  cpkt2=fifo_copy(cpkt);
  pgp_extract_sig(cpkt,hash1,&timestamp,&sigtype,pk); /* Extract sig */
  fifo_destroy(cpkt);  /* More garbage */
  MD5Init(&md5ctx);
  pgp_md5(fifo_copy(pkp),&md5ctx); /* MD5 the public key */
  MD5Update(&md5ctx,&sigtype,1); /* Add this stuff in */
  endian(&timestamp,4);
  MD5Update(&md5ctx,(byte *)&timestamp,4);
  endian(&timestamp,4);
  MD5Final(hash2,&md5ctx); /* Get the MD5 of the public key */
  if(memcmp(hash1,hash2,16)) { /* Revocation bogus */
    if(output) 
      (*output)(pk->keyid,NULL,timestamp,3);
    }
  else { /* Revocation good */
    if(output)
      (*output)(pk->keyid,NULL,timestamp,4);
    pgpk_copy_packet(cpkt2,out); /* Copy revocation to output */
    }
  fifo_destroy(cpkt2); /* Even more garbage */
  }
while(type=pgp_nexttype(key)) { /* Loop through rest of key */
  if(type!=PGP_SIG) { /* Get uid, pass it through */
    if(type==PGP_UID) {
      temp=fifo_copy(key);
      pgpk_extract_username(temp,ck_uid,NULL);
      fifo_destroy(temp);
      if(output)
	(*output)(pk->keyid,ck_uid,pk->timestamp,5);
      }
    pgpk_copy_packet(key,out);
    continue;
    }
  pgp_get_keyid(key,keyid); /* What keyid do we need? */
  if(!(*lookup)(keyid,pk,userid,&trustbyte)) { /* Key not found */
    if(output)
      (*output)(keyid,NULL,timestamp,1);
    pgpk_copy_packet(key,out); /* Pass it through */
    continue;
    }
  cpkt=fifo_mem_create();
  pgpk_copy_packet(key,cpkt);
  cpkt2=fifo_copy(cpkt);
  pgp_extract_sig(cpkt,hash1,&timestamp,&sigtype,pk); /* Extract sig */
  fifo_destroy(cpkt);  /* More garbage */
  MD5Init(&md5ctx);
  pgp_md5(fifo_copy(pkp),&md5ctx); /* MD5 the public key */
  MD5Update(&md5ctx,ck_uid,strlen(ck_uid)); /* And userid */
  MD5Update(&md5ctx,&sigtype,1); /* Add this stuff in */
  endian(&timestamp,4);
  MD5Update(&md5ctx,(byte *)&timestamp,4);
  endian(&timestamp,4);
  MD5Final(hash2,&md5ctx); /* Get the MD5 of the public key */
  if(memcmp(hash1,hash2,16)) { /* Signature bogus */
    if(output) 
      (*output)(pk->keyid,userid,timestamp,0);
    }
  else { /* Signature good */
    if(output)
      (*output)(pk->keyid,userid,timestamp,2);
    pgpk_copy_packet(cpkt2,out); /* Copy signature to output */
    }
  fifo_destroy(cpkt2); /* Even more garbage */
  }
fifo_destroy(key);
fifo_destroy(pkp);

bzero (userid, STRING_SIZE);
bzero (ck_uid, STRING_SIZE);
bzero (pk, sizeof (struct pgp_pubkey));

free(userid);
free(ck_uid);
free(pk);
}
