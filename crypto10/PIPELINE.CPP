#include "pipeline.h"

Pipeline::Pipeline(BufferedTransformation *a, BufferedTransformation *b,
                   BufferedTransformation *c, BufferedTransformation *d,
                   BufferedTransformation *e, BufferedTransformation *f,
                   BufferedTransformation *g, BufferedTransformation *h,
                   BufferedTransformation *i, BufferedTransformation *j)
{
    filterList[0]=a;
    filterList[1]=b;
    filterList[2]=c;
    filterList[3]=d;
    filterList[4]=e;
    filterList[5]=f;
    filterList[6]=g;
    filterList[7]=h;
    filterList[8]=i;
    filterList[9]=j;

    for (filterNumber=0; filterNumber<PIPELINE_MAX_FILTERS
         && filterList[filterNumber]; filterNumber++);
}

Pipeline::~Pipeline()
{
    for (int i=0;i<filterNumber;i++)
        delete filterList[i];
}

unsigned long Pipeline::MaxRetrieveable()
{
    return filterList[filterNumber-1]->MaxRetrieveable();
}

void Pipeline::Close()
{
    filterList[0]->Close();

    for (int i=0; i < filterNumber-1; i++)
    {
        filterList[i]->TransferTo(*filterList[i+1]);
        filterList[i+1]->Close();
    }
}

void Pipeline::Put(const byte *inString, unsigned int length)
{
    filterList[0]->Put(inString, length);

    for (int i=0; i < filterNumber-1; i++)
        filterList[i]->TransferTo(*filterList[i+1]);
}

void Pipeline::Put(byte inByte)
{
    Put(&inByte, 1);
}

int Pipeline::Get(byte &outByte)
{
    return filterList[filterNumber-1]->Get(outByte);
}

unsigned int Pipeline::Get(byte *outString, unsigned int getMax)
{
    return filterList[filterNumber-1]->Get(outString, getMax);
}

