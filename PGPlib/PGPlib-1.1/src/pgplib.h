#undef USE_LONGLONG

/*********************************************************************
 * Filename:      pgplib.h
 * Description:   
 * Author:        Tage Stabell-Kulo <tage@acm.org>
 * Modified at:   Mon Jan 19 09:49:36 1998
 * Modified by:   Tage Stabell-Kuloe <tage@acm.org>
 *
 * 	Copyright (c) University of Tromsø, Norway
 * 	See the file COPYING for details
 *
 * $Id: pgplib.h,v 1.15 1998/01/19 10:08:31 tage Exp $
 *
 *
 ********************************************************************/

#ifndef _PGPLIB_H_
#define _PGPLIB_H_

/* Include the configuration file */
#include <pgplibconf.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>		/* includes sys/types */
#include <errno.h>

/* SSLeay */
#include <bn.h>


/*************************
  These things can (and possibly chould) be configured
  ************************/

/* 
 * armor.c
 */
#define MAXLINE		1024
#define ARMORLINE	64
#define DATALEN	   	(ARMORLINE/4*3)

/* 
 * conventional.c
 */

/* We will not handle packets larger than this */
#define	MAX_ENC_PACKET	(1024*1024)

/*
 * keybase.c
 */
/* Tailor the database, see hash(3); */
#define CACHE_SIZE	(1024*1024)
#define NELEM		(40000)
#ifndef BSIZE
#  define BSIZE		(4096)
#endif

/**************************
  End of configurable 
  *************************/

/* this is a portable view of a pgp key id rather than using 
 * any native type like long long which serious reduces the 
 * portability of the library --tjh
 */
#define PGPKEYID_SIZE 8
#ifndef USE_LONGLONG
typedef struct { 
    unsigned char data[PGPKEYID_SIZE]; 
} PGPKEYID;

typedef unsigned char xPGPKEYID[PGPKEYID_SIZE];

#define PGPKEYID_IsZero(X)	(memcmp(&(X),"\0\0\0\0\0\0\0\0",\
						sizeof(PGPKEYID))==0)
#define PGPKEYID_EQUAL(X,Y)	(memcmp(&(X),&(Y),sizeof(PGPKEYID))==0)
#define PGPKEYID_ZERO(X)	memset(&(X),0,sizeof(PGPKEYID));
#define PGPKEYID_COPY(DST,SRC)	memcpy(&(DST),&(SRC),sizeof(PGPKEYID));

#endif /* !USE_LONGLONG */

#ifdef USE_LONGLONG 
/* this is the "old" view of things ... which is handy to be able to
 * work with if the platform supports it I guess
 */
typedef unsigned long long PGPKEYID;
#define PGPKEYID_IsZero(X)	((X)==0)
#define PGPKEYID_EQUAL(X,Y)	((X)==(Y))
#define PGPKEYID_ZERO(X)	((X)=0)
#define PGPKEYID_COPY(DST,SRC)	(DST)=(SRC)
#endif /* USE_LONGLONG */

extern char *LIB_VERSION;
extern char *PGP_VERSION;
extern char *PGP_COMMENT;


typedef struct {
    long	length;			/* of the original packet */
    u_char	version;
    u_long	created;
    u_short	valid;
    BIGNUM	*N;
    BIGNUM	*E;
    u_char	algorithm;
    u_char	iv[8];
    BIGNUM	*D;
    BIGNUM	*P;
    BIGNUM	*Q;
    BIGNUM	*U;
    u_short	cksum;			/* 16 bit */
} PGPseckey_t;

typedef struct {
    long	length;			/* Of the original packet */
    u_char	version;
    u_long	created;
    u_short	valid;			/* in days */
    u_char	algorithm;
    BIGNUM	*N;
    BIGNUM	*E;
} PGPpubkey_t;
    
typedef struct {
    int		length;			/* of total packet  */
    u_char	version;
    u_char	included;
    u_char	classification;
    u_long	timestamp;
    PGPKEYID	key_id;
    u_char	pk_alg;
    u_char	hash_alg;
    u_char	ck[2];
    BIGNUM	*I;
} PGPsig_t;

