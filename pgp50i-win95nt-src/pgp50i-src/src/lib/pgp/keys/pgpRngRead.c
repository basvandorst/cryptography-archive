/*
 * pgpRngRead.c - Read in various parts of a keyring.
 *
 * Copyright (C) 1994-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * The big function (>500 lines, yeep!) is ringFileOpen(); it opens another
 * keyring and merges it with the collection in memory.	 Most of the others
 * are its helpers. This is where PGPlib's great robustness in the face of
 * badly mangled keyrings is achieved.	*Every* keyring comes through here,
 * and it validates its inputs to the point of paranoia.
 *
 * This file is too big - what should be split out?
 * There are a lot of similar-but-not-quite functions.	Perhaps some
 * rethinking will allow parts of them to be merged?
 *
 * Written by Colin Plumb.
 *
 * $Id: pgpRngRead.c,v 1.24.2.12 1997/06/11 22:29:47 mhw Exp $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "pgpDebug.h"
#include "pgpMakeSig.h"
#include "pgpMemPool.h"
#include "pgpPktByte.h"
#include "pgpRngMnt.h"
#include "pgpRngPars.h"
#include "pgpRngPkt.h"
#include "pgpRngPriv.h"
#include "pgpTrust.h"
#include "pgpTrstPkt.h"		/* for PGP_SIGTRUSTF_CHECKED_TRIED */
#include "pgpHash.h"
#include "pgpKeySpec.h"
#include "pgpMem.h"
#include "pgpCipher.h"
#include "pgpEnv.h"
#include "pgpErr.h"
#include "pgpPubKey.h"
#include "pgpFile.h"
#include "pgpRngRead.h"
#include "pgpSigSpec.h"

#ifndef NULL
#define NULL 0
#endif

/* We have optional code to wrap newer packets in comments */
#define WRAP_NONE 1


/*
 * The largest legal PGP key uses a 64Kbit key, which is 8Kbytes.
 * As a public key, there's also 12 bytes of overhead, plus a
 * public exponent (usually 1 byte, sometimes 3, it's just stupid
 * to make it any larger).
 * Stored in a secret key an extra 11+IV bytes of overhead and
 * the secret exponent (8K), factors p and q (4+4=8K), and multiplicative
 * inverse u (4K). A total of 28K of data, plus 23+IV+e extra bytes.
 * With an 8-byte IV, that's 31+e bytes. Add an extra byte to allow
 * for p and q of differing lengths.
 * But for now, reduce this by a factor of 8, to 8Kbits, which changes
 * the maximum sizes to 1K+overhead and 3.5K + overhead.
 *
 * Without these limits, a non-fatal error (object too big) becomes
 * a fatal error (out of memory) and the implementation becomes
 * less robust. However, the limits can be set quite high without
 * harm. (Keep the maximum key size to 64K, though.)
 *
 * The limits above were derived for RSA keys. DSA/ElG keys have a prime
 * p, a small (~160 bit) prime q, a generator g, and a public exponent y.
 * That's four numbers, one of them small. If we allow enough space for four
 * full-sized numbers that should be safe. Secrets have in addition a
 * secret exponent x, generally small, so allow an additional number.
 * Signatures are small with DSA, two 160 bit numbers. However the new
 * signature packets can in principle have a lot of data. Allow two full
 * sized numbers to give us plenty of room; we may have to increase the value
 * in the future.
 */
#define RINGMPI_MAX	1024				/* Maximum 8Kbits */
#define RINGKEY_MAXLEN	(4*RINGMPI_MAX)	/* Public key maximum size */
#define RINGSEC_MAXLEN	(5*RINGMPI_MAX)	/* Secret key maximum size */
#define RINGNAME_MAXLEN	1024u		 	/* Name maximum size */
#define RINGSIG_MAXLEN	(2*RINGMPI_MAX)	/* Signature maximum size */
#define RINGUNK_MAXLEN	RINGSEC_MAXLEN
/* RINGTRUST_MAXLEN is implicit */

/*** Working with the FilePos chain ***/

/*
 * A note about the FilePos chain. Each object has one FilePos right
 * inside itself, which is the head of a list of external (allocated)
 * ones. There is one FilePos for each key file an object exists in,
 * and they are kept in increasing order by bit number.
 * Every object (except dummy keys, which aren't excessively numerous,
 * especially in large keyrings) is present in at least one physical
 * keyring, so this saves one next pointer when we're trying to conserve
 * memory for MS-DOS, at the expense of complicating the task of
 * adding to and removing from the list.
 *
 * This is because the first entry in the chain is statically allocated.
 * An actual allocation is performed when an entry is added to the
 * chain in a location other than the first, or is bumped from first
 * place by something else. A FilePos is freed when an entry is deleted
 * from the chain, or the first one is deleted and the second moves into
 * its place.
 *
 * One more piece of magic: because there is no need for the FilePos chain
 * to be null-terminated (the number of entries in it is given by the
 * mask), physical key ring MEMRINGBIT (which is the last FilePos in the
 * chain, so its next pointer is unused) is reserved for memory objects.
 * the next pointer and fpos record the location in memory and size of
 * the memory buffer holding the object, respectively.
 */

/* Find the position of an object in the given file */
static struct FilePos *
ringFilePos(union RingObject const *obj, struct RingFile const *file)
{
	struct FilePos const *pos = &obj->g.pos;
	ringmask mask;

	pgpAssert(obj->g.mask & file->set.mask);
	mask = obj->g.mask & file->set.pool->filemask & (file->set.mask-1);
	while (mask) {
		pos = pos->ptr.next;
		mask &= mask-1;
	}
	return (struct FilePos *)pos;
}

/* Allocate a FilePos from a RingFile. */
static struct FilePos *
ringFileNewFilePos(struct RingFile *file)
{
	struct FilePos *pos = file->freepos;

	if (pos) {
		file->freepos = pos->ptr.next;
	} else {
		pos = (struct FilePos *)memPoolNew(&file->fpos,struct FilePos);
		if (!pos)
			ringAllocErr(file->set.pool);
	}

	return pos;
}

static void
ringFileFreeFilePos(struct RingFile *file, struct FilePos *pos)
{
	pos->ptr.next = file->freepos;
	file->freepos = pos;
}

/*
 * Allocate and add a FilePos to the object's chain in the right place.
 * This function makes no attempt to initialize the resultant FilePos.
 *
 * NOTE that the bit specified to add may or may not be present in
 * the ring's filemask. This function must not care.
 */
static struct FilePos *
ringAddFilePos(union RingObject *obj, struct RingFile *file)
{
	ringmask mask = obj->g.mask & file->set.pool->filemask;
	ringmask mask2 = file->set.mask;
	struct FilePos *pos, *pos2;
	int bit;

	pgpAssert(!(mask & mask2));

	if (mask & (mask2-1)) {
		/* FilePos to add is not the first in the chain */
		mask &= (mask2-1);

		pos2 = ringFileNewFilePos(file);
		if (!pos2)
			return NULL;
		/* Find the predecessor of the one to be added */
		pos = &obj->g.pos;
		while ((mask &= mask-1) != 0) {
			pos = pos->ptr.next;
			pgpAssert(pos);
		}
		/* Insert pos2 into the chain after pos */
		pos2->ptr.next = pos->ptr.next;
		pos->ptr.next = pos2;
	} else {
		/* First FilePos in the chain */
		pos2 = &obj->g.pos;
		if (!mask) {
			/* First and only FilePos on chain */
			pos = NULL;
		} else {
			/* First FilePos; bump down the old first */
			bit = ringLsBitFind(mask);
			pgpAssert(bit >= 0);
			file = &file->set.pool->files[bit];
			pos = ringFileNewFilePos(file);
			if (!pos)
				return NULL;
			*pos = *pos2;
		}
		pos2->ptr.next = pos;
	}
	obj->g.mask |= mask2;
	return pos2;
}

static int
ringAddPos(union RingObject *obj, struct RingFile *file, word32 fpos)
{
	struct FilePos *pos;

	pos = ringAddFilePos(obj, file);
	if (!pos)
		return PGPERR_NOMEM;
	pos->fpos = fpos;
	return 0;
}

/*
 * This is needed in one obscure error case to keep things
 * consistent. The case is when a secret key appears in the same
 * file as the corresponding public key, only later.
 */
static void
ringAlterFilePos(union RingObject *obj, struct RingFile const *file,
	word32 fpos)
{
	ringmask mask = obj->g.mask & file->set.pool->filemask;
	struct FilePos *pos;

	pgpAssert(mask & file->set.mask);
	mask &= file->set.mask - 1;
	for (pos = &obj->g.pos; mask; mask &= mask-1)
		pos = pos->ptr.next;
	pos->fpos = fpos;
}

/*
* Remove a FilePos from an object's list.
*
* file is the filepos corresponding to "bit", pos is the head of a
* FilePos chain, mask is the bitmask of physical key rings, and
* bit is the number of the ring to have its position removed.
*
* NOTE that the bit specified to remove may or may not be present in
* the ring->filemask. This function must not care.
*/
static void
ringRemFilePos(union RingObject *obj, struct RingFile *file)
{
	ringmask mask = obj->g.mask & file->set.pool->filemask;
	ringmask mask2 = file->set.mask;
	struct FilePos *pos, *pos2;
	int bit;

	/* Is the bit to remove *not* the least significant bit? */
	if (mask & (mask2-1)) {
		/* FilePos to remove is not the first in the chain */

		/* Find the predecessor of the one to be removed */
		mask &= (mask2-1);
		pos = &obj->g.pos;
		while ((mask &= mask-1) != 0) {
			pos = pos->ptr.next;
			pgpAssert(pos);
		}
		/* pos->next is the one to be removed */
		pos2 = pos->ptr.next;
		pgpAssert(pos2);
		pos->ptr = pos2->ptr;
		if (mask2 == MEMRINGMASK)	/* Debugging aid */
			pos->ptr.next = NULL;
	} else {
		/* First FilePos - copy second to first, remove second */

		/* Clear this bit from the mask (in case we need to) */
		mask &= ~mask2;

		/*
		* That's it? Well, return then. The caller better
		* deallocate this object, 'cause it no longer exists
		* anywhere. Use position -1 to mark an unused slot.
		*/
		if (!mask) {
			obj->g.mask &= ~mask2;
			if (mask2 == MEMRINGMASK)	 /* Debugging aid */
				obj->g.pos.ptr.next = NULL;
			obj->g.pos.fpos = (word32)-1;	 /* Debugging aid */
			return;
		}

		/* Find the bit of the object we are removing */
		bit = ringLsBitFind(mask);
		pgpAssert(bit >= 0);
		file = &file->set.pool->files[bit];

		/* Copy the next pos to the current one */
		pos2 = obj->g.pos.ptr.next;
		pgpAssert(pos2);
		obj->g.pos = *pos2;
	}
	/* Free the FilePos */
	ringFileFreeFilePos(file, pos2);
	obj->g.mask &= ~mask2;
}

/*** Closing a Ringfile ***/

/*
 * Set the destruction function for a RingFile.
 */
void
ringFileSetDestructor(struct RingFile *file,
	void (*destructor)(struct RingFile *, struct PgpFile *, void *),
	void *arg)
{
	file->destructor = destructor;
	file->arg = arg;
}

/*
 * Helper function for ringFileDoClose.
 *
 * Delete the given file's FilePos entries from the objects in
 * the given list, and delete the objects if they are no longer
 * needed (mask has gone to 0). Recurse as necessary.
 *
 * Note that this is not used on the main keys list, because
 * there we need to preserve dummy keys which this does not
 * understand.
 *
 * This also removes any cached names from objects.
 */
static void
ringFileCloseList(union RingObject **objp, struct RingFile *file)
{
	union RingObject *obj;
	ringmask const mask = file->set.mask;
	ringmask const filemask = file->set.pool->filemask;

	while ((obj = *objp) != NULL) {
		if (mask & obj->g.mask) {
			if (!OBJISBOT(obj))
				ringFileCloseList(&obj->g.down, file);
			ringRemFilePos(obj, file);
			/*
			* May have a child obj in the MEMRING which didn't
			* get deleted because it was not in this RingFile.
			*/
			if (!(obj->g.mask & filemask & ~MEMRINGMASK) &&
				(OBJISBOT(obj) || !obj->g.down)) {
				*objp = obj->g.next;
				ringFreeObject(file->set.pool, obj);
			} else {
				if (OBJISNAME(obj))
					ringPurgeCachedName(&obj->n, mask);
				objp = &obj->g.next;
			}
		} else
			objp = &obj->g.next;
	}
}

/*
 * Close the given Ringfile. Returns an error if it can't due to
 * conflicts, in which case the file is NOT closed.
 *
 * This performs four passes over the pool.
 * 1. The first does the bulk of the deletion, removing the
 * FilePos from the objects and deleting all things
 * at levels greater than 1.
 * 2. The second rebuilds the sigs-by lists which were broken by
 * deleting objects in the middle of them.
 * 3. The third finds all keys that are not referenced and do not
 * make any signatures, and deletes those keys.
 * 4. The fourth rebuilds the hash index of the remaining keys.
 *
 * Note that the second and third passes delete any allocated-but-not
 * linked keys, which are left by ringFileOpen if it runs out of memory
 * in mid-operation.
 */
static void
ringFileDoClose(struct RingFile *file)
{
	union RingObject *obj, **objp;
	struct RingPool *pool = file->set.pool;
	ringmask mask = file->set.mask;
	ringmask filemask = pool->filemask;
	ringmask allocmask = ringAllocMask(pool, &file->set);
	int i;

	/* Free some memory right away */
	ringFilePurgeTrouble(file);

	/* 1: Remove everything in the keyring, but don't delete the keys */
	for (obj = pool->keys; obj; obj = obj->g.next) {
		if (mask & obj->g.mask) {
			if (!OBJISBOT(obj))
				ringFileCloseList(&obj->g.down, file);
			ringRemFilePos(obj, file);
		}
	}

	/* 2: Recreate the shattered sigs-by lists */
	ringPoolListSigsBy(pool);

	/* 3: Now purge the unneeded keys */
	objp = &pool->keys;
	while ((obj = *objp) != NULL) {
		pgpAssert(OBJISKEY(obj));
		if ((obj->g.mask & filemask & ~MEMRINGMASK) || obj->g.down) {
			objp = &obj->g.next;
		} else if (obj->k.sigsby) {
			/* Retain key as a dummy key */
			pgpAssert(!(obj->g.mask & allocmask & ~MEMRINGMASK));
			pgpAssert(!obj->g.down);
			obj->g.mask = 0;
			objp = &obj->g.next;
		} else {
			/* Delete the key */
			pgpAssert(!(obj->g.mask & allocmask & ~MEMRINGMASK));
			pgpAssert(!obj->g.down);
			*objp = obj->g.next;
			ringFreeObject(pool, obj);
		}
	}

	/* 4: Re-initialize the hash chains */
	ringPoolHash(pool);

	/* Clean up the file's memory pools */
	memPoolEmpty(&file->strings);
	file->freepos = NULL;
	memPoolEmpty(&file->fpos);

	pgpAssert(!file->set.next);

	/*
	* If there's nothing in the structs MemPool that's
	* allocated, purge all the memory.
	*/
	if (!pool->keys && !pool->sets) {
		for (i = 0; i < RINGTYPE_MAX; i++)
			pool->freeobjs[i] = NULL;
		pool->freesets = NULL;
		pool->freeiter = NULL;
		memPoolEmpty(&pool->structs);
	}

	/* Cal the file's destructor function, if any */
	if (file->destructor) {
		file->destructor(file, file->f, file->arg);
		file->destructor = NULL;
	}

	/* Final deallocation of the file */
	file->flags = 0;
	pool->filemask &= ~mask;
}

/*
 * Check to see if an object anywhere on the list (including children,
 * recursively) is included in "allocmask" but not in "filemask."
 * Such an object is orphaned, an undesirable state of affairs.
 * We have to check the entire keyring, recursively, because a
 * given key or name might be duplicated in another keyring, but
 * a signature lower down might not be.
 */
static int
ringFileCheckList(union RingObject const *obj, ringmask filemask,
	ringmask allocmask)
{
	while (obj) {
		if (obj->g.mask & allocmask) {
			/* Would closing orphan this object? */
			if (!(obj->g.mask & filemask))
				return 1;
			/* Would closing orphan its children? */
			if (!OBJISBOT(obj) && ringFileCheckList(obj->g.down,
				filemask,
				allocmask))
				return 1;
		}
		obj = obj->g.next;
	}
	return 0;
}

/* Is it safe to close the given file? */
int
ringFileCheckClose(struct RingFile const *file)
{
	struct RingPool const *pool = file->set.pool;

	if (!file)
		return 0;
	return ringFileCheckList(pool->keys, pool->filemask & ~file->set.mask,
	ringAllocMask(pool, &file->set));
}

/*
 * Close the given Ringfile. Returns an error if it can't due to
 * conflicts, in which case the file is NOT closed.
*/
int
ringFileClose(struct RingFile *file)
{
	if (!file)
		return 0;	/* close(NULL) is defines as harmless */

	if (ringFileCheckClose(file))
		return -1;

	/* Okay, nothing can fail now */
	ringFileDoClose(file);

	return 0;
}

/*** Routines for fetching things from the keyring ***/

