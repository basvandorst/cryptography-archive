/*____________________________________________________________________________	Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.				$Id: CTempFile.h,v 1.3.20.1 1998/11/12 03:10:38 heller Exp $____________________________________________________________________________*/#pragma once #include <LFileStream.h>class CTempFile	:	public LFileStream {public:					CTempFile(OSType inCreator = '????',						OSType inFileType = '????',						StringPtr inName = nil, Boolean inCreate = true);					CTempFile(LFile * inFile, Boolean inCreate = true);	virtual			~CTempFile();	private:	virtual	void	InitCTempFile(OSType inCreator, OSType inFileType,						Str63 inName, Boolean inCreate);};