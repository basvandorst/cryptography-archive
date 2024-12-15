/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: pgpIPheader.c,v 1.20.4.2 1999/06/17 21:15:20 elowe Exp $
____________________________________________________________________________*/

#ifndef USERLAND_TEST
#define USERLAND_TEST	0
#endif

#if USERLAND_TEST

#define DBG_FUNC(x)
#define DBG_ENTER()
#define DBG_PRINT(x)
#define DBG_LEAVE(x)

#else

#if PGP_WIN32

#include "ndis.h"
#include "vpndbg.h"

#elif PGP_MACINTOSH

#define DBG_FUNC(x)
#define DBG_ENTER()
#define DBG_PRINT(x)
#define DBG_LEAVE(x)

#endif

#endif

#include "pgpIPheader.h"
#include "pgpMem.h"
#include "pgpIPsecErrors.h"
#include "pgpEndianConversion.h"

/* The following constants define the byte locations of some of the
   IP header fields */

#define IPHDR_VERSION_IHL		0
#define IPHDR_TYPEOFSERVICE		1
#define IPHDR_TOTALLENGTH		2
#define IPHDR_IDENTIFICATION	4
#define IPHDR_FLAGS_FRAGOFFSET	6
#define IPHDR_TIMETOLIVE		8
#define IPHDR_PROTOCOL			9
#define IPHDR_CHECKSUM			10
#define IPHDR_SOURCEADDRESS		12
#define IPHDR_DESTADDRESS		16
#define IPHDR_OPTIONS			20

/* These macros extract the fields which aren't aligned on byte boundaries */

#define GetIPhdrVersion(x)		((x & 0xF0) >> 4)
#define GetIPhdrIHL(x)			(x & 0x0F)
#define GetIPhdrFlags(x)		((x & 0xE0) >> 5)
#define GetIPhdrFragOffset(x,y)	(((x & 0x1F) << 8) + y)

/* The immutable IP options are defined here */

#define OPTION_ENDOFOPTIONS			0
#define OPTION_NOP					1
#define OPTION_SECURITY				130
#define OPTION_EXTENDEDSECURITY		133
#define OPTION_COMMERCIALSECURITY	134
#define OPTION_ROUTERALERT			148
#define OPTION_MULTIDESTINATION		149


PGPError pgpAddProtocolToIPHeader(PGPIPsecBuffer *ipPacketIn, 
			PGPByte protocol, PGPIPsecBuffer *ipPacketOut)
{
	PGPSize ipHeaderLength;
	PGPError err = kPGPError_NoErr;

	DBG_FUNC("pgpAddProtocolToIPHeader")

	DBG_ENTER();

	ipHeaderLength = pgpGetIPHeaderLength(ipPacketIn);
	PGPCopyIPsecBuffer(NULL, ipPacketIn, 0, ipPacketOut, 0, ipHeaderLength, 0);
	ipPacketOut->data[IPHDR_PROTOCOL] = protocol;

	DBG_LEAVE(err);
	return err;
}


PGPError pgpGetIPHeaderNextProtocol(PGPIPsecBuffer *ipPacketIn, 
			PGPBoolean tunnelMode, PGPByte *nextProtocol)
{
	PGPError err = kPGPError_NoErr;

	DBG_FUNC("pgpAddProtocolToIPHeader")

	DBG_ENTER();

	if (tunnelMode)
		*nextProtocol = IP_IN_IP;
	else
		*nextProtocol = ipPacketIn->data[IPHDR_PROTOCOL];

	DBG_LEAVE(err);
	return err;
}


PGPError pgpClearMutableIPHeaderFields(PGPIPsecBuffer *ipPacketOut, 
			PGPByte *ipHeader)
{
	PGPSize ipHeaderLength;
	PGPUInt32 index;
	PGPUInt32 oldIndex;
	PGPError err = kPGPError_NoErr;

	ipHeaderLength = pgpGetIPHeaderLength(ipPacketOut);

	pgpCopyMemory(ipPacketOut->data, ipHeader, ipHeaderLength);

	ipPacketOut->data[IPHDR_TYPEOFSERVICE] = 0;
	ipPacketOut->data[IPHDR_FLAGS_FRAGOFFSET] = 0;
	ipPacketOut->data[IPHDR_FLAGS_FRAGOFFSET+1] = 0;
	ipPacketOut->data[IPHDR_TIMETOLIVE] = 0;
	ipPacketOut->data[IPHDR_CHECKSUM] = 0;
	ipPacketOut->data[IPHDR_CHECKSUM+1] = 0;

	if (ipHeaderLength == IPHDR_OPTIONS)
		return err;

	index = IPHDR_OPTIONS;
	while (ipPacketOut->data[index] != OPTION_ENDOFOPTIONS)
	{
		switch (ipPacketOut->data[index])
		{
		case OPTION_NOP:
			index++;
			break;

		case OPTION_SECURITY:
		case OPTION_EXTENDEDSECURITY:
		case OPTION_COMMERCIALSECURITY:
		case OPTION_ROUTERALERT:
		case OPTION_MULTIDESTINATION:
			index++;
			index += ipPacketOut->data[index] + 1;
			break;

		default:
			ipPacketOut->data[index] = 0;
			oldIndex = index + 1;
			index += 2;
			for (;index<=(oldIndex+ipPacketOut->data[oldIndex]); index++)
				ipPacketOut->data[index] = 0;
			ipPacketOut->data[oldIndex] = 0;
			break;
		}
	}

	return err;
}


