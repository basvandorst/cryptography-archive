#ifndef MODES_H
#define MODES_H

#include "cryptlib.h"

class Mode
{
protected:
    Mode(BlockTransformation &cipher, const byte *IV);
    ~Mode();

    BlockTransformation &cipher;
    const int S;
    byte *const reg;    // register is a reserved word
    byte *const buffer;
};

class FeedBack : protected Mode
{
protected:
    // feedBackSize = 0 means use maximum feedback size (i.e., the cipher block size)
    FeedBack(BlockTransformation &cipher, const byte *IV, int feedBackSize);
    void DoFeedBack();

    const int FBS;
    int counter;
};

class CFBEncryption : public StreamCipher, protected FeedBack
{
public:
    // cipher should be an *encryption* object
    CFBEncryption(BlockTransformation &cipher, const byte *IV, int feedBackSize = 0)
    	: FeedBack(cipher, IV, feedBackSize) {}

    byte ProcessByte(byte input)
    {
        if (counter==FBS)
            DoFeedBack();
        buffer[counter] ^= input;
        return buffer[counter++];
    }

    void ProcessString(byte *outString, const byte *inString, unsigned int length);
    void ProcessString(byte *inoutString, unsigned int length);
};

class CFBDecryption : public StreamCipher, protected FeedBack
{
public:
    // cipher should be an *encryption* object
    CFBDecryption(BlockTransformation &cipher, const byte *IV, int feedBackSize = 0)
    	: FeedBack(cipher, IV, feedBackSize) {}

    byte ProcessByte(byte input)
    {
        if (counter==FBS)
            DoFeedBack();
        byte b = buffer[counter] ^ input;
        buffer[counter++] = input;
        return (b);
    }

    void ProcessString(byte *outString, const byte *inString, unsigned int length);
    void ProcessString(byte *inoutString, unsigned int length);
};

class OFB : public RandomNumberGenerator, public StreamCipher, protected FeedBack
{
public:
    // cipher should be an *encryption* object
    OFB(BlockTransformation &cipher, const byte *IV, int feedBackSize = 0)
    	: FeedBack(cipher, IV, feedBackSize) {}

    byte GetByte()
    {
        if (counter==FBS)
            DoFeedBack();
        return buffer[counter++];
    }

    byte ProcessByte(byte input)
        {return (input ^ OFB::GetByte());}

    void ProcessString(byte *outString, const byte *inString, unsigned int length);
    void ProcessString(byte *inoutString, unsigned int length);
};

class CounterMode : public RandomNumberGenerator, public RandomAccessStreamCipher, protected Mode
{
public:
    // cipher should be an *encryption* object
    CounterMode(BlockTransformation &cipher, const byte *IV);
    ~CounterMode();

    byte GetByte()
    {
        if (size==S)
            IncrementCounter();
        return buffer[size++];
    }

    byte ProcessByte(byte input)
        {return (input ^ CounterMode::GetByte());}

    void ProcessString(byte *outString, const byte *inString, unsigned int length);
    void ProcessString(byte *inoutString, unsigned int length);

    void Seek(unsigned long position);

private:
    void IncrementCounter();

    byte *const IV;
    int size;
};

class CBCEncryption : public BlockTransformation, protected Mode
{
public:
    // cipher should be an *encryption* object
    CBCEncryption(BlockTransformation &cipher, const byte *IV)
    	: Mode(cipher, IV) {};

    void ProcessBlock(byte *inoutBlock)
    	{CBCEncryption::ProcessBlock(inoutBlock, inoutBlock);};
    void ProcessBlock(const byte *inBlock, byte *outBlock);
    unsigned int BlockSize() const {return S;};
};

class CBCDecryption : public BlockTransformation, protected Mode
{
public:
    // cipher should be a *decryption* object
    CBCDecryption(BlockTransformation &cipher, const byte *IV)
    	: Mode(cipher, IV) {};

    void ProcessBlock(byte *inoutBlock);
    void ProcessBlock(const byte *inBlock, byte *outBlock);
    unsigned int BlockSize() const {return S;};
};

class PGP_CFBEncryption : public CFBEncryption
{
public:
    // cipher should be an *encryption* object
    PGP_CFBEncryption(BlockTransformation &cipher, const byte *IV)
    	: CFBEncryption(cipher, IV, 0) {}

    void Sync();
};

class PGP_CFBDecryption : public CFBDecryption
{
public:
    // cipher should be an *encryption* object
    PGP_CFBDecryption(BlockTransformation &cipher, const byte *IV)
    	: CFBDecryption(cipher, IV, 0) {}

    void Sync();
};

#endif

