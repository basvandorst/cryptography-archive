#ifndef FILTERS_H
#define FILTERS_H

#include "cryptlib.h"
#include "queue.h"

class Filter : public BufferedTransformation
{
public:
    Filter(BufferedTransformation *outQ)
        : outQueue(outQ) {}
    virtual ~Filter()
        {delete outQueue;}

    boolean Attachable() {return TRUE;}
    void Detach(BufferedTransformation *newOut=new ByteQueue);
    void Attach(BufferedTransformation *newOut);
    void Close()
        {InputFinished(); outQueue->Close();}

    unsigned long MaxRetrieveable()
        {return outQueue->MaxRetrieveable();}

    int Get(byte &outByte)
        {return outQueue->Get(outByte);}
    unsigned int Get(byte *outString, unsigned int getMax)
        {return outQueue->Get(outString, getMax);}

protected:
    BufferedTransformation *outQueue;
};

class BlockFilterBase : public Filter
{
public:
    BlockFilterBase(BlockTransformation &cipher,
                    BufferedTransformation *outQueue);
    virtual ~BlockFilterBase();

    void Put(byte inByte)
    {
        if (inBufSize == S)
            ProcessBuf();
        inBuf[inBufSize++]=inByte;
    }

    void Put(const byte *inString, unsigned int length);

protected:
    void ProcessBuf();

    BlockTransformation &cipher;
    const unsigned int S;
    byte *const inBuf;
    unsigned int inBufSize;
};

class BlockEncryptionFilter : public BlockFilterBase
{
public:
    BlockEncryptionFilter(BlockTransformation &cipher,
                          BufferedTransformation *outQueue = new ByteQueue)
        : BlockFilterBase(cipher, outQueue) {}

protected:
    void InputFinished();
};

class BlockDecryptionFilter : public BlockFilterBase
{
public:
    BlockDecryptionFilter(BlockTransformation &cipher,
                          BufferedTransformation *outQueue = new ByteQueue)
        : BlockFilterBase(cipher, outQueue) {}

protected:
    void InputFinished();
};

class StreamCipherFilter : public Filter
{
public:
    StreamCipherFilter(StreamCipher &c,
                       BufferedTransformation *outQueue = new ByteQueue)
        : cipher(c), Filter(outQueue) {}

    void Put(byte inByte)
        {outQueue->Put(cipher.ProcessByte(inByte));}

    void Put(const byte *inString, unsigned int length);

private:
    StreamCipher &cipher;
};

class HashFilter : public Filter
{
public:
    HashFilter(HashModule &hm, BufferedTransformation *outQueue = new ByteQueue)
        : hash(hm), Filter(outQueue) {}

    void InputFinished();

    void Put(byte inByte)
        {hash.Update(&inByte, 1);}

    void Put(const byte *inString, unsigned int length)
        {hash.Update(inString, length);}

private:
    HashModule &hash;
};

class Source : public Filter
{
public:
    Source(BufferedTransformation *outQ)
        : Filter(outQ) {}

    void Put(byte inByte)
        {Pump(1);}
    void Put(const byte *inString, unsigned int length)
        {Pump(length);}
    void InputFinished()
        {PumpAll();}

    virtual unsigned int Pump(unsigned int size) =0;
    virtual unsigned long PumpAll() =0;
};

class Sink : public BufferedTransformation
{
public:
    unsigned long MaxRetrieveable()
        {return 0;}
    int Get(byte &outByte)
        {return 0;}
    unsigned int Get(byte *outString, unsigned int getMax)
        {return 0;}
};

class BitBucket : public Sink
{
public:
    void Put(byte inByte) {}
    void Put(const byte *inString, unsigned int length) {}
};

BufferedTransformation *Insert(const byte *in, unsigned int length, BufferedTransformation *outQueue);
unsigned int Extract(Source *source, byte *out, unsigned int length);

#endif