/* Make sure the pool's packet buffer is large enough */
static char *
ringReserve(struct RingPool *pool, size_t len)
{
	char *p;

	if (pool->pktbufalloc >= len)
		return pool->pktbuf;
	p = (char *)pgpMemRealloc(pool->pktbuf, len);
	if (!p) {
		ringAllocErr(pool);
		return NULL;
	}
	pool->pktbufalloc = len;
	return pool->pktbuf = p;
}


/*
 * File priorities are handled by a "higherpri" mask with each file,
 * which is a mask of other files of higher priority than that file.
 * If obj->g.mask & pool->filmask && file->higherpri is 0,
 * this is the highest-priority file.
 */
/* Set file to the highest priority, except for the memory file */
void
ringFileHighPri(struct RingFile *file)
{
	struct RingPool *pool = file->set.pool;
	ringmask mask = file->set.mask;
	int i;

	/* Add this file to everything else's higher priority mask */
	for (i = 0; i < MEMRINGBIT; i++)
		pool->files[i].higherpri |= mask;
	/* The only thing higher priority than this file is MEMRING */
	file->higherpri = MEMRINGMASK;
}

/* Set file to the lowest priority */
void
ringFileLowPri(struct RingFile *file)
{
	struct RingPool *pool = file->set.pool;
	ringmask mask = ~file->set.mask;
	int i;

	/* Remove this file from everything else's higher priority mask */
	for (i = 0; i < MEMRINGBIT; i++)
		pool->files[i].higherpri &= mask;
	/* Everything is higher priority than this file (except itself) */
	file->higherpri = mask;
}


/* The mask of sets that this key has *any* secret components in */
static ringmask
ringKeySecMask(union RingObject const *obj)
{
	ringmask secmask = 0;
	pgpAssert(OBJISKEY(obj));

	for (obj = obj->g.down; obj; obj = obj->g.next)
		if (OBJISSEC(obj))
			secmask |= obj->g.mask;
	return secmask;
}

/*
* Given an object, find the best RingFile to fetch it from,
* for fetching purposes. Bits set in "avoidmask" are
* NOT valid for fetching.
*/
static struct RingFile *
ringBestFile(struct RingPool *pool, union RingObject const *obj,
	ringmask avoidmask)
{
	ringmask mask = obj->g.mask & pool->filemask;
	struct RingFile *file;
	int bit;

	if (!(mask & ~avoidmask))
		return NULL;

	/* find highest-priority fetchable file */
	for (;;) {
		/* Is least-significant bit set in mask fetchable? */
		if (!(mask & -mask & avoidmask)) {
			bit = ringLsBitFind(mask);
			file = &pool->files[bit];
			/* Is it highest priority? */
			if (!(mask & file->higherpri & ~avoidmask))
				break;
			pgpAssert(file->f);
		}
		mask &= mask-1;
		pgpAssert(mask);
	}
	return file;
}

/* Macro wrapper to inline the important part */
#define ringReserve(pool, len) \
	((pool)->pktbufalloc < (len) ? ringReserve(pool, len) : (pool)->pktbuf)

/*
 * This is the routine which fetches a packet from a keyring file.
 * It tries the highest-priority file that the object is in which is also
 * listed in "avoidmask." If the memory keyring is one of those, it has
 * absolute priority. (It is also not verified; it is assumed correct.)
 * Otherwise, the object is fetched from the highest-priority open file.
 *
 * The files are assigned priorities for fetching. The default is
 * that the first opened file is highest and subsequent files are of
 * lower priority. This is done by having each Ringfile keep a mask
 * of higher-priority RingFiles. We walk along the list until we hit
 * a RingFile whose higher-priority mask doesn't include any files that
 * the object being sought is in.
 *
 * The packet fetched must pass the following validity checks:
 * - It must be of the given packet type.
 * - It must be no longer than "maxlen".
 * - If those pass, it must be read into memory successfully.
 * - It must then pass the caller-supplied "verify" function,
 * which checks object-type-specific information against the
 * summary information stored in the RingObject.
 *
 * Question: what to return when the avoidmask doesn't allow anything to
 * be fetched? Is this case just an error?
 */
static void const *
ringFetchPacket(struct RingPool *pool, union RingObject const *obj,
	ringmask avoidmask, int pkttype, size_t maxlen, size_t *lenp,
	int (*verify)(union RingObject const *, byte const *, size_t))
{
	struct RingFile *file;
	struct FilePos const *pos;
	word32 len;
	int i;
	void *p;

	/* find highest-priority fetchable file */
	file = ringBestFile(pool, obj, avoidmask);
	if (!file) {
		*lenp = (size_t)0;
		return NULL;	/* Is this The Right Thing? */
	}

	pos = ringFilePos(obj, file);
	pgpAssert(pos);

	/* If it's in memory, that was easy... */
	if (file->set.mask == MEMRINGMASK) {
		pgpAssert (!verify
		|| verify(obj, (byte *)pos->ptr.buf, pos->fpos) == 0);
		*lenp = pos->fpos;
		return pos->ptr.buf;
	}

	/* We now have highest-priority fetchable file */
	pgpAssert(file->f);
	if (pgpFileSeek(file->f, pos->fpos, SEEK_SET) != 0) {
		i = PGPERR_KEYIO_SEEKING;
		goto err;
	}
	i = pktByteGet(file->f, &len, (word32 *)NULL);
	if (i <= 0)
		goto err;
	if (PKTBYTE_TYPE(i) != pkttype || len > maxlen) {
		i = PGPERR_KEYIO_BADPKT;
		goto err;
	}
	p = ringReserve(pool, (size_t)len);
	if (!p)
		goto errmem;	/* ringErr() already called */
	pool->pktbuflen = (size_t)len;
	i = pgpFileRead(pool->pktbuf, (size_t)len, file->f);
	if ((size_t)i != (size_t)len) {
		i = pgpFileError(file->f) ? PGPERR_KEYIO_READING :
					PGPERR_KEYIO_EOF;
		goto err;
	}
	p = pool->pktbuf;

	/* Okay, now verify with supplied function */
	if (verify && (i = verify(obj, (byte const *)p, len)) != 0) {
		if (pkttype == PKTBYTE_SECKEY
			&& len
			&& obj->g.flags & SECF_VERSION_BUG
			&& ((byte *)p)[0] == PGPVERSION_2_6) {
				/*
				* Failure may be due to version bug; fix and try
				* again. If success, put it back so we behave
				* consistently. Verify doesn't always fail with
				* version bug, it depends on whether sec or pub was
				* seen first.
				*/
			((byte *)p)[0] = PGPVERSION_2;
			if (verify &&
			(i = verify(obj, (byte const *)p, len)) != 0)
				goto err;
			((byte *)p)[0] = PGPVERSION_2_6;
		} else {
			goto err;
		}
	}

	/* Success! */
	*lenp = len;
	return p;

	/* Error cases (out of line) */
err:
	ringErr(file, pos->fpos, i);
errmem:
	*lenp = 0;
	return NULL;
}

/*
 * The verify functions here should *never* fail under normal
 * conditions. They fail only if the keyring file has been
 * changed while PGP is accessing it (which causes a fatal error).
 * Did I mention that this code is *paranoid*?
 * ("The computer is your friend. The computer wants you to be happy.")
 *
 * They operate by re-parsing the fetched data and checking that
 * the cached data matches the data just fetched.
 */

/* Verify that the key we just read looks like the one we wanted to read. */
static int
ringKeyVerify(union RingObject const *obj, byte const *p, size_t len)
{
	int i;
	byte pkalg, keyID[8];
	word16 keybits, validity;
	word32 tstamp;

	i = ringKeyParse(p, len, &pkalg, keyID, &keybits,
	&tstamp, &validity, 0);

	if (memcmp(keyID, obj->k.keyID, 8) == 0
		&& keybits == obj->k.keybits
		&& tstamp == obj->k.tstamp
/*		&& validity == obj->k.validity Validity sometimes stored elsewhere */
		&& pkalg == obj->k.pkalg
		&& (i == 0) == !(obj->g.flags & KEYF_ERROR))
		return 0;	/* All copascetic */

	return PGPERR_KEYIO_BADPKT;
}

/* Verify that the secret we just read looks like the one we wanted to read. */
static int
ringSecVerify(union RingObject const *obj, byte const *p, size_t len)
{
	int i;
	byte pkalg, keyID[8];
	word16 keybits, validity;
	word32 tstamp;
	union RingObject *key = obj->g.up;

	pgpAssert(OBJISSEC(obj));
	pgpAssert(OBJISKEY(key));

	i = ringKeyParse(p, len, &pkalg, keyID, &keybits,
	&tstamp, &validity, 1);

	if (ringHashBuf(p, len) == obj->c.hash
		&& memcmp(keyID, key->k.keyID, 8) == 0
		&& keybits == key->k.keybits
		&& tstamp == key->k.tstamp
/*		&& validity == key->k.validity	Validity sometimes stored elsewhere */
		&& pkalg == key->k.pkalg
		&& (i == 0) == !(key->g.flags & KEYF_ERROR))
		return 0;	/* All copascetic */

	return PGPERR_KEYIO_BADPKT;
}

/*
 * Verify that the signature we just read looks like the one we wanted to read.
 */
static int
ringSigVerify(union RingObject const *obj, byte const *p, size_t len)
{
	int i;
	byte pkalg, keyID[8];
	word32 tstamp;
	word16 validity;
	size_t extralen;
	byte type, hashalg;
	byte version;

	i = ringSigParse(p, len, &pkalg, keyID, &tstamp, &validity,
	&type, &hashalg, &extralen, &version);
	if (memcmp(keyID, obj->s.by->k.keyID, 8) == 0
		&& version == obj->s.version
		&& tstamp == obj->s.tstamp
		&& validity == obj->s.validity
		&& type == obj->s.type
		&& hashalg == obj->s.hashalg
		&& (extralen == 5) == !(obj->g.flags & SIGF_NONFIVE)
		&& (i == 0) == !(obj->g.flags & SIGF_ERROR))
		return 0;	/* All copascetic */

	return PGPERR_KEYIO_BADPKT;
}

/*
 * Verify that the unknown we just read looks like the one we wanted to read.
 */
static int
ringUnkVerify(union RingObject const *obj, byte const *p, size_t len)
{
	if (ringHashBuf(p, len) == obj->u.hash)
		return 0;	/* All copascetic */

	return PGPERR_KEYIO_BADPKT;
}

/*
 * Getting names is special due to the in-memory cache.
 * We don't have to support "avoidmask", though.
 *
 * Return a pointer to a name string. Note that the string is NOT
 * null-terminated; all 256 values are legal! The "lenp" argument
 * returns the length. Tries to get it from memory if possible,
 * then tries to load it into the cache. If it can't load it into
 * the preferred file cache, load it into the pktbuf. If even
 * that fails, try to find another cache that it wil fit into.
 *
 * Note that ringNamesDiffer() uses the fact that this either returns
 * with NAMEISCACHED(name) true, *or* the buffer returned is the
 * pktbuf. Never both, and never a third choice.
 *
 * @@@ Is the ability to hande a pool==NULL still useful?
 */
static char const *
ringPoolGetName(struct RingPool *pool, struct RingName *name, size_t *lenp)
{
	int i;
	struct MemPool cut;
	ringmask mask;
	struct RingFile *file, *file2;
	struct FilePos const *pos;
	char *str;
	word32 len;

	*lenp = name->len;

	/* If we already have it, boom. */
	if (NAMEISCACHED(name))
		return name->name.ptr;

	/* If we weren't given a pool, we can't fetch it */
	if (!pool)
		return NULL;

	/* find highest-priority fetchable file */
	file = ringBestFile(pool, (union RingObject *)name, 0);
	pgpAssert(file);
	pgpAssert(file->f || file->set.mask == MEMRINGMASK);

	pos = ringFilePos((union RingObject *)name, file);
	pgpAssert(pos);

	/* If it's in memory, that was fast - set cached if it wasn't already*/
	if (file->set.mask == MEMRINGMASK) {
		pgpAssert(pos->fpos == name->len);
		pgpAssert(ringHashBuf((byte *)pos->ptr.buf, name->len) ==
		name->name.hash);
		NAMESETCACHED(name);
		name->flags &= ~NAMEF_FILEMASK;
		name->flags |= MEMRINGBIT;
		return name->name.ptr = (char *)pos->ptr.buf;
	}

	/* Allocate cache space for it */
	/* Dummy loop to break out of */
	do {
		/* Try the preferred cache */
		file2 = file;
		cut = file2->strings;	/* Remember cutback position */
		str = (char *)memPoolAlloc(&file2->strings, name->len, 1);
		if (str)
			break;
		/* Try the pktbuf */
		str = ringReserve(pool, name->len);
		if (str) {
			file2 = NULL;
			pool->pktbuflen = name->len;
			break;
		}
		/* Okay, desperation time - look for any cache */
		mask = name->mask & pool->filemask & ~MEMRINGMASK;
		pos = &name->pos;
		for (;;) {
			i = ringLsBitFind(mask);
			pgpAssert(i >= 0);
			pgpAssert (i < MEMRINGBIT);
			pgpAssert(pool->files[i].f);
			file2 = &pool->files[i];
			cut = file2->strings;
			str = (char *)memPoolAlloc(&file2->strings,
				name->len, 1);
			if (str)
				break;
			if (!(mask &= mask-1)) {
				ringAllocErr(pool);
				return NULL;
			}
			pos = pos->ptr.next;
		}
	} while (0);	/* Dummy loop to break out of */

	/* Okay, we got buffer space... get the packet */

	if (pgpFileSeek(file->f, pos->fpos, SEEK_SET) != 0) {
		i = PGPERR_KEYIO_SEEKING;
		goto error;
	}
	i = pktByteGet(file->f, &len, (word32 *)NULL);
	if (i < 0)
		goto error;
	if (PKTBYTE_TYPE(i) != PKTBYTE_NAME || len != name->len)
		goto badpkt;

	i = pgpFileRead(str, (size_t)len, file->f);
	if ((size_t)i != (size_t)len) {
		i = pgpFileError(file->f) ? PGPERR_KEYIO_READING :
					PGPERR_KEYIO_EOF;
		goto error;
	}

	/* Double-check that we got the right thing. */
	if (ringHashBuf((byte *)str, len) != name->name.hash)
		goto badpkt;

	/* Success at last! */
	if (file2) {
		/* It's cached in file2 - set flags appropriately */
		NAMESETCACHED(name);
		name->flags &= ~NAMEF_FILEMASK;
		name->flags |= ringLsBitFind(file2->set.mask);
		name->name.ptr = str;
	}
	return str;
	
badpkt:
	i = PGPERR_KEYIO_BADPKT;
error:
	if (file2)
		memPoolCutBack(&file2->strings, &cut);
	ringErr(file, pos->fpos, i);
	return NULL;
}


void const *
ringFetchObject(struct RingSet const *set, union RingObject *obj, size_t *lenp)
{
	byte const *buf = NULL;
	ringmask secmask, bestfile;	/* Needed in RINGTYPE_KEY */
	struct RingPool *pool = set->pool;
	byte pktbyte;

	pgpAssert(obj->g.mask & set->mask);
	pgpAssert(obj->g.mask & pool->filemask);

	switch (ringObjectType(obj)) {
	case RINGTYPE_NAME:
		buf = (byte const *)ringPoolGetName(pool, &obj->n, lenp);
		break;
	case RINGTYPE_SIG:
		buf = (byte const *)ringFetchPacket(pool, obj, 0, PKTBYTE_SIG,
					RINGSIG_MAXLEN, lenp, ringSigVerify);
		break;
	case RINGTYPE_UNK:
		buf = (byte const *)ringFetchPacket(pool, obj, 0,
				PKTBYTE_TYPE(obj->u.pktbyte),
					RINGUNK_MAXLEN, lenp, ringUnkVerify);
		break;
	case RINGTYPE_SEC:
	pktbyte = OBJISTOPKEY(obj->g.up) ? PKTBYTE_SECKEY :
		PKTBYTE_SECSUBKEY;
		buf = (byte const *)ringFetchPacket(pool, obj, 0,
				pktbyte, RINGSEC_MAXLEN,
				lenp, ringSecVerify);
		/* Compensate for version bug */
			 if (buf
		&& *lenp > 0
		&& obj->g.flags & SECF_VERSION_BUG
		&& buf[0] == PGPVERSION_2_6)
			((byte *)buf)[0] = PGPVERSION_2;
		break;
	case RINGTYPE_KEY:
		/* File we'd like to fetch from */
		bestfile = ringBestFile(pool, obj, 0)->set.mask;
		/* Where secrets are located */
		secmask = ringKeySecMask(obj);

		/* Is where we want to fetch from secret? */
		if (bestfile & secmask) {
			pktbyte = OBJISTOPKEY(obj) ? PKTBYTE_SECKEY :
				PKTBYTE_SECSUBKEY;
			/* Have to fetch the secret key and extract. */
			obj = obj->g.down;
			while (!(obj->g.mask & bestfile) || !OBJISSEC(obj)) {
				obj = obj->g.next;
				pgpAssert(obj);
			}
			buf = (byte const *)ringFetchPacket(pool, obj,
					~secmask, pktbyte, RINGSEC_MAXLEN,
					lenp, ringSecVerify);
			if (buf) {
				size_t len;

				/* Compensate for version bug */
				if (*lenp > 0
				&& obj->g.flags & SECF_VERSION_BUG
				&& buf[0] == PGPVERSION_2_6)
					((byte *)buf)[0] = PGPVERSION_2;

				len = ringKeyParsePublicPrefix(buf, *lenp);
				/* If unparseable, take the whole thing. */
				if (len)
					*lenp = len;
			}
		} else {
			pktbyte = OBJISTOPKEY(obj) ? PKTBYTE_PUBKEY :
				PKTBYTE_PUBSUBKEY;
			/* Fetch public components */
			buf = (byte const *)ringFetchPacket(pool, obj, secmask,
				pktbyte, RINGKEY_MAXLEN,
				lenp, ringKeyVerify);
		}
		break;
	default:
		pgpAssert(0);
		break;
	}
	return buf;
}

