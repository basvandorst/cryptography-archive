/*_____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
_____________________________________________________________________________*/

#pragma once

#include <PP_Types.h>

#include "pgpEncode.h"
#include "pgpUserInterface.h"
#include "pgpKeys.h"

// Note: Changing these definitions requires changing the user constants
// in the PPob resource for the drop receiver panes of the main window.

typedef enum PGPtoolsOperation
{
	kPGPtoolsInvalidOperation	= 0,
	
	kPGPtoolsEncryptOperation,
	kPGPtoolsSignOperation,
	kPGPtoolsEncryptSignOperation,
	kPGPtoolsDecryptVerifyOperation,
	
	kPGPtoolsWipeOperation
	
} PGPtoolsOperation;

#include "CComboError.h"

class CToolsError : public CComboError
{
/* leave as placeholder for now */
};

typedef struct PGPtoolsSignatureData
{
	PGPEventSignatureData	data;
	char					signingKeyIDStr[ kPGPMaxKeyIDStringSize ];
	Boolean					haveResourceFork;
	Boolean					didNotVerifyResourceFork;

} PGPtoolsSignatureData;
