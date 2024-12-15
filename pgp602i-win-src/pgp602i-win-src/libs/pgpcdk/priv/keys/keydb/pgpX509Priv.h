/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: pgpX509Priv.h,v 1.1.12.1 1998/11/12 03:22:08 heller Exp $
____________________________________________________________________________*/

#ifndef Included_pgpX509Priv_h
#define Included_pgpX509Priv_h


PGP_BEGIN_C_DECLARATIONS

/* Internal functions */

PGPError		pgpX509CertToSigBuffer (RingPool *pool, PGPByte *buf,
						PGPSize len, PGPSize *pktlen);

PGPError		pgpX509CertToKeyBuffer (RingPool *pool, PGPByte *buf,
						PGPSize len, PGPSize *pktlen);

PGPError		pgpX509CertToNameBuffer (RingPool *pool, PGPByte *buf,
						PGPSize len, PGPSize *pktlen);

PGPError		pgpX509CertInfoPartition( PGPByte *buf, PGPSize len,
						PGPByte **serialbuf, PGPSize *seriallen,
						PGPByte **sigalgbuf, PGPSize *sigalglen,
						PGPByte **issuerbuf, PGPSize *issuerlen,
						PGPByte **validitybuf, PGPSize *validitylen,
						PGPByte **subjectbuf, PGPSize *subjectlen,
						PGPByte **publickeyinfobuf, PGPSize *publickeyinfolen);

PGPError		pgpX509CertPartition( PGPByte *buf, PGPSize len,
						PGPByte **certinfobuf, PGPSize *certinfolen,
						PGPByte **sigalgbuf, PGPSize *sigalglen,
						PGPByte **sigbuf, PGPSize *siglen );


PGP_END_C_DECLARATIONS

#endif /* pgpX509Priv.h */

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */