// files.cpp - written and placed in the public domain by Wei Dai

#include "files.h"
#include "misc.h"

const unsigned int FileSource::BUFFER_SIZE = 1024;

FileSource::FileSource (istream &i, boolean pumpAndClose, BufferedTransformation *outQueue)
    : Source(outQueue), in(i)
{
    if (pumpAndClose)
	{
		PumpAll();
        Close();
	}
}

FileSource::FileSource (const char *filename, boolean pumpAndClose, BufferedTransformation *outQueue)
    : Source(outQueue), file(filename, ios::in | BINARY_MODE | FILE_NO_CREATE), in(file)
{
#ifdef THROW_EXCEPTIONS
    if (!file)
	{
		SecBlock<char> message(strlen(filename)+50);
		strcpy(message, "FileSource: error opening file for reading: ");
		strcat(message, filename);
        throw OpenErr(message);
	}
#endif
    if (pumpAndClose)
	{
		PumpAll();
        Close();
	}
}

unsigned int FileSource::Pump(unsigned int size)
{
    unsigned int total=0;
	SecByteBlock buffer(min(size, BUFFER_SIZE));

	while (size && in.good())
	{
		in.read((byte *)buffer, min(size, BUFFER_SIZE));
		unsigned l = in.gcount();
		outQueue->Put(buffer, l);
		size -= l;
		total += l;
	}

#ifdef THROW_EXCEPTIONS
    if (!in.good() && !in.eof())
        throw ReadErr();
#endif

    return total;
}

unsigned long FileSource::PumpAll()
{
    unsigned long total=0;
    unsigned int l;

    while ((l=Pump(BUFFER_SIZE)) != 0)
        total += l;

    return total;
}

FileSink::FileSink(ostream &o)
    : out(o)
{
}

FileSink::FileSink(const char *filename)
    : file(filename, ios::out | BINARY_MODE | ios::trunc), out(file)
{
#ifdef THROW_EXCEPTIONS
    if (!file)
	{
		SecBlock<char> message(strlen(filename)+50);
		strcpy(message, "FileSink: error opening file for writing: ");
		strcat(message, filename);
        throw OpenErr(message);
	}
#endif
}

void FileSink::InputFinished()
{
    out.flush();
#ifdef THROW_EXCEPTIONS
    if (!out.good())
      throw WriteErr();
#endif
}

void FileSink::Put(const byte *inString, unsigned int length)
{
    out.write(inString, length);
#ifdef THROW_EXCEPTIONS
    if (!out.good())
      throw WriteErr();
#endif
}

