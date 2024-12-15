/*____________________________________________________________________________
    pgp.h

    Copyright(C) 1998,1999 Network Associates, Inc.
    All rights reserved.

	PGP 6.5 Command Line 

    Based on the original code by Philip Zimmermann

    $Id: pgp.h,v 1.6 1999/05/12 21:01:05 sluu Exp $
____________________________________________________________________________*/


#ifndef PGP_H
#define PGP_H

#include "config.h"
#include "prototypes.h"

#define KEYFRAGSIZE 8 /* # of bytes in key ID modulus fragment */
#define SIZEOF_TIMESTAMP 4 /* 32-bit timestamp */


#ifdef ATARI
#define sizeof(x) (int)sizeof(x)
#define fread(a,b,c,d) ((int)fread(a,b,c,d))
#endif

/*
**********************************************************************
*/

/* Cipher Type Byte (CTB) definitions follow...*/
#define CTB_DESIGNATOR 0x80
#define is_ctb(c) (((c) & CTB_DESIGNATOR)==CTB_DESIGNATOR)
#define CTB_TYPE_MASK 0x7c
#define CTB_LLEN_MASK 0x03

/* "length of length" field of packet, in bytes (1, 2, 4, 8 bytes): */
#define ctb_llength(ctb) ((int) 1 << (int) ((ctb) & CTB_LLEN_MASK))

#define is_ctb_type(ctb,type) (((ctb) & CTB_TYPE_MASK)==(4*type))
#define CTB_BYTE(type,llen) (CTB_DESIGNATOR + (4*type) + llen)

#define CTB_PKE_TYPE 1   /* packet encrypted with RSA public key */
#define CTB_SKE_TYPE 2   /* packet signed with RSA secret key */
#define CTB_MD_TYPE 3   /* message digest packet */
#define CTB_CERT_SECKEY_TYPE 5  /* secret key certificate */
#define CTB_CERT_PUBKEY_TYPE 6  /* public key certificate */
#define CTB_COMPRESSED_TYPE 8 /* compressed data packet */
#define CTB_CKE_TYPE 9   /* conventional-key-encrypted data */
#define CTB_LITERAL_TYPE 10  /* raw data with filename and mode */
#define CTB_LITERAL2_TYPE 11 /* Fixed literal packet */
#define CTB_KEYCTRL_TYPE 12  /* key control packet */
#define CTB_USERID_TYPE 13  /* user id packet */
#define CTB_COMMENT_TYPE 14  /* comment packet */

/* Unimplemented CTB packet types follow... */
/* #define CTB_EXTENDED_TYPE 15 */ /* 2-byte CTB, 256 extra CTB types */

#define CTB_PKE CTB_BYTE(CTB_PKE_TYPE,1)
 /* CTB_PKE len16 keyID mpi(RSA(CONKEYPKT)) */
 /*   1   2  SIZE  countbytes()+2 */
#define CTB_SKE CTB_BYTE(CTB_SKE_TYPE,1)
 /* CTB_SKE len16 keyID mpi(RSA(MDPKT)) */
 /*   1   2  SIZE  countbytes()+2 */
#define CTB_MD CTB_BYTE(CTB_MD_TYPE,0)
 /* CTB_MD len8 algorithm MD timestamp */
#define CTB_CERT_SECKEY CTB_BYTE(CTB_CERT_SECKEY_TYPE,1)
 /* CTB_CERT_SECKEY len16 timestamp userID mpi(n) mpi(e) mpi(d)
    mpi(p) mpi(q) mpi(u) crc16 */
#define CTB_CERT_PUBKEY CTB_BYTE(CTB_CERT_PUBKEY_TYPE,1)
 /* CTB_CERT_PUBKEY len16 timestamp userID mpi(n) mpi(e) crc16 */

#define CTB_KEYCTRL CTB_BYTE(CTB_KEYCTRL_TYPE,0)
#define CTB_USERID CTB_BYTE(CTB_USERID_TYPE,0)

#define CTB_CKE CTB_BYTE(CTB_CKE_TYPE,3)
 /* CTB_CKE ciphertext */

#define CTB_LITERAL CTB_BYTE(CTB_LITERAL_TYPE,3)
#define CTB_LITERAL2 CTB_BYTE(CTB_LITERAL_TYPE,3)
 /* CTB_LITERAL data */

#define CTB_COMPRESSED CTB_BYTE(CTB_COMPRESSED_TYPE,3)
 /* CTB_COMPRESSED compressedtext */

/* Public key encryption algorithm selector bytes. */
#define RSA_ALGORITHM_BYTE 1 /* use RSA */

