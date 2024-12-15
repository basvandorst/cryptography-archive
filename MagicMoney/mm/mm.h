/* mm.h */

/* Magic Money Digital Cash System V1.0 */

#define BANK_PUBKEY_FILE "bank.pub" /* Bank's public key only */
#define BANK_ASCKEY_FILE "bank.asc" /* Bank's ascii-armored key */
#define BANK_SECKEY_FILE "bank.sec" /* Bank's secret key only */
#define USER_PUBKEY_FILE "user.pub" /* User's public key only */
#define USER_SECKEY_FILE "user.sec" /* User's secret key only */
#define RANDOM_FILE "rand.dat" /* Randomizer file */
#define BANK_SPENT_FILE "spent.dat" /* Spent coin list */
#define BANK_OLDSPENT_FILE "spent.old" /* Old spent list */
#define BANK_MSG_FILE "msg.txt" /* Message from the bank */
#define BANK_LOG_FILE "log.txt" /* Error log */
#define COIN_NAME_FILE "coinname.txt" /* File which stores coin name */
#define COIN_OUT_FILE "coins.dat" /* Extracted/minted coins */
#define ELIST_FILE "elist.dat" /* List of e's and values */
#define ELIST_OLD_FILE "elist.old" /* Old elist */
#define DLIST_FILE "dlist.dat" /* Server's list of d's */
#define DLIST_OLD_FILE "dlist.old" /* Old dlist */
#define USER_OUTPUT_FILE "output.asc" /* User's output */
#define USER_PCOIN_FILE "proto.dat" /* Proto-coins awaiting signing */
#define USER_COIN_FILE "allcoins.dat" /* User's bank account */
#define MM_REGCOIN 0xC0 /* Regular coin (id,e,c)          */
#define MM_PROCOIN 0xC4 /* Proto-coin (tid,e,bc)          */
#define MM_BLICOIN 0xC8 /* Signed and blinded (tid,e,bc)  */
#define MM_COINNAM 0xCC /* Coin name (string - 0 byte)    */
#define MM_SRVRMSG 0xD0 /* Msg from server (lword-string) */

#define MM_NOELIST 0xD4 /* No new elist                   */
#define MM_NEWELST 0xD8 /* New elist follows              */
#define MM_OLDELST 0xDC /* Old elist follows              */
#define MM_DELOLDE 0xE0 /* Delete old elist               */

#define BUCKETS 2048L /* Buckets in the hash file, must be power of 2 */
#define BUCKETSIZE 16L /* Number of records in each bucket */

/* This will hopefully make it work on big-endian machines. If HIGHFIRST
   is defined and HIGH_KLUDGE is not, the code will use mm_pofs to find
   the proper starting point of mpi's when precision is reduced. This is
   being used on the call to mp_inv (unblinding) and e/d finding. If both
   HIGHFIRST and HIGH_KLUDGE are defined, all set_precisions will be forced
   to max. This will slow it down, but should work if all else fails.
   If HIGHFIRST is not defined, it will compile for little-endian. */

#ifdef HIGHFIRST
#  ifdef HIGH_KLUDGE
#    undef set_precision 
#    define set_precision(i) global_precision=MAX_UNIT_PRECISION
#    define mm_pofs(i) i
#  else /* Highfirst but not kludge */
#    define mm_pofs(i) (i+(MAX_UNIT_PRECISION-global_precision))
#  endif
#else /* Not highfirst */
#  define mm_pofs(i) i
#endif
 
extern byte asn_array[18];

/* Initialize random generator */
void randomize(void);

/* Generate a new proto-coin */
struct mm_protocoin *mm_newpcoin(word32 value,struct mpi *n);

/* Check a coin to see if it is properly formed and 
   as long as the modulus n (must be big-endian swapped) */
int mm_valid_coin(unit *c,struct mpi *n,byte *coinid);

/* Get e given value */
struct mpi *mm_gete(word32 value,char *filename);

/* Get value given e */
word32 mm_getvalue(struct mpi *e,char *filename);

/* Given e, get corresponding d */
struct mpi *mm_getd(struct mpi *e,char *efile,char *dfile);

/* Shows output during key generation */
void gen_output(unsigned status);

/* Truncate a file at a given point */
void mm_truncate_file(byte *filename,word32 pos);

/* Get name of coins */
char *mm_getname(void);

/* Simple one-part armor function */
void mm_armor(struct fifo *i,struct fifo *o,int key);

/* Simple one-part dearmor function */
int mm_dearmor(struct fifo *i,struct fifo *o);

/* Fopen with check */
FILE *mm_safeopen(byte *filename,byte *mode);

struct mm_protocoin { /* A proto-coin, created but not yet signed */
  byte tempid[8]; /* The id used to identify the coin when the bank
		returns it signed. Disposed of after bank signs coin. */
  byte coinid[16]; /* The coin's id, not hashed. Not sent to bank. */
  struct mpi e; /* The bank's e this coin will be signed with */
  struct mpi c; /* The mpi sent to the bank for signing (unblinded) */
  struct mpi bc; /* The blinded coin */
  struct mpi b; /* The blinding factor, used to unblind the signed coin */
  };

struct mm_coin { /* A signed coin, ready for spending */
  byte coinid[16]; /* Coin's id */
  struct mpi e; /* Bank's e */
  struct mpi c; /* Signed coin */
  };

struct mm_elist { /* The e-list record, which matches values with e's */
  struct mpi e;
  word32 value;
  };

