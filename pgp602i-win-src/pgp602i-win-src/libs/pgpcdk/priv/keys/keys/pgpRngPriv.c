/*
 * Private helper functions for keyring manipulation.
 *
 * Written by Colin Plumb.
 *
 * $Id: pgpRngPriv.c,v 1.39 1998/08/07 05:06:23 hal Exp $
 */
#include "pgpConfig.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "pgpDebug.h"
#include "pgpPktByte.h"
#include "pgpRngPriv.h"
#include "pgpRngPars.h"
#include "pgpRngRead.h"		/* For ringFilePurgeTrouble */
#include "pgpTrstPkt.h"
#include "pgpHashPriv.h"
#include "pgpEnv.h"
#include "pgpErrors.h"
#include "pgpMem.h"
#include "pgpSigSpec.h"	/* for PGP_SIGTYPE */
#include "pgpTrust.h"
#include "pgpContext.h"
#include "pgpRegExp.h"

#ifndef NULL
#define NULL 0
#endif

#define PGP_TRUST_DECADE_INTERNAL (PGP_TRUST_DECADE >> 6)
#define PGP_TRUST_OCTAVE_INTERNAL (PGP_TRUST_OCTAVE >> 6)

/*
 * Virtual ring mask functions
 */

#if !VIRTMASK

/* Use old data structure for backwards compatibility */

PGPError
pgpVirtMaskInit (RingPool const *pool, PGPVirtMask *mask)
{
	(void) pool;
	(void) mask;
	*mask = (PGPVirtMask)0;
	return kPGPError_NoErr;
}

/* Need a lighter weight memory allocation package! */
PGPError
pgpVirtMaskCleanup (RingPool const *pool, PGPVirtMask *mask)
{
	(void) pool;
	(void) mask;
	*mask = (PGPVirtMask)0;
	return kPGPError_NoErr;
}

PGPError
pgpVirtMaskOR (RingPool const *pool, PGPVirtMask const *imask,
	PGPVirtMask *omask)
{
	(void) pool;
	*omask |= *imask;
	return kPGPError_NoErr;
}

PGPError
pgpVirtMaskAND (RingPool const *pool, PGPVirtMask const *imask,
	PGPVirtMask *omask)
{
	(void) pool;
	*omask &= *imask;
	return kPGPError_NoErr;
}

PGPError
pgpVirtMaskANDNOT (RingPool const *pool, PGPVirtMask const *imask,
	PGPVirtMask *omask)
{
	(void) pool;
	*omask &= ~*imask;
	return kPGPError_NoErr;
}

PGPError
pgpVirtMaskSetBit (RingPool const *pool, PGPVirtMask *mask,
	PGPUInt32 bitnumber)
{
	(void) pool;
	*mask |= (1 << bitnumber);
	return kPGPError_NoErr;
}

PGPError
pgpVirtMaskClearBit (RingPool const *pool, PGPVirtMask *mask,
	PGPUInt32 bitnumber)
{
	(void) pool;
	*mask &= ~(1 << bitnumber);
	return kPGPError_NoErr;
}

PGPError
pgpVirtMaskClearGreaterBits (RingPool const *pool, PGPVirtMask *mask,
	PGPUInt32 firstbitnumber)
{
	(void) pool;
	if (firstbitnumber < 32)
		*mask &= (1 << firstbitnumber) - 1;
	return kPGPError_NoErr;
}


PGPError
pgpVirtMaskCopy (RingPool const *pool, PGPVirtMask const *imask,
	PGPVirtMask *omask)
{
	(void) pool;
	*omask = *imask;
	return kPGPError_NoErr;
}

PGPError
pgpVirtMaskNOT (RingPool const *pool, PGPVirtMask *mask,
	PGPUInt32 highbitnumber)
{
	(void) pool;
	*mask = ~*mask;
	if (highbitnumber < 32)
		*mask &= (1 << highbitnumber) - 1;
	return kPGPError_NoErr;
}

PGPInt32
pgpVirtMaskLSBit (PGPVirtMask const *mask)
{
	return ringLsBitFind (*mask);
}

PGPBoolean
pgpVirtMaskIsEmpty (PGPVirtMask const *mask)
{
	return !*mask;
}

PGPBoolean
pgpVirtMaskIsEqual (PGPVirtMask const *mask1, PGPVirtMask const *mask2)
{
	return *mask1 == *mask2;
}

PGPBoolean
pgpVirtMaskIsOverlapping (PGPVirtMask const *mask1, PGPVirtMask const *mask2)
{
	return (*mask1 & *mask2) != 0;
}


#else /* VIRTMASK */

/*
 * The original design included a fixed size mask with a bit for
 * each ringset in the system.  However this tended to become
 * exhausted with sufficient user windows open.  This package
 * provides an (almost) unlimited size virtual ring mask function
 * to be compatible with the previous logic but to remove the limits
 * on the number of ringsets being manipulated.
 */

PGPError
pgpVirtMaskInit (RingPool const *pool, PGPVirtMask *mask)
{
	(void)pool;
	mask->nwords = 0;
	mask->words = NULL;
	return kPGPError_NoErr;
}

#define PVM_LEAKS 0

#if PVM_LEAKS
/* leak detection */
static struct pvm {
	struct pvm *next;
	int num;
	void *addr;
} *pvm_head;
static int pvm_index;

static void
pvm_remember (PGPContextRef c, void *addr)
{
	struct pvm *n;

	n = (struct pvm *)pgpContextMemAlloc (c, sizeof(struct pvm), 0);
	n->next = pvm_head;
	n->num = pvm_index++;
	n->addr = addr;
	pvm_head = n;
}

static void
pvm_forget (PGPContextRef c, void *addr)
{
	struct pvm *n = pvm_head;
	struct pvm *pn = NULL;
	while (n) {
		if (n->addr == addr) {
			if (pn)
				pn->next = n->next;
			else
				pvm_head = n->next;
			pgpContextMemFree (c, n);
			return;
		}
		pn = n;
		n = n->next;
	}
	pgpAssert(0);
}
#endif

/* Need a lighter weight memory allocation package! */
PGPError
pgpVirtMaskCleanup (RingPool const *pool, PGPVirtMask *mask)
{
	PGPError err = kPGPError_NoErr;
	if (mask->nwords != 0) {
		err = pgpContextMemFree (pool->context, mask->words);
#if PVM_LEAKS
		pvm_forget (pool->context, mask->words);
#endif
	}
	mask->words = NULL;
	mask->nwords = 0;
	return err;
}

