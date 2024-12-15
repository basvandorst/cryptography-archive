/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: CTempFile.h,v 1.3.10.1 1997/12/05 22:15:10 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#include <LFileStream.h>

class CTempFile	:	public LFileStream {
public:
					CTempFile(OSType inCreator = '????',
						OSType inFileType = '????',
						StringPtr inName = nil, Boolean inCreate = true);
					CTempFile(LFile * inFile, Boolean inCreate = true);
	virtual			~CTempFile();
	
private:
	virtual	void	InitCTempFile(OSType inCreator, OSType inFileType,
						Str63 inName, Boolean inCreate);
};
