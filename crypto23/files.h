#ifndef FILES_H
#define FILES_H

#include "cryptlib.h"
#include "filters.h"

#include "iostream"
#include "fstream"

class FileSource : public Source
{
public:
	class Err : public CryptlibException {public: Err(const char *message) : CryptlibException(message) {}};
	class OpenErr : public Err {public: OpenErr(const char *message) : Err(message) {}};
	class ReadErr : public Err {public: ReadErr() : Err("FileSource: error reading file") {}};

	FileSource(std::istream &in, bool pumpAndClose=false,
			   BufferedTransformation *outQueue = NULL);
	FileSource(const char *filename, bool pumpAndClose=false,
			   BufferedTransformation *outQueue = NULL);

	std::istream& GetStream() {return in;}

	unsigned int Pump(unsigned int size);
	unsigned long PumpAll();

private:
	std::ifstream file;
	std::istream& in;
};

class FileSink : public Sink
{
public:
	class Err : public CryptlibException {public: Err(const char *message) : CryptlibException(message) {}};
	class OpenErr : public Err {public: OpenErr(const char *message) : Err(message) {}};
	class WriteErr : public Err {public: WriteErr() : Err("FileSink: error writing file") {}};

	FileSink(std::ostream &out);
	FileSink(const char *filename);

	std::ostream& GetStream() {return out;}

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
	std::ofstream file;
	std::ostream& out;
};

#endif