static PGPError
pgpVirtMaskSizeUp (RingPool const *pool, PGPVirtMask *mask,
				   PGPUInt32 minwords)
{
	PGPError err;

	/* Make minwords a multiple of 4 to reduce number of resizes */
	minwords += (-minwords) & 3;

	if (mask->nwords < minwords) {
		if (IsntNull (mask->words)) {
			void *vptr = mask->words;
#if PVM_LEAKS
			pvm_forget (pool->context, vptr);
#endif
			err = pgpContextMemRealloc (pool->context, &vptr,
				minwords * sizeof(*mask->words), kPGPMemoryMgrFlags_Clear);
#if PVM_LEAKS			
			pvm_remember (pool->context, vptr);
#endif
			if (IsPGPError(err))
				return err;
			mask->words = vptr;
			mask->nwords = minwords;
		} else {
			mask->words = pgpContextMemAlloc (pool->context,
				minwords * sizeof(*mask->words), kPGPMemoryMgrFlags_Clear);
			if (IsNull(mask->words))
				return kPGPError_OutOfMemory;
#if PVM_LEAKS
			pvm_remember(pool->context, mask->words);
#endif
			mask->nwords = minwords;
		}
	}
	return kPGPError_NoErr;
}

PGPError
pgpVirtMaskOR (RingPool const *pool, PGPVirtMask const *imask,
	PGPVirtMask *omask)
{
	PGPError err;
	PGPUInt32 i;

	err = pgpVirtMaskSizeUp (pool, omask, imask->nwords);
	if (IsPGPError (err))
		return err;
	for (i=0; i<pgpMin(imask->nwords, omask->nwords); ++i)
		omask->words[i] |= imask->words[i];
	return kPGPError_NoErr;
}

PGPError
pgpVirtMaskAND (RingPool const *pool, PGPVirtMask const *imask,
	PGPVirtMask *omask)
{
	PGPUInt32 i;

	(void)pool;
	for (i=0; i<pgpMin(imask->nwords, omask->nwords); ++i)
		omask->words[i] &= imask->words[i];
	for ( ; i<omask->nwords; ++i)
		omask->words[i] = 0;
	return kPGPError_NoErr;

}

PGPError
pgpVirtMaskANDNOT (RingPool const *pool, PGPVirtMask const *imask,
	PGPVirtMask *omask)
{
	PGPUInt32 i;

	(void)pool;
	for (i=0; i<pgpMin(imask->nwords, omask->nwords); ++i)
		omask->words[i] &= ~imask->words[i];
	return kPGPError_NoErr;

}

PGPError
pgpVirtMaskSetBit (RingPool const *pool, PGPVirtMask *mask,
	PGPUInt32 bitnumber)
{
	PGPUInt32 bitword;
	PGPError err;

	bitword = bitnumber / 32;
	err = pgpVirtMaskSizeUp (pool, mask, bitword + 1);
	if (IsPGPError(err))
		return err;
	mask->words[bitword] |= 1 << (bitnumber % 32);
	return kPGPError_NoErr;
}

PGPError
pgpVirtMaskClearBit (RingPool const *pool, PGPVirtMask *mask,
	PGPUInt32 bitnumber)
{
	PGPUInt32 bitword;

	(void) pool;
	bitword = bitnumber / 32;
	if (bitword < mask->nwords)
		mask->words[bitword] &= ~ (1 << (bitnumber % 32) );
	return kPGPError_NoErr;
}

PGPError
pgpVirtMaskClearGreaterBits (RingPool const *pool, PGPVirtMask *mask,
	PGPUInt32 firstbitnumber)
{
	PGPUInt32 bitword;

	(void) pool;
	bitword = firstbitnumber / 32;
	if (bitword < mask->nwords)
		mask->words[bitword] &= (1 << (firstbitnumber % 32) ) - 1;
	for (++bitword; bitword < mask->nwords; ++bitword) {
		mask->words[bitword] = 0;
	}
	return kPGPError_NoErr;
}

/* Complements a mask up to one less than the specified number of bits */
PGPError
pgpVirtMaskNOT (RingPool const *pool, PGPVirtMask *mask,
	PGPUInt32 highbitnumber)
{
	PGPUInt32 highbitword;
	PGPUInt32 i;
	PGPError err;

	highbitword = highbitnumber / 32;
	err = pgpVirtMaskSizeUp (pool, mask, highbitword + 1);
	if (IsPGPError(err))
		return err;
	for (i=0; i<highbitword; ++i)
		mask->words[i] = ~mask->words[i];
	mask->words[highbitword] = ((1 << (highbitnumber % 32)) - 1) &
								~mask->words[highbitword];
	return kPGPError_NoErr;
}
	

PGPError
pgpVirtMaskCopy (RingPool const *pool, PGPVirtMask const *imask,
	PGPVirtMask *omask)
{
	PGPError err;
	PGPUInt32 i;

	err = pgpVirtMaskSizeUp (pool, omask, imask->nwords);
	if (IsPGPError (err))
		return err;
	for (i=0; i<imask->nwords; ++i)
		omask->words[i] = imask->words[i];
	for ( ; i<omask->nwords; ++i)
		omask->words[i] = 0;
	return kPGPError_NoErr;
}

PGPInt32
pgpVirtMaskLSBit (PGPVirtMask const *mask)
{
	PGPUInt32 i;

	for (i=0; i<mask->nwords; ++i) {
		if (mask->words[i])
			return 32*i + ringLsBitFind (mask->words[i]);
	}
	return -1;
}

PGPBoolean
pgpVirtMaskIsEmpty (PGPVirtMask const *mask)
{
	PGPUInt32 i;

	for (i=0; i<mask->nwords; ++i)
		if (mask->words[i])
		return FALSE;
	return TRUE;
}

PGPBoolean
pgpVirtMaskIsEqual (PGPVirtMask const *mask1, PGPVirtMask const *mask2)
{
	PGPUInt32 i;

	/* First compare common length portion */
	for (i=0; i<pgpMin(mask1->nwords, mask2->nwords); ++i)
		if (mask1->words[i] != mask2->words[i])
			return FALSE;
	/* The excess must be all zeros */
	if (mask1->nwords < mask2->nwords) {
		for ( ; i < mask2->nwords; ++i)
			if (mask2->words[i])
				return FALSE;
	} else {
		for ( ; i < mask1->nwords; ++i)
			if (mask1->words[i])
				return FALSE;
	}
	return TRUE;
}

PGPBoolean
pgpVirtMaskIsOverlapping (PGPVirtMask const *mask1, PGPVirtMask const *mask2)
{
	PGPUInt32 i;

	for (i=0; i<pgpMin(mask1->nwords, mask2->nwords); ++i)
		if (mask1->words[i] & mask2->words[i])
			return TRUE;
	return FALSE;
}

#endif /* VIRTMASK */


/*
 * Small helpers to report errors
 */
/* Report an general I/O error */
void
ringErr(RingFile *file, PGPUInt32 pos, PGPError code)
{
	RingError *err = &file->set.pool->e;

	err->f = file;
	err->fpos = pos;
	err->error = code;
	err->syserrno = errno;
}

