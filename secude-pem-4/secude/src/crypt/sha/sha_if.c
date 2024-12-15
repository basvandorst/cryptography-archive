/********************************************************************
 * Copyright (C) 1991, GMD. All rights reserved.                    *
 *                                                                  *
 *                                                                  *
 *                         NOTICE                                   *
 *                                                                  *
 *    Acquisition, use, and distribution of this module             *
 *    and related materials are subject to restrictions             *
 *    mentioned in each volume of the documentation.                *
 *                                                                  *
 ********************************************************************/

/*
 *  SHS interface module between sec_hash, sec_sign, sec_verify and 
 *  the C version by Peter Gutmann, HPACK Conspiracy Secret Laboratory
 *  
 *
 *  WS 8.9.92
 *
 *  Last change: 8.9.92
 *
 *  Imports:
 *
 *  shsInit(shsInfo)
 *  shsUpdate(shsInfo, inBuf, inLen)
 *  shsFinal(shsInfo)
 *
 *
 *  sha_hash(in_octets, hash_result, more)
 *
 */

#include "sha.h"
#include <secure.h>
#include <MEM_func.h>

/***************************************************************
 *
 * Procedure sha_hash
 *
 ***************************************************************/

#ifdef _DOS_
typedef union {
	WORD32 shs_digest[5];
	struct {
		char b0;
		char b1;
		char b2;
		char b3;
	}digest_word[5];
}UN_DIGEST;
#endif

#ifdef ANSI

RC sha_hash(
	OctetString	 *in_octets,
	OctetString	 *hash_result,
	More		  more
)

#else

RC sha_hash(
	in_octets,
	hash_result,
	more
)
OctetString	 *in_octets;
OctetString	 *hash_result;
More		  more;

#endif

{
        static char first = TRUE;
        static SHS_INFO shsInfo;
#ifdef _DOS_
	int		i;
	char		*hash_data;
	UN_DIGEST	un_digest;
#endif


        if(first) {
                shsInit(&shsInfo);
                first = FALSE;
        }
        shsUpdate(&shsInfo, (BYTE *) in_octets->octets, in_octets->noctets);

        if(more == SEC_END) {
                first = TRUE;
                shsFinal(&shsInfo);
                /* memory of hash_result->octets provided by calling program */
#ifdef _DOS_
		for (i=0; i<5; i++)
			un_digest.shs_digest[i] = shsInfo.digest[i];

		hash_data = hash_result->octets;
		for (i=0; i<5; i++) {
			*hash_data++ = un_digest.digest_word[i].b3;
			*hash_data++ = un_digest.digest_word[i].b2;
			*hash_data++ = un_digest.digest_word[i].b1;
			*hash_data++ = un_digest.digest_word[i].b0;
		}
#else

                bcopy(&shsInfo.digest[0], (BYTE *) hash_result->octets, SHS_DIGESTSIZE);
#endif

                hash_result->noctets = SHS_DIGESTSIZE;
        }
        return(0);
}
