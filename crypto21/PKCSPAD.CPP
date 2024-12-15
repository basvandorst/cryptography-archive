// pkcspad.cpp - written and placed in the public domain by Wei Dai

#include "misc.h"
#include "asn.h"

#include "pkcspad.h"

void PKCS_EncryptionPaddingScheme::Pad(RandomNumberGenerator &rng, const byte *input, unsigned int inputLen, byte *pkcsBlock, unsigned int pkcsBlockLen) const
{
    assert (inputLen <= MaxInputLength(pkcsBlockLen));

    pkcsBlock[0] = 0;
    pkcsBlock[1] = 2;  // block type 2

    // pad with non-zero random bytes
    for (unsigned i = 2; i < pkcsBlockLen-inputLen-1; i++)
		pkcsBlock[i] = (byte)rng.GetShort(1, 0xff);

    pkcsBlock[pkcsBlockLen-inputLen-1] = 0;     // separator
    memcpy(pkcsBlock+pkcsBlockLen-inputLen, input, inputLen);
}

unsigned int PKCS_EncryptionPaddingScheme::Unpad(const byte *pkcsBlock, unsigned int pkcsBlockLen, byte *output) const
{
    // Require block type 2.
    if ((pkcsBlock[0] != 0) || (pkcsBlock[1] != 2))
        return 0;

    // skip past the padding until we find the seperator
    unsigned i=2;
    while (i<pkcsBlockLen && pkcsBlock[i++]) { // null body
		}
    assert(i==pkcsBlockLen || pkcsBlock[i-1]==0);

    unsigned int outputLen = pkcsBlockLen - i;
    if (outputLen > MaxInputLength(pkcsBlockLen))
        return 0;

    memcpy (output, pkcsBlock+i, outputLen);
    return outputLen;
}

// ********************************************************

void PKCS_SignaturePaddingScheme::Pad(RandomNumberGenerator &, const byte *input, unsigned int inputLen, byte *pkcsBlock, unsigned int pkcsBlockLen) const
{
    assert (inputLen <= MaxInputLength(pkcsBlockLen));

    pkcsBlock[0] = 0;
    pkcsBlock[1] = 1;   // block type 1

	// padd with 0xff
    memset(pkcsBlock+2, 0xff, pkcsBlockLen-inputLen-3);

    pkcsBlock[pkcsBlockLen-inputLen-1] = 0;               // separator
    memcpy(pkcsBlock+pkcsBlockLen-inputLen, input, inputLen);
}

unsigned int PKCS_SignaturePaddingScheme::Unpad(const byte *pkcsBlock, unsigned int pkcsBlockLen, byte *output) const
{
    // Require block type 1.
    if ((pkcsBlock[0] != 0) || (pkcsBlock[1] != 1))
        return 0;

    // skip past the padding until we find the seperator
    unsigned i=2;
    while (i<pkcsBlockLen && pkcsBlock[i++])
        if (pkcsBlock[i-1] != 0xff)     // not valid padding
            return 0;
    assert(i==pkcsBlockLen || pkcsBlock[i-1]==0);

    unsigned int outputLen = pkcsBlockLen - i;
    if (outputLen > MaxInputLength(pkcsBlockLen))
        return 0;

    memcpy (output, pkcsBlock+i, outputLen);
    return outputLen;
}