/* Report a non-I/O error */
void
ringSimpleErr(RingPool *pool, PGPError code)
{
	pool->e.f = (RingFile *)NULL;
	pool->e.fpos = (PGPUInt32)-1;
	pool->e.error = code;
	pool->e.syserrno = errno;
}

/* Report an allocation failure (called from many places) */
void
ringAllocErr(RingPool *pool)
{
	ringSimpleErr(pool, kPGPError_OutOfMemory);
}

/*
 * Hash a string of bytes.  Uses the CRC-32 polynomial, preset
 * to -1, non-invert.  Used to reduce userID collisions and to
 * create a fake keyID for unparseable keys.
 *
 * CRC-32 polynomial in little-endian order:
 *   1+x+x^2+x^4+x^5+x^7+x^8+x^10+x^11+x^12+x^16+x^22+x^23+x^26+x^32
 *               1   1   2   2   2   3
 *   0   4   8   2   6   0   4   8   2
 * = 111011011011100010000011001000001
 * =    e   d   b   8   8   3   2   0
 * = 0xedb88320
 */
#define CRCPOLY 0xedb88320
PGPUInt32
ringHashBuf(PGPByte const *buf, size_t len)
{
	PGPUInt32 crc;
	int i, j;
	static PGPUInt32 crctable[256];

	if (!crctable[255]) {
		/* crctable[0] is already 0 */
		crctable[128] = crc = CRCPOLY;
		i = 64;
		do {
			crc = crc>>1 ^ (crc & 1 ? CRCPOLY : 0);
			for (j = 0; j < 256; j += 2*i)
				crctable[i+j] = crc ^ crctable[j];
		} while (i >>= 1);
	}

	crc = 0xffffffff;
	while (len--)
		crc = (crc >> 8) ^ crctable[(crc ^ *buf++) & 255];
	return crc;
}

/*
 * Return the index of the least significant bit in the given mask,
 * or -1 if the mask is all 0.  This uses (almost) no branches,
 * so should be nice and fast on modern processors.
 *
 * Oh, how does it *work*, you ask?  I do confess that this uses
 * two evil bit-twiddling tricks.  The first is one to get a mask
 * of the least-significant set bit in few instructions.
 * Consider a binary number x, be it 11010000 or 00101111.
 * Then think about the form of x-1, 11001111 or 00101110.
 * Notice that the only difference is that some least-significant
 * bits have been complemented.  The bits complemented are
 * those up to and including the least-significant set bit.
 * (x+1 does the same with *clear* bits).  So ANDing the two
 * results in a number like the original, only without the
 * least-significant set bit.  XORing them produces a mask
 * with a number of least-significant bits set, depending on
 * the least-significant set bit,    00011111 or 00000001.
 *
 * Other tricks: x & (x-1) returns x, but with the least-significant
 * bit cleared.  Twos complement negation is -x = ~x+1 = ~(x-1),
 * so x & -x = x & ~(x-1) returns only the least-significant bit
 * set in x.
 *
 * All we need is a routine to count the bits, and we're done.
 * This is the *second* trick.  Consider an 8-bit word:
 * +-+-+-+-+-+-+-+-+
 * |a|b|c|d|e|f|g|h|
 * +-+-+-+-+-+-+-+-+
 * Now copy this word, shift one copy down one bit, and AND both
 * copies with 0x55 (01010101), to produce even and odd bits:
 * +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 * | |b| |d| |f| |h|  | |a| |c| |e| |g| (the blank squares are zero)
 * +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 * Then add the words together:
 * +-+-+-+-+-+-+-+-+
 * |a+b|c+d|e+f|g+h|
 * +-+-+-+-+-+-+-+-+
 * Note that each two-bit field contains a count of the number of
 * bits set in that part of the original word.  Repeating this produces:
 * +-+-+-+-+-+-+-+-+   +-+-+-+-+-+-+-+-+   +-+-+-+-+-+-+-+-+
 * |   |c+d|   |g+h| + |   |a+b|   |e+f| = |a+b+c+d|e+f+g+h|
 * +-+-+-+-+-+-+-+-+   +-+-+-+-+-+-+-+-+   +-+-+-+-+-+-+-+-+
 * and once more produces:
 * +-+-+-+-+-+-+-+-+   +-+-+-+-+-+-+-+-+   +-+-+-+-+-+-+-+-+
 * |       |e+f+g+h| + |       |a+b+c+d| = |a+b+c+d+e+f+g+h|
 * +-+-+-+-+-+-+-+-+   +-+-+-+-+-+-+-+-+   +-+-+-+-+-+-+-+-+
 *
 * Ther masking is needed so that fields don't overflow into
 * adjacent fields.  Once the fields have gotten wide enough,
 * some of it can be reduced or eliminated.  In the last step,
 * ab+c+d and e+f+g+h are both at most 4 (binary 100) and
 * their sum is at most 8 (binary 1000), which still fits into a
 * 4-bit field, so it is possible to not mask the inputs to
 * the addition.  It's still necessary to mask the output, though,
 * since the next step (adding up to a maximum of 16) won't
 * fit into a 4-bit field.  Once you have an 8-bit field, though,
 * you can stop masking until the very end unless you have a
 * 256-bit word.
 */
#define SIZEOFMASK	32
int
ringLsBitFind(PGPUInt32 mask)
{
	if (!mask)
		return -1;
	mask ^= mask-1;	/* Number of bits set is position of lsbit + 1 */
#if SIZEOFMASK > 32
	mask = (mask & 0x5555555555555555) + (mask >> 1 & 0x5555555555555555);
	mask = (mask & 0x3333333333333333) + (mask >> 2 & 0x3333333333333333);
	mask = (mask + (mask >> 4)) & 0x0F0F0F0F0F0F0F0F;
	mask += mask >> 8;
	mask += mask >> 16;
	mask += mask >> 32;
	return (int)(mask & 255)-1;
#elif SIZEOFMASK > 16
	mask = (mask & 0x55555555) + (mask >> 1 & 0x55555555);
	mask = (mask & 0x33333333) + (mask >> 2 & 0x33333333);
	mask = (mask + (mask >> 4)) & 0x0F0F0F0F;
	mask += mask >> 8;
	mask += mask >> 16;
	return (int)(mask & 255)-1;
#elif SIZEOFMASK > 8
	mask = (mask & 0x5555) + (mask >> 1 & 0x5555);
	mask = (mask & 0x3333) + (mask >> 2 & 0x3333);
	mask = (mask + (mask >> 4)) & 0x0F0F;
	mask += mask >> 8;
	return (int)(mask & 255)-1;
#else
	mask = (mask & 0x55) + (mask >> 1 & 0x55);
	mask = (mask & 0x33) + (mask >> 2 & 0x33);
	mask = (mask + (mask >> 4)) & 0x0F;
	return (int)(mask - 1);
#endif
}

