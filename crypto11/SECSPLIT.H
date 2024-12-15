#ifndef SECSPLIT_H
#define SECSPLIT_H

#include "forkjoin.h"

class SplitFork : public Fork
{
public:
    SplitFork(RandomNumberGenerator &inRng, int n)
        : Fork(n), rng(inRng) {}
    SplitFork(RandomNumberGenerator &inRng, int n, BufferedTransformation *const *outports)
        : Fork(n, outports), rng(inRng) {}

    void Put(byte inByte);
    void Put(const byte *inString, unsigned int length);

private:
    RandomNumberGenerator &rng;
};

class SplitJoin : public Join
{
public:
    SplitJoin(int n, BufferedTransformation *outQ=new ByteQueue)
        : Join(n, outQ) {}

    void NotifyInput(int interfaceId, unsigned int length);
};

#endif

