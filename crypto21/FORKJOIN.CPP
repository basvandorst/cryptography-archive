// forkjoin.cpp - written and placed in the public domain by Wei Dai

#include "forkjoin.h"
#include "queue.h"

Fork::Fork(int n)
    : numberOfPorts(n), outPorts(n)
{
    currentPort = 0;

    for (int i=0; i<numberOfPorts; i++)
        outPorts[i].reset(new ByteQueue);
}

Fork::Fork(int n, BufferedTransformation *const *givenOutPorts)
    : numberOfPorts(n), outPorts(n)
{
    currentPort = 0;

    for (int i=0; i<numberOfPorts; i++)
        outPorts[i].reset(givenOutPorts[i]);
}

void Fork::SelectOutPort(int portNumber)
{
    currentPort = portNumber;
}

void Fork::Detach(BufferedTransformation *newOut)
{
    outPorts[currentPort]->Close();
    outPorts[currentPort]->TransferTo(*newOut);
    outPorts[currentPort].reset(newOut ? newOut : new ByteQueue);
}

void Fork::Attach(BufferedTransformation *newOut)
{
    if (outPorts[currentPort]->Attachable())
        outPorts[currentPort]->Attach(newOut);
    else
        Detach(newOut);
}

void Fork::Close()
{
    InputFinished();

    for (int i=0; i<numberOfPorts; i++)
        outPorts[i]->Close();
}

void Fork::Put(byte inByte)
{
    for (int i=0; i<numberOfPorts; i++)
        outPorts[i]->Put(inByte);
}

void Fork::Put(const byte *inString, unsigned int length)
{
    for (int i=0; i<numberOfPorts; i++)
        outPorts[i]->Put(inString, length);
}

// ********************************************************

Join::Join(int n, BufferedTransformation *outQ)
    : Filter(outQ),
      numberOfPorts(n),
      inPorts(n),
      interfacesOpen(n),
      interfaces(n)
{
    for (int i=0; i<numberOfPorts; i++)
    {
        inPorts[i].reset(new ByteQueue);
        interfaces[i].reset(new Interface(*this, *inPorts[i], i));
    }
}

Interface * Join::ReleaseInterface(int i)
{
    return interfaces[i].release();
}

void Join::NotifyInput(int interfaceId, unsigned int /* length */)
{
    interfaces[interfaceId]->TransferTo(*outQueue);
}

void Join::NotifyClose(int /* id */)
{
    if ((--interfacesOpen) == 0)
        outQueue->Close();
}

// ********************************************************

void Interface::Put(byte inByte)
{
    bq.Put(inByte);
    parent.NotifyInput(id, 1);
}

void Interface::Put(const byte *inString, unsigned int length)
{
    bq.Put(inString, length);
    parent.NotifyInput(id, length);
}

unsigned long Interface::MaxRetrieveable() 
{
	return parent.MaxRetrieveable();
}

void Interface::Close() 
{
	parent.NotifyClose(id);
}

void Interface::Detach(BufferedTransformation *bt) 
{
	parent.Detach(bt);
}

void Interface::Attach(BufferedTransformation *bt) 
{
	parent.Attach(bt);
}

unsigned int Interface::Get(byte &outByte) 
{
	return parent.Get(outByte);
}

unsigned int Interface::Get(byte *outString, unsigned int getMax)
{
	return parent.Get(outString, getMax);
}

unsigned int Interface::Peek(byte &outByte) const
{
	return parent.Peek(outByte);
}
