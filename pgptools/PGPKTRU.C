/* pgpktru.c */

#include <stdio.h>
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

#define TABLESIZE 256 /* Buckets in trust table */
#define ALLOCBYTES 1024 /* Block size to allocate */

struct key_trust {
  struct key_trust *next;
  byte keyid[8];
  byte ownertrust;
  byte old_ownertrust;
  byte revoked;
  struct uid_trust *uids;
  struct sig_list *sigs;
  };

struct uid_trust {
  struct uid_trust *next;
  byte uidtrust;
  struct sig_trust *sigs;
  };

struct sig_trust {
  struct sig_trust *next;
  byte sigid[8];
  byte sigtrust;
  };

struct sig_list {
  struct sig_list *next;
  struct key_trust *key;
  };

static struct key_trust **trust_table; /* Trust table */
static struct fifo *keyring;
static byte (*pgpt_ask_user)();
static unsigned bytesleft; /* Bytes left in tru_alloc block */
static byte *blockptr; /* Pointer to tru_alloc block */
static struct fifo *tru_blocklist; /* List of blocks allocated */
static unsigned max_trust_depth;
static unsigned valid_level; /* marginals_needed * completes_needed */
static unsigned marginal_value; /* valid_level / marginals_needed */
static unsigned complete_value; /* valid_level / completes_needed */

/* Load up the trust table from the keyring. */
static void pgpt_load(struct fifo *keyring,int keep_ownertrust);

/* Finds all signatures made by the keys given, and updates their sigtrust
   bytes. If the update makes a key fully validated, ask the user for his
   or her opinion of that person, then process that key as well.
   Called once for each buckstop key. Call with depth 0, this will increase
   with recursion. */
static void pgpt_recurse_trust(struct key_trust *key,unsigned depth);

/* Look up a key in the table, null if not found */
static struct key_trust *pgpt_lookup(byte *keyid);

/* Looks up key and calls user-supplied query function */
static byte pgpt_query_user(byte *keyid);

/* Reads a trust packet */
static byte pgpt_get_trust(struct fifo *f);

/* Put a trust packet "inplace" */
static void pgpt_put_trust(byte trust,struct fifo *f);

/* Write a trust packet to output fifo */
static void pgpt_write_trust(byte trust,struct fifo *f);

/* Copy from i to o until a trust packet is found. Then skip the trust
   packet and return. Used to update in memory. */
static void pgpt_goto_trust(struct fifo *i,struct fifo *o);

/* Updates a disk fifo keyring in place */
static void pgpt_update_inplace(struct fifo *keyring);

/* Updates a memory fifo, making a copy */
static struct fifo *pgpt_update_inmem(struct fifo *keyring);

/* Release all memory before exiting module */
static void pgpt_dump(void);

/* Special memory-allocation function. This has two features which
   safemalloc doesn't: it allocates small chunks from within a larger
   block, and everything can be thrown away with one call to pgpt_dump
   when we are done. * DO NOT * free() anything allocated using this
   function, you will crash the program. Do not allocate anything bigger
   than the blocksize. */
static void *tru_alloc(size_t size);

struct fifo *pgpt_update_trust(struct fifo *ikeyring,struct fifo *buckstop,
	    byte (*query_user)(),unsigned marginals_needed,
	    unsigned completes_needed,unsigned max_depth,
	    int keep_oldtrust)
{
struct key_trust *k;
struct fifo *krc,*buckstop2;
byte keyid[8];
unsigned i;

keyring=ikeyring;
pgpt_ask_user=query_user;
valid_level=marginals_needed*completes_needed;
marginal_value=valid_level/marginals_needed;
complete_value=valid_level/completes_needed;
max_trust_depth=max_depth;

tru_blocklist=fifo_mem_create();
bytesleft=0;
trust_table=safemalloc(TABLESIZE*sizeof(struct key_trust *));
for(i=0;i<TABLESIZE;i++)
  trust_table[i]=NULL;

krc=fifo_copy(keyring);
pgpt_load(krc,keep_oldtrust);
fifo_destroy(krc);
buckstop2=fifo_copy(buckstop);

while(fifo_aget(keyid,8,buckstop)) {
  k=pgpt_lookup(keyid);
  if(k) 
    k->ownertrust=k->revoked?130:135;
  }
fifo_destroy(buckstop);

while(fifo_aget(keyid,8,buckstop2)) {
  k=pgpt_lookup(keyid);
  if(k) 
    pgpt_recurse_trust(k,0);
  }
fifo_destroy(buckstop2);

if(keyring->fp) { /* Disk fifo */
  krc=fifo_copy(keyring);
  pgpt_update_inplace(keyring);
  }
else /* Memory fifo */
  krc=pgpt_update_inmem(keyring);
pgpt_dump();
return(krc);
}

