/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#pragma once

#include "CPGPtoolsBufferTask.h"

class CPGPtoolsTextTask : public CPGPtoolsBufferTask
{
public:
						CPGPtoolsTextTask(PGPtoolsOperation operation);
	virtual				~CPGPtoolsTextTask(void);
	virtual CToolsError	DecryptVerify(PGPContextRef	context,
								CDecodeParams *state);
	virtual	CToolsError	EncryptSign(PGPContextRef context,
								CEncodeParams *state);
	virtual	void		SetSourceFileName(ConstStringPtr name);

protected:

	virtual	void		GetDecryptPassphrasePromptString(StringPtr prompt);
	virtual	void		GetTaskItemName(StringPtr name);
	virtual	CToolsError	ProcessOutputData(void);
	
private:

	Str32				mSourceFileName;
	
	virtual	void		BuildErrorMessage(CToolsError err,
							short errorStrIndex, StringPtr msg);
};