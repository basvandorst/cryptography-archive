// specification file for an unlimited queue for storing bytes

#ifndef QUEUE_H
#define QUEUE_H

#include "cryptlib.h"

// The queue is implemented as a linked list of arrays, but you don't need to
// know about that.  So just ignore this next line. :)
class ByteQueueNode;

class ByteQueue : public BufferedTransformation
{
public:
    ByteQueue();
    ~ByteQueue();

    // how many bytes currently stored
    unsigned long CurrentSize() const;
    unsigned long MaxRetrieveable()
        {return CurrentSize();}

    void Put(byte inByte);
    void Put(const byte *inString, unsigned int length);

    // both functions returns the number of bytes actually retrived
    int Get(byte &outByte);
    unsigned int Get(byte *outString, unsigned int getMax);

private:
    ByteQueueNode *head, *tail;
};

#endif