/* Conventional encryption algorithm selector bytes. */
#define IDEA_ALGORITHM_BYTE 1 /* use the IDEA cipher */

/* Message digest algorithm selector bytes. */
#define MD5_ALGORITHM_BYTE 1 /* MD5 message digest algorithm */

/* Data compression algorithm selector bytes. */
#define ZIP2_ALGORITHM_BYTE  1 /* Zip-based deflate compression algorithm */

/* Signature classification bytes. */
#define SB_SIGNATURE_BYTE 0x00 /* Signature of a binary msg or doc */
#define SM_SIGNATURE_BYTE 0x01 /* Signature of canonical msg or doc */
#define K0_SIGNATURE_BYTE 0x10 /* Key certification, generic */
#define K1_SIGNATURE_BYTE 0x11 /* Key certification, persona */
#define K2_SIGNATURE_BYTE 0x12 /* Key certification, casual ID */
#define K3_SIGNATURE_BYTE 0x13 /* Key certification, positive ID */
#define KC_SIGNATURE_BYTE 0x20 /* Key compromise */
#define KR_SIGNATURE_BYTE 0x30 /* Key revocation */
#define TS_SIGNATURE_BYTE 0x40 /* Timestamp someone else's signature */

/* Public key encrypted data classification bytes. */
#define MD_ENCRYPTED_BYTE 1 /* Message digest is encrypted */
#define CK_ENCRYPTED_BYTE 2 /* Conventional key is encrypted */

/* Version byte for data structures created by this version of PGP */
#define VERSION_BYTE_OLD 2 /* PGP2 */
#define VERSION_BYTE_NEW 3

/* Values for trust bits in keycntrl packet after key packet */
#define KC_OWNERTRUST_MASK 0x07 /* Trust bits for key owner */
#define KC_OWNERTRUST_UNDEFINED 0x00
#define KC_OWNERTRUST_UNKNOWN 0x01
#define KC_OWNERTRUST_NEVER 0x02
/* 2 levels reserved */
#define KC_OWNERTRUST_USUALLY 0x05
#define KC_OWNERTRUST_ALWAYS 0x06
#define KC_OWNERTRUST_ULTIMATE 0x07 /* Only for keys in secret ring */
#define KC_BUCKSTOP  0x80 /* This key is in secret ring */
#define KC_DISABLED  0x20 /* key is disabled */

/* Values for trust bits in keycntrl packet after userid packet */
#define KC_LEGIT_MASK  0x03 /* Key legit bits for key */
#define KC_LEGIT_UNKNOWN 0x00
#define KC_LEGIT_UNTRUSTED 0x01
#define KC_LEGIT_MARGINAL 0x02
#define KC_LEGIT_COMPLETE 0x03
#define KC_WARNONLY  0x80

/* Values for trust bits in keycntrl packet after signature packet */
#define KC_SIGTRUST_MASK 0x07 /* Trust bits for key owner */
#define KC_SIGTRUST_UNDEFINED 0x00
#define KC_SIGTRUST_UNKNOWN 0x01
#define KC_SIGTRUST_UNTRUSTED 0x02
/* 2 levels reserved */
#define KC_SIGTRUST_MARGINAL 0x05
#define KC_SIGTRUST_COMPLETE 0x06
#define KC_SIGTRUST_ULTIMATE 0x07
#define KC_SIG_CHECKED  0x40 /* This sig has been checked */
#define KC_CONTIG  0x80 /* This sig is on a cert. path */

#define is_secret_key(ctb) is_ctb_type(ctb,CTB_CERT_SECKEY_TYPE)

#define MPILEN (2+MAX_BYTE_PRECISION)
#define MAX_SIGCERT_LENGTH (1+2+1 +1+7 +KEYFRAGSIZE+2+2+MPILEN)
#define MAX_KEYCERT_LENGTH (1+2+1+4+2+1 +(2*MPILEN) +1+8 +(4*MPILEN) +2)

/* Modes for CTB_LITERAL2 packet */
#define MODE_BINARY 'b'
#define MODE_TEXT 't'
#define MODE_LOCAL 'l'

/* Prototype for the 'more' function, which blorts a file to the screen with
   page breaks, intelligent handling of line terminators, truncation of
   overly long lines, and zapping of illegal chars.  Implemented in MORE.C */

/*int moreFile(char *fileName);*/

/* Prototypes for the transport armor routines */

/*PGPBoolean isArmorFile(char *infile, long startline);*/

struct pgpfileBones; /* see globals.h */
void userError(struct pgpfileBones *filebPtr, int *perrorLvl);

extern int compressSignature(PGPByte *header, char **compressName,
        char **compressExt);

#endif /* PGP_H */