/*** Various bookkeeping helper functions ***/

/*
 * Sort all the keys in a pool into keyID order. This uses 8 passes
 * of a byte-wise radix sort. Each pass is stable, so sorting on the
 * least significant byte, proceeding to the most will result in a
 * completely sorted list.
 *
 * Actually, it's sorted with the *visible* part (the low 32 bits) of the
 * keyID more significant than the invisible part. This makes the ordering
 * more sensible to a human watching what's going on.
 *
 * There are 256 lists, with a head and a tail pointer.	 The tail
 * pointer is a pointer to a pointer, namely the slot the pointer to
 * the next entry to be added to the list goes in. It is initialized
 * to point to the head pointer. So adding an element to the list
 * consists of setting *tail = object; and then tail = &object->next;
 *
 * After each pass, concatenate the lists, starting at the end.
 * Begin with an empty list and keep appending the current list to
 * the tail of the one before it, grabbing the head as the new
 * current list.
 */
#if 0
static int
ringKeyIDcmp(byte const id1[8], byte const id2[8])
{
	int i;

	i = memcmp(id1+4, id2+4, 4);
	return i ? i : memcmp(id1, id2, 4);
}
#endif

static void
ringSortKeys(struct RingPool *pool)
{
#if 1
	/*
	* Disable sort, users who switch back to old versions of
	* PGP are unhappy to see their keyring reordered. The reason
	* for the sort was to hide the order with which keys had been
	* added to the keyring, and to make merges more efficient.
	* For now neither of those is compelling enough to keep.
	*/
	(void)pool;
#else
	int i, j;
	int pass;
	int lastpass;
	union RingObject *list = pool->keys;
	union RingObject *head[256];
	union RingObject **tail[256];

	for (pass=0; pass<9; ++pass) {
		/* XXX Experimental backwards compat code - put DSA keys at end */
		/* On last pass we sort by pkalg */
		lastpass = (pass==8);
		i = (pass < 4) ? (3-pass) : (11-pass); /* 3,2,1,0,7,6,5,4 */

		/* Clear the table for the next distribution pass */
		for (j = 0; j < 256; j++)
			tail[j] = head+j;

		/* Distribute the list elements among the sublists */
		while (list) {
			if (lastpass)
				j = list->k.pkalg;
			else
				j = list->k.keyID[i];
			*tail[j] = list;
			tail[j] = &list->k.next;
			list = list->k.next;
		}

		j = 256;
		/* list is already 0 from the previous loop */

		/* Gather the sublists back into one big list */
		while (j--) {
			*tail[j] = list;
			list = head[j];
		}
	}


	pool->keys = list;
#endif
}

static void
ringPoolLinkKey(struct RingPool *pool, union RingObject *parent,
	union RingObject *key, byte pkalg, byte const keyID[8])
{
	union RingObject **ptr;

	pgpAssert(OBJISKEY(key));
	memcpy(key->k.keyID, keyID, 8);
	key->k.pkalg = pkalg;

	if (parent) {
		key->g.up = parent;
		ptr = &parent->g.down;
		while (*ptr)
			ptr = &(*ptr)->g.next;
	} else {
		ptr = &pool->keys;
	}
	key->g.next = *ptr;
	*ptr = key;
	RINGPOOLHASHKEY(pool, key);
}

/* Remove specified key from the top-level keys list */
static void
ringPoolUnlinkKey(struct RingPool *pool, union RingObject *key)
{
	union RingObject *obj, **objp;

	pgpAssert(pool && key);

	objp = &pool->keys;
	while ((obj = *objp) != NULL && obj != key) {
		objp = &obj->g.next;
	}
	pgpAssert(obj == key);
	*objp = key->g.next;
	key->g.next = NULL;
	return;
}

/*
* Same as ringPoolFindKey, but creates a dummy key with the given parent
* if one is not found.
* Note that a dummy key is a RingObject with its mask set to 0.
*/
static union RingObject *
ringPoolFindDummyKey(struct RingPool *pool, union RingObject *parent,
	byte pkalg, byte const keyID[8])
{
	union RingObject *key = ringPoolFindKey(pool, pkalg, keyID);

	if (!key) {
		key = ringNewKey(pool);
		if (key) {
			if (parent)
				key->k.flags |= RINGOBJF_SUBKEY;
			ringPoolLinkKey(pool, parent, key, pkalg, keyID);
		}
	}
	return key;
}

/*
 * Free an entire tree of objects.
 * This does not do anything with the FilePos chain, but since the
 * first entry is preallocated, if the object has at most one FilePos,
 * (as is the case in newly created objects), no memory is leaked.
 */
static void
ringFreeTree(struct RingPool *pool, union RingObject *obj)
{
	union RingObject *down;

	if (!OBJISBOT(obj)) {
		while ((down = obj->g.down) != NULL) {
			obj->g.down = down->g.next;
			ringFreeTree(pool, down);
		}
	}
	ringFreeObject(pool, obj);
}

/*
 * Free up a newly created dummy key.
 * Unlink it from the pool and free it and all descendents.
 */
static void
ringFreeDummyKey(struct RingPool *pool, union RingObject *key)
{
	union RingObject **objp;

	pgpAssert(OBJISKEY(key));

	/* Find head of list this object is on */
	if (OBJISTOP(key))
		objp = &pool->keys;
	else
		objp = &key->g.up->g.down;

	while (*objp != key) {
		pgpAssert(*objp);
		objp = &(*objp)->g.next;
	}
	*objp = key->g.next;
	ringFreeTree(pool, key);
}

/*
 * Return 0 if the packet in the pktbuf is the same as the packet in
 * the given file at the given offset, and the file packet is of type
 * pkttype. Returns 1 if they differ, and -1 (and sets the ring's error
 * status) if there is an error, including an unexpected packet byte.
 * Compare at most max bytes.
 *
 * This does NOT examine any more of the object than its filepos
 * chain; in particular, it does NOT examine the object's type.
 * Thus, it is possible to have a key object and use it to
 * fetch a secret-key packet.
 *
 * Special case: returns pktbuf[0] if pktbuf[0] is 2 or 3 and the file's
 * packet begins with 5-pktbuf[0]. This is used by the key difference
 * code to detect the version byte bug. The other things can ignore it,
 * and just treat all positive return values as "different".
 */
static int
ringPacketDiffers(struct RingFile *file, union RingObject const *obj,
	int pkttype, word32 max)
{
	struct RingPool *pool = file->set.pool;
	struct FilePos const *pos;
	byte *p;
	struct PgpFile *f;
	word32 len;
	int i;
	byte c;
	int magic;

	pos = ringFilePos(obj, file);

	/* Memory file, special case for comparison */
	if (file->set.mask == MEMRINGMASK) {
		len = pos->fpos;
		if (len > max)
			len = max;
		if (max > pool->pktbuflen)
			max = pool->pktbuflen;
		if (len != max)
			return 1;	 /* Different */
		if (!len)
			return 0;
		/* Check first character specially */
		p = (byte *)pos->ptr.buf;
		magic = 0;
		if (p[0] != ((byte *)pool->pktbuf)[0]) {
			if ((p[0] ^ ((byte *)pool->pktbuf)[0]) != 1
			|| (p[0] & ((byte *)pool->pktbuf)[0]) != 2)
				return 1;	/* First char different */
			magic = ((byte *)pool->pktbuf)[0]; /* First char magic */
		}
		return memcmp(p+1, pool->pktbuf+1, (size_t)len-1) ? 1 : magic;
	}

	/* Usual case - external file */
	f = file->f;
	pgpAssert(f);

	i = pgpFileSeek(f, pos->fpos, SEEK_SET);
 	if (i != 0) {
		ringErr(file, pos->fpos, PGPERR_KEYIO_SEEKING);
		return PGPERR_KEYIO_SEEKING;
	}
	i = pktByteGet(f, &len, (word32 *)NULL);
	if (i < 0) {
		ringErr(file, pos->fpos, i);
		return i;
	}
	if (PKTBYTE_TYPE(i) != pkttype) {
		ringErr(file, pos->fpos, PGPERR_KEYIO_BADPKT);
		return PGPERR_KEYIO_BADPKT;
	}
	if (len > max)
		len = max;
	if (max > pool->pktbuflen)
		max = pool->pktbuflen;
	if (len != max)
		return 1;	/* Different */
	if (!len)
		return 0;
	/* Check first character specially */
	if (pgpFileRead(&c, 1, f) != 1) {
		i = pgpFileError(f) ? PGPERR_KEYIO_READING : PGPERR_KEYIO_EOF;
		ringErr(file, pos->fpos, i);
		return i;
	}
	i = c & 255;
	magic = 0;	/* First char the same */
	if (i != ((byte *)pool->pktbuf)[0]) {
		if ((i ^ ((byte *)pool->pktbuf)[0]) != 1
			|| (i & ((byte *)pool->pktbuf)[0]) != 2)
			return 1;
		magic = ((byte *)pool->pktbuf)[0]; /* First char magic */
	}
	i = fileUnequalBuf(f, pool->pktbuf+1, (size_t)len-1);
	if (i < 0)
		ringErr(file, pos->fpos, i);

	return i ? i : magic;
}

/*
 * Return 1 if the key in the ring's pktbuf differs from the
 * key in its other homes, 0 if it is the same, and <0 if there
 * is some sort of error.
 * Does *not* alter file1's read position unless there is an error.
 * (I.e. if it needs to seek file1, it saves and restores the file
 * position. It doesn't bother for other files. Use the MEMRING file
 * if you don't need this feature.)
 *
 * SPECIAL CASE:
 * Returns >1 if the version byte bug was detected. This is the case
 * wherein version 2.6 would write out an edited secret key with a
 * version byte of 3 even if it was originally 2.
 * This function returns the current pktbuf's version byte (2 or 3)
 * if the keys are identical except that the version byte differs
 * from something read previously.
 * The caller must decide what sort of trouble to log.
 */
static int
keysdiffer(struct RingFile *file1, union RingObject const *key, int pktbyte)
{
	struct RingPool *pool = file1->set.pool;
	struct RingFile *file2;
	size_t savelen, publen;
	ringmask secmask;
	word32 max;
	int i, type;
	long retpos=0;

	pgpAssert(OBJISKEY(key));
	pgpAssert(file1->f || file1->set.mask == MEMRINGMASK);


	/*
	* If this is a secret key, find the prefix which is a public key,
	* and limit it to that if reasonable.
	*/
	savelen = pool->pktbuflen;
	if ((PKTBYTE_TYPE(pktbyte) == PKTBYTE_SECKEY ||
		PKTBYTE_TYPE(pktbyte) == PKTBYTE_SECSUBKEY)
	&& !(key->g.flags & KEYF_ERROR))
	{
		publen = ringKeyParsePublicPrefix((byte const *)pool->pktbuf,
		pool->pktbuflen);
		if (publen)
			pool->pktbuflen = publen;
	}

	/* Find a file containing the key - try for public first */
	secmask = ringKeySecMask(key);

	file2 = ringBestFile(pool, key, secmask);
	if (file2) {
		max = (word32)-1;
		type = OBJISTOPKEY(key) ? PKTBYTE_PUBKEY : PKTBYTE_PUBSUBKEY;
	} else {
		file2 = ringBestFile(pool, key, 0);
		pgpAssert(file2);
		max = (word32)pool->pktbuflen;
		type = OBJISTOPKEY(key) ? PKTBYTE_SECKEY : PKTBYTE_SECSUBKEY;
	}

	if (file2 == file1 && file1->f && (retpos=pgpFileTell(file1->f)) < 0) {
		ringErr(file1, pgpFileTell(file1->f), PGPERR_KEYIO_FTELL);
		return PGPERR_KEYIO_FTELL;
	}
		
	i = ringPacketDiffers(file2, key, type, max);
	/*
	* If we compared against a secret key, and encountered the version
	* bug, and the version bug has already been noted, ignore the
	* difference.
	*/
	if (i == PGPVERSION_2
	&& type == PKTBYTE_SECKEY
	&& key->g.flags & SECF_VERSION_BUG)
		i = 0;

	if (file2 == file1 && file1->f &&
			pgpFileSeek(file1->f, retpos, SEEK_SET) != 0) {
		ringErr(file1, pgpFileTell(file1->f), PGPERR_KEYIO_SEEKING);
		return PGPERR_KEYIO_SEEKING;
	}
	
	/* Restore pktbuflen, may have been changed above */
	pool->pktbuflen = savelen;

	return i;
}

/*
 * Return 1 if the secret in the ring's pktbuf differs from the
 * key in its other homes, 0 if it is the same, and <0 if there
 * is some sort of error.
 * Does *not* alter file1's read position unless there is an error.
 *
 * SPECIAL CASE:
 * Returns >1 if the version byte bug was detected. This is the case
 * wherein version 2.6 would write out an edited secret key with a
 * version byte of 3 even if it was originally 2.
 * This function returns the current pktbuf's version byte (2 or 3)
 * if the keys are identical except that the version byte differs
 * from something read previously.
 * The caller must decide what sort of trouble to log.
 */
/*
 * Return 1 if the signature in the ring's pktbuf differs from the
 * sig in its various homes, 0 if it is the same, and <0 if there
 * is some sort of error.
 * Does *not* alter file1's read position unless there is an error.
 * (I.e. if it needs to seek file1, it saves and restores the file
 * position. It doesn't bother for other files. Use the MEMRING file
 * if you don't need this feature.)
 */
static int
secsdiffer(struct RingFile *file1, union RingObject const *sec)
{
	struct RingPool *pool = file1->set.pool;
	struct RingFile *file2;
	long retpos=0;
	byte pktbyte;
	int i;

	pgpAssert(OBJISSEC(sec));

	/* Find a matching signature */
	file2 = ringBestFile(pool, sec, 0);
	pgpAssert(file2);
	pgpAssert(file2->f || file2->set.mask == MEMRINGMASK);
	if (file2 == file1 && file1->f && (retpos=pgpFileTell(file1->f)) < 0) {
		ringErr(file1, pgpFileTell(file1->f), PGPERR_KEYIO_FTELL);
		return PGPERR_KEYIO_FTELL;
	}

	pktbyte = OBJISTOPKEY(sec->g.up) ? PKTBYTE_SECKEY :
	PKTBYTE_SECSUBKEY;
	i = ringPacketDiffers(file2, sec, pktbyte, (word32)-1);
	if (i < 0)
		return i;

	if (file2 == file1 && file1->f &&
			pgpFileSeek(file1->f, retpos, SEEK_SET) != 0) {
		ringErr(file1, sec->g.pos.fpos, PGPERR_KEYIO_SEEKING);
		return PGPERR_KEYIO_SEEKING;
	}
	return i;
}

static union RingObject *
ringFindSec(struct RingFile *file, union RingObject *parent)
{
	struct RingPool *pool = file->set.pool;
	union RingObject *sec, **secp;
	int i;

	/* Properties of the secret */
	word32 hash;

	hash = ringHashBuf((byte const *)pool->pktbuf, pool->pktbuflen);
	
	/* Search for matching sigs */
	for (secp=&parent->g.down; (sec=*secp) != NULL; secp=&sec->g.next) {
		if (OBJISSEC(sec)
		&& sec->c.hash == hash
		&& (i = secsdiffer(file, sec)) <= 0)
			return i<0 ? NULL : sec;
	}

	/* Not found - allocate a new secret */
	sec = ringNewSec(pool);
	if (sec) {
			/*
		* Make secret object the first thing. This is assumed by
		* ringFindSig which tries to keep sigs together just past
		* the sec obj. Especially important with subkeys where it's
		* hard to tell which sigs go with which keys as we read.
			*/
		sec->g.next = parent->g.down;
		parent->g.down = sec;
		sec->g.up = parent;
		sec->c.hash = hash;
	}
	return sec;
}

/*
* Return 1 if the name in the ring's pktbuf differs from the
* name in its various homes, 0 if it is the same, and <0 if there
* is some sort of error.
* Does *not* alter file1's read position unless there is an error.
* (I.e. if it needs to seek file1, it saves and restores the file
* position. It doesn't bother for other files. Use the MEMRING file
* if you don't need this feature.)
*/
static int
namesdiffer(struct RingFile *file1, union RingObject const *name)
{
	struct RingPool *pool = file1->set.pool;
	struct RingFile *file2;
	long retpos=0;
	int i;

	/* Find a matching name */
	file2 = ringBestFile(pool, name, 0);
	pgpAssert (file2->f || file2->set.mask == MEMRINGMASK);
	if (file2 == file1 && file1->f && (retpos=pgpFileTell(file1->f)) < 0) {
		ringErr(file1, pgpFileTell(file1->f), PGPERR_KEYIO_FTELL);
		return PGPERR_KEYIO_FTELL;
	}

	if (file2->f &&
	pgpFileSeek(file2->f, name->g.pos.fpos, SEEK_SET) != 0) {
		ringErr(file2, name->g.pos.fpos, PGPERR_KEYIO_SEEKING);
		return PGPERR_KEYIO_SEEKING;
	}
	i = ringPacketDiffers(file2, name, PKTBYTE_NAME, (word32)-1);

	if (file2 == file1 && file1->f &&
			pgpFileSeek(file1->f, retpos, SEEK_SET) != 0) {
		ringErr(file1, name->g.pos.fpos, PGPERR_KEYIO_SEEKING);
		return PGPERR_KEYIO_SEEKING;
	}
	return i;
}