/*
 * Return in *omask the mask of bits which are actually in use, excepting the
 * given RingSet, if non-null.  I.e. the mask which would be in use
 * if that RingSet were discarded.  RingFile structures are not
 * included on the sets list, so they are accounted for in the
 * pool->filemask.
 */
PGPError
ringAllocMask(RingPool const *pool, RingSet const *set0,
			  PGPVirtMask *omask)
{
	RingSet const *set;
	PGPVirtMask mask;
	PGPError err = kPGPError_NoErr;

	if (IsPGPError(err = pgpVirtMaskCleanup (pool, omask)))
		return err;
	if (IsPGPError(err = pgpVirtMaskInit (pool, &mask)))
		return err;
	if (IsPGPError(err = pgpVirtMaskCopy (pool, &pool->filemask, &mask)))
		return err;

	for (set = pool->sets; set; set = set->next) {
		if (IsPGPError( err =
				pgpVirtMaskOR (pool, &set->mask, &mask)))
			return err;
	}
	if (set0) {		/* set0 may or may not be on the sets list */
		if (IsPGPError( err =
				pgpVirtMaskANDNOT (pool, &set0->mask, &mask)))
			return err;
	}
	*omask = mask;
	/* No cleanup on mask since it is copied to the output */
	return err;
}

/* Helper function for ringGarbageCollect */
PGPError
ringClearMask(RingPool *pool, union RingObject **objp, PGPVirtMask *andmask,
			  PGPVirtMask *andnotmask, PGPVirtMask *omask)
{
	union RingObject *robj;
	PGPVirtMask objmask;
	PGPVirtMask remmask;
	PGPVirtMask tmpmask;
	PGPError err = kPGPError_NoErr;

	if (IsPGPError( err = pgpVirtMaskInit (pool, &objmask) ) )
		goto error;
	if (IsPGPError( err = pgpVirtMaskInit (pool, &remmask) ) )
		goto error;
	if (IsPGPError( err = pgpVirtMaskInit (pool, &tmpmask) ) )
		goto error;
	while ((robj = *objp) != (union RingObject *) 0) {
		if (IsntNull(andmask) &&
			IsPGPError( err =
				pgpVirtMaskAND (pool, andmask, &robj->g.mask) ) )
			goto error;
		if (IsntNull(andnotmask) &&
			IsPGPError( err =
				pgpVirtMaskANDNOT (pool, andnotmask, &robj->g.mask) ) )
			goto error;
		if (IsPGPError( err =
				pgpVirtMaskCopy (pool, &robj->g.mask, &objmask) ) )
			goto error;
	    if (!OBJISBOT(robj)) {
			if (IsPGPError( err = 
				ringClearMask (pool, &robj->g.down, andmask,
							   andnotmask, &tmpmask) ) )
				goto error;
			if (IsPGPError( err =
					pgpVirtMaskOR (pool, &tmpmask, &remmask) ) )
				goto error;
		}
	    /*  Skip dummy objects (robj->g.mask == 0), but delete
	        objects that are only in the memory ring. Also skip
			keys which must be kept as dummy keys (because they've
			signed something). */
		if (!pgpVirtMaskIsEmpty(&robj->g.mask)) {
			if (IsPGPError( err =
					pgpVirtMaskCopy(pool, &objmask, &tmpmask) ) )
				goto error;
			if (IsPGPError( err =
					pgpVirtMaskANDNOT(pool, &pool->memringmask, &tmpmask) ) )
				goto error;
			if (pgpVirtMaskIsEmpty (&tmpmask)) {
				/* Therefore object is ONLY on MEMRING */
				if (!(OBJISTOPKEY(robj) && robj->k.sigsby != NULL)) {
					/* Also object hasn't made any sigs */
					/*
					 * Delete no-longer-used object.
					 * This does not free the memring data area, however
					 * it will be reclaimed in ringGarbageCollect once no
					 * objects are using that area.
					 */
					pgpAssert(OBJISBOT(robj) || !robj->g.down);
					*objp = robj->g.next;
					ringFreeObject(pool, robj);
					continue;
				}
			}
		}
		/* Skip to next object */
		if( IsPGPError( err = 
				pgpVirtMaskOR (pool, &objmask, &remmask) ) )
			goto error;
		objp = &robj->g.next;
	}
	pgpVirtMaskCleanup (pool, omask);
	*omask = remmask;
error:
	pgpVirtMaskCleanup (pool, &tmpmask);
	pgpVirtMaskCleanup (pool, &objmask);
	return err;
}

/*
 * Reclaim all unused bits and delete any unreferenced memory objects.
 * Return TRUE if did some reclamation.
 */
PGPBoolean
ringGarbageCollect(RingPool *pool)
{
	PGPVirtMask mask;
	PGPError err;

	/*  Build sig lists so we know which keys act as signers.
	    These should be left as dummy keys rather than freed. */
	ringPoolListSigsBy (pool);
	if (IsPGPError( err = pgpVirtMaskInit (pool, &mask) ) )
		return FALSE;
	if (IsPGPError (err = ringAllocMask(pool, (RingSet const *)NULL, &mask)))
		return FALSE;
	if (!pgpVirtMaskIsEqual (&mask, &pool->allocmask)) {
		pgpVirtMaskCopy (pool, &mask, &pool->allocmask);
		if (IsPGPError( err = ringClearMask(pool, &pool->keys, &mask,
											NULL, &mask)))
			return FALSE;
		pgpVirtMaskAND(pool, &pool->memringmask, &mask);
		if (pgpVirtMaskIsEmpty(&mask)) {
			memPoolEmpty(&pool->files[MEMRINGBIT]->strings);
			memPoolEmpty(&pool->files[MEMRINGBIT]->fpos);
			pool->files[MEMRINGBIT]->freepos = NULL;
			pgpVirtMaskCleanup (pool, &mask);
			return TRUE;	/* Something freed */
		}
	}
	pgpVirtMaskCleanup (pool, &mask);
	return FALSE;	/* Nothing freed */
}

/* Remove a single key from its hash chain */
static void
ringGarbageHackKey(RingPool *pool, RingKey *key)
{
	RingKey **keyp;
	
	keyp = &pool->hashtable[key->keyID[0]];

	while (*keyp != key) {
		pgpAssert(*keyp);
		keyp = &(*keyp)->util;
	}

	*keyp = key->util;
}

/* Remove a single signature from its sigsby list */
static void
ringGarbageHackSig(RingSig *sig)
{
	RingKey *key;
	RingSig **sigp;
	
	key = &sig->by->k;
	pgpAssert(KEYISKEY(key));

	/*
	 * This could be one loop but for type rules, sigh...
	 * The problem doesn't happen often, fortunately.
	 * (The single loop can be expressed in C using the
	 * cheat sigp = (RingSig **)&key->sigsby; but
	 * while that's portable in practice, we eschew it
	 * for the sake of ANSI C purity.)
	 */
	if (&key->sigsby->s == sig) {
		key->sigsby = (union RingObject *)sig->nextby;
	} else {
		sigp = &key->sigsby->s.nextby;
		while (*sigp != sig)
			sigp = &(*sigp)->nextby;
		*sigp = sig->nextby;
	}
}

