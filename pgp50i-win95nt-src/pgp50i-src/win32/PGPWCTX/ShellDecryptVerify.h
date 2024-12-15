/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
/*The idea with IgnorePassphrase is that it allows the calling function
 *to pass in a non-existant passphrase the first time, let ShellDecryptVerify
 *set it (by calling the passphrase dialog) and then use it for subsequent
 *calls.
 */

#ifdef __cplusplus
extern "C" {
#endif
UINT PGPExport ShellDecryptVerify(HWND hwnd,
						char* InputFile,
						BOOL JustVerify,
						BOOL PrimarilyAddKeys,
						char **PassPhrase,
						BOOL IsFirstCallWithThisPassphrase);
#ifdef __cplusplus
}
#endif
