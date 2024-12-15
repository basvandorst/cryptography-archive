/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: pgpIPsecComp.h,v 1.7 1999/05/24 20:21:56 dgal Exp $
____________________________________________________________________________*/
#ifndef Included_pgpIPsecComp_h	/* [ */
#define Included_pgpIPsecComp_h

#include "pgpCompress.h"
#include "pgpIPsecContext.h"
#include "pgpIPsecBuffer.h"



PGP_BEGIN_C_DECLARATIONS

/*

pgpIPsecCompress

Compresses the outgoing packet data using the IPCOMP protocol. If the 
resulting packet is larger than the original, ipPacketOut will be copied with
the exact contents from ipPacketIn, so that the receiving machine
won't have to process IPCOMP in this case.

    ipsec			- The IPsec context.
	maxBufferSize	- The maximum size to allocate buffers in the buffer
					  list. Set this to 0 if you only want one big
					  output buffer.
	ipPacketIn		- The buffer containing the packet to be modified.
	tunnelMode		- TRUE if the packet should be sent in Tunnel Mode, FALSE 
					  if not.
	gatewayIP		- The network byte-order IP address of the gateway IF 
					  you're sending in tunnel mode. If not in tunnel mode, 
					  this parameter is ignored.
	alg				- The compression algorithm to use.
	ipPacketOut		- The modified IP packet.
	usedIPCOMP		- TRUE if IPCOMP was actually used, FALSE otherwise.

*/

PGPError pgpIPsecCompress(PGPIPsecContextRef ipsec, PGPUInt32 maxBufferSize,
			PGPIPsecBuffer *ipPacketIn, PGPBoolean tunnelMode, 
			PGPUInt32 gatewayIP, PGPCompressionAlgorithm alg, 
			PGPIPsecBuffer *ipPacketOut, PGPBoolean *usedIPCOMP);

/*

pgpIPsecDecompress

Decompresses the incoming packet data using the IPCOMP protocol.

    ipsec			- The IPsec context.
	maxBufferSize	- The maximum size to allocate buffers in the buffer
					  list. Set this to 0 if you only want one big
					  output buffer.
	tunnelMode		- TRUE if the packet was sent in Tunnel Mode, FALSE if 
					  not.
	ipPacketIn		- The buffer containing the packet to be modified.
	alg				- The decompression algorithm to use.
	ipPacketOut		- The modified IP packet.

*/

PGPError pgpIPsecDecompress(PGPIPsecContextRef ipsec, PGPUInt32 maxBufferSize, 
			PGPIPsecBuffer *ipPacketIn, PGPBoolean tunnelMode,
			PGPCompressionAlgorithm alg, PGPIPsecBuffer *ipPacketOut);

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpIPsecComp_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
