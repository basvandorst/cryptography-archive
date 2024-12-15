#include "forkjoin.h"

Fork::Fork(int n)
    : numberOfPorts(n), outPorts(new BufferedTransformation* [n])
{
    currentPort = 0;

    for (int i=0; i<numberOfPorts; i++)
        outPorts[i] = new ByteQueue;
}

Fork::Fork(int n, BufferedTransformation *const *givenOutPorts)
    : numberOfPorts(n), outPorts(new BufferedTransformation* [n])
{
    currentPort = 0;

    for (int i=0; i<numberOfPorts; i++)
        outPorts[i] = givenOutPorts[i];
}

Fork::~Fork()
{
    for (int i=0; i<numberOfPorts; i++)
        delete outPorts[i];

    delete [] outPorts;
}

void Fork::SelectOutPort(int portNumber)
{
    currentPort=portNumber;
}

void Fork::Detach(BufferedTransformation *newOut)
{
    outPorts[currentPort]->Close();
    outPorts[currentPort]->TransferTo(*newOut);
    delete outPorts[currentPort];
    outPorts[currentPort]=newOut;
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

Join::Join(int n, BufferedTransformation *outQ)
    : Filter(outQ),
      numberOfPorts(n),
      inPorts(new ByteQueue* [n]),
      interfacesOpen(n),
      interfaces(new Interface* [n])
{
    for (int i=0; i<numberOfPorts; i++)
    {
        inPorts[i] = new ByteQueue;
        interfaces[i] = new Interface(*this, *inPorts[i], i);
    }
}

Join::~Join()
{
    for (int i=0; i<numberOfPorts; i++)
        delete inPorts[i];

    delete [] inPorts;
    delete [] interfaces;
}

Interface * Join::SelectInterface(int i)
{
    return interfaces[i];
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

