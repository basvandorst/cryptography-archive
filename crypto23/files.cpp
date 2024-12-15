// files.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "files.h"

USING_NAMESPACE(std)

static const unsigned int BUFFER_SIZE = 1024;

FileSource::FileSource (std::istream &i, bool pumpAndClose, BufferedTransformation *outQueue)
	: Source(outQueue), in(i)
{
	if (pumpAndClose)
	{
		PumpAll();
		Close();
	}
}

FileSource::FileSource (const char *filename, bool pumpAndClose, BufferedTransformation *outQueue)
	: Source(outQueue), file(filename, ios::in | BINARY_MODE | FILE_NO_CREATE), in(file)
{
#ifdef THROW_EXCEPTIONS
	if (!file)
	{
		string message = "FileSource: error opening file for reading: ";
		message += filename;
		throw OpenErr(message.c_str());
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
	SecByteBlock buffer(STDMIN(size, BUFFER_SIZE));

	while (size && in.good())
	{
		in.read((char *)buffer.ptr, STDMIN(size, BUFFER_SIZE));
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
		string message = "FileSource: error opening file for writing: ";
		message += filename;
		throw OpenErr(message.c_str());
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
	out.write((const char *)inString, length);
#ifdef THROW_EXCEPTIONS
	if (!out.good())
	  throw WriteErr();
#endif
}