/* We null-terminate the name, although PGP does not. */
typedef struct {
    char	*name;
} PGPuserid_t;

typedef struct {
    int		length;
    u_char	mode;
    char 	*filename;
    u_long	timestamp;
    int		datalen;
    u_char	*data;
} PGPliteral_t;

typedef struct {
    u_char	byte;
} PGPtrust_t;

typedef struct {
    int		length;
    u_char	version;
    PGPKEYID	key_id;
    u_char	algorithm;
    BIGNUM	*E;
} PGPpke_t;


/* our file-like i/o functions */

typedef enum { FLIO_FILE, FLIO_BUFF} flio_magic_t;

typedef struct {
    flio_magic_t magic;
    union {
	struct {
	    u_char *b;
	    u_long p;
	    u_long size;
	} buff;
	FILE *f;
    } d;
} flio_t;

typedef struct {
    flio_magic_t magic;
    union {
	fpos_t f;
	long   p;
    } d;
} flio_pos_t;



/* What is returned from buffers containing keyrings ? */
typedef enum {NONE, SIG, PUBKEY, UID, SECKEY, TRUSTBYTE} res_t;

/* How to open a database */
typedef enum {NOMODE, READER, WRITER, NEWDB} openmode_t;

/* What to do with data in databases */
typedef enum {NOACTION, INSERT, REPLACE} repl_t;


/*
 * armor.c
 */

int
dearmor_buffer(char	*armor_buf, 
	       long	armor_len,
	       int	*used,
	       u_char	**buffer,
	       int	*buf_len);

int
dearmor_file(FILE	*fin,
	     FILE	*fout);

int dearmor_flio(flio_t *, flio_t *);

int armor_flio (flio_t *, flio_t *);

int armor_file(FILE *, FILE *);

int
armor_buffer(u_char	*buf,
	     int	buf_len,
	     int	*used,
	     char	**a_buf,
	     int	*a_len);
int 
pemDecode(char 		*inBuffer, 
	  u_char 	*outBuffer, 
	  int 		*outbuflen);
int
pemEncode(u_char 	*inBuffer,  
	  int		inbuflen,
	  char		*outBuffer,
	  int 		*outbuflen);

int 
read_armored_package(FILE 	*fin, 
		     u_char 	**buffer, 
		     int 	*buf_len);
/*
 * conventional.c
 */

int
conventional_key_decrypt(u_char	*enc_p,
			 u_char	digest[16],
			 int	*outlen,
			 u_char	**data);

int
conventional_key_encrypt(u_char	*inbuf,
			 int	inbuflen,
			 u_char digest[16],
			 int	do_compress,
			 int	*outbuflen,
			 u_char	**outbuf);

/*
 * decompress.c
 */

int
decompress_buffer(u_char	*buf,
		  int		buf_len,
		  u_char	**data,
		  int		*data_len);

int
decompress_file(FILE	*fin,
		FILE	*fout);

int
compress_buffer(u_char	*inbuf,
		int	inbuflen,
		u_char	**outbuf,
		int	*outbuflen);


/*
 * literal.c
 */
void
free_literal(PGPliteral_t *literal);

int
file2literal(FILE		*fin,
	     PGPliteral_t	*lit);
int
buf2literal(u_char	*inbuf,
	    int  	inbuflen,
	    PGPliteral_t *literal);
int
literal2buf(PGPliteral_t	*literal,
	       u_char		**buf);
int
create_literal(char		*filename,
	       PGPliteral_t	*literal);

/*
 * keybase.c
 */

extern int dbase_open;
typedef struct {
    res_t		what;
    union {
	PGPpubkey_t	*key;
	PGPsig_t	*sig;
	PGPuserid_t	*uid;
	PGPtrust_t	*trust;
    } u;
} keyent_t;

int
keydb_init(char		*caller_name,
	   openmode_t	caller_mode);
