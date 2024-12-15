/*
* pgpRngWrit.c -- Functions to write out a keyring.
* The main function is ringPoolWriteVirt, which converts
* a virtual keyring into a physical one.
*
* Copyright (C) 1994-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by Colin Plumb.
*
* $Id: pgpRngWrit.c,v 1.2.2.1 1997/06/07 09:50:43 mhw Exp $
*/

#include <stdio.h>
#include <string.h>

#include "pgpDebug.h"
#include "pgpPktByte.h"
#include "pgpRngPars.h"
#include "pgpRngPriv.h"
#include "pgpTrust.h"
#include "pgpErr.h"
#include "pgpRngRead.h"
#include "pgpUsuals.h"

#include "pgpKludge.h"

/* This should go in a header somewhere... */
#ifndef PGPVERSION_2
#define PGPVERSION_2 2
#endif

/*
* Copy the packet to the given file. If "trust" is non-negative,
* it is appended as a trust packet. If "file" is non-NULL and the
* write is successful, the location in that file is listed as a
* position for the object.
*/
static int
ringCopyObject(struct RingSet const *set, union RingObject *obj, FILE *f,
			int trust, struct RingFile *file)
	{
			void const *buf;
			size_t len;
			int i;
			long pos;

			static int const pktbytes[RINGTYPE_MAX] = {
					PKTBYTE_BUILD(PKTBYTE_PUBKEY, 1),
					PKTBYTE_BUILD(PKTBYTE_SECKEY, 1),
					PKTBYTE_BUILD(PKTBYTE_USERID, 0),
					PKTBYTE_BUILD(PKTBYTE_SIG, 1)
			};
			buf = ringFetchPacket(set, obj);
			if (!buf)
				 return ringSetErr(set)->error;

			if (file) {
				 pos = ftell(f);
	if (pos == -1)
		return PGPERR_KEYIO_FTELL;
}

i = ringObjectType(obj);
pgpAssert(i > 0 && i <= RINGTYPE_MAX);

			i = pktBytePut(f, pktbytes[i-1], len);
			if (i < 0)
				 return i;
			if (fwrite(buf, 1, len, f) != len)
				 return PGPERR_KEYIO_WRITING;
			if (trust > 0)
					if (putc(PKTBYTE_BUILD(PKTBYTE_TRUST, 0), f) == EOF
						|| putc(1, f) == EOF
						|| putc(trust, f) == EOF)
								return PGPERR_KEYIO_WRITING;
				}

			if (file)
				 return ringAddPos(obj, file, (word32)pos);
			else
				 return 0;
	}

#define WRITETRUST_PUB	1
#define WRITETRUST_SEC	2
int
ringPoolWrite(struct RingSet const *set, FILE *f, struct RingFile **filep,
			int flags)
	{
			struct RingFile *file;
			struct RingIterator *iter;
			union RingObject *obj;
			unsigned level;
			ringmask mask;
			int i;
			int type;
			int writetrust;
			int trust;

			pgpAssert(set);
			pgpAssert(f);
			if (filep) {

					pgpAssert(f);
					pgpAssert(!(pool->filemask & ~pool->allocmask));

					i = ringBitAlloc(pool);
					if (i < 0) {
						*filep = (struct RingFile *)0;
						return PGPERR_NO_KEYBITS;
					}
					pgpAssert(i < MEMRINGBIT);

					file = &pool->files[i];
					mask = (ringmask)1 << i;
					pgpAssert(file->set.mask == mask);
				pgpAssert(!(pool->filemask & mask));

				pool->filemask |= mask;
				pool->allocmask |= mask;

				pgpAssert(!file->set.next);

				file->f = f;
				file->destructor = 0;	/* May be set later */
				file->arg = 0;
	} else {
			file = (struct RingFile *)0;
			mask = 0;
		}

iter = ringIterNew(set);
if (!iter) {
	ringfileClose(file);	/* Okay for file to be NULL */
	return ringSetError(set)->error;
}

			level = 1;
			for (;;) {
					obj = ringIterNextObject(iter);
					if (!obj) {
						if (!--level)
							return 0;		/* Successful completion */
						continue;
					}
					type = ringObjectType(obj);
					if (level++ == 1) {
						if (type == RINGTYPE_KEY && ringKeyIsSec(set, obj)) {
							 writetrust = flags & WRITETRUST_SEC;
							 continue;	/* Skip in favour of sec */
						}
						writetrust = flags & WRITETRUST_PUB;
				}
					if (writetrust) {
						switch (type) {
						case RINGTYPE_KEY:
									trust = obj->k.trust;
									break;
								case RINGTYPE_SEC:
									trust = obj->g.up->k.trust;
									break;
								case RINGTYPE_NAME:
									trust = obj->n.trust;
									break;
								case RINGTYPE_SIG:
									/*
									* 2.x compatibility kludge: Don't write trust
									* on good compromise certificates. PGP 2.x
									* maintenance dies (assert fail) if it finds
									* trust packets on key sigs.
									*/
									if (level == 2 && obj->g.up == obj->s.by
									&& obj->s.type == SIGTYPE_COMPROMISE
									&& !(~trust & SIGTRUSTF_CHECKED_TRIED))
										trust = -1;
								else
										trust = obj->s.trust;
								break;
						}
				} else {
						trust = -1;
					}
					i = ringCopyObject(set, obj, trust, f, file);
					if (i < 0)
						break;
		} /* for (;;) */
	
		/*
			* We break out on error. We could leave the partial file open,
			* as all the pointers that exist are valid. Is that useful?
			*/
			ringfileClose(file);	 /* Okay for file to be NULL */
			*filep = NULL;
			return i;
	}
