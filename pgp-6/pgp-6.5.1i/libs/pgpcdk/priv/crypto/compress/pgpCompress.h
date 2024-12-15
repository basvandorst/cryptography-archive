/*
 * pgpCompress.h -- this defines the compression and decompression
 * functions for PGP.. It provides a generalized interface to the
 * compression modules, and keeps applications from requiring
 * knowledge of all the different types of compression that may be
 * supported.
 *
 * This is a Public API Function Header.
 *
 * $Id: pgpCompress.h,v 1.2 1997/05/13 01:18:57 mhw Exp $
 */

#ifndef Included_pgpCompress_h
#define Included_pgpCompress_h

/* Defintions for known compression algorithms */

#define PGP_COMPRESSALG_ZIP 1

#endif /* Included_pgpCompress_h */
