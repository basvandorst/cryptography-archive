/*
* pgpSig.c
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpSig.c,v 1.4.2.2 1997/06/07 09:51:31 mhw Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpDebug.h"
#include "pgpPktList.h"
#include "pgpSig.h"
#include "pgpAnnotate.h"
#include "pgpHash.h"
#include "pgpErr.h"
#include "pgpPubKey.h"
#include "pgpUsuals.h"

#define SIGVERSION_1PASS (PGPVERSION_2_6 + 1)
#define SIGBUF_1PASS(b,l) ((l) == 13)
#define SIGPKT_1PASS(p) SIGBUF_1PASS((p)->pkt.buf,(p)->pkt.len)


struct PgpSig {
	struct PktList pkt;
};

/*
* Format of one-pass signature packets:
*
*	Offset Length Meaning
*	0	1	Version byte (=3)
*	1	1	Signature type
*	2	1	Hash Algorithm
*	3	1	PK Algorithm
*	4	8	KeyID
*	12	1	nested flag
*
*
* Format of regular signature packets:
*
*	Offset Length Meaning
*	0 1 Version byte (= 2 or 3).
*	1 1 x, Length of following material included in MD5 (=5)
*	2 1 Signature type (=0 or 1)
*	3 4 32-bit timestamp of signature
* -------- MD5 additional material stops here, at offset 2+x ---------
*	2+x	8	KeyID
*	10+x	1	PK algorithm type (1 = RSA)
*	11+x	1	MD algorithm type (1 = MD5)
*	12+x	2	First 2 bytes of message digest (16-bit checksum)
*	14+x	2+y	MPI of PK-encrypted integer
*	16+x+y
*/
static int
sigValidate(byte const *sig, size_t len, byte *pkalg)
{
	unsigned extra, bits;
	byte alg;
	struct PgpHash const *hash;

	if (pkalg)
		*pkalg = 0;
	if (len < 1)
		return PGPERR_SIG_TOOSHORT;
	if (SIGBUF_1PASS(sig, len)) {
		/* One-pass signature packet */
		if (len < 13)
			return PGPERR_SIG_TOOSHORT;
		if (len > 13)
			return PGPERR_SIG_TOOLONG;
		alg = sig[3];
		if (pkalg)
			*pkalg = alg;
		hash = pgpHashByNumber(sig[2]);
		if (!hash)
			return PGPERR_BAD_HASHNUM;
		return 0;
	} else if (sig[0] == PGPVERSION_3) {
		/* New-format key signature packet - validated in pgpRngPars.c */
		return 0;
	}
	if (sig[0] != PGPVERSION_2 && sig[0] != PGPVERSION_2_6)
		return PGPERR_SIG_BADVERSION;
	if (len < 2)
		return PGPERR_SIG_TOOSHORT;
	extra = sig[1];
	if (len < 11+extra)
		return PGPERR_SIG_TOOSHORT;
	alg = sig[10+extra];
	if (pkalg)
		*pkalg = alg;
#if 0
	if (alg != PGP_PKALG_RSA)
		return PGPERR_SIG_BADALGORITHM;
#endif
	hash = pgpHashByNumber(sig[11+extra]);
	if (!hash)
		return PGPERR_BAD_HASHNUM;
	/* This part here gets RSA-specific */
	if (len < 16+extra)
		return PGPERR_SIG_TOOSHORT;
	if (alg==PGP_PKALG_RSA) {
		bits = ((unsigned)sig[14+extra]<<8) + sig[15+extra];
		if (len != 16+extra+(bits+7)/8)
			return len < 16+extra+(bits+7)/8 ? PGPERR_SIG_TOOSHORT
							: PGPERR_SIG_TOOLONG;
		if (bits && sig[16+extra] >> ((bits-1) & 7) != 1)
			return PGPERR_SIG_BITSWRONG;
	}
	return 0;
}

static struct PgpSig **
sigListTail(struct PgpSig **head)
{
	while (*head)
		head = (struct PgpSig **)&(*head)->pkt.next;
	return head;
}