void
keydb_end(void);

void
keydb_sync(void);

int
keys_from_string(char	*string,
		 int	save_case,
		 int	*num_keys,
		 PGPKEYID  **keys);
int
find_shortid(char 	*short_id,
	     PGPKEYID	*keyid);

int	
store_pubkey(PGPpubkey_t 	*pubkey, 
	     repl_t 		replace);
int
key_exists(PGPKEYID	key_id);

int
get_pubkey_rec(PGPKEYID	key_id,
	       int	*len,
	       u_char 	**buf);

int
store_pubkey_rec(PGPKEYID 	key_id, 
		 repl_t 	replace, 
		 u_char 	*buf, 
		 int 	size);

int
store_uid(PGPuserid_t 	*user_id, 
	  PGPKEYID 		key_id);


/* higer level routines */
int
add_uid(PGPKEYID 	key_id,
	PGPuserid_t	*user_id);

int
add_sig(PGPKEYID	key_id,
	PGPuserid_t	*uid,
	PGPsig_t	*sig);

int
get_only_pubkey(PGPKEYID 	key_id,
		PGPpubkey_t 	**key);

int
get_keycontents(PGPKEYID	key_id,
		keyent_t	***arr);

int
get_keycontents_seq(keyent_t ***keyent, int where);

int
uids_from_keyid(PGPKEYID	key_id,
		PGPuserid_t	***uidpp);

int 
add_trust(PGPKEYID, PGPuserid_t *, PGPtrust_t *);

int
scan_pubring(FILE *fp);

/*
 * Keymgnt.c
 */


int
buf2seckey(u_char	*buf,
	   int		buflen,
	   PGPseckey_t	*key);

void
free_pubkey(PGPpubkey_t	*key);

int
copy_pubkey(PGPpubkey_t *in,
	    PGPpubkey_t *out);

void
free_keyrecords(keyent_t	**records);

int 
pub_fingerprint(PGPpubkey_t	*key,
		   char		*str);

int
print_keyrec(u_char		*buffer, 
	     int 		buf_len, 
	     FILE 		*fd);
int
pubkey2buf(PGPpubkey_t	*key,
	   u_char 	**buf);

int
buf2pubkey(	u_char 		*buf, 
		int 		len, 
		PGPpubkey_t 	*key);
int
decrypt_seckey(	PGPseckey_t 	*old, 
		u_char 		*digest,
		PGPseckey_t	*new);

void
print_seckey(int	verbose,
	     FILE 	*fd,
	     PGPseckey_t *key);

void
free_seckey(PGPseckey_t		*key);

void
clear_seckey(PGPseckey_t	*key);

/*
 * keyring.c
 */

int
get_keyentry_from_buf(u_char		*buf,
		      int		buf_len,
		      res_t		*what,
		      PGPsig_t		**sig,
		      PGPpubkey_t	**pubkey,
		      PGPuserid_t	**uid,
		      PGPseckey_t	**seckey,
		      PGPtrust_t	**trust,
		      int		*used);
int
get_seckey_from_buf(u_char		*buf, 
		    int 		buflen, 
		    PGPseckey_t 	**key);
int
get_seckey_from_file(	FILE 		*keyring, 
			PGPseckey_t 	**key);

int
get_keyentry_from_file(	FILE		*fd,
			res_t		*what,
			PGPsig_t	**sig,
			PGPpubkey_t	**pubkey,
			PGPuserid_t	**uid,
			PGPseckey_t	**seckey,
			PGPtrust_t	**trust);

/*
 * misc.c
 */

int
bufgets(u_char 	*line, 
	int 	linelen, 
	u_char 	*buf, 
	int buflen);

int
make_ctb(u_char		ctb_type, 
	 u_long 	length, 
	 int 		force, 
	 u_char 	*buf);


int
PGPKEYID_FromBytes(PGPKEYID 	*key_id, 
		   u_char 	*buf);

int
PGPKEYID_ToBytes(PGPKEYID *, 
		 u_char *);

