#ifndef RANDPOOL_H
#define RANDPOOL_H

#include "cryptlib.h"
#include "misc.h"

class RandomPool : public RandomNumberGenerator,
                   public BufferedTransformation
{
public:
    // poolSize must be greater than 16
    RandomPool(unsigned int poolSize=384);

    // interface for BufferedTransformation
    void Put(byte inByte);
    void Put(const byte *inString, unsigned int length);
    int Get(byte &outByte);
    unsigned int Get(byte *outString, unsigned int getMax);
    // return 0 to prevent infinite loops
    unsigned long MaxRetrieveable() {return 0;}

    // interface for RandomNumberGenerator
    byte GetByte()
        {byte b; RandomPool::Get(b); return b;}
    void GetBlock(byte *output, unsigned int size)
        {Get(output, size);}

protected:
    void Stir();

private:
    SecByteBlock pool, key;
    unsigned int addPos, getPos;
};

#endif

