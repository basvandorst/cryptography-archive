#ifndef PIPELINE_H
#define PIPELINE_H

#include "cryptlib.h"

// if you change this next constant, be sure to change the constructor as well.
const int PIPELINE_MAX_FILTERS=10;

class Pipeline : public BufferedTransformation
{
public:
    Pipeline(BufferedTransformation * , BufferedTransformation * =0,
             BufferedTransformation * =0, BufferedTransformation * =0,
             BufferedTransformation * =0, BufferedTransformation * =0,
             BufferedTransformation * =0, BufferedTransformation * =0,
             BufferedTransformation * =0, BufferedTransformation * =0);

    virtual ~Pipeline();

    unsigned long MaxRetrieveable();
    void Close();

    void Put(byte inByte);
    void Put(const byte *inString, unsigned int length);

    int Get(byte &outByte);
    unsigned int Get(byte *outString, unsigned int getMax);

private:
    BufferedTransformation* filterList[PIPELINE_MAX_FILTERS];
    int filterNumber;
};

#endif

