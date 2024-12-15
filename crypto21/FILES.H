#ifndef FILES_H
#define FILES_H

#include <iostream.h>
#include <fstream.h>

#include "cryptlib.h"
#include "filters.h"
#include "misc.h"

class FileSource : public Source
{
public:
	class Err : public CryptlibException {public: Err(const char *message) : CryptlibException(message) {}};
	class OpenErr : public Err {public: OpenErr(const char *message) : Err(message) {}};
	class ReadErr : public Err {public: ReadErr() : Err("FileSource: error reading file") {}};

    FileSource(istream &in=cin, boolean pumpAndClose=FALSE,
               BufferedTransformation *outQueue = NULL);
    FileSource(const char *filename, boolean pumpAndClose=FALSE,
               BufferedTransformation *outQueue = NULL);

    istream& GetStream() {return in;}

    unsigned int Pump(unsigned int size);
    unsigned long PumpAll();

private:
	static const unsigned int BUFFER_SIZE;

    ifstream file;
    istream& in;
};

class FileSink : public Sink
{
public:
	class Err : public CryptlibException {public: Err(const char *message) : CryptlibException(message) {}};
	class OpenErr : public Err {public: OpenErr(const char *message) : Err(message) {}};
	class WriteErr : public Err {public: WriteErr() : Err("FileSink: error writing file") {}};

    FileSink(ostream &out=cout);
    FileSink(const char *filename);

    ostream& GetStream() {return out;}

    void InputFinished();
    void Put(byte inByte)
    {
        out.put(inByte);
#ifdef THROW_EXCEPTIONS
        if (!out.good())
          throw WriteErr();
#endif
    }

    void Put(const byte *inString, unsigned int length);

private:
    ofstream file;
    ostream& out;
};

#endif
