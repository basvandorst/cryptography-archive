#ifndef FORKJOIN_H
#define FORKJOIN_H

#include "cryptlib.h"
#include "filters.h"

class Fork : public BufferedTransformation
{
public:
    Fork(int number_of_outports);
    Fork(int number_of_outports, BufferedTransformation *const *outports);
    virtual ~Fork();

    void SelectOutPort(int portNumber);

    boolean Attachable() {return TRUE;}
    void Detach(BufferedTransformation *newOut=new ByteQueue);
    void Attach(BufferedTransformation *newOut);
    virtual void Close();

    unsigned long MaxRetrieveable()
        {return outPorts[currentPort]->MaxRetrieveable();}
//    virtual void InputFinished()
//        {outPorts[currentPort]->InputFinished();}

    int Get(byte &outByte)
        {return outPorts[currentPort]->Get(outByte);}
    unsigned int Get(byte *outString, unsigned int getMax)
        {return outPorts[currentPort]->Get(outString, getMax);}

    virtual void Put(byte inByte);
    virtual void Put(const byte *inString, unsigned int length);

protected:
    const int numberOfPorts;
    BufferedTransformation * *const outPorts;

private:
    int currentPort;
};

class Interface;

class Join : public Filter
{
public:
    Join(int number_of_inports, BufferedTransformation *outQ=new ByteQueue);
    virtual ~Join();

    Interface * SelectInterface(int);

    virtual void NotifyInput(int interfaceId, unsigned int length);
    virtual void NotifyClose(int interfaceId);

    void Put(byte inByte) {outQueue->Put(inByte);}
    void Put(const byte *inString, unsigned int length)
        {outQueue->Put(inString, length);}

protected:
    const int numberOfPorts;
    ByteQueue * *const inPorts;

private:
    int interfacesOpen;
    Interface * *const interfaces;
};

class Interface : public BufferedTransformation
{
public:
    Interface(Join &p, ByteQueue &b, int i)
        : parent(p), bq(b), id(i) {}

    unsigned long MaxRetrieveable() {return parent.MaxRetrieveable();}

    void Close() {parent.NotifyClose(id);}

    boolean Attachable() {return TRUE;}
    void Detach(BufferedTransformation *bt) {parent.Detach(bt);}
    void Attach(BufferedTransformation *bt) {parent.Attach(bt);}

    void Put(byte inByte)
    {
        bq.Put(inByte);
        parent.NotifyInput(id, 1);
    }

    void Put(const byte *inString, unsigned int length)
    {
        bq.Put(inString, length);
        parent.NotifyInput(id, length);
    }

    int Get(byte &outByte) {return parent.Get(outByte);}
    unsigned int Get(byte *outString, unsigned int getMax)
        {return parent.Get(outString, getMax);}

private:
    Join &parent;
    ByteQueue &bq;
    const int id;
};

#endif

