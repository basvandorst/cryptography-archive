/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */
//: PGPphraseVer.h - PGPphrase version history
//
//	$Id: PGPphraseVer.h,v 1.5 1997/05/28 15:45:21 pbj Exp $
//
//	970228	001	-first version
//
//	970303	002 -cleaned up variable names and types
//				-changed code to handle unlimited phrase length
//
//	970305	003	-rebuilt to be consistent with new version of
//				 of PGPcomdlg.  No changes to code.
//
//	970307	004	-fixed allocation bugs (thanks Brett) which caused
//				 problems for pgpMemAlloc
//				-switched from HeapAlloc to pgpMemAlloc
//				-added PGPQueryCacheSecsRemaining for Brett and TED
//				-added C++ friendly declarations
//
//	970314	005	-rebuilt with static runtime linkage
//
//	970324	006	-added sign phrase caching
//
//	970324	007	-relinked to PGPcmdlg.lib
//
//	970403	008 -fixed bug where cancelling caused NULL string errors
//
//	970407	009	-added code to support signing key selection combo box
//
//	970408	010	-changed API to PGPGetSignCachedPhrase to return KeyID
//				 strings and hash algorithm.  Now caches these values also.
//				-changed error codes
//
//	970410	011	-supported new API to PGPcomdlgGetPhrase (for retaining
//				 signing keys)
//
//	970414	012	-added code to pass along options parameter to
//				 PGPcomdlgGetPhrase
//
//	970506	013	-changed the "Purge" routines to return the status of the
//				 respective caches
//
//	970527	014	-added code to post warning on mix of RSA encryption and DSA
//				 signing
//
//	970528	015	-added caching of signing options
//