/*
 * Exported variant of namesdiffer. This has a similar function, but
 * a very different interface - it does not assume that the name has been
 * fetched into the pktbuf.
 * Returns 1 if the two names differ, 0 if the same, < 0 on error.
 */
int
ringNamesDiffer (struct RingSet const *set, union RingObject *name1,
	union RingObject *name2)
{
	char const *buf1, *buf2;	/* Pointers to the names */
	unsigned len, tlen;
	word32 hash1, hash2;		/* Hashes of the two names */
	
	pgpAssert (OBJISNAME(name1));
	pgpAssert (OBJISNAME(name2));
	pgpAssert (set->mask & name1->g.mask & name2->g.mask);

	/* Quick test of lengths */
	len = name1->n.len;
	if (len != name2->n.len)
		return 1;

	/* Trivial case: both names are in memory */
	if (NAMEISCACHED(&name1->n) && NAMEISCACHED(&name2->n))
		return memcmp(name1->n.name.ptr, name2->n.name.ptr, len) != 0;

	/* First, compare hashes to see what's what */
	hash1 = NAMEISCACHED(&name1->n)
		? ringHashBuf((const unsigned char *) name1->n.name.ptr, len)
		: name1->n.name.hash;
	hash2 = NAMEISCACHED(&name2->n)
		? ringHashBuf((const unsigned char *) name2->n.name.ptr, len)
		: name2->n.name.hash;
	if (hash1 != hash2)
		return 1;
		/*
	* At this point, we're 99% sure the names are the same, but
	* we need to confirm...
	* Load the first name. This may go into a cache, or the pktbuf.
		*/
	buf1 = ringPoolGetName(ringSetPool(set),
						&name1->n,
						(unsigned long *) &tlen);
	if (!buf1)
		return ringSetError(set)->error;
	pgpAssert(tlen == len);
	/* If name2 is available without using the pktbuf, great. */
	if (NAMEISCACHED(&name2->n)) {
		buf2 = name2->n.name.ptr;
		return memcmp(buf1, buf2, len) != 0;
		}
		/*
	* Otherwise, if name1 isn't in the pktbuf, we can fetch name2
	* without fear of clobbering it.
		*/
	if (NAMEISCACHED(&name1->n)) {
		pgpAssert(buf1 == name1->n.name.ptr);
		buf2 = ringPoolGetName(ringSetPool(set),
							&name2->n,
							(unsigned long *) &tlen);
		if (!buf2)
			return ringSetError(set)->error;
		pgpAssert(tlen == len);
		return memcmp(buf1, buf2, len) != 0;
	}
	/* name1 is not cached, so it's in the pktbuf... */
	pgpAssert(buf1 == ringSetPool(set)->pktbuf);
	/*
	* Otherwise, compare name1 in pktbuf to name2 on disk.
	* Use MEMRING as dummy entry for file1; it is only used to
	* make sure that any seeks which move its position get undone,
	* and we don't care about that at this point.
	*/
	return namesdiffer(&ringSetPool(set)->files[MEMRINGBIT], name2);
}
	

/*
 * Get a struct RingName on the given chain matching the one in the ring's
 * pktbuf. Returns 0 if it runs out of memory or can't read the file.
 * (The ring's error is set to reflect this.)
 * It does *not* add the FilePos or set the mask bit.
 * Does *not* alter the given file's read position unless there is an error.
 */
static union RingObject *
ringFindName(struct RingFile *file, union RingObject *parent)
{
	struct RingPool *pool = file->set.pool;
	union RingObject *name, **np, **pname=NULL;
	size_t const len = pool->pktbuflen;
	word32 hash;
	int i;
	char *buf;	/* Used when trying to cache name */

	hash = ringHashBuf((byte const *)pool->pktbuf, len);

	pname = &parent->g.down;
	for (np = &parent->g.down; (name=*np) != NULL; np = &name->g.next) {
		/* Position name after names, key-sigs, sec's */
		if (OBJISNAME(name) || OBJISSIG(name) || OBJISSEC(name))
			pname = &name->g.next;
		/* If not a name or wrong length, no match */
		if (!OBJISNAME(name) || name->n.len != len)
			continue;

		/* If in memory, compare that */
		if (NAMEISCACHED(&name->n)) {
			if (memcmp(name->n.name.ptr, pool->pktbuf, len) == 0)
				return name;	/* Success */
		} else {
			if (name->n.name.hash == hash
			&& (i = namesdiffer(file, name)) <= 0)
				return i<0 ? NULL : name;
		}
	}

	/* Failed to find a name - create one */
	name = ringNewName(pool);
	if (name) {
		name->g.next = *pname;
		*pname = name;
		name->g.up = parent;
		name->n.len = len;
		name->n.name.hash = hash;	/* May overwrite below */
		/* Default new names to unknown trust */
		name->n.trust = PGP_NAMETRUST_UNKNOWN;
#if PGPTRUSTMODEL>0
		name->n.valid = name->n.validity = 0;
		name->n.confidence = PGP_NEWTRUST_UNDEFINED;
		NAMESETNEWTRUST(&name->n);
#endif
#if 1
		/* You might want to disable this for MSDOS */
		if (!(file->set.mask & MEMRINGMASK)) {
			/* Don't "cache" in memring, it is for new creations */
			buf = (char *)memPoolAlloc(&file->strings, len, 1);
			if (buf) {
				memcpy(buf, pool->pktbuf, len);
				name->n.name.ptr = buf;
				name->g.flags &= ~NAMEF_FILEMASK;
				name->g.flags |= ringLsBitFind(file->set.mask);
				NAMESETCACHED(&name->n);
			}
		}
#endif
	}
	return name;
}

/*
 * Return 1 if the signature in the ring's pktbuf differs from the
 * sig in its various homes, 0 if it is the same, and <0 if there
 * is some sort of error.
 * Does *not* alter file1's read position unless there is an error.
 */
static int
sigsdiffer(struct RingFile *file1, union RingObject const *sig)
{
	struct RingPool *pool = file1->set.pool;
	struct RingFile *file2;
	long retpos=0;
	int i;
	int bit;

	/* Find a matching signature */
	bit = ringLsBitFind(sig->g.mask & pool->filemask);
	pgpAssert(bit >= 0);
	file2 = &pool->files[bit];
	pgpAssert(file2->f || file2->set.mask == MEMRINGMASK);
	if (file2 == file1 && file1->f &&
			(retpos = pgpFileTell(file1->f)) < 0) {
		ringErr(file1, pgpFileTell(file1->f), PGPERR_KEYIO_FTELL);
		return PGPERR_KEYIO_FTELL;
	}

	if (file2->f &&
			pgpFileSeek(file2->f, sig->g.pos.fpos, SEEK_SET) != 0) {
		ringErr(file2, sig->g.pos.fpos, PGPERR_KEYIO_SEEKING);
		return PGPERR_KEYIO_SEEKING;
	}
	i = ringPacketDiffers(file2, sig, PKTBYTE_SIG, (word32)-1);

	if (file2 == file1 && file1->f &&
			pgpFileSeek(file1->f, retpos, SEEK_SET) != 0) {
		ringErr(file1, sig->g.pos.fpos, PGPERR_KEYIO_SEEKING);
		return PGPERR_KEYIO_SEEKING;
	}
	return i;
}

static union RingObject *
ringFindSig(struct RingFile *file, union RingObject *parent)
{
	struct RingPool *pool = file->set.pool;
	union RingObject *key;
	union RingObject *sig, **sigp, **fsigp;
	int i;

	/* Properties of the signature */
	int err;
	byte pkalg, keyID[8];
	word32 tstamp;
	word16 validity;
	byte type;	
	byte hashalg;
	byte version;
	size_t extralen;

	pkalg = 0;
	err = ringSigParse((byte const *)pool->pktbuf, pool->pktbuflen,
	&pkalg, keyID, &tstamp, &validity, &type,
	&hashalg, &extralen, &version);

	/* Get key this signature is by */
	key = ringPoolFindDummyKey(pool, NULL, pkalg, keyID);
	if (!key)
		return NULL;

	/* Search for matching sigs */
	fsigp = &parent->g.down;
	for (sigp = fsigp; (sig=*sigp) != NULL; sigp = &sig->g.next) {
		if (OBJISSEC(sig) || OBJISSIG(sig))
			fsigp = &sig->g.next; /* predecessor to new sig */
		if (OBJISSIG(sig)
		&& sig->s.by == key
		&& (sig->s.by->k.pkalg == pkalg || pkalg==1) /* ViaCrypt */
		&& sig->s.tstamp == tstamp
		&& sig->s.type == type
			&& sig->s.version == version
				&& sig->s.validity == validity
				&& sig->s.hashalg == hashalg
				&& !(sig->g.flags & SIGF_ERROR) == !err
				&& !(sig->g.flags & SIGF_NONFIVE) == (extralen == 5)
				&& (i = sigsdiffer(file, sig)) <= 0)
			{
			return i<0 ? NULL : sig;
		}
	}

	/* Not found - allocate a new sig */
	sig = ringNewSig(pool);
	if (sig) {
		/* Add new sig before any user ID's */
		sig->g.next = *fsigp;
		*fsigp = sig;
		sig->s.by = (union RingObject *)key;
		sig->g.up = parent;
		if (err)
			sig->g.flags |= SIGF_ERROR;
		if (extralen != 5)
			sig->g.flags |= SIGF_NONFIVE;
		if (OBJISKEY(parent))
			sig->g.flags |= SIGF_KEY;
		sig->s.tstamp = tstamp;
		sig->s.validity = validity;
		sig->s.type = type;
		sig->s.version = version;
		sig->s.hashalg = hashalg;
		sig->s.trust = 0;
	}
	return sig;
}

/*
 * Return 1 if the blob in the ring's pktbuf differs from the
 * blob in its various homes, 0 if it is the same, and <0 if there
 * is some sort of error.
 * Does *not* alter file1's read position unless there is an error.
 */
static int
unkdiffer(struct RingFile *file1, union RingObject const *unk)
{
	struct RingPool *pool = file1->set.pool;
	struct RingFile *file2;
	long retpos=0;
	int i;
	int bit;

	/* Find a matching unk */
	file2 = ringBestFile(pool, unk, 0);
	bit = ringLsBitFind(unk->g.mask & pool->filemask);
	pgpAssert(bit >= 0);
	file2 = &pool->files[bit];
	pgpAssert (file2->f || file2->set.mask == MEMRINGMASK);
	if (file2 == file1 && file1->f && (retpos=pgpFileTell(file1->f)) < 0) {
		ringErr(file1, pgpFileTell(file1->f), PGPERR_KEYIO_FTELL);
		return PGPERR_KEYIO_FTELL;
	}

	if (file2->f &&
			pgpFileSeek(file2->f, unk->g.pos.fpos, SEEK_SET) != 0) {
		ringErr(file2, unk->g.pos.fpos, PGPERR_KEYIO_SEEKING);
		return PGPERR_KEYIO_SEEKING;
	}
	i = ringPacketDiffers(file2, unk, PKTBYTE_TYPE(unk->u.pktbyte),
	pool->pktbuflen+1);

	if (file2 == file1 && file1->f &&
			pgpFileSeek(file1->f, retpos, SEEK_SET) != 0) {
		ringErr(file1, unk->g.pos.fpos, PGPERR_KEYIO_SEEKING);
		return PGPERR_KEYIO_SEEKING;
	}
	return i;
}

/*
 * Get a struct RingUnk on the given chain matching the one in the ring's
 * pktbuf with the given pktbyte. Returns 0 if it runs out of memory
 * or can't read the file. (The ring's error is set to reflect this.)
 * It does *not* add the FilePos or set the mask bit.
 * Does *not* alter the given file's read position unless there is an error.
*/
static union RingObject *
ringFindUnk(struct RingFile *file, union RingObject *parent, byte pktbyte)
{
	struct RingPool *pool = file->set.pool;
	union RingObject *unk, **unkp;
	word32 hash;
	int i;

	hash = ringHashBuf((byte const *)pool->pktbuf, pool->pktbuflen);

	for (unkp = &parent->g.down; (unk=*unkp) != NULL;
			unkp = &unk->g.next) {
			/*
		* If the type, hash and pktbyte match, try to compare...
		* Note that the pktbyte *must* match or namesdiffer()
		* will complain loudly.
			*/
		if (OBJISUNK(unk)
		&& unk->u.hash == hash
		&& unk->u.pktbyte == pktbyte
		&& (i = unkdiffer(file, unk)) <= 0)
		{
			return i<0 ? NULL : unk;
		}
	}

	/* Failed to find a name - create one */
	unk = ringNewUnk(pool);
	if (unk) {
		*unkp = unk;
		unk->g.up = parent;
		unk->u.pktbyte = pktbyte;
		unk->u.hash = hash;
		/* Default new names to unknown trust */
	}
	return unk;
}

/* Return the list of trpuble associated with a RingFile. */
struct RingTrouble const *
ringFileTrouble(struct RingFile const *file)
{
	return file->trouble;
}

/*
 * Functions for manipulating the Trouble list.
*/

/* Zero out a RingFile's Trouble list; it has been dealt with. */
void
ringFilePurgeTrouble(struct RingFile *file)
{
	memPoolEmpty(&file->troublepool);
	file->trouble = NULL;
	file->troubletail = &file->trouble;
}

/*
 * Log some trouble with a RingFile.
 */
static int
ringFileLog(struct RingFile *file, union RingObject *obj, word32 num,
	word32 fpos, int type)
{
	struct RingTrouble *t;

	t = (struct RingTrouble *)memPoolNew(&file->troublepool,
			struct RingTrouble);
	if (!t)
		return PGPERR_NOMEM;

	t->next = NULL;
	t->obj = obj;
	t->num = num;
	t->fpos = fpos;
	t->type = type;

	*file->troubletail = t;
	file->troubletail = &t->next;
	return 0;
}

/*
 * @@@ This needs fixing - we should complain about a key without
 * names in *this* file, even if it has names in others.
 */
static union RingObject const *
ringKeyHasName(union RingObject const *obj)
{
	for (obj = obj->g.down; obj; obj = obj->g.next)
		if (OBJISNAME(obj))
			break;
	return obj;
}

/*
 * How to skip things. Various helper functions return either
 * negative fatal error codes or these codes indicating what to
 * do to recover from any warnings. These never exceed 15 bits,
 * so "int" is the right type.
*/

typedef int skip_t;
#define SKIP_TRUST 1
#define SKIP_SIG 2
#define SKIP_NAME 4
#define SKIP_SUBKEY 8
#define SKIP_KEY 16

#define SKIP_SIGS (SKIP_TRUST | SKIP_SIG)
#define SKIP_TO_KEY (SKIP_SIGS | SKIP_NAME | SKIP_SUBKEY)

#define PB_PUBKEY(key) (OBJISTOPKEY(key) ? \
	PKTBYTE_BUILD(PKTBYTE_PUBKEY, 0) : \
	PKTBYTE_BUILD(PKTBYTE_PUBSUBKEY, 0))
#define PB_SECKEY(key) (OBJISTOPKEY(key) ? \
	PKTBYTE_BUILD(PKTBYTE_SECKEY, 0) : \
	PKTBYTE_BUILD(PKTBYTE_SECSUBKEY, 0))
#define PB_SIG PKTBYTE_BUILD(PKTBYTE_SIG, 0)

