/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: pgpIPsecESP.h,v 1.20 1999/05/22 04:16:14 dgal Exp $
____________________________________________________________________________*/
#ifndef Included_pgpIPsecESP_h	/* [ */
#define Included_pgpIPsecESP_h

#include "pgpPubTypes.h"
#include "pgpIPsecContext.h"
#include "pgpIPsecBuffer.h"

PGP_BEGIN_C_DECLARATIONS

/*

pgpIPsecESPGetSPI

Gets the SPI from the ESP header of an incoming packet. You'll need this in 
conjuction with the IP address to match an SA. 

	inPacket	- The buffer containing the IP packet
	spi			- A pointer to the SPI returned

*/

PGPError pgpIPsecESPGetSPI(PGPIPsecBuffer *inPacket, PGPUInt32 *spi);

/*

pgpIPsecEncrypt

Applies the ESP protocol to an outgoing packet. This not only encrypts the
packet data but can also provide authentication, thereby eliminating the
need for using AH in many cases. If in Tunnel Mode, the packet header is
encrypted as well.

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
	cipher			- The symmetric cipher algorithm to be used for 
					  encryption.
	cryptKey		- A buffer containing the cipher key to encrypt with.
	iv				- The IV (initialization vector) used when encrypting,
					  since CBC encryption is being used. Contains last 8 
					  bytes of encrypted data upon return from function.
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

PGPError pgpIPsecEncrypt(PGPIPsecContextRef ipsec, PGPUInt32 maxBufferSize,
			PGPIPsecBuffer *ipPacketIn, PGPBoolean tunnelMode, 
			PGPUInt32 gatewayIP, PGPCipherAlgorithm cipher, 
			const PGPByte *cryptKey, PGPByte *iv, PGPHashAlgorithm hash,
			PGPByte *authKey, PGPUInt32 spi, PGPUInt32 sequenceNumber, 
			PGPIPsecBuffer *ipPacketOut);

/*

pgpIPsecDecrypt

Processes an incoming packet for the ESP protocol, decrypting the packet
data. If in Tunnel Mode, the original packet header is decrypted as well
and replaces the current header. If the authentication option is used,
the data is also authenticated.

	ipsec			- The IPsec context.
	maxBufferSize	- The maximum size to allocate buffers in the buffer
					  list. Set this to 0 if you only want one big
					  output buffer.
	ipPacketIn		- The buffer containing the packet to be modified. 
	tunnelMode		- TRUE if the packet was sent in Tunnel Mode, FALSE if 
					  not.
	cipher			- The symmetric cipher algorithm to be used for 
					  decryption.
	cryptKey		- A buffer containing the cipher key to decrypt with.
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

PGPError pgpIPsecDecrypt(PGPIPsecContextRef ipsec, PGPUInt32 maxBufferSize,
			PGPIPsecBuffer *ipPacketIn, PGPBoolean tunnelMode, 
			PGPCipherAlgorithm cipher, const PGPByte *cryptKey, 
			PGPHashAlgorithm hash, PGPByte *authKey, 
			PGPUInt32 *sequenceWindow, PGPUInt32 *upperSequence, 
			PGPUInt32 *lowerSequence, PGPIPsecBuffer *ipPacketOut);

PGP_END_C_DECLARATIONS

#endif /* ] Included_pgpIPsecESP_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