int
pgpSigSigType (byte const *buf, size_t len)
{
	int i;

	i = sigValidate(buf, len, NULL);
	if (i < 0)
		return i;
	if (SIGBUF_1PASS(buf, len)) {
		return buf[1];
	} else if (buf[0] == PGPVERSION_3) {
		return buf[1];
	}
	if (buf[1] < 1)
		return PGPERR_SIG_BADEXTRA;
	return buf[2] & 255;
}

int
pgpSigAdd (struct PgpSig **siglist, int type, byte const *buf, size_t len)
{
	struct PgpSig *sig;
	struct PgpSig **psig2;
	byte pkalg;
	byte *b2;
	int err;

	switch (type) {
	case PGPANN_SIGNED_SIG:
		err = sigValidate (buf, len, &pkalg);
		if (err)
			return err;
		sig = (struct PgpSig *)pgpPktListNew(pkalg, buf, len);
		if (!sig)
			return PGPERR_NOMEM;
		*sigListTail(siglist) = sig;
		return 0;
	case PGPANN_SIGNED_SIG2:
		/* Second packet, should be merged with an existing one */
		if (SIGBUF_1PASS(buf, len))
			return PGPERR_SIG_BADTYPE;
		if (buf[1] != 5) /* extra bytes must be standard */
			return PGPERR_SIG_BADTYPE;
		err = sigValidate (buf, len, &pkalg);
		if (err)
			return err;
		/* Search for matching 1-pass signature on the list */
		for (psig2=siglist; *psig2;
		psig2 = (struct PgpSig **)&(*psig2)->pkt.next) {
			if (!SIGPKT_1PASS(*psig2))
				continue;
			b2 = (*psig2)->pkt.buf;
			if (b2[1] != buf[2]) /* sig type */
				continue;
			if (b2[2] != buf[11+buf[1]]) /* hash alg */
				continue;
			if (b2[3] != buf[10+buf[1]])/* pkalg */
				continue;
			if (memcmp(b2+4, buf+2+buf[1], 8)) /* key id */
				continue;
			/* Here we have a match */
			break;
		}
		/* Error if found no match */
		if (!*psig2)
			return PGPERR_SIG_BADTYPE;

		/* Replace *psig2 with new sig */
		sig = (struct PgpSig *)pgpPktListNew(pkalg, buf, len);
		if (!sig)
			return PGPERR_NOMEM;
		sig->pkt.next = (*psig2)->pkt.next;
		pgpPktListFreeOne((struct PktList *)*psig2);
		*psig2 = sig;

		return 0;
	}
		
return PGPERR_SIG_BADTYPE;
}

/* Return true if this signature is followed immediately by signed data */
int
pgpSigNestFlag(byte const *buf, size_t len)
{
	(void)len;
	if (SIGBUF_1PASS(buf, len))
		return buf[12];	/* nest flag */
	else
		return 1;	/* always true for old packets */
}

/* How here come some access functions */
int
pgpSigPKAlg(struct PgpSig const *sig)
{
	pgpAssert(sig);
	if (SIGPKT_1PASS(sig))
		return sig->pkt.buf[3];
	else
		return sig->pkt.buf[10+sig->pkt.buf[1]];
}

struct PgpHash const *
pgpSigHash(struct PgpSig const *sig)
{
	int alg;

	pgpAssert(sig);
	if (SIGPKT_1PASS(sig))
		alg = sig->pkt.buf[2];
	else
		alg = sig->pkt.buf[11+sig->pkt.buf[1]];
	return pgpHashByNumber(alg);
}

byte const *
pgpSigExtra(struct PgpSig const *sig, unsigned *len)
{
	pgpAssert(sig);
	if (SIGPKT_1PASS(sig)) {
		*len = 0;
		return 0;
	}
	*len = sig->pkt.buf[1];
	return sig->pkt.buf+2;
}

word32
pgpSigTimestamp (struct PgpSig const *sig)
{
	byte const *extra;
	unsigned extralen;

	pgpAssert (sig);
	if (SIGPKT_1PASS(sig))
		return 0;
	extra = pgpSigExtra (sig, &extralen);
	if (!extra || extralen < 5)
		return 0;
	return ((word32)extra[1]<<24) + ((word32)extra[2]<<16) +
	((word32)extra[3]<<8) + (word32)extra[4];
}