/*
 * Add a new key to the Pool, with its *parent* at the given level
 * (0 means adding top-level key) in the RingIterator. Leave the
 * RingIterator pointing to the newly created key. Return <0 on error,
 * 0 if the key was created, and a skip code > 0 if it was created with
 * a warning of some sort.
*/
static int
ringAddKey(struct RingFile *file, struct RingIterator *iter, word32 fpos,
	int trusted, byte pkttype)
{
	union RingObject *key, *sec, *parent;
	byte pkalg, keyID[8];
	word16 keybits;
	word32 tstamp;
	word16 validity;
	int i, err, level;

	pkalg = 0;
	err = ringKeyParse((byte const *)file->set.pool->pktbuf,
	file->set.pool->pktbuflen, &pkalg,
	keyID, &keybits, &tstamp, &validity, 0);

	if (pkttype == PKTBYTE_PUBSUBKEY) {
		/* Subkey */
		if (!iter->level) {
			i = ringFileLog(file, NULL, 0, fpos,
					PGPERR_TROUBLE_UNXSUBKEY);
			return i < 0 ? i : SKIP_SIGS;
		}
		parent = iter->stack[0];
		level = 1;
	} else {
		parent = NULL;
		level = 0;

		if (iter->level) {
			key = iter->stack[0];
			pgpAssert(OBJISKEY(key));

			/* Complain about a key with no Names */
			if (!ringKeyHasName(key)) {
				i = ringFileLog(file, key, 0,
						ringFilePos(key, file)->fpos,
						PGPERR_TROUBLE_BAREKEY);
				if (i < 0)
					return i;
			}

			/* @@@ Should we not care about this? */
			/* If keys not in sorted order, dirty */
#if 0
			/* @@@ ringSortKeys orders keys by algorithm
			so this test no longer works. */
			if (ringKeyIDcmp(key->k.keyID, keyID) > 0)
				ringFileMarkDirty(file);
#endif
		}
	}

	/* Find the matching key structure */
	key = ringPoolFindDummyKey(file->set.pool, parent, pkalg, keyID);
	if (!key)
		return ringFileError(file)->error;

	/* See if it's a new key or the same key */
	if (key->g.mask == 0) {
		/* Newly created dummy key; fill in info */
		if (pkttype == PKTBYTE_PUBSUBKEY && OBJISTOPKEY(key)) {
				/*
			* Here on a former dummy top-level key which
			* has turned out to be a subkey. Dummy key
			* would have been created if we saw a sig by
			* it. Change key to a subkey.
				*/
			i = ringFileLog(file, NULL, 0, fpos,
					PGPERR_TROUBLE_SIGSUBKEY);
			if (i < 0)
				return i;
			ringPoolUnlinkKey(file->set.pool, key);
			key->k.flags |= RINGOBJF_SUBKEY;
			ringPoolLinkKey(file->set.pool, parent, key,
					pkalg, keyID);
		}

		key->k.pkalg = pkalg;	/* ViaCrypt */
		key->k.tstamp = tstamp;
		key->k.validity = validity;
		key->k.keybits = keybits;
		key->k.trust = 0;
		if (err)
			key->g.flags |= KEYF_ERROR;

	} else if (keybits != key->k.keybits
			|| pkalg != key->k.pkalg		/* ViaCrypt */
			|| tstamp != key->k.tstamp
/*			|| validity != key->k.validity	Validity may be in sig */
			|| parent != (OBJISTOP(key) ? NULL : key->g.up)
			|| !(key->g.flags & KEYF_ERROR) != !err)
	{
		i = ringFileLog(file, key, 0, fpos,
		                PGPERR_TROUBLE_DUPKEYID);
		return i < 0 ? i : SKIP_TO_KEY;

	} else if ((i=keysdiffer(file,key,PB_PUBKEY(key))) != 0) {
		if (i < 0)
			return i;
		if ((OBJISTOPKEY(key) && pkttype==PKTBYTE_PUBSUBKEY) ||
		(OBJISSUBKEY(key) && pkttype==PKTBYTE_PUBKEY)) {
			i = ringFileLog(file, key, 0, fpos,
				PGPERR_TROUBLE_KEYSUBKEY);
			return (i < 0) ? i : SKIP_TO_KEY;
		}
/* KLUDGE: version byte bug */
			/*
		* A key with a version byte of 2 only overrides previous
		* keys with a byte of 3 if they are all untrusted
		* secret keys and this is either a secret key or from
		* a trusted public keyring. A key with a version byte
		* of 3 is only overridden if it's secret and previous
		* keys include a secret key or a trusted public key.
			*/
		if (i == PGPVERSION_2
				&& trusted
				&& !(key->g.flags & RINGOBJF_TRUST)
				&& !((ringKeySecMask(key) ^ key->g.mask)
				& file->set.pool->filemask))
			{
			i = ringFileLog(file, key, 0, fpos,
			PGPERR_TROUBLE_VERSION_BUG_PREV);
			if (i < 0)
				return i;
			/* Set all those secret version bug flags */
			for (sec = key->g.down; sec; sec = sec->g.next)
				if (OBJISSEC(sec))
					sec->g.flags |= SECF_VERSION_BUG;
		} else {
/* End of KLUDGE */
			i = ringFileLog(file, key, 0, fpos,
					PGPERR_TROUBLE_DUPKEYID);
			if (i < 0)
				return i;
			return i < 0 ? i : SKIP_TO_KEY;
			}
		}

		/*
		* Already present in this keyring?
		* If so, accept following sigs & userids (they may
		* not be duplicates), but flag a warning.
		*/
	if (key->g.mask & file->set.mask) {
		i = ringFileLog(file, key, 0, fpos, PGPERR_TROUBLE_DUPKEY);
		if (i < 0)
			goto failed;
		i = SKIP_TRUST;
	} else {
		/* Add the FilePos */
		i = ringAddPos(key, file, fpos);
		if (i < 0)
			goto failed;
	}
	/* Add successful; indicate it in the mask */
	key->g.mask |= iter->set.mask;
	iter->stack[level] = key;
	iter->level = level+1;
	return i;

failed:
	if (!key->g.mask)
		ringFreeDummyKey(file->set.pool, key);
	return i;
}

/*
 * Add a new secret to the Pool, as two objects: a key (with its *parent*
 * at the given level in the RingIterator; 0 means add a top-level key)
 * and a signature as its child. Leave the RingIterator pointing to the
 * newly created secret. Return <0 on error, 0 if the key was created,
 * and a skip code >0 if they key was created with a warning of some sort.
 *
 * This is *ridiculously* hairy. Is there a way to clean it up?
 * @@@ TODO: Add PGPERR_TROUBLE_OLDSEC and _NEWSEC handling.
 */
static int
ringAddSec(struct RingFile *file, struct RingIterator *iter, word32 fpos,
	byte pkttype)
{
	union RingObject *key, *sec, *parent;
	byte pkalg, keyID[8];
	word16 keybits;
	word32 tstamp;
	word16 validity;
	int i, err;
	int level;
	int flags = 0;

	pkalg = 0;
	err = ringKeyParse((byte const *)file->set.pool->pktbuf,
	file->set.pool->pktbuflen, &pkalg,
	keyID, &keybits, &tstamp, &validity, 1);

	if (pkttype == PKTBYTE_SECSUBKEY) {
		/* Subkey */
		if (!iter->level) {
			i = ringFileLog(file, NULL, 0, fpos,
					PGPERR_TROUBLE_UNXSUBKEY);
			return i < 0 ? i : SKIP_SIGS;
		}
		parent = iter->stack[0];
		level = 1;
	} else {
		parent = NULL;
		level = 0;

		if (iter->level) {
			key = iter->stack[0];
			pgpAssert(OBJISKEY(key));

			/* Complain about a key with no Names */
			if (!ringKeyHasName(key)) {
				i = ringFileLog(file, key, 0,
						ringFilePos(key, file)->fpos,
						PGPERR_TROUBLE_BAREKEY);
				if (i < 0)
					return i;
			}

			/* @@@ Should we not care about this? */
			/* If keys not in sorted order, dirty */
#if 0
			/* @@@ ringSortKeys orders keys by algorithm
			so this test no longer works. */
			if (ringKeyIDcmp(key->k.keyID, keyID) > 0)
				ringFileMarkDirty(file);
#endif
		}
	}

	/* Find the matching key structure */
	key = ringPoolFindDummyKey(file->set.pool, parent, pkalg, keyID);
	if (!key)
		return ringFileError(file)->error;

	/* See if it's a new key or the same key */
	if (key->g.mask == 0) {
		/* Newly created dummy key; fill in info */
		if (pkttype == PKTBYTE_SECSUBKEY && OBJISTOPKEY(key)) {
				/*
			* Here on a former dummy top-level key which
			* has turned out to be a subkey. Dummy key
			* would have been created if we saw a sig by
			* it. Change key to a subkey.
				*/
			i = ringFileLog(file, NULL, 0, fpos,
					PGPERR_TROUBLE_SIGSUBKEY);
			if (i < 0)
				return i;
			ringPoolUnlinkKey(file->set.pool, key);
			key->k.flags |= RINGOBJF_SUBKEY;
			ringPoolLinkKey(file->set.pool, parent, key,
					pkalg, keyID);
		}

		/* Newly created dummy key; fill in info */
		key->k.pkalg = pkalg;	/* ViaCrypt */
		key->k.tstamp = tstamp;
		key->k.validity = validity;
		key->k.keybits = keybits;
		key->k.trust = 0;
		if (err)
			key->g.flags |= KEYF_ERROR;

	} else if (keybits != key->k.keybits
			|| pkalg != key->k.pkalg		/* ViaCrypt */
			|| tstamp != key->k.tstamp
/*			|| validity != key->k.validity	Validity may be in sig */
			|| parent != (OBJISTOP(key) ? NULL : key->g.up)
			|| !(key->g.flags & KEYF_ERROR) != !err)
	{
		i = ringFileLog(file, key, 0, fpos, PGPERR_TROUBLE_DUPKEYID);
		return i < 0 ? i : SKIP_TO_KEY;

	} else if ((i=keysdiffer(file,key,PB_SECKEY(key))) != 0) {
		if (i < 0)
			return i;
		if ((OBJISTOPKEY(key) && pkttype==PKTBYTE_PUBSUBKEY) ||
		(OBJISSUBKEY(key) && pkttype==PKTBYTE_PUBKEY)) {
			i = ringFileLog(file, key, 0, fpos,
				PGPERR_TROUBLE_KEYSUBKEY);
			return (i < 0) ? i : SKIP_TO_KEY;
		}
/* KLUDGE: version byte bug */
		/*
		* A key with a version byte of 2 only overrides previous
		* keys with a byte of 3 if they are all untrusted
		* secret keys and this is either a secret key or from
		* a trusted public keyring. A key with a version byte
		* of 3 is only overridden if it's secret and previous
		* keys include a secret key or a trusted public key.
			*/
		if (i == PGPVERSION_2
		&& !(key->g.flags & RINGOBJF_TRUST)
		&& !((ringKeySecMask(key) ^ key->g.mask) &
			file->set.pool->filemask))
		{
			i = ringFileLog(file, key, 0, fpos,
			PGPERR_TROUBLE_VERSION_BUG_PREV);
			if (i < 0)
				return i;
			/* Set all those secret version bug flags */
			for (sec = key->g.down; sec; sec = sec->g.next)
				if (OBJISSEC(sec))
					sec->g.flags |= SECF_VERSION_BUG;
		} else if (i == PGPVERSION_2_6
		&& (key->g.flags & RINGOBJF_TRUST
		|| ringKeySecMask(key)))
		{
			i = ringFileLog(file, key, 0, fpos,
			PGPERR_TROUBLE_VERSION_BUG_CUR);
			if (i < 0)
				return i;
			flags |= SECF_VERSION_BUG;
			/* Fix the problem */
			((byte *)file->set.pool->pktbuf)[0] = PGPVERSION_2;
		} else {
/* End of KLUDGE */
			i = ringFileLog(file, key, 0, fpos,
					PGPERR_TROUBLE_DUPKEYID);
			if (i < 0)
				return i;
			return i < 0 ? i : SKIP_TO_KEY;
		}
	}

	/* Okay, we've got the public key. Now add the secret. */

	sec = ringFindSec(file, key);
	if (!sec) {
		i = ringFileError(file)->error;
		goto failed;
	}
	sec->c.flags |= flags;

	if (sec->g.mask & file->set.mask) {
		i = ringFileLog(file, key, 0, fpos, PGPERR_TROUBLE_DUPSEC);
		if (i < 0)
			goto failed;
		i = SKIP_TRUST;
	} else {
		/* Add the FilePos */
		i = ringAddPos(sec, file, fpos);
		if (i < 0) {
			pgpAssert(sec->g.mask);
			goto failed;
			}
		}

		/*
		* Already present in this keyring?
		* If so, accept following sigs & userids (they may
		* not be duplicates), but flag a warning.
		* If the previous instances were all public keys,
		* but this is a secret key, "upgrade" the reference.
		*/
	if (key->g.mask & file->set.mask) {
		/* Only do this if it wasn't a duplicate *secret* */
		if (i != SKIP_TRUST) {
			i = ringFileLog(file, key, 0, fpos,
					PGPERR_TROUBLE_DUPKEY);
			if (i < 0)
				goto failed;
			ringAlterFilePos(key, file, fpos);
			i = SKIP_TRUST;
		}
	} else {
		/* Add the FilePos */
		err = i;
		i = ringAddPos(key, file, fpos);
		if (i < 0)
			goto failed;
		i = err;
	}
	/* Add successful; indicate it in the mask */
	key->g.mask |= iter->set.mask;
	sec->g.mask |= iter->set.mask;
	iter->stack[level] = key;
	iter->stack[level+1] = sec;
	iter->level = level+2;
	return i;

failed:
	if (!key->g.mask)
		ringFreeDummyKey(file->set.pool, key);
	return i;
}

/*
 * Add the name in the RingPool's pktbuf to the sets mentioned
 * in the given iterator, which points to the location where the
 * name should be added. Leaves the RingIterator pointing to the
 * name just added.
 */
static int
ringAddName(struct RingFile *file, struct RingIterator *iter, word32 fpos)
{
	union RingObject *name;
	int i;

	pgpAssert(iter->level >= 1);
	name = ringFindName(file, iter->stack[0]);
	if (!name)
		return ringFileError(file)->error;

	/*
	* Already present in this keyring?
	*
	* If so, accept following sigs (they may not be
	* duplicates), but flag a warning.
	*/
	if (name->g.mask & file->set.mask) {
		i = ringFileLog(file, name, 0, fpos, PGPERR_TROUBLE_DUPNAME);
		if (i < 0)
			return i;
		i = SKIP_TRUST;
	} else {
		/* Add the FilePos */
		i = ringAddPos(name, file, fpos);
		if (i < 0) {
			pgpAssert(name->g.mask);
			return i;
		}
	}
	name->g.mask |= iter->set.mask;	/* All done */
	iter->stack[1] = name;
	iter->level = 2;
	return i;
}

static int
ringAddSig(struct RingFile *file, struct RingIterator *iter, word32 fpos)
{
	union RingObject *sig;
	int level;
	int i;

	pgpAssert(iter->level >= 1);
	/* Stack position of parent: 0 or 1 */
	level = (iter->level > 1 &&
		(OBJISNAME(iter->stack[1]) || OBJISSUBKEY(iter->stack[1])));
	sig = ringFindSig(file, iter->stack[level]);
	if (!sig)
		return ringFileError(file)->error;

	/* Already present in this keyring? Complain. */
	if (sig->g.mask & file->set.mask) {
		i = ringFileLog(file, sig, 0, fpos, PGPERR_TROUBLE_DUPSIG);
		if (i < 0)
			return i;
		i = SKIP_TRUST;
	} else {
		/* Add the FilePos */
		i = ringAddPos(sig, file, fpos);
		if (i < 0) {
			pgpAssert(sig->g.mask);
			return i;
		}
	}
	sig->g.mask |= iter->set.mask;	/* All done */
	iter->stack[level+1] = sig;
	iter->level = level+2;
	return i;
}

/*
 * Add the unknown object in the RingPool's pktbuf to the sets mentioned
 * in the given iterator, which points to the location where the
 * object should be added. Leaves the RingIterator pointing to the
 * object just added.
 */
static int
ringAddUnk(struct RingFile *file, struct RingIterator *iter, word32 fpos,
	byte pktbyte)
{
	union RingObject *unk;
	int level;
	int i;

	pgpAssert(iter->level >= 1);
	
	/* Stack position of parent: 0 or 1 */
	level = iter->level - 1;
	/* Place this */
	if (OBJISUNK(iter->stack[level]) && level)
		level--;
	/* Can't put it below a bottom-level object, make it a sib */
	if (OBJISBOT(iter->stack[level]) && level)
		level--;
	unk = ringFindUnk(file, iter->stack[level], pktbyte);
	if (!unk)
		return ringFileError(file)->error;

	/*
	* Already present in this keyring? If so, warn...
	*/
	if (unk->g.mask & file->set.mask) {
		i = ringFileLog(file, unk, 0, fpos, PGPERR_TROUBLE_DUPUNK);
		if (i < 0)
			return i;
		i = SKIP_TRUST;
	} else {
		/* Add the FilePos */
		i = ringAddPos(unk, file, fpos);
		if (i < 0) {
			pgpAssert(unk->g.mask);
			return i;
		}
	}
	/* All done, we're happy... */
	unk->g.mask |= iter->set.mask;
	iter->stack[level+1] = unk;
	iter->level = level+2;
	return i;
}


 /*** The feature presentation ***/

/*
 * KLUDGE: PGP 2.6 had a bug wherein if you changed the passphrase on
 * a secret key, the newly encrpyted secret key would be written out
 * with a version byte of PGPVERSION_2_6 (= 3), even if the original
 * was PGPVERSION_2 (= 2). PGP 2.6 didn't notice the problem and
 * would continue to function very happily with the problem.
 *
 * The code here notices and attempts to undo the problem,
 * "overriding" the version byte of 3 and forming a consistent
 * idea of the key's version byte.
 * The only problem is avoiding a denial-of-service attack if
 * someone sends me a public key with a version byte of 2 and
 * I compare it with my secret key with a (correct) version
 * byte of 3, and fix my secret key to 2 to correspond.
 * It could cause problems unfixable except with a binary file
 * editor. So we only accept evidence of the bug under certain
 * conditions. To be precise:
 *
 * - A key with a version byte of 2 can only override the previous
 * version bytes of 3 if all previous keys are secret keys
 * without trust information and the key is either a secret key
 * or from a trusted public keyring.
 * - A key with a version byte of 3 is only overridden by previous
 * keys if it is a secret key and the previous keys include a
 * secret key or a key from a trusted keyring.
 *
 * A Trouble record is logged in either case. TROUBLE_VERSION_BUG_PREV
 * if previous keys had the version bug and TROUBLE_VERSION_BUG_CUR if
 * the current key has it.
 *
 * - Fixing instructions:
 * - We want to split out a function to add a single object.
 * - Replace parse tree state with ringIterator.
 * - Fiddle with trust packets somehow.
 * - Use the fact that ringFileDoClose gets rid of half-built
 * keys to simplify the public-key-gathering part.
 * DANGER, WILL ROBINSON: What about running out of memory when creating
 * a new key? That's one case when it's *not* desirable to close
 * the whole file. Grumble moan bitch complain...
 *
 * Okay a few cases:
 * - The secret is old, as is the key.
 * In that case, we're only ZZ
 */