/*
 * Delete a single object from the global pool if it is an unreferenced
 * memory object.
 */
void
ringGarbageCollectObject(RingPool *pool, union RingObject *robj)
{
	union RingObject **objp;

	if (pgpVirtMaskIsEqual (&robj->g.mask, &pool->memringmask)) {
		pgpAssert(!robj->g.down);
		objp = OBJISTOP(robj) ? &pool->keys : &robj->g.up->g.down;
		while (*objp != robj) {
			pgpAssert(*objp);
			objp = &(*objp)->g.next;
		}
		*objp = robj->g.next;
		if (OBJISKEY(robj))
			ringGarbageHackKey(pool, &robj->k);
		else if (OBJISSIG(robj))
			ringGarbageHackSig(&robj->s);
		ringFreeObject(pool, robj);
	}
}

/* Find and allocate a new, unused mask bit.  Search through all bits
 * until we find one unused.
 */
PGPError
ringBitAlloc(RingPool *pool, PGPUInt32 *newbit)
{
	PGPVirtMask mask;

#if VIRTMASK
	PGPUInt32 maskbit;
	PGPError err;

	maskbit = MEMRINGBIT + 1;
	if( IsPGPError( err = pgpVirtMaskInit (pool, &mask) ) )
		return err;
	for ( ; ; ) {
		if( IsPGPError( err =
				pgpVirtMaskSetBit (pool, &mask, maskbit) ) )
			return err;
		if (!pgpVirtMaskIsOverlapping (&mask, &pool->allocmask)) {
			/* Occasionally reclaim unused bits */
			if (maskbit % 32 == 0) {
				ringGarbageCollect(pool);
			}
			*newbit = maskbit;
			err = pgpVirtMaskCleanup (pool, &mask);
			return err;
		}
		pgpVirtMaskClearBit (pool, &mask, maskbit);
		++maskbit;
	}
	/* NOTREACHED */
#else
	/* Allocate a new bit */
	pgpVirtMaskInit (pool, &mask);
	pgpVirtMaskCopy (pool, &pool->allocmask, &mask);
	pgpVirtMaskNOT (pool, &mask, 32);
	if (pgpVirtMaskIsEmpty(&mask)) {
		/* Wups, out of bits - try something before dying */
		ringGarbageCollect(pool);
		pgpVirtMaskCopy (pool, &pool->allocmask, &mask);
		pgpVirtMaskNOT (pool, &mask, 32);
		if (pgpVirtMaskIsEmpty(&mask)) {
			ringSimpleErr(pool, kPGPError_OutOfRings);
			return kPGPError_OutOfRings;
		}
	}
	*newbit = pgpVirtMaskLSBit(&mask);
	return kPGPError_NoErr;
#endif
}

/*
 * Allocate and deallocate useful structures.
 * Note the interesting shenanigans used to allocate
 * a structure the alignment of an enclosing union, ensuring
 * that even on a maximally-perverse ANSI C implementation,
 * it is safe to cast the returned structure pointer to a union
 * pointer.
 */
union RingObject *
ringNewObject(RingPool *pool, int objtype)
{
	union RingObject *robj;
	/* How to initialize each object to empty */
	static RingKey const nullkey = NULLRINGKEY;
	static RingSec const nullsec = NULLRINGSEC;
	static RingName const nullname = NULLRINGNAME;
	static RingSig const nullsig = NULLRINGSIG;
	static RingUnk const nullunk = NULLRINGUNK;
	static void const *nullobjs[RINGTYPE_MAX] = {
		&nullkey, &nullsec, &nullname, &nullsig, &nullunk
	};
	size_t const sizes[RINGTYPE_MAX] = {
		sizeof(RingKey), sizeof(RingSec),
		sizeof(RingName), sizeof(RingSig),
		sizeof(RingUnk)
	};

	/* Object types are 1-based */
	pgpAssert(objtype > 0);
	pgpAssert(objtype <= RINGTYPE_MAX);

	robj = pool->freeobjs[objtype-1];
	if (robj) {
		pool->freeobjs[objtype-1] = robj->g.next;
	} else {
		robj = (union RingObject *)
			memPoolAlloc(&pool->structs, sizes[objtype-1],
				     alignof(union RingObject));
		if (!robj) {
			ringAllocErr(pool);
			return NULL;
		}
	}
	memcpy(robj, nullobjs[objtype-1], sizes[objtype-1]);
	pgpAssert(ringObjectType(robj) == objtype);
	return robj;
}

/*
 * Free an object.  This does not do any cleanup with any pointers in the
 * object, except the regexp which belongs to sig objects.
 */
void
ringFreeObject(RingPool *pool, union RingObject *obj)
{
	int type = ringObjectType(obj);

	pgpAssert(type > 0);
	pgpAssert(type <= RINGTYPE_MAX);

	if (OBJISSIG(obj)) {
		if (obj->s.regexp) {
			pgpContextMemFree( pool->context, obj->s.regexp );
			obj->s.regexp = NULL;
		}
	}

	pgpVirtMaskCleanup (pool, &obj->g.mask);

	obj->g.next = pool->freeobjs[type-1];
	pool->freeobjs[type-1] = obj;
}

/*
 * Remove an object from its parent and free it.  This does not do
 * anything with the object's FilePos list.
 */
void
ringRemObject(RingPool *pool, union RingObject *obj)
{
	union RingObject **objp;

	pgpAssert(!OBJISTOP(obj));
	objp = &obj->g.up->g.down;

	/* Unlink the object from its parent */
	while (*objp != obj) {
		pgpAssert(*objp);
		objp = &(*objp)->g.next;
	}
	*objp = obj->g.next;
	ringFreeObject(pool, obj);
}

/*
 * Rebuild the pool's hash table from scratch,
 * inserting all keys and subkeys.
 */
void
ringPoolHash(RingPool *pool)
{
	union RingObject *key, *subkey;
	int i;

	for (i = 0; i < 256; i++)
		pool->hashtable[i] = NULL;

	for (key = pool->keys; key; key = key->g.next) {
		pgpAssert(OBJISKEY(key));
		RINGPOOLHASHKEY(pool, key);
		for (subkey = key->g.down; subkey; subkey = subkey->g.next) {
			if (OBJISKEY(subkey))
				RINGPOOLHASHKEY(pool, subkey);
		}
	}
}

/*
 * Find a key given a keyID.
 *
 * ViaCrypt added pkalgs 2 and 3 which are limited RSA, but doesn't
 * completely distinguish beterrn them, so this doesn't either.  Sigh.
 */
