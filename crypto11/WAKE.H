#ifndef WAKE_H
#define WAKE_H

#include "cryptlib.h"
#include "misc.h"
#include "filters.h"

class WAKE
{
protected:
    inline word32 M(word32 x, word32 y);
    inline void enc(word32 &V);
    inline void dec(word32 &V);
    void genkey(word32 k0, word32 k1, word32 k2, word32 k3);

    word32 t[257];
    word32 r3, r4, r5, r6;
};

class WAKEEncryption : public Filter, protected WAKE
{
public:
    WAKEEncryption(const byte *key, BufferedTransformation *outQueue = new ByteQueue);

    void Put(byte inByte)
    {
        if (inbufSize==INBUFMAX)
            ProcessInbuf();
        inbuf[inbufSize++] = inByte;
    }

    void Put(const byte *inString, unsigned int length);
    void InputFinished();

protected:
    virtual void ProcessInbuf();
    enum {INBUFMAX=256};
    SecByteBlock inbuf;
    unsigned int inbufSize;
};

class WAKEDecryption : public WAKEEncryption
{
public:
    WAKEDecryption(const byte *key, BufferedTransformation *outQueue = new ByteQueue)
        : WAKEEncryption(key, outQueue) {lastBlock=FALSE;}

    void InputFinished();

protected:
    virtual void ProcessInbuf();
    boolean lastBlock;
};

#endif