int 
PGPKEYID_FromBN(PGPKEYID *key_id,
		BIGNUM *a);

u_short 
checksum(u_char	*, int);

int
mpi2bignum(u_char	*buf,
	   int		buflen,
	   int		*used,
	   BIGNUM	**N);

int 
ctb_llength(int ctb);


int 
PGPKEYID_ExtractShort(PGPKEYID *key_id,
		      unsigned long *short_id);
int 
PGPKEYID_FromAscii(char *str,
		   PGPKEYID *key_id);
int 
PGPKEYID_ToAscii(PGPKEYID *key_id,
		 char *buf,
		 int maxsize);
char *
PGPKEYID_ToStr(PGPKEYID *key_id);

/*
 * packet.c
 */

int
read_packet(FILE	*fd,
	    u_char	**packet);

int
packet_length(u_char	*p);

int
find_packet_in_buffer(u_char	*buffer,
		      int	buf_len,
		      u_char	**packet);

/*
 * pke.c
 */

int
buf2pke (u_char		*buffer,
	 int		length,
	 PGPpke_t	*pke);

void
free_pke(PGPpke_t	*pke);


/*
 * pubencrypt.c
 */

int
pub_encrypt_buffer(PGPpubkey_t	*key,
		   u_char	*inbuf,
		   int		inbuflen,
		   int		do_compress,
		   u_char	digest[16],
		   int		*outbuflen,
		   u_char 	**outbuf);

int
pub_encrypt_session_key(PGPpubkey_t	*key,
                        u_char		*skey,
			int		*outbuflen,
			u_char		**outbuf);


/*
 * pubdecrypt.c
 */

int
sec_decrypt_packet(PGPseckey_t 	*key,
		   u_char 	*PKE_packet,
		   int 		pke_l,
		   u_char 	*conv_packet,
		   int 		conv_l,
		   u_char 	**outbuf,
		   int 		*outlen);

/*
 * secencrypt
 */

void
free_signature(PGPsig_t *sig);

int
signature2buf(PGPsig_t	*sig, 
	      u_char	**buf);

int
buf2signature(u_char 	*buf, 
	      int 	len, 
	      PGPsig_t 	*sig);

int
add_sig_to_key(PGPKEYID	key_id, 
	       PGPsig_t *sig);

int
check_signature(u_char 	CBT, 
		u_char 	*file_ptr, 
		int 	*length, 
		u_char 	**data);

int
verify_sig(u_char	*data,
	   int		dlen,
	   PGPsig_t	*sig,
	   PGPpubkey_t	*signkey);

int
verify_keysig(	PGPpubkey_t	*pubkey,
		PGPuserid_t	*uid,
		PGPsig_t	*sig,
		PGPpubkey_t	*signkey);
int
sign_buffer(	u_char		*buffer, 
		int 		bufsize,
		PGPseckey_t	*key, 
		PGPsig_t 	*sig);
int
make_sig(u_char		*buffer, 
	 int 		bufsize,
	 u_char		class,
	 PGPseckey_t	*key, 
	 PGPsig_t 	*sig);

int is_compromised(PGPKEYID);

/*
 * trust.c
 */
int
trust2buf(PGPtrust_t	*trust,
	  u_char	**buf);
int
buf2trust(u_char 	*packet,
	  int 		p_len,
	  PGPtrust_t 	*trust);

int
purge_trust(u_char *, long, long *);

/*
 * userid.c
 */

void
free_uid(PGPuserid_t	*uid);

int
uid2buf(PGPuserid_t	*uid,
	u_char		**buf);

int
buf2uid(u_char 	*buf, 
	int		length, 
	PGPuserid_t	*uid);


/*
 * error reporting, written by Thomas Roessler <roessler@guug.de>
 */

#ifndef _ERROR_C
extern const char *pgplib_errlist[];
extern int pgplib_errno;
#endif

