/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
/*4514 is "unreferenced inline func"*/
#pragma warning (disable : 4214 4201 4115 4514)
#include <windows.h>
#pragma warning (default : 4214 4201 4115)
#include <assert.h>

#include "..\include\config.h"
#include "..\include\pgpmem.h"

#include "errs.h"

unsigned long WrapBuffer(char **WrappedBuffer,
						 char *OriginalBuffer,
						 unsigned short WordWrapThreshold)
{
	unsigned long ReturnCode = SUCCESS;
	char *StartOfCurrentBlock, *StartOfCurrentLine,*EndOfCurrentBlock;
	unsigned long CharsOnCurrentLine = 0, WrappedBufferLength = 1,
					CurrentBlockLength = 0;
	unsigned short SpaceOffset = 1;

	assert(WrappedBuffer);
	assert(OriginalBuffer);

	//Do an initial, one char Allocation:
	*WrappedBuffer = (char *) pgpAlloc(WrappedBufferLength * sizeof(char));

	if(*WrappedBuffer)
	{
		**WrappedBuffer = '\0';
		StartOfCurrentBlock=StartOfCurrentLine=EndOfCurrentBlock=OriginalBuffer;

		//Blocks run until we hit something that needs to be word-wrapped
		//(ie > 78 chars on a line).  Blocks can be multiple lines.

		//Run until the end of the original buffer:
		while(EndOfCurrentBlock &&
			  *EndOfCurrentBlock &&
			  ReturnCode == SUCCESS)
		{
			if((*EndOfCurrentBlock == '\r') ||
			    (*EndOfCurrentBlock == '\n'))
			{
				CharsOnCurrentLine = 0;
				StartOfCurrentLine=EndOfCurrentBlock+1;
			}
			else
			{
				if(WordWrapThreshold && CharsOnCurrentLine >= WordWrapThreshold)
				{
					//Uh-oh!  Time to wrap!  Back up to the last space...
					while((EndOfCurrentBlock > StartOfCurrentLine) &&
						  (*EndOfCurrentBlock != ' ' &&
						   *EndOfCurrentBlock != '\t' &&
						   *EndOfCurrentBlock != '\n' &&
						   *EndOfCurrentBlock != '\r' &&
						   *EndOfCurrentBlock != '-'))
						  --EndOfCurrentBlock;

					//We only get here if there WERE no spaces!
					if(EndOfCurrentBlock == StartOfCurrentLine)
					{
						EndOfCurrentBlock =
						 StartOfCurrentLine + WordWrapThreshold;
						SpaceOffset=1; //SpaceOffset = 0;
					}
					else
						SpaceOffset = 1;

					//Allocate the new block space (+2 is one for the
					//newline and one for the return):
					CurrentBlockLength = (EndOfCurrentBlock -
											StartOfCurrentBlock);
					WrappedBufferLength += (CurrentBlockLength + 2);

					pgpRealloc(WrappedBuffer, WrappedBufferLength);
					if(*WrappedBuffer)
					{
						strncat(*WrappedBuffer, StartOfCurrentBlock,
								CurrentBlockLength);
						//Force NULL termination:
						*((*WrappedBuffer) + WrappedBufferLength - 2) = '\0';
						strcat(*WrappedBuffer, "\r\n");

						//Start the next block, skipping the space:
						StartOfCurrentBlock=StartOfCurrentLine=
							EndOfCurrentBlock + SpaceOffset;
					}
					else
						ReturnCode = ERR_NO_MEM;

					CharsOnCurrentLine = 0;
				}
				else
					++CharsOnCurrentLine;
			}
			++EndOfCurrentBlock;
		}
		//They did wrap; we need to clear up any trailing block.
		if(EndOfCurrentBlock > StartOfCurrentBlock)
		{
			CurrentBlockLength = (EndOfCurrentBlock - StartOfCurrentBlock);
			WrappedBufferLength += CurrentBlockLength + 1;

			pgpRealloc(WrappedBuffer, WrappedBufferLength);

			if(*WrappedBuffer)
			{
				strncat(*WrappedBuffer, StartOfCurrentBlock,
						CurrentBlockLength);
				//Force NULL termination:
				*((*WrappedBuffer) + WrappedBufferLength - 2) = '\0';
			}
			else
				ReturnCode = ERR_NO_MEM;
		}
	}
	else
		ReturnCode = ERR_NO_MEM;

	return(ReturnCode);
}
