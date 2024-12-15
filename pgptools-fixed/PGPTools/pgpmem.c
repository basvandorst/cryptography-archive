/* pgpmem.c */

#include <stdio.h>
#ifdef MSDOS
# include <alloc.h>
#else
# include <malloc.h>
#endif
#include <string.h>
#include <time.h>
#include "usuals.h"
#include "md5.h"
#include "mpilib.h"
#include "pgpmem.h"
#include "fifo.h"
#include "pgptools.h"

/* Hash table */
static struct hash_record **hash_table;
static int hash_init=FALSE; /* Set true once table initialized */

void *safemalloc(size_t size)
{
void *p;
p=malloc(size);
if(p) 
  return(p);
if(!hash_init)
  out_of_memory();
pgp_hash_clear();
p=malloc(size);
if(!p)
  out_of_memory();

bzero (p, size);
return(p);
}

void out_of_memory(void)
{
fprintf(stderr,"PGPMEM: out of memory\n");
exit(1);
}

/* Initializes or wipes out the hash table, freeing all memory used by it.
   Call before using hash table. Called by safemalloc if it runs out of
   memory. Call when finished using hash table to free memory. */
void pgp_hash_clear(void)
{
unsigned i;
struct hash_record *hpa,*hpb;

if(!hash_init) {
  hash_table=safemalloc(HASH_BUCKETS*sizeof(struct hash_record *));
  hash_init=TRUE;
  for(i=0;i<HASH_BUCKETS;i++)
    hash_table[i]=NULL;
  }
else {
  for(i=0;i<HASH_BUCKETS;i++) {
    hpa=hash_table[i];
    while(hpa) {
      hpb=hpa->next;
      if(hpa->userid) {
	bzero (hpa->userid, strlen (hpa->userid));
	free(hpa->userid);
	}
      if(hpa->key) {
	bzero (hpa->key, sizeof (struct pgp_pubkey));
	free(hpa->key);
	}
      bzero (hpa, sizeof (struct hash_record));
      free(hpa);
      hpa=hpb;
      }
    hash_table[i]=NULL;
    }
  }
}

/* Put a key into the hash table. Keyid is the 8-byte key id. Userid is
   the user id string to store. Key is the public key to store.
   Trust is the ownertrust byte. Either userid or key can be passed
   null if you don't need to store them (and save memory) */
void pgp_hash_put(byte *keyid,byte *userid,
		  struct pgp_pubkey *key,byte trust)
{
struct hash_record *hpa;
unsigned i;

if(!hash_init)
  return;
hpa=safemalloc(sizeof(struct hash_record));
memcpy(hpa->keyid,keyid,8);
hpa->trust=trust;
if(userid) {
  hpa->userid=safemalloc(strlen(userid)+1);
  strcpy(hpa->userid,userid);
  }
else
  hpa->userid=NULL;
if(key) {
  hpa->key=safemalloc(sizeof(struct pgp_pubkey));
  memcpy(hpa->key,key,sizeof(struct pgp_pubkey));
  }
else
  hpa->key=NULL;
i=256*keyid[0]+keyid[1];
i%=HASH_BUCKETS;
hpa->next=hash_table[i];
hash_table[i]=hpa;
}
  
/* Looks up a key in the hash table. Returns TRUE if found, FALSE if not.
   Any components not needed can be passed NULL pointers to ignore. */
int pgp_hash_get(byte *keyid,byte *userid,
		  struct pgp_pubkey *key,byte *trust)
{
struct hash_record *hpa;
unsigned i;
if(!hash_init)
  return(FALSE);
i=256*keyid[0]+keyid[1];
i%=HASH_BUCKETS;
hpa=hash_table[i];
while(hpa) {
  if(!memcmp(keyid,hpa->keyid,8)) {
    if(userid&&hpa->userid)
      strcpy(userid,hpa->userid);
    if(key&&hpa->key)
      memcpy(key,hpa->key,sizeof(struct pgp_pubkey));
    if(trust)
      *trust=hpa->trust;
    return(TRUE);
    }
  else
    hpa=hpa->next;
  }
return(FALSE);
}
 
/* Gets rid of the hash table altogether */
void pgp_hash_off(void)
{
if(!hash_init)
  return;
pgp_hash_clear();

bzero (hash_table, HASH_BUCKETS*sizeof(struct hash_record *));
free(hash_table);
hash_init=FALSE;
}



  
      