PGPError pgpRestoreIPHeader(const PGPByte *ipHeader, 
			PGPIPsecBuffer *ipPacketOut)
{
	PGPSize ipHeaderLength;
	PGPError err = kPGPError_NoErr;

	ipHeaderLength = pgpGetIPHeaderLength(ipPacketOut);

	pgpCopyMemory(ipHeader, ipPacketOut->data, ipHeaderLength);

	return err;
}


PGPError pgpFindProtocolHeader(PGPIPsecBuffer *ipPacket, PGPByte protocol, 
			PGPUInt32 *headerStart)
{
	PGPError err = kPGPError_NoErr;

	*headerStart = 0;

	if (ipPacket->data[IPHDR_PROTOCOL] != protocol)
		return kPGPIPsecError_NoIPsecHeaderFound;

	*headerStart = pgpGetIPHeaderLength(ipPacket);

	return err;
}


PGPError pgpRemoveProtocolFromIPHeader(PGPIPsecBuffer *ipPacketIn, 
			PGPByte protocol, PGPByte nextProtocol, 
			PGPIPsecBuffer *ipPacketOut)
{
	PGPSize ipHeaderLength;
	PGPError err = kPGPError_NoErr;

	if (ipPacketIn->data[IPHDR_PROTOCOL] != protocol)
		return kPGPIPsecError_NoIPsecHeaderFound;

	ipHeaderLength = pgpGetIPHeaderLength(ipPacketIn);
	PGPCopyIPsecBuffer(NULL, ipPacketIn, 0, ipPacketOut, 0, ipHeaderLength, 0);
	ipPacketOut->data[IPHDR_PROTOCOL] = nextProtocol;

	return err;
}


PGPUInt32 pgpGetIPHeaderLength(PGPIPsecBuffer *ipPacket)
{
	return GetIPhdrIHL(ipPacket->data[IPHDR_VERSION_IHL]) * 4;
}


PGPUInt16 pgpGetPacketSize(PGPIPsecBuffer *ipPacket)
{
	return PGPEndianToUInt16(kPGPNetworkByteOrder,
			&(ipPacket->data[IPHDR_TOTALLENGTH]));
}


void pgpSetPacketSize(PGPUInt16 packetSize, PGPIPsecBuffer *ipPacket)
{
	PGPUInt16ToEndian(packetSize, kPGPNetworkByteOrder,
		&(ipPacket->data[IPHDR_TOTALLENGTH]));

	return;
}


PGPUInt16 pgpCalculateIPHeaderChecksum(PGPIPsecBuffer *ipPacket)
{
	PGPUInt32 total;
	PGPUInt16 checksum;
	PGPSize index;
	PGPSize ipHeaderLength;
	PGPByte bigEndianShift = 0;
	PGPByte littleEndianShift = 0;

	total = 0;
	ipHeaderLength = pgpGetIPHeaderLength(ipPacket);

	checksum = PGPEndianToUInt16(kPGPNetworkByteOrder, 
					&(ipPacket->data[IPHDR_CHECKSUM]));

	ipPacket->data[IPHDR_CHECKSUM] = 0;
	ipPacket->data[IPHDR_CHECKSUM+1] = 0;

	if (kPGPNetworkByteOrder == kPGPBigEndian)
		bigEndianShift = 8;
	else
		littleEndianShift = 8;

	for (index=0; index<ipHeaderLength; index += sizeof(PGPUInt16))
	{
		total += (ipPacket->data[index] << bigEndianShift) | 
					(ipPacket->data[index+1] << littleEndianShift);
	}

	PGPUInt16ToEndian(checksum, kPGPNetworkByteOrder, 
		&(ipPacket->data[IPHDR_CHECKSUM]));

	checksum = ~((PGPUInt16) total + (PGPUInt16)(total >> 16));
	return checksum;
}


void pgpSetIPHeaderChecksum(PGPIPsecBuffer *ipPacket)
{
	PGPUInt16 checksum;

	checksum = pgpCalculateIPHeaderChecksum(ipPacket);
	
	PGPUInt16ToEndian(checksum, kPGPNetworkByteOrder, 
		&(ipPacket->data[IPHDR_CHECKSUM]));

	return;
}


void pgpGetSrcIPAddress(PGPIPsecBuffer *ipPacket, PGPUInt32 *ipAddress)
{
	pgpCopyMemory(&(ipPacket->data[IPHDR_SOURCEADDRESS]), ipAddress, 4);
	return;
}


void pgpGetDestIPAddress(PGPIPsecBuffer *ipPacket, PGPUInt32 *ipAddress)
{
	pgpCopyMemory(&(ipPacket->data[IPHDR_DESTADDRESS]), ipAddress, 4);
	return;
}


void pgpSetSrcIPAddress(PGPIPsecBuffer *ipPacket, PGPUInt32 ipAddress)
{
	pgpCopyMemory(&ipAddress, &(ipPacket->data[IPHDR_SOURCEADDRESS]), 4);
	return;
}


void pgpSetDestIPAddress(PGPIPsecBuffer *ipPacket, PGPUInt32 ipAddress)
{
	pgpCopyMemory(&ipAddress, &(ipPacket->data[IPHDR_DESTADDRESS]), 4);
	return;
}


void pgpClearDFBit(PGPIPsecBuffer *ipPacket)
{
	ipPacket->data[IPHDR_FLAGS_FRAGOFFSET] &= 0xBF;
	return;
}


PGPBoolean pgpIsFragmented(PGPByte *ipPacket)
{
	if (ipPacket[IPHDR_FLAGS_FRAGOFFSET] & 0x20)
		return TRUE;
	else 
	{
		if ((ipPacket[IPHDR_FLAGS_FRAGOFFSET] & 0x1F) ||
			(ipPacket[IPHDR_FLAGS_FRAGOFFSET+1]))
			return TRUE;
		else
			return FALSE;
	}
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
