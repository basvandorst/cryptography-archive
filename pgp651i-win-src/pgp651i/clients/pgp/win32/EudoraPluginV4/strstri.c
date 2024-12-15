/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.
	
	

	strstri.c,v 1.3 1997/07/16 05:51:53 elrod Exp
____________________________________________________________________________*/

// System Headers
#include <stdlib.h>

char* strstri(char * inBuffer, char * inSearchStr)
{
	char*  currBuffPointer = inBuffer;

    while (*currBuffPointer != 0x00)
	{
        char* compareOne = currBuffPointer;
        char* compareTwo = inSearchStr;

        while (tolower(*compareOne) == tolower(*compareTwo))
		{
            compareOne++;
            compareTwo++;
            if (*compareTwo == 0x00)
			{
				return (char*) currBuffPointer;
            }
        }
        currBuffPointer++;
    }
    return NULL;
}