union RingObject *
ringPoolFindKey(RingPool const *pool, PGPByte pkalg, PGPByte const keyID[8])
{
	RingKey *key;

	if ((pkalg | 1) == 3)
		pkalg = 1;
	for (key = pool->hashtable[keyID[0]]; key; key = key->util) {
		if (memcmp(keyID, key->keyID, 8) == 0) {
			if (pkalg == key->pkalg)
				break;
			/* Cope with ViaCrypt's things */
			if (pkalg == 1 && (key->pkalg | 1) == 3)
				break;
		}
	}

	return (union RingObject *)key;
}


/*
 * Ensure that each key's list of the signatures by it is
 * valid.  This also establishes the extra invariant (used in
 * pgpRngMnt.c) that all signatures by one key on another object
 * are adjacent on that key's sigsby list.
 */
void
ringPoolListSigsBy(RingPool *pool)
{
	union RingObject *key, *n, *s;

	/* Initialize sigsby lists to null */
	for (key = pool->keys; key; key = key->g.next) {
		pgpAssert(OBJISTOPKEY(key));
		key->k.sigsby = NULL;
	}

	/* Install every sig on a sigsby list */
	for (key = pool->keys; key; key = key->g.next) {
	    for (n = key->k.down; n; n = n->g.next) {
	        if (OBJISSIG(n)) {
		    n->s.nextby = (RingSig *) n->s.by->k.sigsby;
		    n->s.by->k.sigsby = n;
		} else for (s = n->g.down; s; s = s->g.next) {
		    if (OBJISSIG(s)) {
		        s->s.nextby = (RingSig *) s->s.by->k.sigsby;
			s->s.by->k.sigsby = s;
		    }
		}
	    }
	}
}


/*
 * Return the mask of RingFiles that are "better" (higher priority
 * for fetching) than *any* home of the specified object.
 */
	PGPError
ringObjBetters(union RingObject const *obj, RingPool const *pool,
			   PGPVirtMask *omask)
{
	PGPVirtMask better;
	PGPVirtMask mask;
	PGPInt32 bit;

	pgpVirtMaskCleanup (pool, omask);

	pgpVirtMaskInit (pool, &better);
	pgpVirtMaskInit (pool, &mask);
	pgpVirtMaskCopy (pool, &obj->g.mask, &mask);
	pgpVirtMaskAND (pool, &pool->filemask, &mask);
	pgpVirtMaskCopy (pool, &pool->filemask, &better);

	pgpAssert(!pgpVirtMaskIsEmpty(&mask));

	while (!pgpVirtMaskIsEmpty(&mask)) {
		bit = pgpVirtMaskLSBit(&mask);
		pgpVirtMaskAND (pool, &pool->files[bit]->higherpri, &better);
		pgpVirtMaskClearBit(pool, &mask, bit);
	}

	pgpVirtMaskCleanup (pool, &mask);
	*omask = better;
	return kPGPError_NoErr;
}


/*
 * Return TRUE if the specified subkey has a valid sig from the main key.
 * Assumes subkey sigs are always tried, which should happen when they are
 * created or added to the keyring.  The only time this isn't true is when
 * we are considering adding a key.  We will give the sig the benefit of
 * the doubt in that case as we aren't using it yet.
 */
int
ringSubkeyValid(RingSet const *set, union RingObject *subkey,
	PGPBoolean unExpired)
{
	union RingObject *sig;
	union RingObject *key;
	PGPUInt32		  curtime;
	PGPUInt32		  exptime;

	pgpAssert(OBJISSUBKEY(subkey));
	pgpAssert(pgpIsRingSetMember(set, subkey));
	key = subkey->g.up;
	pgpAssert(OBJISTOPKEY(key));

	if (subkey->k.trust & PGP_KEYTRUSTF_REVOKED)
	    return 0;
	if (unExpired) {
		/* Don't use key if has expired or creation time is > 24 hours
			in future */
		if (subkey->k.trust & PGP_KEYTRUSTF_EXPIRED)
			return 0;
		curtime = (PGPUInt32) PGPGetTime();
		exptime = (PGPUInt32) ringKeyExpiration(set, subkey);
		if ((exptime != 0  &&  curtime > exptime)  ||
			curtime < ringKeyCreation(set, subkey) - 24*60*60)
			return 0;
	}
	/* Check legality of subkey */
	for (sig = subkey->g.down; sig; sig = sig->g.next) {
		if (OBJISSIG(sig) &&
			pgpIsRingSetMember(set, sig) &&
		    ringSigMaker(set, sig, set)==key &&
		    ringSigType(set, sig) == PGP_SIGTYPE_KEY_SUBKEY) {
			if (!ringSigTried(set, sig))
				return 1; /* could check it here... */
			if (ringSigChecked(set, sig))
				return 1;
		}
	}
	return 0;
}

void
ringPurgeCachedName(RingPool const *pool, RingName *name, PGPVirtMask *mask)
{
	pgpAssert(NAMEISNAME(name));

	if (NAMEISCACHED(name)) {
		PGPInt32 bit = NAMEFILEMASK(name);
		PGPVirtMask tmask;
		pgpVirtMaskInit (pool, &tmask);
		pgpVirtMaskSetBit (pool, &tmask, bit);

		if (pgpVirtMaskIsOverlapping (&tmask, mask)) {
			/* Replace buffer with a hash of it */
			name->name.hash = ringHashBuf((PGPByte const *)name->name.ptr,
										  name->len);
			NAMECLEARCACHED(name);
		}
		pgpVirtMaskCleanup (pool, &tmask);
	}
}

/*
 * This function is called by the MemPool code when it runs out of memory.
 * We try to free up more memory by purging the uids from cache.
 * Returns zero if it was unable to make more memory available;
 * non-zero if it might be useful to retry an allocation.
 */
static int
ringPurgeUidCache(void *arg)
{
	RingPool *pool = (RingPool *)arg;
	PGPVirtMask notmemringmask;
	union RingObject *k, *n;
	unsigned i;

	/*
	 * Quick check to see if we can do anything.  As memory gets
	 * full, the full walk needed to clear the cache gets expensive,
	 * so avoid it unless it does some good.
	 */
	i = MEMRINGBIT+1;
	while (IsntNull(pool->files[i]) &&
		   memPoolIsEmpty(&pool->files[i]->strings)) {
		if (++i == pool->nfiles) /* Last resort: try garbage collect */
			return ringGarbageCollect(pool);
	}

	/*
	 * Okay, we have something cached to free; replace all the
	 * pointers to non-MEMRINGBIT cached named with hashes
	 * of the names and then deallocate the names.
	 */

	pgpVirtMaskInit (pool, &notmemringmask);
	pgpVirtMaskCopy (pool, &pool->memringmask, &notmemringmask);
	pgpVirtMaskNOT (pool, &notmemringmask, pool->nfiles);

	for (k = pool->keys; k; k = k->g.next) {
		pgpAssert(OBJISKEY(k));
		for (n = k->g.down; n; n = n->g.next) {
			if (OBJISNAME(n))
				ringPurgeCachedName(pool, &n->n, &notmemringmask);
		}
	}

	/* Free the pools */
	for (i = 0; i < pool->nfiles; i++)
		if (IsntNull(pool->files[i]))
			memPoolEmpty(&pool->files[i]->strings);

	pgpVirtMaskCleanup (pool, &notmemringmask);

	return 1;	/* We freed some memory */
}

