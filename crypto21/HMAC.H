// hmac.h - written and placed in the public domain by Wei Dai

#ifndef HMAC_H
#define HMAC_H

#include "cryptlib.h"
#include "misc.h"

template <class T> class HMAC : public MessageAuthenticationCode
{
public:
    HMAC(const byte *userKey)
        : key(KEYLENGTH)
    {
		memcpy(key, userKey, KEYLENGTH);
		Init();
    }

	void Init()
	{
        F.Update(key, KEYLENGTH);
		assert((DATASIZE-KEYLENGTH)%4 == 0);
		for (unsigned i=KEYLENGTH; i<DATASIZE; i+=4)
			F.Update(pad2, 4);
	}

    void Update(const byte *input, unsigned int length)
	{
		F.Update(input, length);
	}

    void Final(byte *mac)
	{
		F.Final(mac);
		F.Update(key, KEYLENGTH);
		assert((DATASIZE-KEYLENGTH)%4 == 0);
		for (unsigned i=KEYLENGTH; i<DATASIZE; i+=4)
			F.Update(pad1, 4);
		F.Update(mac, DIGESTSIZE);
		F.Final(mac);
		Init();
	}

    unsigned int DigestSize() const {return DIGESTSIZE;}

    enum {KEYLENGTH=T::DIGESTSIZE, DIGESTSIZE=T::DIGESTSIZE, DATASIZE=T::DATASIZE};

private:
	static const byte pad1[4], pad2[4];

    SecByteBlock key;
	T F;
};

template <class T> const byte HMAC<T>::pad1[4] = {0x36, 0x36, 0x36, 0x36};
template <class T> const byte HMAC<T>::pad2[4] = {0x5c, 0x5c, 0x5c, 0x5c};

#endif
