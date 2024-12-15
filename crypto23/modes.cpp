// modes.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "modes.h"

Mode::Mode(BlockTransformation &c, const byte *IV)
	: cipher(c),
	  S(cipher.BlockSize()),
	  reg(IV, S),
	  buffer(S)
{
}

FeedBack::FeedBack(BlockTransformation &cipher, const byte *IV, int fbs)
	: Mode(cipher, IV), FBS(fbs ? fbs : S)
{
	cipher.ProcessBlock(reg, buffer);
	counter = 0;
}

void FeedBack::DoFeedBack()
{
	for (int i=0; i<(S-FBS); i++)
		reg[i] = reg[FBS+i];
	memcpy(reg+S-FBS, buffer, FBS);
	cipher.ProcessBlock(reg, buffer);
	counter = 0;
}

void CFBEncryption::ProcessString(byte *outString, const byte *inString, unsigned int length)
{
	while(length--)
		*outString++ = CFBEncryption::ProcessByte(*inString++);
}

void CFBEncryption::ProcessString(byte *inoutString, unsigned int length)
{
	while(length--)
		*inoutString++ = CFBEncryption::ProcessByte(*inoutString);
}

void CFBDecryption::ProcessString(byte *outString, const byte *inString, unsigned int length)
{
	while(length--)
		*outString++ = CFBDecryption::ProcessByte(*inString++);
}

void CFBDecryption::ProcessString(byte *inoutString, unsigned int length)
{
	while(length--)
		*inoutString++ = CFBDecryption::ProcessByte(*inoutString);
}

void OFB::ProcessString(byte *outString, const byte *inString, unsigned int length)
{
	while(length--)
		*outString++ = *inString++ ^ OFB::GetByte();
}

void OFB::ProcessString(byte *inoutString, unsigned int length)
{
	while(length--)
		*inoutString++ ^= OFB::GetByte();
}

CounterMode::CounterMode(BlockTransformation &cipher, const byte *IVin)
	: Mode(cipher, IVin), IV(IVin, S)
{
	cipher.ProcessBlock(reg, buffer);
	size=0;
}

void CounterMode::ProcessString(byte *outString, const byte *inString, unsigned int length)
{
	while(length--)
		*outString++ = *inString++ ^ CounterMode::GetByte();
}

void CounterMode::ProcessString(byte *inoutString, unsigned int length)
{
	while(length--)
		*inoutString++ ^= CounterMode::GetByte();
}

void CounterMode::Seek(unsigned long position)
{
	unsigned long blockIndex = position / S;

	// set register to IV+blockIndex
	int carry=0;
	for (int i=S-1; i>=0; i--)
	{
		int sum = IV[i] + byte(blockIndex) + carry;
		reg[i] = (byte) sum;
		carry = sum >> 8;
		blockIndex >>= 8;
	}

	cipher.ProcessBlock(reg, buffer);
	size = int(position % S);
}

void CounterMode::IncrementCounter()
{
	for (int i=S-1, carry=1; i>=0 && carry; i--)
    	carry=!++reg[i];

	cipher.ProcessBlock(reg, buffer);
	size=0;
}

void CBCEncryption::ProcessBlock(const byte *inBlock, byte *outBlock)
{
	xorbuf(reg, inBlock, S);
	cipher.ProcessBlock(reg);
	memcpy(outBlock, reg, S);
}

void CBCDecryption::ProcessBlock(const byte *inBlock, byte *outBlock)
{
	cipher.ProcessBlock(inBlock, outBlock);
	xorbuf(outBlock, reg, S);
	memcpy(reg, inBlock, S);
}

void CBCDecryption::ProcessBlock(byte *inoutBlock)
{
	memcpy(buffer, inoutBlock, S);
	CBCDecryption::ProcessBlock(buffer, inoutBlock);
}

void PGP_CFBEncryption::Sync()
{
	if (counter)
	{
		for (int i=0; i<counter; i++)
			buffer[S-counter+i] = buffer[i];
		memcpy(buffer, reg+counter, S-counter);
		counter = 0;
	}
}

// this is exactly the same function as above
void PGP_CFBDecryption::Sync()
{
	if (counter)
	{
		for (int i=0; i<counter; i++)
			buffer[S-counter+i] = buffer[i];
		memcpy(buffer, reg+counter, S-counter);
		counter = 0;
	}
}
