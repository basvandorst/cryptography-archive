/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#pragma once

#include "CPGPtoolsCryptoTask.h"

class CPGPtoolsBufferTask : public CPGPtoolsCryptoTask
{
public:
						CPGPtoolsBufferTask(PGPtoolsOperation operation);
	virtual				~CPGPtoolsBufferTask(void);
							
	virtual	OSStatus	CalcProgressBytes(PGPContextRef context,
							ByteCount *bytes);
	virtual CToolsError	DecryptVerify(PGPContextRef	context,
								CDecodeParams *state);
	virtual	CToolsError	EncryptSign(PGPContextRef context,
								CEncodeParams *state);
	PGPError			SetText(PGPByte *text, PGPSize textLength);

protected:

	void				*mText;
	PGPContextRef		mTextContext;
	PGPSize				mTextLength;

	virtual	CToolsError	ProcessOutputData(void) = 0;
};