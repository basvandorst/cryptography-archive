#ifndef FORKJOIN_H
#define FORKJOIN_H

#include "cryptlib.h"
#include "filters.h"
#include "queue.h"

class Fork : public BufferedTransformation
{
public:
    Fork(int number_of_outports);
    Fork(int number_of_outports, BufferedTransformation *const *outports);

    void SelectOutPort(int portNumber);

    boolean Attachable() {return TRUE;}
    void Detach(BufferedTransformation *newOut = NULL);
    void Attach(BufferedTransformation *newOut);
    virtual void Close();

    unsigned long MaxRetrieveable()
        {return outPorts[currentPort]->MaxRetrieveable();}
//    virtual void InputFinished()
//        {outPorts[currentPort]->InputFinished();}

    unsigned int Get(byte &outByte)
        {return outPorts[currentPort]->Get(outByte);}
    unsigned int Get(byte *outString, unsigned int getMax)
        {return outPorts[currentPort]->Get(outString, getMax);}
    unsigned int Peek(byte &outByte) const
        {return outPorts[currentPort]->Peek(outByte);}

    virtual void Put(byte inByte);
    virtual void Put(const byte *inString, unsigned int length);

protected:
    const int numberOfPorts;
    vector_auto_ptrs<BufferedTransformation> outPorts;

private:
	Fork(const Fork &);
    int currentPort;
};

class Join;

class Interface : public BufferedTransformation
{
public:
    Interface(Join &p, ByteQueue &b, int i)
        : parent(p), bq(b), id(i) {}

    unsigned long MaxRetrieveable();
    void Close();
    boolean Attachable() {return TRUE;}
    void Detach(BufferedTransformation *bt);
    void Attach(BufferedTransformation *bt);

    void Put(byte inByte);
    void Put(const byte *inString, unsigned int length);
    unsigned int Get(byte &outByte);
    unsigned int Get(byte *outString, unsigned int getMax);
    unsigned int Peek(byte &outByte) const;

private:
    Join &parent;
    ByteQueue &bq;
    const int id;
};

class Join : public Filter
{
public:
    Join(int number_of_inports, BufferedTransformation *outQ = NULL);

	// Note that ReleaseInterface is similar but not completely compatible 
	// with SelectInterface of version 2.0.  ReleaseInterface can be called
	// only once for each interface, and if an interface is released,
	// the caller will be responsible for deleting it.
    Interface *ReleaseInterface(int i);

    virtual void NotifyInput(int interfaceId, unsigned int length);
    virtual void NotifyClose(int interfaceId);

    void Put(byte inByte) {outQueue->Put(inByte);}
    void Put(const byte *inString, unsigned int length)
        {outQueue->Put(inString, length);}

protected:
    const int numberOfPorts;
    vector_auto_ptrs<ByteQueue> inPorts;

private:
	Join(const Join &);
    int interfacesOpen;
	vector_auto_ptrs<Interface> interfaces;
};

#endif
