/* pgpkmgt.h */

/* Copy one packet from i to o and return its type. */
byte pgpk_copy_packet(struct fifo *i,struct fifo *o);

/* Skip next packet */
void pgpk_skip_packet(struct fifo *f);

/* Get next key from a key ring, returns a memory FIFO containing the key
   if TRUE, or NULL if at the end of the keyring */
struct fifo *pgpk_getnextkey(struct fifo *keyring);

/* Search a keyring for a particular key. If userid is true, the function
   does a case-insensitive substring search for a particular user id.
   If userid is false, it looks for an 8-byte key id. Returns a memory
   FIFO of the key if found, NULL if not. Keyring is at end if not found,
   or immediately after key if found. */ 
struct fifo *pgpk_findkey(struct fifo *keyring,
			  byte *key,int userid);

/* Extract the key material from a key.
   If the input is a secret key and sk is non-null, the secret key
   components will also be returned. In this case, the function will
   return TRUE if the secret key components are encrypted. It returns
   FALSE in all other cases. Trustbyte and compromise are ignored if
   the pointers are NULL */
int pgpk_extract_key(struct fifo *f,struct pgp_pubkey *pk,
		     struct pgp_seckey *sk,byte *trustbyte,
		     int *compromise);

/* Gets a username and corresponding trust byte from a key. Returns
   TRUE if it found one, FALSE if done. This can be called multiple times
   to get all the names associated with the key. Username string should
   be at least 256 bytes, and trustbyte can be null to ignore */
int pgpk_extract_username(struct fifo *f,byte *username,
			  byte *trustbyte);

/* Extracts a key signature packet, returning TRUE if it found one
   (next type was a signature) and FALSE if not. Keyid, if non-null,
   receives the 8-byte key id. Trust, if non-null, receives the
   signature trust byte, and sig, if non-null, receives a copy
   of the signature packet (for checking) */
int pgpk_extract_sig(struct fifo *f,byte *keyid,
		     byte *trust,struct fifo *sig);

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
			byte *keyid);

/* Puts in or takes out trust packets. If putin is TRUE, zeroed trust
   packets are put in everywhere that one belongs. If putin is FALSE,
   all trust packets are removed. Normally, you would take out trust
   packets when extracting a key, and put them in when preparing a key
   for signature checking and adding to the keyring. */
void pgpk_trustpackets(struct fifo *i,struct fifo *o,int putin);

/* Keyring add. Merges a set of keys with the keyring, combining the
   userids and signatures of those found, and adding the rest.
   Inputs i and keys are destroyed */
void pgpk_keyring_add(struct fifo *i,struct fifo *o,struct fifo *keys);

/* Merges two copies of a key, returning the new one. Combines their
   userids and signatures. Ownertrust byte of key1 is retained.
   All other trust packets are cleared to zero. Inputs are destroyed. */
void pgpk_merge_keys(struct fifo *key1,struct fifo *key2,
			     struct fifo *newkey);

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
		     int (*lookup)(), void (*output)() );
