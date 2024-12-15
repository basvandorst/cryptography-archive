/* pgptools.h */

/* These are the packet types, with the 2 lsb's (size) zeroed out */
#define PGP_PKE 0x84  /* Public-key encrypted */
#define PGP_SIG 0x88  /* Signature            */
#define PGP_SK  0x94  /* Secret key           */
#define PGP_PK  0x98  /* Public key           */
#define PGP_CMP 0xA0  /* Compressed data      */
#define PGP_CKE 0xA4  /* IDEA encrypted       */
#define PGP_PT  0xAC  /* Plaintext with mode  */
#define PGP_TRU 0xB0  /* Keyring trust        */
#define PGP_UID 0xB4  /* Keyring user ID      */
#define PGP_COM 0xB8  /* Keyring comment      */

typedef unit unitarr[MAX_UNIT_PRECISION];

struct mpi {
  unitarr value;
  unsigned bits;
  unsigned bytes;
  };

struct pgp_pubkey {
  byte keyid[8];
  time_t timestamp;
  word16 validity;
  struct mpi n;
  struct mpi e;
  };

struct pgp_seckey {
  byte iv[8];
  struct mpi d;
  struct mpi p;
  struct mpi q;
  struct mpi u;
  word16 checksum;
  };

/* Get type of next packet */
#define pgp_nexttype(i) (fifo_rget(0,i)&0xFC)

/* Endian swap */
#ifdef HIGHFIRST
# define endian(a,b)
#else
#ifndef NON_ANSI
void endian(void *data,unsigned length);
#else
void endian();
#endif
#endif

#ifndef NON_ANSI
/* Look at a packet; return its length and type, setting packet to
   start of data (after type/length) */
void pgp_examine_packet(struct fifo *f,byte *type,word32 *length);

/* Initializes the random number generator. Input fifo provides random
   data and is destroyed. */
void pgp_initrand(struct fifo *f);

/* Returns a random byte */
byte pgp_randombyte();

/* Create a literal packet, destroys input */
void pgp_create_literal(struct fifo *i,struct fifo *o,int text,
				char *filename,time_t timestamp);

/* Extract data from a literal packet */
void pgp_extract_literal(struct fifo *i,struct fifo *o,int *text,
				char *filename,time_t *timestamp);

/* Convert text formats: canonical to produce crlf form, otherwise
   produces lf form (or local if file opened ascii)
   stripspaces true removes trailing spaces after the end of a line
   destroys input fifo */
void pgp_textform(struct fifo *i,struct fifo *o,
		  int canonical,int stripspaces);

/* Calculate the MD5 hash of a fifo (destroys fifo) */
void pgp_md5(struct fifo *i,MD5_CTX *md5ctx);

/* Create a conventionally encrypted (IDEA) data packet (uses random) */
void pgp_create_idea(struct fifo *i,struct fifo *o,
		     byte key[16]);

/* Extract data from a conventionally encrypted (IDEA) data packet
   Returns TRUE if decryption successful, FALSE if wrong key
   Consumes packet in either case, leaving input at next packet */
int pgp_extract_idea(struct fifo *i,struct fifo *o,
		     byte key[16]);

/* Calculates bits and bytes for an mpi */
void pgp_fix_mpi(struct mpi *n);

/* Write an MPI to a fifo from an mpi struct */
void pgp_create_mpi(struct fifo *f,struct mpi *n);

/* Retrieve an MPI from a packet and put it in an mpi struct */
void pgp_extract_mpi(struct fifo *f,struct mpi *n);

/* Look for specific packet types */
/* Searches for up to 3 types; returns type or 0 if end of fifo */
byte pgp_find_packet(struct fifo *f,byte a,
			      byte b,byte c);

/* Updates a 16 bit checksum; used to check secret keys */
void pgp_checksum(void *buf,unsigned length,word16 *checksum);

/* Decrypts an encrypted secret key, returns TRUE if successful */
int pgp_decrypt_sk(struct pgp_seckey *sk,byte key[16]);

/* Create an RSA-encrypted packet (uses random) */
void pgp_create_rsa(struct fifo *f,byte ideakey[16],
		    struct pgp_pubkey *pk);

/* Create an RSA signature packet */
void pgp_create_sig(struct fifo *f,byte hash[16],
     time_t timestamp,byte sigtype,
     struct pgp_pubkey *pk,struct pgp_seckey *sk);

/* Returns the 64-bit keyid needed to process an RSA-encrypted or
   signature packet, or the keyid of a public or secret key. Does not
   alter the input fifo */
void pgp_get_keyid(struct fifo *f,byte keyid[8]);

/* Decrypts and extracts the key from an RSA-encrypted block */
/* Returns true if successful, false if not */
int pgp_extract_rsa(struct fifo *f,byte ideakey[16],
     struct pgp_pubkey *pk,struct pgp_seckey *sk);

/* Decrypts and extracts the MD from a signature packet */
/* Returns true if successful, false if not */
int pgp_extract_sig(struct fifo *f,byte hash[16],
    time_t *timestamp,byte *sigtype,
    struct pgp_pubkey *pk);

/* These are the packet errors from pgp_check_packets */
#define PGP_BAD_LENGTH 1     /* Packet length bad */
#define PGP_BAD_VERSION 2    /* Packet created by new version */
#define PGP_BAD_TYPE 3       /* Unknown packet type */

/* This checks a series of packets for any problems. If there aren't
   any, it returns FALSE (0). If there are problems, it returns
   one of the PGP_BAD_* errors. If offset is a null pointer, it will
   be ignored; if not, it will return the offset (first packet is 0,
   next is 1, etc) of the bad packet. If type is non-null, it will
   return the type of the bad packet. This function uses random reads
   and does not alter the input fifo */
int pgp_check_packets(struct fifo *f,unsigned *offsetp,unsigned *typep);
#else
void pgp_examine_packet();
void pgp_initrand();
byte pgp_randombyte();
void pgp_create_literal();
void pgp_extract_litereal();
void pgp_textform();
void pgp_md5();
void pgp_create_idea();
int pgp_extract_idea();
void pgp_fix_mpi();
void pgp_create_mpi();
void pgp_extract_mpi();
byte pgp_find_packet();
void pgp_checksum();
int pgp_decrypt_sk();
void pgp_create_rsa();
void pgp_create_sig();
void pgp_get_keyid();
int pgp_extract_rsa();
int pgp_extract_sig();

/* These are the packet errors from pgp_check_packets */
#define PGP_BAD_LENGTH 1     /* Packet length bad */
#define PGP_BAD_VERSION 2    /* Packet created by new version */
#define PGP_BAD_TYPE 3       /* Unknown packet type */

int pgp_check_packets();
#endif