byte const *
pgpSigId8(struct PgpSig const *sig)
{
	pgpAssert(sig);
	if (SIGPKT_1PASS(sig))
		return sig->pkt.buf+4;
	else
		return sig->pkt.buf+2+sig->pkt.buf[1];
}

/* Return a count of the number of distinct hash algorithms in the list */
unsigned
pgpSigDistinctHashCount(struct PgpSig const *sig)
{
	struct PktList const *next;
	unsigned total;
	int alg, nextalg;

	for (total = 0; sig; sig = (struct PgpSig const *)sig->pkt.next) {
		total++;
		/*
		* If another one later on the list has the same alg,
		* don't count this one.
		*/
		if (SIGPKT_1PASS(sig))
			alg = sig->pkt.buf[2];
		else
			alg = sig->pkt.buf[11+sig->pkt.buf[1]];
		for (next = sig->pkt.next; next; next = next->next) {
			if (SIGBUF_1PASS(next->buf, next->len))
				nextalg = next->buf[2];
			else
				nextalg = next->buf[11+next->buf[1]];
			
			if (nextalg == alg) {
				total--;
				break;
			}
		}
	}
	return total;
}

/*
* Return a buffer full of byte hash identifiers. The buffer must be
* of legnth sigDistincthashCount(len) length, and that number is
* returned for convenience.
*/
unsigned
pgpSigDistinctHashes(struct PgpSig const *sig, byte *buf)
{
	unsigned len;
	int alg;

	for (len = 0; sig; sig = (struct PgpSig const *)sig->pkt.next) {
		if (SIGPKT_1PASS(sig))
			alg = sig->pkt.buf[2];
		else
			alg = sig->pkt.buf[11+sig->pkt.buf[1]];
		if (!memchr(buf, alg, len))
			buf[len++] = (byte)alg;
	}
	return len;
}


/*
* The internal checking function, not for public use.
*/
int
pgpSigCheckBuf(byte const *sig, size_t len, struct PgpPubKey const *pub,
	byte const *hash)
{
	unsigned extra;
	byte type;
	struct PgpHash const *h;

	if (sig[0] == PGPVERSION_3) {
		/* New signature format; see pgpMakeSig.c */
		extra = (unsigned)sig[4]<<8 | sig[5];
		extra += (unsigned)sig[extra+6]<<8 | sig[extra+7];
		/* Quick rejection test: check the given two bytes first */
		if (memcmp (hash, sig+8+extra, 2) != 0)
			return 0;
		h = pgpHashByNumber (sig[3]);
		type = sig[1];
		/* Skip to signature data */
		extra += 10;
	} else {
		extra = sig[1];
		/* Quick rejection test: check the given two bytes first */
		if (memcmp (hash, sig+12+extra, 2) != 0)
			return 0;
		h = pgpHashByNumber (sig[11+extra]);
		type = sig[2];
		/* Skip to signature data */
		extra += 14;
	}

	/* XXX Should "die" gracefully here */
	pgpAssert (pub->verify);
	return pgpPubKeyVerify (pub, type, sig+extra, len-extra, h, hash);
}

/*
* Check a signature against a given public key and hash.
* Returns 0 if it did not check, and 1 if it did.
* Returns <0 on some sort of error.
* (The hash better be the right algorithm.)
*/
int
pgpSigCheck(struct PgpSig const *sig, struct PgpPubKey const *pub,
	byte const *hash)
{
	return pgpSigCheckBuf(sig->pkt.buf, sig->pkt.len, pub, hash);
}

struct PgpSig *
pgpSigNext (struct PgpSig const *sig)
{
	if (sig)
		return (struct PgpSig *)sig->pkt.next;

	return NULL;
}

void
pgpSigFreeList (struct PgpSig *siglist)
{
	pgpPktListFreeList((struct PktList *)siglist);
}


/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