/* 
 * Initialize a new RingFile structure.  This does not set the mask field.
 */
static void
ringFileInit(
	RingPool *pool, RingFile *file)
{
	pgpAssert( pgpContextIsValid( pool->context ) );
	
	file->set.pool = pool;
	file->set.next = NULL;
	pgpVirtMaskInit (pool, &file->set.mask);
	file->set.type = RINGSET_FILE;

	file->f = NULL;
	file->destructor = NULL;
	file->arg = NULL;
	memPoolInit( pool->context, &file->strings);
	memPoolInit( pool->context, &file->troublepool);
	memPoolSetPurge(&file->troublepool, ringPurgeUidCache, (void *)pool);
	file->trouble = NULL;
	file->troubletail = &file->trouble;
	memPoolInit( pool->context, &file->fpos);
	memPoolSetPurge(&file->fpos, ringPurgeUidCache, (void *)pool);
	file->freepos = NULL;
	pgpVirtMaskInit (pool, &file->higherpri);
	file->flags = 0;
}

/*
 * Initialize pool->files to hold entries so we can access newfile
 */
void
ringFilesInit(RingPool *pool, PGPUInt32 newfile)
{
	void *vfiles;
	RingFile *file;

	if (pool->nfiles < newfile+1) {
		vfiles = pool->files;
		pgpContextMemRealloc (pool->context, &vfiles,
				  (newfile+1) * sizeof(RingFile *), kPGPMemoryMgrFlags_Clear);
		pool->files = (RingFile **) vfiles;
		pool->nfiles = newfile+1;
	}
	/* Init newly allocated file struct */
	pool->files[newfile] = (RingFile *)pgpContextMemAlloc (pool->context,
								sizeof(RingFile), kPGPMemoryMgrFlags_Clear);
	file = pool->files[newfile];
	ringFileInit (pool, file);
	
	pgpVirtMaskSetBit (pool, &file->set.mask, newfile);
}

/*
 * Initialize a newly allocated RingPool.
 */
void
ringPoolInit(
	PGPContextRef	context,
	RingPool *		pool,
	PGPEnv const *	env)
{
	int i;

	pgpAssert( pgpContextIsValid( context ) );
	
	pool->context	= context;
	
	memPoolInit( context, &pool->structs);
	memPoolSetPurge(&pool->structs, ringPurgeUidCache, (void *)pool);
	pool->keys = NULL;

	for (i = 0; i < RINGTYPE_MAX; i++)
		pool->freeobjs[i] = NULL;
	pool->sets = NULL;
	pool->freesets = NULL;
	pool->freeiter = NULL;

	pool->pktbuf = NULL;
	pool->pktbuflen = 0;
	pool->pktbufalloc = 0;

	/* Reserve first keyring for memory */
	pgpVirtMaskInit (pool, &pool->memringmask);
	pgpVirtMaskInit (pool, &pool->allocmask);
	pgpVirtMaskInit (pool, &pool->filemask);
	pgpVirtMaskSetBit (pool, &pool->memringmask, MEMRINGBIT);
	pgpVirtMaskCopy (pool, &pool->memringmask, &pool->allocmask);
	pgpVirtMaskCopy (pool, &pool->memringmask, &pool->filemask);

	pool->flags = 0;

	if (env) {
		i = pgpenvGetInt(env, PGPENV_CERTDEPTH, NULL, NULL);
		pool->certdepth = i;

		/* Values used for TRUSTMODEL 0 */
		i = pgpenvGetInt(env, PGPENV_MARGINALS, NULL, NULL);
		pool->num_marginals = (i < 0) ? 0 : (i > 255) ? 255 : i;
		i = pgpenvGetInt(env, PGPENV_COMPLETES, NULL, NULL);
		pool->num_completes = (i < 0) ? 0 : (i > 255) ? 255 : i;

		/* Values used for TRUSTMODEL 1 */
		i = pgpenvGetInt(env, PGPENV_TRUSTED, NULL, NULL);
		pool->threshold = (i > PGP_NEWTRUST_INFINITE) ?
			PGP_NEWTRUST_INFINITE : (i < 0) ? 0 : i;

		i = pgpenvGetInt(env, PGPENV_MARGINALS, NULL, NULL);
		i = (i < 1) ? 0 : (pool->threshold+i-1)/i;
		pool->marginalconfidence = i;
	
		i = pgpenvGetInt(env, PGPENV_COMPLETES, NULL, NULL);
		i = (i < 1) ? 0 : (pool->threshold+i-1)/i;
		pool->completeconfidence = i;
	} else {
		pool->certdepth = 4;
		pool->num_marginals = 2;
		pool->num_completes = 1;
		pool->threshold = 3*PGP_TRUST_DECADE_INTERNAL;
		pool->marginalconfidence = 3*PGP_TRUST_DECADE_INTERNAL/2;
		pool->completeconfidence = 3*PGP_TRUST_DECADE_INTERNAL;
	}
#if PGPTRUSTMODEL==2
	memPoolInit (context, &pool->pathpool);
	pool->paths = NULL;
	pool->pathlists = NULL;
#endif
	
	ringPoolClearError(pool);

	for (i = 0; i < 256; i++)
		pool->hashtable[i] = NULL;

	pool->nfiles = 0;
	pool->files = NULL;
	ringFilesInit (pool, MEMRINGBIT);

	/* Also purge strings cache if needed to create a new object. */
	memPoolSetPurge(&pool->files[MEMRINGBIT]->strings,
	                ringPurgeUidCache, (void *)pool);
}

/*
 * Deallocate everything in sight on a RingPool preparatory to
 * deallocating it.
 */
void
ringPoolFini(RingPool *pool)
{
	RingFile *file;
	int bit;
	
	/*
	 * Do this first part, until the destructors are called,
	 * "properly" so structures aren't dangling undefined.
	 */
	for (bit = 0; bit < (int)pool->nfiles; bit++)
		if (IsntNull(pool->files[bit]))
			ringFilePurgeTrouble(pool->files[bit]);

	for (bit = 0; bit < (int)pool->nfiles; bit++) {
		file = pool->files[bit];
		if (file && file->destructor) {
			file->destructor(file, file->f, file->arg);
			file->destructor = NULL;
		}
	}

	memPoolEmpty(&pool->structs);
#if PGPTRUSTMODEL==2
	memPoolEmpty (&pool->pathpool);
#endif

	for (bit = 0; bit < (int)pool->nfiles; bit++) {
		file = pool->files[bit];
		if (IsntNull(file)) {
			memPoolEmpty(&file->strings);
			memPoolEmpty(&file->fpos);
			pgpVirtMaskCleanup(pool, &file->set.mask);
			pgpContextMemFree (pool->context, file);
			pool->files[bit] = NULL;
		}
	}

	if (IsntNull (pool->files)) {
		pgpContextMemFree (pool->context, pool->files);
		pool->files = NULL;
		pool->nfiles = 0;
	}

	if( IsntNull( pool->pktbuf ) )
		pgpContextMemFree( pool->context, pool->pktbuf);

	pgpVirtMaskCleanup (pool, &pool->memringmask);
	pgpVirtMaskCleanup (pool, &pool->allocmask);
	pgpVirtMaskCleanup (pool, &pool->filemask);

	/* Nuke the lot */
	pgpClearMemory( pool,  sizeof(*pool));
}

