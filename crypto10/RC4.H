#ifndef RC4_H
#define RC4_H

#include "cryptlib.h"
#include "misc.h"

class RC4 : public RandomNumberGenerator,
            public StreamCipher
{
public:
    enum {KEYLENGTH=16};    // default key length

    RC4(const byte *userKey, int keyLength=KEYLENGTH);
    ~RC4();

    inline byte GetByte()
    {
        x++;
        y += state[x];
        swap(state[x], state[y]);
        return (state[(state[x] + state[y]) & 255]);
    }

    inline byte ProcessByte(byte input)
        {return (input ^ RC4::GetByte());}

    void ProcessString(byte *outString, const byte *inString, unsigned int length);
    void ProcessString(byte *inoutString, unsigned int length);

private:
    byte *const state;
    byte x;
    byte y;
};

#endif