struct RingFile *
ringFileOpen(struct RingPool *pool, struct PgpFile *f, int trusted, int *error)
{
	/* Oy, what a lot of variables! */

	/* The RingFile being opened */
	struct RingFile *file;
	ringmask mask;
	int bit;

	/* The current state of the parse tree */
	struct RingIterator iter;
	union RingObject *obj;

	/* Current packet info */
	int pktbyte;
	word32 len, fpos;
	char *buf;		/* Current pktbuf */

	/* Information from selfsig subpackets associated with parent key */
	union RingObject *sigkeyparent = NULL;
	word32				sigkeyvalidity = 0;

	/* Various temporaries */
	int i;			/* Multi-purpose temp */
#if PGPTRUSTMODEL>0
	int j, k;	 	/* Additional trust packet bytes */
#endif
	byte c;			/* for pgpFileRead */
	size_t size;		/* Return value from pgpFileRead() */
	byte *tp;	 	/* Pointer to trust byte */
	struct RingTrouble const *trouble;

	/* Flags */
	int dirty = 0, trustdirty = 0;
	skip_t skip;
	int trustf;		/* 1 for trust, 2 for optional trust */
	int trustmissing;	 /* OR of trustf values */

	/*
	* The code starts here
	*/
	if (!f || !pool) {
		*error = 0;
		return (struct RingFile *)NULL;
	}
		
	pgpAssert(!(pool->filemask & ~pool->allocmask));

	bit = ringBitAlloc(pool);
	if (bit < 0) {
		*error = PGPERR_NO_KEYBITS;
		return NULL;
	}
	pgpAssert(bit < MEMRINGBIT);

	file = &pool->files[bit];
	mask = (ringmask)1 << bit;
	pgpAssert(file->set.mask == mask);

	pool->filemask |= mask;
	pool->allocmask |= mask;

	/* Set this file as the lowest-priority file */
	ringFileLowPri(file);

	pgpAssert(!file->set.next);

	file->f = f;
	file->destructor = NULL;
	file->arg = NULL;
	file->version = PGPVERSION_2;

	/* Start out with a clean slate */
	*error = 0;

	/* Initialize a null RingIterator */
	iter.set.pool = pool;
	iter.set.mask = 0;

	/* There is no current key */
	iter.level = 0;
	/* There is no current packet to give trust to */
	trustf = 0;
	/* No trust is missing, yet */
	trustmissing = 0;
	/* Pay attention to all the packets */
	skip = 0;
	

	/*
	* The main loop over each packet.
	*/
	while ((pktbyte = pktByteGet(file->f, &len, &fpos)) > 0) {
		switch (PKTBYTE_TYPE(pktbyte)) {
skipkey:
			skip = SKIP_TO_KEY;
			goto skippkt;
skipname:
			skip = SKIP_SIGS;
			goto skippkt;
skipunk:
skipsig:
			skip = SKIP_TRUST;
			goto skippkt;
			default:
				/*
			* Add it as an unknown object if it's not the first
			* thing in the keyring...
				*/
			if (!iter.level) {
				i = ringFileLog(file, (union RingObject *)NULL,
					0, fpos, PGPERR_TROUBLE_UNXUNK);
				if (i < 0)
					goto failed;
				goto skipunk;
			}
			if (len > RINGUNK_MAXLEN) {	/* Too big */
				i = ringFileLog(file, (union RingObject *)NULL,
					len, fpos, PGPERR_TROUBLE_UNK2BIG);
				if (i < 0)
					goto failed;
				goto skipunk;
			}
			/* Read in the unknown for hashing */
			buf = ringReserve(pool, (size_t)len);
			if (!buf)
				goto fatal;
			size = pgpFileRead(buf, (size_t)len, file->f);
			if (size != (size_t)len)
				goto readerr;
			pool->pktbuflen = size;

			i = ringAddUnk(file, &iter, fpos, pktbyte);
			if (i < 0)
				goto failed;
			obj = iter.stack[iter.level-1];
			
			/* Unexpected packet */
			i = ringFileLog(file, obj, pktbyte, fpos,
			PGPERR_TROUBLE_UNKPKTBYTE);
			if (i < 0)
				goto failed;
			break;
		case PKTBYTE_COMMENT:
			/* Silently ignore this packet */
#ifndef WRAP_NONE
				/* peek inside comment packets if we wrap them */
				if (len) {
					byte innerbyte;
					/* Peek at first byte, then back up */
					i = pgpFileRead(&innerbyte, 1, file->f);
					pgpFileSeek (file->f, -1, SEEK_CUR);
					if (i < 0)
						goto readerr;
					if (PKTBYTE_TYPE(innerbyte) == PKTBYTE_PUBKEY ||
						PKTBYTE_TYPE(innerbyte) == PKTBYTE_PUBSUBKEY) {
						break;	/* Read contents of comment packet */
					}
			}
#endif
skippkt:
			trustmissing |= trustf;
			trustf = 0;
			dirty = 1;
skiptrust:
			i = pgpFileSeek(file->f, len, SEEK_CUR);
			if (i != 0)
				goto readerr;
			break;

		case PKTBYTE_PUBKEY:
		case PKTBYTE_PUBSUBKEY:
			trustmissing |= trustf;
			trustf = 1;

			if (skip & SKIP_KEY)
				goto skippkt;
			skip = 0;

			/* Check for grossly oversized key */
			if (len > RINGKEY_MAXLEN) {
				i = ringFileLog(file, (union RingObject *)NULL,
					len, fpos, PGPERR_TROUBLE_KEY2BIG);
				if (i < 0)
					goto failed;
				goto skipkey;
			}
			buf = ringReserve(pool, (size_t)len);
			if (!buf)
				goto fatal;
			size = pgpFileRead(buf, (size_t)len, file->f);
			if (size != (size_t)len)
				goto readerr;
			pool->pktbuflen = size;

			i = ringAddKey(file, &iter, fpos, trusted,
				PKTBYTE_TYPE(pktbyte));
			if (i < 0)
				goto failed;
			skip = i;
			trustf = 1;

			break;

		case PKTBYTE_SECKEY:
		case PKTBYTE_SECSUBKEY:
			trustmissing |= trustf;
			trustf = 1;

			if (skip & SKIP_KEY)
				goto skippkt;
			skip = 0;

			/* Check for grossly oversized key */
			if (len > RINGSEC_MAXLEN) {
				i = ringFileLog(file, (union RingObject *)NULL,
					len, fpos, PGPERR_TROUBLE_KEY2BIG);
				if (i < 0)
					goto failed;
				goto skipkey;
			}
			buf = ringReserve(pool, (size_t)len);
			if (!buf)
				goto fatal;
			size = pgpFileRead(buf, (size_t)len, file->f);
			if (size != (size_t)len)
				goto readerr;
			pool->pktbuflen = size;

			i = ringAddSec(file, &iter, fpos,
					PKTBYTE_TYPE(pktbyte));
			if (i < 0)
				goto failed;
			skip = i;
			trustf = 1;

			break;

		case PKTBYTE_NAME:
			trustmissing |= trustf;
			trustf = 1;

			if (skip & SKIP_NAME)
				goto skippkt;
			skip = 0;

			if (iter.level < 1) {
				i = ringFileLog(file, (union RingObject *)NULL,
					0, fpos, PGPERR_TROUBLE_UNXNAME);
				if (i < 0)
					goto failed;
				goto skipname;
			}
			/* Check for grossly oversized name */
			if (len > RINGNAME_MAXLEN) {
				i = ringFileLog(file, (union RingObject *)NULL,
					len, fpos, PGPERR_TROUBLE_NAME2BIG);
				if (i < 0)
					goto failed;
				goto skipname;
			}
			buf = ringReserve(pool, (size_t)len);
			if (!buf)
				goto fatal;
			size = pgpFileRead(buf, (size_t)len, file->f);
			if (size != (size_t)len)
				goto readerr;
			pool->pktbuflen = size;

			i = ringAddName(file, &iter, fpos);
			if (i < 0)
				goto failed;
			skip = i;

			trustf = 1;

			break;

		case PKTBYTE_SIG:
			trustmissing |= trustf;
			trustf = 1;
			
			if (skip & SKIP_SIG)
				goto skippkt;
			skip = 0;

			if (iter.level < 1) {	 /* No key yet - huh? */
				i = ringFileLog(file, (union RingObject *)NULL,
					0, fpos, PGPERR_TROUBLE_UNXSIG);
				if (i < 0)
					goto failed;
				goto skipsig;
			}
			if (len > RINGSIG_MAXLEN) {	/* Sig too damn big */
				i = ringFileLog(file, (union RingObject *)NULL,
					len, fpos, PGPERR_TROUBLE_SIG2BIG);
				if (i < 0)
					goto failed;
				goto skipsig;
			}
			/* Read in the signature for future analysis */
			buf = ringReserve(pool, (size_t)len);
			if (!buf)
				goto fatal;
			size = pgpFileRead(buf, (size_t)len, file->f);
			if (size != (size_t)len)
				goto readerr;
			pool->pktbuflen = size;

			i = ringAddSig(file, &iter, fpos);
			if (i < 0)
				goto failed;
			skip = i;
			trustf = 1;

			/* We will put these values into the key if sig is valid */
			sigkeyparent = NULL;
			sigkeyvalidity = 0;
			obj = iter.stack[iter.level-1];
			if (i == 0) {
				/* See if a self signature; if so, store some info */
				RingObject *sigowner = obj; /* top level key above sig */
				RingObject *sigparent = NULL; /* first key above sig */
				/* Find key above sig */
				do {
					sigowner = sigowner->g.up;
					if (OBJISKEY(sigowner) && !sigparent) {
						sigparent = sigowner;
					}
				} while (!OBJISTOPKEY(sigowner));
				if (obj->s.by == sigowner) {
					/* Self signature */
					byte const *pk;
					pk = ringSigFindSubpacket ((const unsigned char *) buf,
												SIGSUB_KEY_EXPIRATION,
												0, NULL, NULL, NULL,
												NULL, NULL);
					if (pk) {
						word32 keyexp;
						keyexp = (word32)((unsigned)pk[0]<<8|pk[1]) << 16 |
										((unsigned)pk[2]<<8|pk[3]);
						/* These values will be used later if sig is trusted */
						sigkeyparent = sigparent;
						sigkeyvalidity = (word16) (keyexp / (24 * 3600));
					}
				}
			}
			break;

		case PKTBYTE_TRUST:

			if (!trustf) {
				i = ringFileLog(file, NULL, len, fpos,
				PGPERR_TROUBLE_UNXTRUST);
				if (i < 0)
					goto failed;
				goto skippkt;
			}
			pgpAssert(iter.level);
			trustf = 0;
			if (skip & SKIP_TRUST)
				goto skippkt;
			skip = 0;
			/* If not a trusted keyring, ignore trust packets */
			if (!trusted)
				goto skiptrust;
			obj = iter.stack[iter.level-1];
			if (OBJISSEC(obj)) {
				pgpAssert(iter.level >= 2);
				obj = iter.stack[iter.level-2];
			}
#if PGPTRUSTMODEL==0
			/* Skip bad trust packets */
			if (len != 1) {
				i = ringFileLog(file, obj, len, fpos,
					PGPERR_TROUBLE_BADTRUST);
				goto skippkt;
			}
			if (pgpFileRead(&c, 1, file->f) != 1)
				goto readerr;
			i = c & 255;

			/* Set the appropriate flags */
			pgpAssert(!OBJISSEC(obj));
			switch (ringObjectType(obj)) {
			default:
			pgpAssert(0);
			case RINGTYPE_KEY:
				tp = &obj->k.trust;
				break;
			case RINGTYPE_NAME:
				tp = &obj->n.trust;
				break;
			case RINGTYPE_SIG:
				tp = &obj->s.trust;
				break;
			case RINGTYPE_UNK:
				tp = &obj->u.trust;
				break;
			}
			if (obj->g.flags & RINGOBJF_TRUST) {
				if (i != *tp)
					trustdirty = 1;
			} else {
				*tp = (byte)i;
				obj->g.flags |= RINGOBJF_TRUST;
			}
#else /* NEWTRUST */
			if (OBJISNAME(obj)) {
					/*
				* Name trust packets can be up to 3 bytes
				* long:
				* Byte 1: old-style trust packet
				* Byte 2: validity of name
				* Byte 3: confidence in name as introducer
					*/
				if (len < 1 || len > 3) {
					i = ringFileLog(file, obj, len,
						fpos, PGPERR_TROUBLE_BADTRUST);
					goto skippkt;
				}
				if (pgpFileRead(&c, 1, file->f) != 1)
					goto readerr;
				i = c & 255;
				/* Default trust and validity bytes */
				switch (i & PGP_NAMETRUST_MASK) {
				case PGP_NAMETRUST_COMPLETE:
				/* j = pool->threshold; */
					j = pool->completeconfidence;
					break;
				case PGP_NAMETRUST_MARGINAL:
				/* j = pool->threshold/2; */
					j = pool->marginalconfidence;
					break;
				default:
					j = 0;
					break;
				}
				pgpAssert(OBJISKEY(obj->g.up));
				k = obj->g.up->k.trust;
				switch (k & PGP_KEYTRUST_MASK) {
				case PGP_KEYTRUST_ULTIMATE:
					k = PGP_NEWTRUST_INFINITE;
					break;
				case PGP_KEYTRUST_COMPLETE:
					k = pool->completeconfidence;
					break;
				case PGP_KEYTRUST_MARGINAL:
					k = pool->marginalconfidence;
					break;
				case PGP_KEYTRUST_UNDEFINED:
					k = PGP_NEWTRUST_UNDEFINED;
					break;
				default:
					k = 0;
					break;
				}

				if (len > 1) {
				file->version = PGPVERSION_3;
					/* Fetch validity */
					if (pgpFileRead(&c, 1, file->f) != 1)
						goto readerr;
					j = c & 255;
					if (len > 2) {
						/* Fetch confidence */
						if (pgpFileRead(&c, 1, file->f)
						!= 1)
							goto readerr;
						k = c & 255;
					}
				}
				if (obj->g.flags & RINGOBJF_TRUST) {
					if (i != obj->n.trust
					|| j != obj->n.validity
					|| k != obj->n.confidence
					|| (len == 1) !=
					!NAMEHASNEWTRUST(&obj->n))
						trustdirty = 1;
				} else {
					obj->n.trust = (byte)i;
					obj->n.validity = (byte)j;
					obj->n.confidence = (byte)k;
					obj->g.flags |= RINGOBJF_TRUST;
					if (len > 1)
						NAMESETNEWTRUST(&obj->n);
					obj->n.valid = ringTrustToIntern (j);
				}
			} else {	/* Not a name */
				if (len != 1) {
					i = ringFileLog(file, obj, len,
						fpos, PGPERR_TROUBLE_BADTRUST);
					goto skippkt;
				}
				if (pgpFileRead(&c, 1, file->f) != 1)
					goto readerr;
				i = c & 255;

				/* Set the appropriate flags */
				pgpAssert(!OBJISSEC(obj));
				switch (ringObjectType(obj)) {
				default:
					pgpAssert(0);
				case RINGTYPE_KEY:
					tp = &obj->k.trust;
					break;
				case RINGTYPE_SIG:
					tp = &obj->s.trust;
					break;
				case RINGTYPE_UNK:
					tp = &obj->u.trust;
					break;
				}
				if (obj->g.flags & RINGOBJF_TRUST) {
					if (i != *tp)
						trustdirty = 1;
				} else {
					*tp = (byte)i;
					obj->g.flags |= RINGOBJF_TRUST;
				}
			}
#endif
			/* Handle information from self-signatures */
			if (OBJISSIG(obj) && (obj->s.trust & PGP_SIGTRUSTF_CHECKED)) {
				if (sigkeyparent) {
					sigkeyparent->k.validity = sigkeyvalidity;
					sigkeyparent = NULL;
					sigkeyvalidity = 0;
				}
			}

			break;
		} /* switch (PKTBYTE_TYPE(pktbyte)) */

	} /* while ((pktbyte = pktByteGet(file->f)) > 0) */

	/* Okay, we're done - handle errors in pktByteGet() and return */
	if (pktbyte < 0) {
		/*
		* Call it fatal if it was on the first packet. May not
		* have been a keyring file at all.
		*/
		if (iter.level == 0) {
			ringErr (file, fpos, pktbyte);
			goto fatal;
		}
		len = (word32)errno;	/* Capture before *ANY* libc calls */
ioerror:
		*error = pktbyte;
		i = ringFileLog(file, (union RingObject *)NULL, len,
		fpos, pktbyte);
		if (i < 0)
			goto failed;
		/* A keyring with errors is *always* dirty */
		dirty = 1;
	}

	/*
	* Did we get any bad trouble reports? "Dirty" means that
	* the result of writing out the RingFile's set will not
	* be the same as the original file. Some troubles don't
	* have that property.
	*/
	for (trouble = file->trouble; trouble; trouble = trouble->next) {
		if (trouble->type != PGPERR_TROUBLE_VERSION_BUG_PREV
		&& trouble->type != PGPERR_TROUBLE_OLDSEC
		&& trouble->type != PGPERR_TROUBLE_NEWSEC
		&& trouble->type != PGPERR_TROUBLE_BAREKEY)
		{
			break;
		}
	}
	if (dirty || trouble)
		ringPoolMarkDirty(pool, mask);
	if (trusted && (trustdirty || (trustmissing & 1)))
		ringPoolMarkTrustChanged(pool, mask);

	ringSortKeys(pool);
	ringPoolListSigsBy(pool);

	return file;

	/*
	* Read error: figure out error code and use general error
	* handler. Not fatal.
	*/
readerr:
	/* If an error, or neither error nor eof (i.e. unknown), error */
	len = (word32)errno;	/* Capture before *ANY* libc cals */
	if (pgpFileError(file->f) || !pgpFileEof(file->f))
		pktbyte = PGPERR_KEYIO_READING;
	else
		pktbyte = PGPERR_KEYIO_EOF;
	goto ioerror;

	/*
	* Fatal errors: undo all work in memory, return NULL,
	* *error is non-zero.
	*/
fatal:
	i = ringPoolError(pool)->error;
failed:			/* Generic fatal entry point, error in "i" */
	*error = i;
	(void)ringFileClose(file);

	return NULL;
}