/*
 * This is defined as a macro.
 *
 * void
 * ringFileMarkDirty(RingFile *file)
 * {
 *	file->flags |= RINGFILEF_DIRTY;
 * }
 */

/*
 * Mark every file under a given mask as dirty.
 */
void
ringPoolMarkDirty(RingPool *pool, PGPVirtMask *mask)
{
	PGPVirtMask tmask;

	pgpVirtMaskInit (pool, &tmask);
	pgpVirtMaskCopy (pool, mask, &tmask);
	pgpVirtMaskAND (pool, &pool->filemask, &tmask);

	while (!pgpVirtMaskIsEmpty (&tmask)) {
		PGPInt32 bit = pgpVirtMaskLSBit (&tmask);
		pgpAssert (bit >= 0);
		ringFileMarkDirty(pool->files[bit]);
		pgpVirtMaskClearBit (pool, &tmask, bit);
	}
	pgpVirtMaskCleanup (pool, &tmask);
}

void
ringPoolMarkTrustChanged(RingPool *pool, PGPVirtMask *mask)
{
	PGPVirtMask tmask;

	pgpVirtMaskInit (pool, &tmask);
	pgpVirtMaskCopy (pool, mask, &tmask);
	pgpVirtMaskAND (pool, &pool->filemask, &tmask);

	while (!pgpVirtMaskIsEmpty (&tmask)) {
		PGPInt32 bit = pgpVirtMaskLSBit (&tmask);
		pgpAssert (bit >= 0);
		pool->files[bit]->flags |= RINGFILEF_TRUSTCHANGED;
		pgpVirtMaskClearBit (pool, &tmask, bit);
	}
	pgpVirtMaskCleanup (pool, &tmask);
}


/*
 * Do a fingerprint20 (SHA-1) hash on the specified buffer, which
 * should be key data.  We prefix it with the type and length bytes
 * for compatibility with key signature hashes (once they become SHA
 * based).  Return the number of bytes in the hash, or negative on
 * error.
 */
int
pgpFingerprint20HashBuf(PGPContextRef context, PGPByte const *buf, size_t len,
						PGPByte *hash)
{
	PGPHashVTBL const *h;
	PGPHashContext *hc;
	PGPByte tmpbuf[3];
	PGPByte const *p;
    PGPMemoryMgrRef	memoryMgr	= PGPGetContextMemoryMgr( context );

	h = pgpHashByNumber (kPGPHashAlgorithm_SHA);
	if (!h)
		return kPGPError_BadHashNumber;
	hc = pgpHashCreate( memoryMgr, h);
	if (!hc)
		return kPGPError_OutOfMemory;
	/* We use this format even for subkeys */
	tmpbuf[0] = PKTBYTE_BUILD(PKTBYTE_PUBKEY, 1);
	tmpbuf[1] = (PGPByte)(len>>8);
	tmpbuf[2] = (PGPByte)len;
	PGPContinueHash(hc, tmpbuf, 3);
	PGPContinueHash(hc, buf, len);
	p = (PGPByte *) pgpHashFinal(hc);
	memcpy(hash, p, h->hashsize);
	PGPFreeHashContext(hc);
	return h->hashsize;
}

/* Call this for sigs known to use a regexp, to return the regexp.  Loads
 * from disk if necessary.  Returns NULL on error.
 */
	void *
ringSigRegexp( RingSet const *set, RingObject *sig )
{
	PGPByte *buf;
	PGPSize len;
	regexp *rexp;
	char const *packet;

	pgpAssert( OBJISSIG( sig ) );
	pgpAssert( pgpIsRingSetMember(set, sig) );
	pgpAssert( SIGUSESREGEXP( &sig->s ) );

	if( !sig->s.regexp ) {
		/* Here we must load the regexp */

		buf = (PGPByte *)ringFetchObject(set, sig, &len);
		if( !buf )
			return NULL;
		/* Note that this may alter the contents of buf */
		packet = (char *)ringSigFindSubpacket(buf, SIGSUB_REGEXP, 0, &len,
											  NULL, NULL, NULL, NULL);
		pgpAssert( packet );
		if (IsPGPError( pgpRegComp( set->pool->context, packet, &rexp ) ) )
			return NULL;
		sig->s.regexp = (void *) rexp;
	}
	return sig->s.regexp;
}

	
/*
 * Return true if sig is a valid revocation signature.  It must either be
 * a self sig, or it must be by a revocation key.  May be on either a subkey
 * or main key.  This does not check for expirations.
 */
	PGPBoolean
sigRevokesKey (RingSet const *set, RingObject *sig)
{
	RingObject 			*parent;
	RingObject 			*top;

	pgpAssert (OBJISSIG (sig));
	pgpAssert (pgpIsRingSetMember(set, sig));
	
	parent = sig->g.up;
	if (!OBJISKEY(parent))
		return FALSE;
	top = parent;
	while (!OBJISTOPKEY(top))
		top = top->g.up;

	if (sig->s.type != ( (top==parent) ? PGP_SIGTYPE_KEY_REVOKE
						 			   : PGP_SIGTYPE_KEY_SUBKEY_REVOKE ))
		return FALSE;
	if ((sig->s.trust & PGP_SIGTRUSTF_TRIED) &&
		!(sig->s.trust & PGP_SIGTRUSTF_CHECKED))
		return FALSE;
	/*
	 * If untried, don't accept on a subkey.  Accept on a top-level key
	 * if already shown as revoked because we don't store trust packets
	 * on such keys, so as not to break PGP 2.X.
	 */
	if (!(sig->s.trust & PGP_SIGTRUSTF_TRIED) &&
		((top != parent) ||
		 !(parent->k.trust & PGP_KEYTRUSTF_REVOKED)))
		return FALSE;

	if (sig->s.by == top)
		return TRUE;

	/*
	 * Here we have a revocation signature which is valid but is by some
	 * other key.  We will accept it only if that is a key which is marked
	 * as a revocation authorization key by this one.
	 */

	return ringKeyIsRevocationKey (top, set, sig->s.by);
}
