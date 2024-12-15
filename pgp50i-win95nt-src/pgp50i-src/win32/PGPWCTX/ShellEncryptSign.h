/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
/*The idea with IgnorePassphrase is that it allows the calling function
 *to pass in a non-existant passphrase the first time, let ShellEncryptSign
 *set it (by calling the passphrase dialog) and then use it for subsequent
 *calls.
 *
 *szOutputFile can be NULL, in which case the user will be prompted.
 *
 */

UINT ShellEncryptSign(HWND hwnd,
					  char* szInputFile,
					  char* szOutputFileArg,
					  BOOL Encrypt,
					  BOOL Sign,
					  BOOL bDefaultASCIIArmor,
					  BOOL bAllowDetachedSig,
					  BOOL bAllowASCIIArmor,
					  PRECIPIENTDIALOGSTRUCT pRecipientDialogStruct,
					  BOOL IsFirstCallWithThisRecipientDialogStruct,
					  char **PassPhrase);
