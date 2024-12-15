#ifndef CRYPTOPP_ARC4_H
#define CRYPTOPP_ARC4_H

#include "cryptlib.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

class ARC4 : public RandomNumberGenerator,
            public StreamCipher
{
public:
    enum {KEYLENGTH=16};    // default key length

    ARC4(const byte *userKey, unsigned int keyLength=KEYLENGTH);
    ~ARC4();

    byte GetByte();

    byte ProcessByte(byte input);
    void ProcessString(byte *outString, const byte *inString, unsigned int length);
    void ProcessString(byte *inoutString, unsigned int length);

	static unsigned int KeyLength(unsigned int keylength)
		{return keylength < 1 ? 1 : (keylength <= 256 ? keylength : 256);}

private:
    SecByteBlock state;
    byte x;
    byte y;
};

NAMESPACE_END

#endif
