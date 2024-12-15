/*
* pgpArmor.h -- Create an armor write module
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpArmor.h,v 1.5.2.1 1997/06/07 09:50:52 mhw Exp $
*/

#ifndef PGPARMOR_H
#define PGPARMOR_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPipeline;
#ifndef TYPE_PGPPIPELINE
#define TYPE_PGPPIPELINE 1
typedef struct PgpPipeline PgpPipeline;
#endif

struct PgpFifoDesc;
#ifndef TYPE_PGPFIFODESC
#define TYPE_PGPFIFODESC 1
typedef struct PgpFifoDesc PgpFifoDesc;
#endif

struct PgpRandomContext;
#ifndef TYPE_PGPRANDOMCONTEXT
#define TYPE_PGPRANDOMCONTEXT 1
typedef struct PgpRandomContext PgpRandomContext;
#endif

struct PgpEnv;
#ifndef TYPE_PGPENV
#define TYPE_PGPENV 1
typedef struct PgpEnv PgpEnv;
#endif

/*
* This creates a module that will take its input and mask it in ASCII
* Armor. The armortype should be one of ARMOR_NORMAL, for a normal
* armor, or ARMOR_SEPSIG, which is a separate signature.
*/
struct PgpPipeline PGPExport **
pgpArmorWriteCreate (struct PgpPipeline **head, struct PgpEnv const *env,
		struct PgpFifoDesc const *fd,
		struct PgpRandomContext const *rc, PgpVersion version,
		byte armortype);

/*
* These are the valid armortypes that can be passed into pgpArmorWriteCreate
*/
#define PGP_ARMOR_NORMAL		0
#define PGP_ARMOR_SEPSIG		1

/*
* XXX: PGP_ARMOR_SEPSIGMSG is identical to PGP_ARMOR_SEPSIG, except that
* it outputs "BEGIN PGP MESSAGE" instead of "BEGIN PGP SIGNATURE" for
* use in a PGP/MIME clear-signed message. This is only temporary, and
* should be deprecated when the library handles PGP/MIME internally.
*/
#define PGP_ARMOR_SEPSIGMSG	 	2

/*
* Create an armor pipeline for clear signatures. You should pass in the
* tail pointers for the end of the text stream and the end of the signature
* stream. This will return a pointer to the stream that will contain the
* clearsigned message. Pass in a list of hash-type-bytes so it can say
* what is being used.
*/
struct PgpPipeline PGPExport **
pgpArmorWriteCreateClearsig (struct PgpPipeline **texthead,
							struct PgpPipeline **signhead,
							struct PgpEnv const *env,
							struct PgpFifoDesc const *fd,
							PgpVersion version,
							byte *hashlist, unsigned hashlen);

/*
* Create an armor pipeline for clear signatures, based on the PGPMIME
* spec. You should pass in the tail pointers for the end of the text
* stream and the end of the signature stream.
* This will return a pointer to the stream
* that will contain the clearsigned message. Pass in a list of
* hash-type-bytes so it can say what is being used.
*/
struct PgpPipeline PGPExport **
pgpArmorWriteCreatePgpMimesig (struct PgpPipeline **texttail,
							struct PgpPipeline **signtail,
							struct PgpEnv const *env,
							struct PgpFifoDesc const *fd,
							PgpVersion version,
							byte *hashlist, unsigned hashlen);

#ifdef __cplusplus
}
#endif

#endif /* PGPARMOR_H */
