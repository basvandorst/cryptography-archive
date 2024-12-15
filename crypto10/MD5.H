#ifndef MD5_H
#define MD5_H

#include "cryptlib.h"
#include "misc.h"

class MD5 : public HashModule
{
public:
    MD5();
    void Update(const byte *input, unsigned int length);
    void Final(byte *hash);
    int DigestSize() const {return DIGESTSIZE;}

    static void CorrectEndianess(word32 *out, const word32 *in, unsigned int byteCount)
    {
#ifndef LITTLE_ENDIAN
        byteReverse(out, in, byteCount);
#else
        if (in!=out)
            memcpy(out, in, byteCount);
#endif
    }

    static void Transform (word32 *buf, const word32 *in);

    enum {DIGESTSIZE = 16, DATASIZE = 64};

private:
    void Init();
    inline void do_block (const word32 *input);

    SecBlock<word32> buffer;
    word32 *const digest;    // Message digest
    word32 *const data;      // SHS data buffer
    word32 countLo, countHi; // 64-bit bit count
};

#endif

