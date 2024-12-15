#include "files.h"
#include "misc.h"

FileSource::FileSource (istream &i, boolean pumpAndClose, BufferedTransformation *outQueue)
    : Source(outQueue), in(i)
{
    if (pumpAndClose)
        Close();
}

FileSource::FileSource (const char *filename, boolean pumpAndClose, BufferedTransformation *outQueue)
    : Source(outQueue), file(filename, ios::in | BINARY_MODE | ios::nocreate), in(file)
{
#ifdef THROW_EXCEPTIONS
    if (!file)
        throw OpenErr();
#endif
    if (pumpAndClose)
        Close();
}

unsigned int FileSource::Pump(unsigned int size)
{
    unsigned int l;

    if (in.good())
    {
        SecByteBlock buf(size);
        in.read((byte *)buf, size);
        l = in.gcount();
        outQueue->Put(buf, l);
    }
    else
        l = 0;

#ifdef THROW_EXCEPTIONS
    if (in.bad())
        throw ReadErr();
#endif

    return l;
}

unsigned long FileSource::PumpAll()
{
    unsigned long total=0;
    unsigned int l;

    while ((l=Pump(1024)) != 0)
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
        throw OpenErr();
#endif
}

void FileSink::InputFinished()
{
    out.flush();
}

void FileSink::Put(const byte *inString, unsigned int length)
{
    out.write(inString, length);
}

