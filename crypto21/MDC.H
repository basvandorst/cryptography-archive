 // mdc.h - written and placed in the public domain by Wei Dai

#ifndef MDC_H
#define MDC_H

#include "cryptlib.h"
#include "misc.h"

template <class T> class MDC : public BlockTransformation
{
public:
    enum {KEYLENGTH=T::DATASIZE, BLOCKSIZE=T::DIGESTSIZE};

    MDC(const byte *userKey)
        : key(KEYLENGTH/4)
    {
        T::CorrectEndianess(key, (word32 *)userKey, KEYLENGTH);
    }

    void ProcessBlock(byte *inoutBlock)
    {
        T::CorrectEndianess((word32 *)inoutBlock, (word32 *)inoutBlock, BLOCKSIZE);
        T::Transform((word32 *)inoutBlock, key);
        T::CorrectEndianess((word32 *)inoutBlock, (word32 *)inoutBlock, BLOCKSIZE);
    }

    void ProcessBlock(const byte *inBlock, byte *outBlock)
    {
        T::CorrectEndianess((word32 *)outBlock, (word32 *)inBlock, BLOCKSIZE);
        T::Transform((word32 *)outBlock, key);
        T::CorrectEndianess((word32 *)outBlock, (word32 *)outBlock, BLOCKSIZE);
    }

    unsigned int BlockSize() const {return BLOCKSIZE;}

private:
    SecBlock<word32> key;
};

#endif

