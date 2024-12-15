#include "misc.h"
#include "secsplit.h"

void SplitFork::Put(byte inByte)
{
    SecByteBlock buf(numberOfPorts);

    rng.GetBlock(buf, numberOfPorts-1);
    buf[numberOfPorts-1] = inByte;

    for (int i=0; i<numberOfPorts; i++)
    {
        outPorts[i]->Put(buf[i]);
        buf[numberOfPorts-1] ^= buf[i];
    }
}

void SplitFork::Put(const byte *inString, unsigned int length)
{
    SecByteBlock randomBlock(length);
    SecByteBlock lastBlock(length);

    memcpy(lastBlock, inString, length);

    for (int i=0; i<numberOfPorts-1; i++)
    {
        rng.GetBlock(randomBlock, length);
        outPorts[i]->Put(randomBlock, length);
        xorbuf(lastBlock, randomBlock, length);
    }

    outPorts[numberOfPorts-1]->Put(lastBlock, length);
}

void SplitJoin::NotifyInput(int /* interfaceId */, unsigned int /* length */)
{
    unsigned long n=inPorts[0]->MaxRetrieveable();

    for (int i=1; n && i<numberOfPorts; i++)
        n = min(n, inPorts[i]->MaxRetrieveable());

    if (n)
    {
        const unsigned int l = (unsigned int) n;    // convert long to int
        SecByteBlock original(l);
        SecByteBlock buf(l);

        inPorts[numberOfPorts-1]->Get(original, l);
        for (int i=0; i<numberOfPorts-1; i++)
        {
            inPorts[i]->Get(buf, l);
            xorbuf(original, buf, l);
        }
        outQueue->Put(original, l);
    }
}

