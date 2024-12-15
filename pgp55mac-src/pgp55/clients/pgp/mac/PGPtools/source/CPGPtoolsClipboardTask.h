/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#pragma once

#include "CPGPtoolsBufferTask.h"

class CPGPtoolsClipboardTask : public CPGPtoolsBufferTask
{
public:
						CPGPtoolsClipboardTask(PGPtoolsOperation operation);
	virtual				~CPGPtoolsClipboardTask(void);
							
	virtual	OSStatus	CalcProgressBytes(PGPContextRef context,
							ByteCount *bytes);
	virtual CToolsError	DecryptVerify(PGPContextRef	context,
								CDecodeParams *state);
	virtual	CToolsError	EncryptSign(PGPContextRef context,
								CEncodeParams *state);

protected:

	virtual	void		GetDecryptPassphrasePromptString(StringPtr prompt);
	virtual	void		GetTaskItemName(StringPtr name);
	virtual	CToolsError	ProcessOutputData(void);
	
private:

	OSErr			GetClipboardData(PGPContextRef context);
};