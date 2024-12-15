/* pgpktru.h */

/* Call this function after any changes to a public keyring which could
   affect trust values */

/* Updates a public keyring's trust information. If the input is a disk
   fifo, the update will be performed "in place" and the returned fifo will
   be a copy of the input, with the changes made. If the input is a memory
   fifo, the return will be a new memory fifo containing the updated
   keyring. In either case, the input keyring will be destroyed.
   Buckstop is a list of 8-byte keyids of buckstop keys (keys present in the
   secret keyring). It will be destroyed. Query_user will be called to ask
   the user whether he or she trusts a particular person to certify keys.
   Its prototype is:
byte query_user(byte *keyid,byte *userid);
   Where userid is the first username. Query_user should return one of
   1=unknown 2=untrusted 5=marginal 6=complete 
   If keep_oldtrust is FALSE, it will query for all keys. If TRUE, it
   will only query for uninitialized keys */
struct fifo *pgpt_update_trust(struct fifo *keyring,struct fifo *buckstop,
	    byte (*query_user)(),unsigned marginals_needed,
	    unsigned completes_needed,unsigned max_depth,
	    int keep_oldtrust);