#define PGPLIB_ERROR_SYSERR -1
#define PGPLIB_ERROR_OK 0
#define PGPLIB_ERROR_BAD_CTB_LENGTH 1
#define PGPLIB_ERROR_BAD_CTB 2
#define PGPLIB_ERROR_EINVAL 3
#define PGPLIB_ERROR_COMPRESSED_CTB 4
#define PGPLIB_ERROR_EOF 5
#define PGPLIB_ERROR_IMPOSSIBLE 6
#define PGPLIB_ERROR_BAD_PACKET_LENGTH 7
#define PGPLIB_ERROR_BADCRC 8
#define PGPLIB_ERROR_BAD_ALGORITHM 9
#define PGPLIB_ERROR_ZIP 10
#define PGPLIB_ERROR_WRONG_PASS 11
#define PGPLIB_ERROR_DB_FATAL 12
#define PGPLIB_ERROR_DB_NOT_OPEN 13

#define PGPLIB_ERROR_MAX 13

const char *pgplib_strerror(int);
/* For compatibility */
char * strerror(int num);


void flio_close(flio_t *p);
flio_t *flio_fopen(const char *fname, const char *mode);
flio_t *flio_newbuff(void);
int flio_getpos(flio_t *p, flio_pos_t *pos);
int flio_setpos(flio_t *p, flio_pos_t *pos);
int flio_getc(flio_t *p);
char *flio_gets(char *b, u_long s, flio_t *p);
int flio_putc(int c, flio_t *p);
int flio_puts(const char *s, flio_t *p);
int flio_write(void *ptr, u_long s, u_long n, flio_t *p);


/* --------- END OF PGPLIB ------- */

/**************************************************************************
 *
 * Pretty Good(tm) Privacy - RSA public key cryptography for the masses
 * Written by Philip Zimmermann, Phil's Pretty Good(tm) Software.
 * Version 1.0 - 5 Jun 91, last revised 6 Jul 91 by PRZ
 * 
 * This file defines the various formats, filenames, and general control
 * methods used by PGP, as well as a few global switches which control
 * the functioning of the driver code.
 *
 */


/* **************************************************************
 * Tage Stabell-Kulø copied this file from the PGP sources.  Large parts
 * has been removed, the remaining contents is unaltered.
 * ************************************************************** */


#define KEYFRAGSIZE 8	/* # of bytes in key ID modulus fragment */
#define SIZEOF_TIMESTAMP 4 /* 32-bit timestamp */

/*
**********************************************************************
*/

/* Cipher Type Byte (CTB) definitions follow...*/
#define CTB_DESIGNATOR 0x80
#define is_ctb(c) (((c) & CTB_DESIGNATOR)==CTB_DESIGNATOR)
#define CTB_TYPE_MASK 0x7c
#define CTB_LLEN_MASK 0x03

#define is_ctb_type(ctb,type) (((ctb) & CTB_TYPE_MASK)==(4*type))
#define CTB_BYTE(type,llen) (CTB_DESIGNATOR + (4*type) + llen)

#define CTB_PKE_TYPE 1			/* packet encrypted with RSA public
					   key */
#define CTB_SKE_TYPE 2			/* packet signed with RSA secret key */
#define CTB_MD_TYPE 3			/* message digest packet */
#define CTB_CERT_SECKEY_TYPE 5  /* secret key certificate */
#define CTB_CERT_PUBKEY_TYPE 6  /* public key certificate */
#define CTB_COMPRESSED_TYPE 8	/* compressed data packet */
#define CTB_CKE_TYPE 9			/* conventional-key-encrypted data */
#define	CTB_LITERAL_TYPE 10		/* raw data with filename and mode */
#define CTB_LITERAL2_TYPE 11	/* Fixed literal packet */
#define CTB_KEYCTRL_TYPE 12		/* key control packet */
#define CTB_USERID_TYPE 13		/* user id packet */
#define CTB_COMMENT_TYPE 14		/* comment packet */

/* Unimplemented CTB packet types follow... */
/* #define CTB_EXTENDED_TYPE 15 */ /* 2-byte CTB, 256 extra CTB types */

