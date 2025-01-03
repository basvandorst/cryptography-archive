/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: pgpIPsecAH.h,v 1.17 1999/03/20 04:52:09 dgal Exp $
____________________________________________________________________________*/
#ifndef Included_pgpIPsecAH_h	/* [ */
#define Included_pgpIPsecAH_h

#include "pgpPubTypes.h"
#include "pgpIPsecContext.h"
#include "pgpIPsecBuffer.h"

PGP_BEGIN_C_DECLARATIONS

/*

pgpIPsecAHGetSPI

Gets the SPI from the AH header of an incoming packet. You'll need this in 
conjuction with the IP address to match an SA. 

	inPacket	- The buffer containing the IP packet
	spi			- A pointer to the SPI returned

*/

PGPError pgpIPsecAHGetSPI(PGPIPsecBuffer *inPacket, PGPUInt32 *spi);

/*

pgpApplyIPsecAuthentication

Applies the AH protocol to an outgoing packet, which means that the packet
data, and header if in Tunnel Mode, are going to be authenticated.

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
	hash			- The hash algorithm to be used for authentication.
	authKey			- A buffer containing the HMAC key to be used in the hash 
					  algorithm.
	spi				- The SPI used in the SA for this packet. This value 
					  should come from IKE.
	sequenceNumber	- This value is used to prevent packet hijacking by 
					  labeling every packet with a sequence number. It's your 
					  responsibility to store the current number with the SA 
					  and increment it after sending each packet.
	ipPacketOut		- The modified IP packet.

*/

PGPError pgpApplyIPsecAuthentication(PGPIPsecContextRef ipsec,
			PGPUInt32 maxBufferSize, PGPIPsecBuffer *ipPacketIn, 
			PGPBoolean tunnelMode, PGPUInt32 gatewayIP, PGPHashAlgorithm hash,
			PGPByte *authKey, PGPUInt32 spi, PGPUInt32 sequenceNumber, 
			PGPIPsecBuffer *ipPacketOut);

/*

pgpVerifyIPsecAuthentication

Processes an incoming packet for the AH protocol, verifying the
authentication of the packet data, and header if in Tunnel Mode.

	ipsec			- The IPsec context.
	maxBufferSize	- The maximum size to allocate buffers in the buffer
					  list. Set this to 0 if you only want one big
					  output buffer.
	ipPacketIn		- The buffer containing the packet to be modified. 
	tunnelMode		- TRUE if the packet was sent in Tunnel Mode, FALSE if 
					  not.
	hash			- The hash algorithm to be used in verifying the
					  authentication.
	authKey			- A buffer containing the HMAC key to be used in the hash
					  algorithm.
	sequenceWindow	- A set of bits indicating which packet sequence numbers 
					  have been received. The MSB represents the sequence 
					  number stored in upperSequence, while the LSB represents
					  the sequence number stored in lowerSequence. This value 
					  should be stored with the SA and initialized to 0 when 
					  the SA is first established.
	upperSequence	- Pointer to the upper bound of the sequence window. This
					  should be stored with the SA and initialized to 
					  (sizeof(*sequenceWindow) << 3) - 1.
	lowerSequence	- Pointer to the lower bound of the sequence window. This
					  should be stored with the Sa and initialized to 0.
	ipPacketOut		- The modified IP packet.
*/

PGPError pgpVerifyIPsecAuthentication(PGPIPsecContextRef ipsec,
			PGPUInt32 maxBufferSize, PGPIPsecBuffer *ipPacketIn, 
			PGPBoolean tunnelMode, PGPHashAlgorithm hash, PGPByte *authKey, 
			PGPUInt32 *sequenceWindow, PGPUInt32 *upperSequence, 
			PGPUInt32 *lowerSequence, PGPIPsecBuffer *ipPacketOut);

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpIPsecAH_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
