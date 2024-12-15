#include "misc.h"
#include "queue.h"
#include <memory.h>

// this class for use by ByteQueue only
class ByteQueueNode
{
public:
    ByteQueueNode();
    ~ByteQueueNode();

    enum {MAXSIZE=256};

    unsigned int CurrentSize() const
        {return tail-head;}
    unsigned int UsedUp() const
        {return (head==MAXSIZE);}

    unsigned int Put(byte inByte);
    unsigned int Put(const byte *inString, unsigned int length);

    unsigned int Get(byte &outByte);
    unsigned int Get(byte *outString, unsigned int getMax);

    ByteQueueNode *next;

private:
    byte *const buf;
    unsigned int head, tail;
};


ByteQueueNode::ByteQueueNode()
    : buf(SecAlloc(byte, MAXSIZE))
{
    head = tail = 0;
    next = 0;
}

ByteQueueNode::~ByteQueueNode()
{
    SecFree(buf, MAXSIZE);
}

unsigned int ByteQueueNode::Put(byte inByte)
{
    if (MAXSIZE==tail)
        return 0;

    buf[tail++]=inByte;
    return 1;
}

unsigned int ByteQueueNode::Put(const byte *inString, unsigned int length)
{
    unsigned int l = min(length, MAXSIZE-tail);
    memcpy(buf+tail, inString, l);
    tail += l;
    return l;
}

unsigned int ByteQueueNode::Get(byte &outByte)
{
    if (tail==head)
        return 0;

    outByte=buf[head++];
    return 1;
}

unsigned int ByteQueueNode::Get(byte *outString, unsigned int getMax)
{
    unsigned int l = min(getMax, tail-head);
    memcpy(outString, buf+head, l);
    head += l;
    return l;
}

ByteQueue::ByteQueue()
{
    head = tail = new ByteQueueNode;
}

ByteQueue::~ByteQueue()
{
    ByteQueueNode *next;

    for (ByteQueueNode *current=head; current; current=next)
    {
        next=current->next;
        delete current;
    }
}

unsigned long ByteQueue::CurrentSize() const
{
    unsigned long size=0;

    for (ByteQueueNode *current=head; current; current=current->next)
        size += current->CurrentSize();

    return size;
}

void ByteQueue::Put(byte inByte)
{
    if (!tail->Put(inByte))
    {
        tail->next = new ByteQueueNode;
        tail = tail->next;
        tail->Put(inByte);
    }
}

void ByteQueue::Put(const byte *inString, unsigned int length)
{
    unsigned int l;

    while ((l=tail->Put(inString, length)) < length)
    {
        tail->next = new ByteQueueNode;
        tail = tail->next;
        inString += l;
        length -= l;
    }
}

int ByteQueue::Get(byte &outByte)
{
    int l = head->Get(outByte);
    if (head->UsedUp())
    {
        ByteQueueNode *temp=head;
        head = head->next;
        delete temp;
        if (!head)  // just deleted the last node
            head = tail = new ByteQueueNode;
    }
    return l;
}

unsigned int ByteQueue::Get(byte *outString, unsigned int getMax)
{
    unsigned int getMaxSave=getMax;
    ByteQueueNode *current=head;

    while (getMax && current)
    {
        int l=current->Get(outString, getMax);

        outString += l;
        getMax -= l;

        current = current->next;
    }

    while (head && head->UsedUp())
    {
        current=head;
        head=head->next;
        delete current;
    }

    if (!head)  // every single node has been used up and deleted
        head = tail = new ByteQueueNode;

    return (getMaxSave-getMax);
}

