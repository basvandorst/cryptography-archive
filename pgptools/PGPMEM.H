/* pgpmem.h */

/* Number of buckets in the hash table. More buckets => faster lookup
   but a bigger table */
#define HASH_BUCKETS 256

/* This is the record for the hash table. The hash table uses the least
significant bits of the keyid (which should be fairly random) as the key,
and each bucket is a linked list. */

struct hash_record {
  byte keyid[8];
  byte *userid;
  byte trust;
  struct pgp_pubkey *key;
  struct hash_record *next;
  };

/* Never returns null. If it runs out of memory, it first tries to clear
   some by dumping the hash table. If it still can't get any memory, it
   calls out_of_memory. */
void *safemalloc(size_t size);

/* Put your out-of-memory handler here; this one just bails out */
void out_of_memory(void);

/* If pgp_hash_put is called without first calling pgp_hash_init, it will
   ignore its input and do nothing. pgp_hash_get will return FALSE.
   If you don't want to use the hash table, just don't initialize it. */

/* Put a key into the hash table. Keyid is the 8-byte key id. Userid is
   the user id string to store. Key is the public key to store.
   Trust is the ownertrust byte. Either userid or key can be passed
   null if you don't need to store them (and save memory) */
void pgp_hash_put(byte *keyid,byte *userid,
		  struct pgp_pubkey *key,byte trust);

/* Looks up a key in the hash table. Returns TRUE if found, FALSE if not.
   Any components not needed can be passed NULL pointers to ignore. */
int pgp_hash_get(byte *keyid,byte *userid,
		  struct pgp_pubkey *key,byte *trust);

/* Initializes or wipes out the hash table, freeing all memory used by it.
   Call before using hash table. Called by safemalloc if it runs out of
   memory. Call when finished using hash table to free memory. */
void pgp_hash_clear(void);

/* Gets rid of the hash table altogether */
void pgp_hash_off(void);