static void pgpt_load(struct fifo *keyring,int keep_ownertrust)
{
unsigned i;
byte type;
struct key_trust *k,*ko;
struct uid_trust *u,*uo;
struct sig_trust *s,*so;
struct sig_list *l;

while(pgp_nexttype(keyring)==PGP_PK) { /* For each public key... */
  k=tru_alloc(sizeof(struct key_trust)); /* Get a new key record */
  k->sigs=NULL; /* List of sigs made by this key */
  pgp_get_keyid(keyring,k->keyid); /* Get the keyid */
  pgpk_skip_packet(keyring);
  k->ownertrust=0;
  if(keep_ownertrust) {
    k->old_ownertrust=pgpt_get_trust(keyring);
    if(k->old_ownertrust&128) /* Buckstop */
      k->old_ownertrust=0;
    }
  else {
    k->old_ownertrust=0;
    pgpk_skip_packet(keyring); /* Ignore ownertrust */
    }
  k->uids=uo=NULL;
  if(pgp_nexttype(keyring)==PGP_SIG) { /* Revocation */
    k->revoked=TRUE;
    pgpk_skip_packet(keyring);
    }
  else
    k->revoked=FALSE;    
  while(pgp_nexttype(keyring)==PGP_UID) { /* For each uid */
    u=tru_alloc(sizeof(struct uid_trust)); /* Get a uid record */
    if(uo) {
      uo->next=u; /* Attach to end */
      uo=u;
      }
    else 
      k->uids=uo=u;  /* First record */
    pgpk_skip_packet(keyring); /* Userid packet */
    u->uidtrust=0;
    pgpk_skip_packet(keyring); /* Uidtrust packet */
    u->sigs=so=NULL;
    while(pgp_nexttype(keyring)==PGP_SIG) { /* For each sig */
      s=tru_alloc(sizeof(struct sig_trust)); /* Get a sig record */
      if(so) {
	so->next=s; /* Attach to end */
	so=s;
	}
      else
	u->sigs=so=s; /* First record */
      pgp_get_keyid(keyring,s->sigid); /* Get sigid */
      pgpk_skip_packet(keyring); /* Skip sig */
      s->sigtrust=0;
      pgpk_skip_packet(keyring); /* Sigtrust */
      s->next=NULL;
      } /* while(sigs); */
    u->next=NULL;
    } /* while(uids); */
  i=256*k->keyid[0]+k->keyid[1];
  i%=TABLESIZE;
  k->next=trust_table[i];
  trust_table[i]=k;
  } /* while(keys); */

/* Now we index signatures with the keys that made them */
for(i=0;i<TABLESIZE;i++) {
  k=trust_table[i];
  while(k) {
    u=k->uids;
    while(u) {
      s=u->sigs;
      while(s) {
	ko=pgpt_lookup(s->sigid); /* Lookup key that made signature */
	if(ko) { /* If we have the key... */
	  l=tru_alloc(sizeof(struct sig_list));
	  l->key=k;
	  l->next=ko->sigs;
	  ko->sigs=l;
	  }
	s=s->next;
	}
      u=u->next;
      }
    k=k->next;
    }
  }
}

/* Finds all signatures made by the keys given, and updates their sigtrust
   bytes. If the update makes a key fully validated, ask the user for his
   or her opinion of that person, then process that key as well.
   Called once for each buckstop key. Call with depth 0, this will increase
   with recursion. */
static void pgpt_recurse_trust(struct key_trust *key,unsigned depth)
{
unsigned trustvalue;
int valid;
struct key_trust *k;
struct uid_trust *u;
struct sig_trust *s;
struct sig_list *l;

if(key->ownertrust&128) {
  u=key->uids;
  while(u) {
    u->uidtrust=3; /* uids on a buckstop key are trusted */
    u=u->next;
    }
  }

l=key->sigs;
while(l) {
  valid=FALSE;
  k=l->key;
  u=k->uids;
  while(u) {
    s=u->sigs;
    trustvalue=0;
    while(s) {
      if(!memcmp(s->sigid,key->keyid,8))
	s->sigtrust=(key->ownertrust&7)|((key->ownertrust&7)?128:0);
      switch(s->sigtrust&7) {
	case(5):
	  trustvalue+=marginal_value;
	  break;
	case(6):
	  trustvalue+=complete_value;
	  break;
	case(7):
	  trustvalue=valid_level;
	  break;
	default:
	  break;
	}
      s=s->next;
      }
    if(k->ownertrust&128) /* Buckstop */
      trustvalue=valid_level; /* Always trust origin of buckstop keys */
    if(trustvalue==0)
      u->uidtrust=0; /* Untrusted */
    else if(trustvalue>=valid_level) {
      valid=TRUE;
      u->uidtrust=3; /* Complete */
      }
    else if(trustvalue>=(valid_level/2)) 
      u->uidtrust=2; /* Marginal */
    else
      u->uidtrust=1; /* Untrusted */
    u=u->next;
    }
  if(valid&&(!(k->ownertrust&7))) {
    if(k->revoked)
      k->ownertrust=2;
    else
      k->ownertrust=k->old_ownertrust ? k->old_ownertrust:
					pgpt_query_user(k->keyid);
    if(depth<max_trust_depth)
      pgpt_recurse_trust(k,depth+1);
    }
  l=l->next;
  }
}