#define CTB_PKE CTB_BYTE(CTB_PKE_TYPE,1)
	/* CTB_PKE len16 keyID mpi(RSA(CONKEYPKT)) */
	/*	  1		 2	 SIZE  countbytes()+2 */
#define CTB_SKE CTB_BYTE(CTB_SKE_TYPE,1)
	/* CTB_SKE len16 keyID mpi(RSA(MDPKT)) */
	/*	  1		 2	 SIZE  countbytes()+2 */
#define CTB_MD CTB_BYTE(CTB_MD_TYPE,0)
	/* CTB_MD len8 algorithm MD timestamp */
#define CTB_CERT_SECKEY CTB_BYTE(CTB_CERT_SECKEY_TYPE,1)
	/* CTB_CERT_SECKEY len16 timestamp userID mpi(n) mpi(e) mpi(d)
	   mpi(p) mpi(q) mpi(u) crc16 */
#define CTB_CERT_PUBKEY CTB_BYTE(CTB_CERT_PUBKEY_TYPE,1)
	/* CTB_CERT_PUBKEY len16 timestamp userID mpi(n) mpi(e) crc16 */

#define CTB_KEYCTRL CTB_BYTE(CTB_KEYCTRL_TYPE,0)
#define	CTB_USERID	CTB_BYTE(CTB_USERID_TYPE,0)

#define CTB_CKE CTB_BYTE(CTB_CKE_TYPE,3)
	/*	CTB_CKE ciphertext */

#define CTB_LITERAL CTB_BYTE(CTB_LITERAL_TYPE,3)
#define CTB_LITERAL2 CTB_BYTE(CTB_LITERAL_TYPE,3)
	/*	CTB_LITERAL data */

#define CTB_COMPRESSED CTB_BYTE(CTB_COMPRESSED_TYPE,3)
	/*	CTB_COMPRESSED compressedtext */

/*	Public key encryption algorithm selector bytes. */
#define RSA_ALGORITHM_BYTE	1	/*	use RSA	*/

/*	Conventional encryption algorithm selector bytes. */
#define IDEA_ALGORITHM_BYTE	1	/*	use the IDEA cipher */

/*	Message digest algorithm selector bytes. */
#define MD5_ALGORITHM_BYTE 1	/* MD5 message digest algorithm */

/*	Data compression algorithm selector bytes. */
#define ZIP2_ALGORITHM_BYTE  1	/* Zip-based deflate compression algorithm */

/* Signature classification bytes. */
#define SB_SIGNATURE_BYTE	0x00	/* Signature of a binary msg or doc */
#define SM_SIGNATURE_BYTE       0x01    /* Signature of canonical msg or doc */
#define K0_SIGNATURE_BYTE       0x10    /* Key certification, generic */
#define K1_SIGNATURE_BYTE       0x11    /* Key certification, persona */
#define K2_SIGNATURE_BYTE       0x12    /* Key certification, casual ID */
#define K3_SIGNATURE_BYTE       0x13    /* Key certification, positive ID */
#define KC_SIGNATURE_BYTE       0x20    /* Key compromise */
#define KR_SIGNATURE_BYTE       0x30    /* ID or Certification revocation */
#define TS_SIGNATURE_BYTE       0x40    /* Timestamp someone else's */

/* Public key encrypted data classification bytes. */
#define MD_ENCRYPTED_BYTE	1	/* Message digest is encrypted */
#define CK_ENCRYPTED_BYTE	2	/* Conventional key is encrypted */

/* Version byte for data structures created by this version of PGP */
#define	VERSION_BYTE_OLD	2	/* PGP2 */
#define	VERSION_BYTE_NEW	3

#define is_secret_key(ctb) is_ctb_type(ctb,CTB_CERT_SECKEY_TYPE)

/* Modes for CTB_LITERAL2 packet */
#define	MODE_BINARY	'b'
#define	MODE_TEXT	't'
#define MODE_LOCAL	'l'


#endif /* PGPLIB_H */

