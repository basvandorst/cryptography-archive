/*____________________________________________________________________________	Copyright (C) 1994-1998 Network Associates Inc. and affiliated companies.	All rights reserved.		$Id: CGetPassphraseDialog.h,v 1.3 1999/03/10 02:24:40 heller Exp $____________________________________________________________________________*/#pragma once#include "CPassphraseDialog.h"class CPassphraseField;class CGetPassphraseDialog : public CPassphraseDialog{public:	enum { class_ID = 'GPhr' };						CGetPassphraseDialog();	virtual			~CGetPassphraseDialog();					CGetPassphraseDialog(LStream *inStream);	Boolean			GetReadOnlyStatus(void);	void			SetReadOnlyStatus(Boolean readOnly,							Boolean disableCheckbox, Boolean hideCheckbox);};OSStatus	DoMountDiskPassphraseDialog(const FSSpec *fileSpec,					Boolean forceReadOnly, StringPtr passphrase,					Boolean *mountReadOnly);OSStatus	DoGetOldPassphraseDialog(ConstStr255Param diskName,					StringPtr passphrase);OSStatus	DoGetMasterPassphraseDialog(ConstStr255Param diskName,					StringPtr masterPassphrase);OSStatus	DoGetRemovePassphraseDialog(ConstStr255Param diskName,					StringPtr masterPassphrase);