/** Keyring writing (this is a *lot* simpler!) **/

/*
 * Write out the trust packet for an object.
 *
 * This is generally pretty simple, but there are different formats based
 * on the object type and version, and there is a kludge to omit
 * writing out trust on signatures on keys if the signature is good.
 * This is a PGP 2.x compatibility feature, since that chokes if it
 * finds a trust packet on a key signature.
 */
static int
ringCopyTrust(union RingObject const *obj, struct PgpFile *f,
PgpVersion version)
{
	char trust[3];
	size_t trustlen = 1;
	int i;

	/* Suppress warning */
	(void) version;

	switch (ringObjectType(obj)) {
	case RINGTYPE_KEY:
		trust[0] = (char)obj->k.trust;
		break;
	case RINGTYPE_SEC:
		trust[0] = (char)obj->g.up->k.trust;
		break;
	case RINGTYPE_NAME:
		/*
		* Names have 1 to 3 bytes of trust. The first byte
		* is a 2.x-compatible trust byte. The second is
		* a validity value (how sure are we that this name
		* is correct - computed), and the third is a confidence
		* in the named individual as an introducer.
		*
		* Note: If we are requested to write out a pre-PGP 3.0
		* keyring, the name validity is not converted back to
		* an old KEYLEGIT value. We could do this, but it's
		* better that a maintenance pass is run prior to
		* using such a keyring.
		* @@@ Is this really the best way to handle it?
				*/
		trust[0] = (char)obj->n.trust;
#if PGPTRUSTMODEL>0
		if (version >= PGPVERSION_3) {
					trust[1] = (char)obj->n.validity;
					trust[2] = (char)obj->n.confidence;
					trustlen = 3;
			}
#endif
		break;
		case RINGTYPE_SIG:
			/*
			* 2.x compatibility kludge: Don't write trust
			* on good compromise certificates. PGP 2.x
			* maintenance dies (assert fail) if it finds
			* trust packets on key sigs.
			*/
		trust[0] = (char)obj->s.trust;
		if (/*version < PGPVERSION_3 &&*/
		OBJISKEY(obj->g.up) && obj->g.up == obj->s.by
		&& obj->s.type == PGP_SIGTYPE_KEY_COMPROMISE)
			return 0;
		break;
	default:
		pgpAssert(0);
	}
	i = pktBytePut(f, PKTBYTE_BUILD(PKTBYTE_TRUST, 0), trustlen);
	if (i < 0)
		return i;
	if (pgpFileWrite(trust, trustlen, f) != trustlen)
		return PGPERR_KEYIO_WRITING;
	return 0;
}

/* Return the length of the packet we will create for the object */
static int
ringPacketSize (struct RingSet const *set, union RingObject *obj)
{
	void const *buf;
	size_t len;
	int i;
	byte pktbyte;
	static int const pktbytes[RINGTYPE_MAX] = {
		PKTBYTE_BUILD(PKTBYTE_PUBKEY, 1),
		PKTBYTE_BUILD(PKTBYTE_SECKEY, 1),
		PKTBYTE_BUILD(PKTBYTE_NAME, 0),
		PKTBYTE_BUILD(PKTBYTE_SIG, 1)
	};

	pgpAssert (obj->g.mask & set->mask);
	buf = ringFetchObject(set, obj, &len);
	i = ringObjectType(obj);
	pgpAssert(i > 0 && i <= RINGTYPE_MAX);
	if (OBJISSUBKEY(obj))
		pktbyte = PKTBYTE_BUILD(PKTBYTE_PUBSUBKEY, 1);
	else if (OBJISSEC(obj) && OBJISSUBKEY(obj->g.up))
		pktbyte = PKTBYTE_BUILD(PKTBYTE_SECSUBKEY, 1);
	else
		pktbyte = (i == RINGTYPE_UNK) ? obj->u.pktbyte : pktbytes[i-1];
	return len + pktByteLen (pktbyte, len);
}

/* Return the length of the object and all its sub objects */
static int
ringPacketSizeAll (struct RingSet const *set, union RingObject *obj)
{
	size_t len;
	union RingObject *obj1, *obj2;

	len = ringPacketSize (set, obj);
	if (!OBJISBOT(obj)) {
		for (obj1 = obj->g.down; obj1; obj1 = obj1->g.next) {
			if (!(obj1->g.mask & set->mask))
				continue;
			pgpAssert (!OBJISSEC(obj1));
			len += ringPacketSize (set, obj1);
			if (!OBJISBOT(obj1)) {
				for (obj2 = obj1->g.down; obj2; obj2 = obj2->g.next) {
					if (!(obj2->g.mask & set->mask))
						continue;
					pgpAssert (!OBJISSEC(obj2));
					len += ringPacketSize (set, obj2);
					}
			}
		}
	}
	return len;
}
		

/*
 * For backwards compatibility we are wrapping the new key types.
 * For secret objects, don't call this.
 * We actually have three modes, WRAP_NONE where we don't wrap
 * anything, WRAP_SUB where we only wrap the subkey, and WRAP_NEW
 * where we wrap keys with post RSA algorithms.
 * We output a comment header of the necessary size for obj, which
 * is in set, to file f. We return an error code or 0 for success.
 */
static int
ringWrapObject (struct RingSet const *set, union RingObject *obj,
	struct PgpFile *f)
{
	int i;
	size_t len;

#if defined(WRAP_NONE)
	(void)set;
	(void)obj;
	(void)f;
	return 0;
#elif defined(WRAP_SUB);
	/*
	* Just wrap subkeys, we'll let old PGP complain about DSA keys.
	* It can't handle subkeys as we have them now.
	*/
	if (!OBJISSUBKEY(obj))
		return 0;
#else
	* Wrap top level DSA keys and everything under them */
	if (!OBJISTOPKEY(obj))
		return 0;
#endif
	if (obj->k.pkalg <= 3)
		return 0;
	if (ringBestSec(set, obj))
		return 0;
	len = ringPacketSizeAll (set, obj);
	i = pktBytePut (f, PKTBYTE_BUILD(PKTBYTE_COMMENT, 0), len);
	return i;
}

/*
 * Copy the packet to the given file. If "trust" is non-negative,
 * it is appended as a trust packet. If "file" is non-NULL and the
 * write is successful, the location in that file is listed as a
 * position for the object.
 *
 * This function is careful to not add a FilePos to the object until
 * it is known to be completely written out.
 *
 * If keyobj is non-null, it is a key object corresponding to the secret
 * object we are writing out, and we must set the FilePos for it, too.
 */
static int
ringCopyObject(struct RingSet const *set, union RingObject *obj,
	union RingObject *keyobj, struct PgpFile *f, int writetrust,
	PgpVersion version, struct RingFile *file)
{
	void const *buf;
	size_t len;
	int i;
	byte pktbyte;
	long pos = 0;	 /* Initialized to suppress warnings */

	static int const pktbytes[RINGTYPE_MAX] = {
		PKTBYTE_BUILD(PKTBYTE_PUBKEY, 1),
		PKTBYTE_BUILD(PKTBYTE_SECKEY, 1),
		PKTBYTE_BUILD(PKTBYTE_NAME, 0),
		PKTBYTE_BUILD(PKTBYTE_SIG, 1)
	};

	/* XXX experimental backwards compat code */
	/* Wrap new-style objects for backwards compatibility when exporting */
	if (!file) {
		i = ringWrapObject (set, obj, f);
		if (i < 0)
			return i;
	}

	buf = ringFetchObject(set, obj, &len);
	if (!buf)
		return ringSetError(set)->error;

	/* If this is a secret object and it has the version bug,
		then *do not* fix it on disk. This ensures that if the
		packet exists in more than ringFile, the packets remain
		consistent. We'll always have to check for the bug in
		the future anyway, so doing this doesn't cause any harm. */
	
	if (OBJISSEC(obj) && (obj->g.flags & SECF_VERSION_BUG))
	((byte *) buf)[0] = PGPVERSION_2_6;

	if (file) {
		pos = pgpFileTell(f);
		if (pos == -1)
			return PGPERR_KEYIO_FTELL;
	}

	i = ringObjectType(obj);
	pgpAssert(i > 0 && i <= RINGTYPE_MAX);
	if (OBJISSUBKEY(obj))
		pktbyte = PKTBYTE_BUILD(PKTBYTE_PUBSUBKEY, 1);
	else if (OBJISSEC(obj) && OBJISSUBKEY(obj->g.up))
		pktbyte = PKTBYTE_BUILD(PKTBYTE_SECSUBKEY, 1);
	else
		pktbyte = (i == RINGTYPE_UNK) ? obj->u.pktbyte : pktbytes[i-1];

	i = pktBytePut(f, pktbyte, len);
	if (i < 0)
		return i;
	if (pgpFileWrite(buf, len, f) != len)
		return PGPERR_KEYIO_WRITING;
	if (writetrust) {
		i = ringCopyTrust(obj, f, version);
		if (i < 0)
			return i;
	}

	/* All successful - add to file if requested */
	if (file) {
		i = ringAddPos (obj, file, (word32)pos);
		if (keyobj && !i)
			i = ringAddPos (keyobj, file, (word32)pos);
		return i;
	}
	return 0;
}

int
ringSetWrite(struct RingSet const *set, struct PgpFile *f,
	struct RingFile **filep, PgpVersion version, int flags)
{
	struct RingFile *file;
	struct RingIterator *iter;
	union RingObject *obj, *sec;
	union RingObject *keyobj;
	unsigned level;
	ringmask mask;
	int i;
	int writetrust;

	pgpAssert(set);
	pgpAssert(f);

	if (filep) {
		/* Set up a RingFile to add to */
		pgpAssert(f);
		pgpAssert(!(set->pool->filemask & ~set->pool->allocmask));

		i = ringBitAlloc(set->pool);
		if (i < 0) {
			*filep = (struct RingFile *)0;
			return PGPERR_NO_KEYBITS;
		}
		pgpAssert(i < MEMRINGBIT);

		file = &set->pool->files[i];
		mask = (ringmask)1 << i;
		pgpAssert(file->set.mask == mask);
		pgpAssert(!(set->pool->filemask & mask));

		set->pool->filemask |= mask;
		set->pool->allocmask |= mask;

		pgpAssert(!file->set.next);

		file->f = f;
		file->destructor = NULL;	/* May be set later */
		file->arg = 0;
		*filep = NULL;	/* For now, to be fixed later */
	} else {
		file = NULL;
		mask = 0;
	}

	iter = ringIterCreate(set);
	if (!iter) {
		ringFileClose(file);	/* Okay for file to be NULL */
		return ringSetError(set)->error;
	}

	/* Okay, the main loop */
	level = 1;
	writetrust = 0;	/* Silence warnings */

	while ((i = ringIterNextObjectAnywhere(iter)) > 0) {
		obj = ringIterCurrentObject(iter, (unsigned)i);
		keyobj = NULL;
		pgpAssert(obj);
		if (OBJISKEY(obj)) {
			writetrust = flags & PGP_WRITETRUST_PUB;
			sec = ringBestSec(set, obj);
			if (sec) {
				keyobj = obj;
				obj = sec;	/* Write out the sec */
				if (OBJISTOP(obj))
					writetrust = flags&PGP_WRITETRUST_SEC;
			}
		} else if (OBJISSEC(obj)) {
			continue;
		}
		i = ringCopyObject(set, obj, keyobj, f, writetrust, version, file);
		if (i < 0)
			break;
	} /* while ringIternextObjectAnywhere */
	ringIterDestroy(iter);
	
	/*
	* If we broke out on error, close the output file. We could leave
	* the partial file open, as all the pointers that exist are valid.
	* Is that useful?
	*/
	if (i < 0 && file)
		ringFileClose(file);
	else if (filep)
		*filep = file;

	return i;
}


/*
 * Bring an object into the MEMRING.
*/
static int
ringCacheObject(struct RingSet *set, union RingObject *obj)
{
	byte *buf;
	byte const *pktbuf;
	size_t buflen;
	struct MemPool cut;
	struct RingFile *file;
	int err;

	/* Do nothing if already there */
	if (obj->g.mask & MEMRINGMASK) {
		return 0;
	}
	
	/* Get the object into the pktbuf */
	pktbuf = (byte const *)ringFetchObject (set, obj, &buflen);
	if (!pktbuf) {
		return ringSetError(set)->error;
	}
	
	/* Allocate space in memory ringfile */
	file = &set->pool->files[MEMRINGBIT];
	cut = file->strings;
	buf = (byte *)memPoolAlloc(&file->strings, (unsigned)buflen, 1);
	if (!buf) {
		return PGPERR_NOMEM;
	}

	/* Copy to memory file buffer */
	memcpy(buf, pktbuf, buflen);

	/* Add the FilePos */
	err = ringAddPos(obj, file, (word32)buflen);
	if (err < 0) {
		memPoolCutBack(&file->strings, &cut);
		return ringSetError(set)->error;
	}
	ringFilePos(obj, file)->ptr.buf = buf;

	return 0;
}

/*
 * Bring the ancestors of an object into the MEMRING.
 *
 * When we create a new object using the routines below, it is necessary
 * that any parents of the object be brought into the MEMRING. This is
 * so that we can maintain our global invariant that all of an object's
 * ancestors are in all sets that the object is in.
 */
static int
ringCacheParents (struct RingSet *set, union RingObject *obj)
{
	int err;

	if (OBJISTOP(obj)) {
		return 0;
	}

	do {
		obj = obj->g.up;
		err = ringCacheObject(set, obj);
		if (err < 0)
			return err;
	} while (!OBJISTOP(obj));
	
	return 0;
}


/*
 *
 * Create a new name in a mutable RingSet.
 *
 * This is example code. Modifying it a bit is probably a good idea.
 */
union RingObject *
ringCreateName(struct RingSet *dest, union RingObject *key,
	char const *str, size_t len)
{
	struct RingIterator iter;
	struct MemPool cut;
	struct RingFile *file;
	byte *pktbuf, *buf;
	int i;

	pgpAssert(RINGSETISMUTABLE(dest));
	pgpAssert(OBJISKEY(key));
	pgpAssert(key->g.mask & dest->mask);

	iter.set = *dest;

	if (len >= RINGNAME_MAXLEN) {
		ringSimpleErr(dest->pool, PGPERR_TROUBLE_NAME2BIG);
		return NULL;
	}

	pktbuf = (byte *)ringReserve(dest->pool, len);
	if (!pktbuf) {
		ringAllocErr(dest->pool);
		return NULL;
	}

	/* Add to memory file */
	file = &dest->pool->files[MEMRINGBIT];
	cut = file->strings;
	buf = (byte *)memPoolAlloc(&file->strings, (unsigned)len, 1);
	if (!buf) {
		ringAllocErr(dest->pool);
		return NULL;
	}

	memcpy(pktbuf, str, len);
	dest->pool->pktbuflen = len;
	iter.stack[0] = key;
	iter.level = 1;
	i = ringAddName(file, &iter, (word32)len);
	pgpAssert(i < 0 || iter.level == 2);	/* error or object created */

	/* ringAddName returns <0 on error, and >0 on duplicate!
	Note: "Duplicate" means it's already on the memring,
	*not* that it's already on dest. ringAddName adds
	it to dest, so we can't easily check to see if it's a
	real duplicate. */
	if (i != 0) {
		memPoolCutBack(&file->strings, &cut);
		return i < 0 ? NULL : iter.stack[1];
	}

	/* Make sure name's parent key is in the MEMRING too */
	i = ringCacheParents(dest, iter.stack[1]);
	if (i < 0) {
		memPoolCutBack(&file->strings, &cut);
		return NULL;
	}

	/* Okay, success - we can't fail */
	/*
	* Remember that the memory pool's FilePos pointers are a bit
	* wierd. Instead of pos->ptr.next, we have pos->ptr.buf,
	* a pointer to a buffer holding the object. And pos->fpos
	* is the length of the object. That is already filled in
	* by ringAddName(), but the ptr->buf needs to be done
	* explicitly. (The ringAddName() code is optimized for
	* the reading-from-a-file case, since that's by far the
	* most common one.)
	*/
	memcpy(buf, str, len);
	ringFilePos(iter.stack[1], file)->ptr.buf = buf;

	/* Ta-dah! */
	return iter.stack[1];
}