static void pgpt_update_inplace(struct fifo *keyring)
{
byte keyid[8];
struct key_trust *k;
struct uid_trust *u;
struct sig_trust *s;

while(pgp_find_packet(keyring,PGP_PK,0,0)==PGP_PK) {
  pgp_get_keyid(keyring,keyid);
  pgpk_skip_packet(keyring);
  k=pgpt_lookup(keyid);
  if(k) {
    u=k->uids;
    pgpt_put_trust(k->ownertrust,keyring);
    while(u) {
      pgpt_put_trust(u->uidtrust,keyring);
      s=u->sigs;
      while(s) {
	pgpt_put_trust(s->sigtrust,keyring);
	s=s->next;
	}
      u=u->next;
      }
    }	
  else
    pgpk_skip_packet(keyring); /* If not found */
  }
fifo_destroy(keyring);
}

/* Updates a memory fifo, making a copy */
static struct fifo *pgpt_update_inmem(struct fifo *keyring)
{
struct fifo *key,*newring;
struct key_trust *k;
struct uid_trust *u;
struct sig_trust *s;
byte keyid[8];

newring=fifo_mem_create();
while(key=pgpk_getnextkey(keyring)) {
  pgp_get_keyid(key,keyid);
  k=pgpt_lookup(keyid);
  if(k) {
    u=k->uids;
    pgpt_goto_trust(key,newring);
    pgpt_write_trust(k->ownertrust,newring);
    while(u) {
      pgpt_goto_trust(key,newring);
      pgpt_write_trust(u->uidtrust,newring);
      s=u->sigs;
      while(s) {
	pgpt_goto_trust(key,newring);
	pgpt_write_trust(s->sigtrust,newring);
	s=s->next;
	}
      u=u->next;
      }
    fifo_destroy(key);
    }	
  else
    fifo_append(newring,key); /* If not found */
  }
fifo_destroy(keyring);
return(newring);
}
		   
static void pgpt_goto_trust(struct fifo *i,struct fifo *o)
{
byte type;
while(type=pgp_nexttype(i)) {
  if(type==PGP_TRU) {
    pgpk_skip_packet(i);
    return;
    }
  else 
    pgpk_copy_packet(i,o);
  }
}

static byte pgpt_query_user(byte *keyid)
{
struct fifo *krc;
struct fifo *key;
byte *buf=safemalloc(256);
byte result;

krc=fifo_copy(keyring);
key=pgpk_findkey(krc,keyid,FALSE);
fifo_destroy(krc);
if(!key) {
  free(buf);
  return(1); /* Unknown (shouldn't happen) */
  }
pgp_find_packet(key,PGP_UID,0,0); /* Find first username */
pgpk_extract_username(key,buf,NULL); /* Get username */
fifo_destroy(key);
result=(*pgpt_ask_user)(keyid,buf);
free(buf);
return(result);
}

static void *tru_alloc(size_t size)
{
void *p;

if(bytesleft<size) { /* We need a new block */
  blockptr=safemalloc(ALLOCBYTES);
  fifo_aput(&blockptr,sizeof(blockptr),tru_blocklist);
  bytesleft=ALLOCBYTES;
  }
bytesleft-=size;
p=blockptr;
blockptr+=size;
return(p);
}

static struct key_trust *pgpt_lookup(byte *keyid)
{
unsigned i;
struct key_trust *k;

i=256*keyid[0]+keyid[1];
i%=TABLESIZE;
k=trust_table[i];
while(k) {
  if(!memcmp(keyid,k->keyid,8))
    return(k); /* Found it */
  k=k->next;
  }
return(NULL);
}

static byte pgpt_get_trust(struct fifo *f)
{
if(pgp_nexttype(f)==PGP_TRU) {
  fifo_skipn(f,2);
  return(fifo_get(f)); /* Get trust */
  }
else /* No trust packet */
  return(0);
}

static void pgpt_put_trust(byte trust,struct fifo *f)
{
if(pgp_find_packet(f,PGP_TRU,PGP_PK,0)!=PGP_TRU)
  return;
fifo_rput(trust,2,f);
pgpk_skip_packet(f);
}

static void pgpt_write_trust(byte trust,struct fifo *f)
{
fifo_aput("\xb0\x1",2,f);
fifo_put(trust,f);
}

static void pgpt_dump(void)
{
void *p;

free(trust_table);
while(fifo_aget(&p,sizeof(&p),tru_blocklist))
  free(p);
fifo_destroy(tru_blocklist);
}
