#ifndef LUBYRACK_H
#define LUBYRACK_H

#include "cryptlib.h"
#include "misc.h"

template <class T> class LRBase : public BlockTransformation
{
public:
    enum {KEYLENGTH=16};    // default key length
protected:
    LRBase(const byte *userKey, unsigned int keyLen)
        : S(2*hm.DigestSize()), L(keyLen),
          KL(SecAlloc(byte, L+S)), KR(KL+L/2),
          Lbuf(KL+L), Rbuf(KL+L+S/2)
    {
        memcpy(KL, userKey, L);
    }

    ~LRBase() {SecFree(KL, L+S);}

    unsigned int BlockSize() const {return S;}

    T hm;
    const unsigned int S;    // block size
    const unsigned int L;    // key length
    byte *const KL; // left half of key
    byte *const KR; // right half of key
    byte *const Lbuf;
    byte *const Rbuf;
};

template <class T> class LREncryption : public LRBase<T>
{
public:
    // keyLen must be even
    LREncryption(const byte *userKey, int keyLen=KEYLENGTH)
        : LRBase<T>(userKey, keyLen) {}

    void ProcessBlock(byte * inoutBlock)
        {LREncryption<T>::ProcessBlock(inoutBlock, inoutBlock);}

    void ProcessBlock(const byte *inBlock, byte * outBlock);
};

template <class T> class LRDecryption : public LRBase<T>
{
public:
    // keyLen must be even
    LRDecryption(const byte *userKey, int keyLen=KEYLENGTH)
        : LRBase<T>(userKey, keyLen) {}

    void ProcessBlock(byte * inoutBlock)
        {LRDecryption<T>::ProcessBlock(inoutBlock, inoutBlock);}

    void ProcessBlock(const byte *inBlock, byte * outBlock);
};

template <class T> void LREncryption<T>::ProcessBlock(const byte *inBlock, byte * outBlock)
{
    const byte *const IL=inBlock;
    const byte *const IR=inBlock+S/2;
    byte *const OL=outBlock;
    byte *const OR=outBlock+S/2;

    hm.Update(KL, L/2);
    hm.Update(IL, S/2);
    hm.Final(Rbuf);
    xorbuf(Rbuf, IR, S/2);

    hm.Update(KR, L/2);
    hm.Update(Rbuf, S/2);
    hm.Final(Lbuf);
    xorbuf(Lbuf, IL, S/2);

    hm.Update(KL, L/2);
    hm.Update(Lbuf, S/2);
    hm.Final(OR);
    xorbuf(OR, Rbuf, S/2);

    hm.Update(KR, L/2);
    hm.Update(OR, S/2);
    hm.Final(OL);
    xorbuf(OL, Lbuf, S/2);
}

template <class T> void LRDecryption<T>::ProcessBlock(const byte *inBlock, byte * outBlock)
{
    const byte *const IL=inBlock;
    const byte *const IR=inBlock+S/2;
    byte *const OL=outBlock;
    byte *const OR=outBlock+S/2;

    hm.Update(KR, L/2);
    hm.Update(IR, S/2);
    hm.Final(Lbuf);
    xorbuf(Lbuf, IL, S/2);

    hm.Update(KL, L/2);
    hm.Update(Lbuf, S/2);
    hm.Final(Rbuf);
    xorbuf(Rbuf, IR, S/2);

    hm.Update(KR, L/2);
    hm.Update(Rbuf, S/2);
    hm.Final(OL);
    xorbuf(OL, Lbuf, S/2);

    hm.Update(KL, L/2);
    hm.Update(OL, S/2);
    hm.Final(OR);
    xorbuf(OR, Rbuf, S/2);
}

#endif