/*
 * Create a new signature object in a mutable RingSet.
 */
union RingObject *
ringCreateSig(struct RingSet *dest, union RingObject *obj,
	byte *sig, size_t siglen)
{
	struct RingIterator iter;
	struct MemPool cut;
	struct RingFile *file;
	union RingObject *newsig;
	byte *pktbuf, *buf;
	int i;

	pgpAssert(RINGSETISMUTABLE(dest));
	pgpAssert(obj->g.mask & dest->mask);

	iter.set = *dest;
	iter.set.type = RINGSET_ITERATOR; /* kludge for ringIterSeekTo() */

	i = ringIterSeekTo(&iter, obj);
	pgpAssert(i >= 0);

	if (siglen >= RINGSIG_MAXLEN) {
		ringSimpleErr(dest->pool, PGPERR_TROUBLE_SIG2BIG);
		return NULL;
	}

	pktbuf = (byte *)ringReserve(dest->pool, siglen);
	if (!pktbuf) {
		ringAllocErr(dest->pool);
		return NULL;
	}

	/* Add to memory file */
	file = &dest->pool->files[MEMRINGBIT];
	cut = file->strings;
	buf = (byte *)memPoolAlloc(&file->strings, (unsigned)siglen, 1);
	if (!buf) {
		ringAllocErr(dest->pool);
		return NULL;
	}

	memcpy(pktbuf, sig, siglen);
	dest->pool->pktbuflen = siglen;
	i = ringAddSig(file, &iter, (word32)siglen);
	newsig = iter.stack[iter.level - 1];

	/* ringAddSig returns <0 on error, and >0 on duplicate! */
	if (i != 0) {
		memPoolCutBack(&file->strings, &cut);
		return i < 0 ? NULL : newsig;
	}

	/* Make sure sig's parent objects are in the MEMRING too */
	i = ringCacheParents(dest, newsig);
	if (i < 0) {
		memPoolCutBack(&file->strings, &cut);
		return NULL;
	}

	/* Okay, success - we can't fail */
	/*
	* Remember that the memory pool's FilePos pointers are a bit
	* wierd. Instead of pos->ptr.next, we have pos->ptr.buf,
	* a pointer to a buffer holding the object. And pos->fpos
	* is the length of the object. That is already filled in
	* by ringAddName(), but the ptr->buf needs to be done
	* explicitly. (The ringAddName() code is optimized for
	* the reading-from-a-file case, since that's by far the
	* most common one.)
	*/
	memcpy(buf, sig, siglen);
	ringFilePos(newsig, file)->ptr.buf = buf;

	return newsig;
}


/*
 * Create a new public key in a mutable RingSet. To create a
 * public/secret key pair, use ringCreateSec. That is normally
 * the routine to use, not this one.
 */
union RingObject *
ringCreateKey(struct RingSet *dest, union RingObject *parent,
	struct PgpPubKey const *key, struct PgpKeySpec const *ks, byte pkalg)
{
	struct RingIterator iter;
	struct MemPool cut;
	struct RingFile *file;
	union RingObject *newkey;
	byte *pktbuf, *buf;
	size_t len, prefixlen;
	byte pkttype;
	int i;

	pgpAssert(RINGSETISMUTABLE(dest));
	
	prefixlen = ringKeyBufferLength(ks, pkalg);
	len = prefixlen + pgpPubKeyBufferLength(key);
	if (len >= RINGKEY_MAXLEN) {
		ringSimpleErr(dest->pool, PGPERR_TROUBLE_KEY2BIG);
		return NULL;
	}

	/* Add to pktbuf for compatibility with other routines */
	pktbuf = (byte *)ringReserve(dest->pool, len);
	if (!pktbuf) {
		ringAllocErr(dest->pool);
		return NULL;
	}

	ringKeyToBuffer(pktbuf, ks, pkalg);
	pgpPubKeyToBuffer(key, pktbuf+prefixlen);
	dest->pool->pktbuflen = len;

	/* Add to memory file as permanent home for data */
	file = &dest->pool->files[MEMRINGBIT];
	cut = file->strings;
	buf = (byte *)memPoolAlloc(&file->strings, (unsigned)len, 1);
	if (!buf) {
		ringAllocErr(dest->pool);
		return NULL;
	}
	memcpy(buf, pktbuf, len);

	/* Set iter to point at beginning */
	iter.set = *dest;
	if (parent) {
		iter.stack[0] = parent;
		iter.level = 1;
	} else {
		iter.level = 0;
	}
	
	pkttype = parent ? PKTBYTE_PUBSUBKEY : PKTBYTE_PUBKEY;

	i = ringAddKey(file, &iter, (word32)len, 1/*trusted*/, pkttype);

	/* ringAddKey returns <0 on error, and >0 on duplicate! */
	pgpAssert(i < 0 || (int)iter.level == 1 + (parent != NULL));

	if (i != 0) {
		memPoolCutBack(&file->strings, &cut);
		return i < 0 ? NULL : iter.stack[1];
	}
	
	/* Set buffer pointer in FilePos */
	newkey = iter.stack[0];
	pgpAssert(OBJISKEY(newkey));
	ringFilePos(newkey, file)->ptr.buf = buf;

	/* Put key into right place in key ring (sorted by keyID) */
	ringSortKeys(file->set.pool);

	return newkey;
}


/*
 * Create a new public/secret keypair. Return pointer to the RingKey object,
 * which will be followed by a RingSec.
*/
union RingObject *
ringCreateSec(struct RingSet *dest, union RingObject *parent,
	struct PgpSecKey const *sec, struct PgpKeySpec const *ks, byte pkalg)
{
	struct RingIterator iter;
	struct MemPool cut;
	struct RingFile *file;
	union RingObject *newkey, *newsec;
	byte *pktbuf, *buf;
	size_t len, prefixlen;
	byte pkttype;
	int i;

	pgpAssert(RINGSETISMUTABLE(dest));
	
	prefixlen = ringSecBufferLength(ks, pkalg);
	len = prefixlen + pgpSecKeyBufferLength(sec);
	if (len >= RINGSEC_MAXLEN) {
		ringSimpleErr(dest->pool, PGPERR_TROUBLE_SEC2BIG);
		return NULL;
	}

	/* Add to pktbuf for compatibility with other routines */
	pktbuf = (byte *)ringReserve(dest->pool, len);
	if (!pktbuf) {
		ringAllocErr(dest->pool);
		return NULL;
	}

	ringSecToBuffer(pktbuf, ks, pkalg);
	pgpSecKeyToBuffer(sec, pktbuf+prefixlen);
	dest->pool->pktbuflen = len;

	/* Add to memory file as permanent home for data */
	file = &dest->pool->files[MEMRINGBIT];
	cut = file->strings;
	buf = (byte *)memPoolAlloc(&file->strings, (unsigned)len, 1);
	if (!buf) {
		ringAllocErr(dest->pool);
		return NULL;
	}
	memcpy(buf, pktbuf, len);

	/* Set iter to point at beginning */
	iter.set = *dest;
	if (parent) {
		iter.stack[0] = parent;
		iter.level = 1;
	} else {
		iter.level = 0;
	}

	pkttype = parent ? PKTBYTE_SECSUBKEY : PKTBYTE_SECKEY;

	/*
	* Clear trouble list so we can distinguish duplicate keys from
	* duplicate sigs
	*/
	ringFilePurgeTrouble(file);

	/* Create the secret object and the parent key */
	i = ringAddSec(file, &iter, (word32)len, pkttype);

	/* ringAddSec returns <0 on error, and >0 on duplicate! */
	pgpAssert(i < 0 || (int)iter.level == 2 + (parent != NULL));

	if (i > 0) {
		struct RingTrouble const *trouble = ringFileTrouble(file);
		pgpAssert (trouble);
		/*
		* If this sec was a dup, just return it; if it was
		* something else it was probably a duplicate key and
		* so we can ignore it and use the new mempool data
		*/
		if (trouble->type == PGPERR_TROUBLE_DUPSEC) {
			memPoolCutBack(&file->strings, &cut);
			return iter.stack[1];
		}
	}
	if (i < 0) {
		memPoolCutBack(&file->strings, &cut);
		return NULL;
	}
	
	/* Set buffer pointer in FilePos */
	newkey = iter.stack[iter.level-2];
	pgpAssert(OBJISKEY(newkey));
	newsec = iter.stack[iter.level-1];
	pgpAssert(OBJISSEC(newsec));
	ringFilePos(newkey, file)->ptr.buf = buf;
	ringFilePos(newsec, file)->ptr.buf = buf;
	
	/* Put key into right place in key ring (sorted by keyID) */
	ringSortKeys(file->set.pool);

	return newkey;
}

/*
 * Takes a name and moves it to the front of the list of names on a
 * key. Normally ringCreateName puts the new name at the end; this can
 * move it up.
 */
int
ringRaiseName(struct RingSet *dest, union RingObject *name)
{
	union RingObject *key;
	union RingObject **np, **np1;

	pgpAssert (RINGSETISMUTABLE(dest));
	pgpAssert (OBJISNAME(name));
	pgpAssert (name->g.mask & dest->mask);
	pgpAssert (!OBJISTOP(name));
	key = name->g.up;
	pgpAssert (OBJISKEY(key));
	
	/* Guaranteed to hit at least one name, ours */
	np = &key->g.down;
	while (!(dest->mask & (*np)->g.mask && OBJISNAME(*np))) {
		np = &(*np)->g.next;
	}

	/* Continue till we hit pointer to our name */
	np1 = np;
	while (*np1 != name)
		np1 = &(*np1)->g.next;

	/* Swap if not already first */
	if (np != np1) {
		*np1 = name->g.next;	/* Delete name from the list */
		name->g.next = *np;	/* Set its tail pointer as *np */
		*np = name;		/* Insert it in new spot */
	}
	return 0;
}


/*
 * Signs an object (and its parents); then deposits the new signature
 * object in place on the set.
*/
int
ringSignObject(struct RingSet *set, union RingObject *obj,
	struct PgpSigSpec *spec, struct PgpRandomContext const *rc)
{
	byte *sig;
	int siglen;
	union RingObject *sigobj;

	siglen = pgpMakeSigMaxSize (spec);
	sig = (byte *)pgpMemAlloc (siglen);

	siglen = ringSignObj (sig, set, obj, spec, rc);
	if (siglen < 0) {
		pgpMemFree (sig);
		return siglen;
	}
	if (!siglen) {
		pgpMemFree (sig);
		return PGPERR_NOMEM;
	}
	sigobj = ringCreateSig (set, obj, sig, siglen);
	sigobj->s.trust = PGP_SIGTRUSTF_CHECKED_TRIED | PGP_KEYTRUST_COMPLETE;

	pgpMemFree (sig);
	return sigobj ? 0 : PGPERR_NOMEM;
}

/*
 * Given a seckey, two mutable ringsets, and some other information,
 * create the ring objects and put them on the appropriate ringsets.
 * Then self-sign the pubkey.
 */
int
ringCreateKeypair (struct PgpEnv const *env, struct PgpSecKey *seckey,
			struct PgpKeySpec *keyspec,
			char const *name, size_t namelen,
			struct PgpRandomContext const *rc,
			struct RingSet *pubset, struct RingSet *secset)
{
	union RingObject *keyobj, *nameobj;
	struct PgpSigSpec *sigspec = NULL;
	word16 validity;
	int keyv3;
	int error = 0;

#define CHECKRETVAL(val, err) if (val) { error = err; goto cleanup; }

	/* SECRET KEY stuff */

	keyv3 = seckey->pkAlg > PGP_PKALG_RSA + 2;
	if (keyv3)
		pgpKeySpecSetVersion (keyspec, PGPVERSION_3);
	keyobj = ringCreateSec (secset, NULL, seckey, keyspec, seckey->pkAlg);
	CHECKRETVAL (!keyobj, PGPERR_NOMEM);
	pgpAssert(OBJISKEY(keyobj));
	memcpy(seckey->keyID, keyobj->k.keyID, sizeof(keyobj->k.keyID));
	nameobj = ringCreateName (secset, keyobj, name, namelen);
	CHECKRETVAL (!nameobj, PGPERR_NOMEM);

	/* PUBLIC KEY stuff */
	error = ringSetAddObject (pubset, keyobj);
	CHECKRETVAL (error, error);
	error = ringSetAddObject (pubset, nameobj);
	CHECKRETVAL (error, error);

	/* Self-Sign the new pubkey */
	if (pgpKeyUse(pgpPkalgByNumber(seckey->pkAlg)) & PGP_PKUSE_SIGN) {
		sigspec = pgpSigSpecCreate (env, seckey,
					PGP_SIGTYPE_KEY_GENERIC);
		CHECKRETVAL (!sigspec, PGPERR_NOMEM);
		if (keyv3) {
			/* New keys self-sign with special info! */
			static byte prefs[] = {PGP_CIPHER_CAST5, PGP_CIPHER_IDEA,
								PGP_CIPHER_3DES};
			pgpSigSpecSetPrefAlgs (sigspec, prefs, sizeof(prefs));
			pgpSigSpecSetVersion (sigspec, PGPVERSION_3);
			if ((validity=pgpKeySpecValidity (keyspec)) != 0) {
				pgpSigSpecSetKeyExpiration (sigspec,
					(word32)validity*24*60*60);
				keyobj->k.validity = validity;
			}
#define KRTEST 0
#if KRTEST
			{
				/* For testing, add all other keys on ring as recovery keys */
				RingIterator *iter;
				RingSet *secset2 = ringSetCreate (ringSetPool(secset));
				int rkeynum = 0;
				ringSetAddSet (secset2, secset);
				ringSetFreeze (secset2);
				iter = ringIterCreate (secset2);
				while (ringIterNextObject (iter, 1) > 0) {
					byte krinfo[22];
					RingObject *krkey = ringIterCurrentObject (iter, 1);
					if (krkey == keyobj)
						continue;
					/* Point to the encryption key */
					if (ringKeySubkey (pubset, krkey))
						krkey = ringKeySubkey (pubset, krkey);
					krinfo[0] = 1;	/* must be nonzero, some kind of status */
					krinfo[1] = krkey->k.pkalg;
					ringKeyFingerprint20 (secset2, krkey, krinfo+2);
					pgpSigSpecSetRecoveryKey (sigspec, krinfo, sizeof(krinfo),
												rkeynum++);
				}
				ringSetDestroy (secset2);
				ringIterDestroy (iter);
			}
#endif
		}
		error = ringSignObject (pubset, nameobj, sigspec, rc);
	}

	/* This sets both key and name trust */
	ringKeySetAxiomatic(secset, keyobj);

 cleanup:
	if (sigspec)
		pgpSigSpecDestroy (sigspec);
	return error;
}


/*
 * Given a seckey, two mutable ringsets, and some other information,
 * create subkey ring objects and put them on the appropriate ringsets.
 * Then sign the pubkey using the master signature key.
 * seckey is the secret key coresponding to the master signature key,
 * subseckey is the secret key to be used for creating the new subkey.
 */
int
ringCreateSubkeypair (struct PgpEnv const *env, struct PgpSecKey *seckey,
	struct PgpSecKey *subseckey, struct PgpKeySpec *keyspec,
	struct PgpRandomContext const *rc,
	struct RingSet *pubset, struct RingSet *secset)
{
	union RingObject *keyobj;
	union RingObject *subkeyobj;
	struct PgpSigSpec *sigspec;
	int error;
	word16 validity;
	int keyv3;

	keyv3 = seckey->pkAlg > PGP_PKALG_RSA + 2;
	if (keyv3)
		pgpKeySpecSetVersion (keyspec, PGPVERSION_3);

	/* Get keyobj for the master key */
	keyobj = ringKeyById8(secset, seckey->pkAlg, seckey->keyID);
	pgpAssert(keyobj);

	/* Create the subkey object on the secret keyring */
	subkeyobj = ringCreateSec (secset, keyobj, subseckey,
				keyspec, subseckey->pkAlg);
	if (!subkeyobj)
		return PGPERR_NOMEM;
	memcpy(subseckey->keyID, subkeyobj->k.keyID,
	sizeof(subkeyobj->k.keyID));

	/* Add it to the public keyring */
	error = ringSetAddObject (pubset, subkeyobj);
	if (error)
		return error;

	/* Sign the encryption key with the master signature key */
	sigspec = pgpSigSpecCreate (env, seckey, PGP_SIGTYPE_KEY_SUBKEY);
	if (!sigspec)
		return PGPERR_NOMEM;
	if (keyv3) {
		/* New keys self-sign with special info */
		if ((validity=pgpKeySpecValidity (keyspec)) != 0) {
			pgpSigSpecSetVersion (sigspec, PGPVERSION_3);
			pgpSigSpecSetKeyExpiration (sigspec, (word32)validity*24*3600);
			subkeyobj->k.validity = validity;
		}
	}
	error = ringSignObject (pubset, subkeyobj, sigspec, rc);
	pgpSigSpecDestroy (sigspec);
	if (error)
		return error;
	ringKeySetAxiomatic(secset, subkeyobj);

	return 0;	 	/* success */
}


/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
