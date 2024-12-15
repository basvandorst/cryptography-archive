/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: pgpIPheader.h,v 1.16.4.2 1999/06/17 21:15:20 elowe Exp $
____________________________________________________________________________*/
#ifndef Included_pgpIPheader_h	/* [ */
#define Included_pgpIPheader_h

#include "pgpEndianConversion.h"
#include "pgpIPsecBuffer.h"

#define kPGPNetworkByteOrder	kPGPBigEndian

#define IPHDR_MAXSIZE	64

#define IP_IN_IP	4	/* Procotol for encapsulated IP header */


PGPError pgpAddProtocolToIPHeader(PGPIPsecBuffer *ipPacketIn, 
			PGPByte protocol, PGPIPsecBuffer *ipPacketOut);

PGPError pgpGetIPHeaderNextProtocol(PGPIPsecBuffer *ipPacketIn, 
			PGPBoolean tunnelMode, PGPByte *nextProtocol);

PGPError pgpClearMutableIPHeaderFields(PGPIPsecBuffer *ipPacketOut, 
			PGPByte *ipHeader);

PGPError pgpRestoreIPHeader(const PGPByte *ipHeader, 
			PGPIPsecBuffer *ipPacketOut);

PGPError pgpFindProtocolHeader(PGPIPsecBuffer *ipPacket, PGPByte protocol, 
			PGPUInt32 *headerStart);

PGPError pgpRemoveProtocolFromIPHeader(PGPIPsecBuffer *ipPacketIn, 
			PGPByte protocol, PGPByte nextProtocol, 
			PGPIPsecBuffer *ipPacketOut);

PGPUInt32 pgpGetIPHeaderLength(PGPIPsecBuffer *ipPacket);

PGPUInt16 pgpGetPacketSize(PGPIPsecBuffer *ipPacket);

void pgpSetPacketSize(PGPUInt16 packetSize, PGPIPsecBuffer *ipPacket);

PGPUInt16 pgpCalculateIPHeaderChecksum(PGPIPsecBuffer *ipPacket);

void pgpSetIPHeaderChecksum(PGPIPsecBuffer *ipPacket);

void pgpGetSrcIPAddress(PGPIPsecBuffer *ipPacket, PGPUInt32 *ipAddress);

void pgpGetDestIPAddress(PGPIPsecBuffer *ipPacket, PGPUInt32 *ipAddress);

void pgpSetSrcIPAddress(PGPIPsecBuffer *ipPacket, PGPUInt32 ipAddress);

void pgpSetDestIPAddress(PGPIPsecBuffer *ipPacket, PGPUInt32 ipAddress);

void pgpClearDFBit(PGPIPsecBuffer *ipPacket);

PGPBoolean pgpIsFragmented(PGPByte *ipPacket);

#endif /* ] Included_pgpIPheader_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/