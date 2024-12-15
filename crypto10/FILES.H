#ifndef FILES_H
#define FILES_H

#include <fstream.h>

#include "cryptlib.h"
#include "filters.h"

class FileErr {};
class OpenErr : public FileErr {};
class ReadErr : public FileErr {};
class WriteErr : public FileErr {};

class FileSource : public Source
{
public:
    FileSource(istream &in=cin, boolean pumpAndClose=FALSE,
               BufferedTransformation *outQueue = new ByteQueue);
    FileSource(const char *filename, boolean pumpAndClose=FALSE,
               BufferedTransformation *outQueue = new ByteQueue);

    istream& GetStream() {return in;}

    unsigned int Pump(unsigned int size);
    unsigned long PumpAll();

private:
    ifstream file;
    istream& in;
};

class FileSink : public Sink
{
public:
    FileSink(ostream &out=cout);
    FileSink(const char *filename);

    ostream& GetStream() {return out;}

    void InputFinished();
    void Put(byte inByte)
    {
        out.put(inByte);
#ifdef THROW_EXCEPTIONS
        if (out.bad())
          throw WriteErr();
#endif
    }

    void Put(const byte *inString, unsigned int length);

private:
    ofstream file;
    ostream& out;
};

#endif

