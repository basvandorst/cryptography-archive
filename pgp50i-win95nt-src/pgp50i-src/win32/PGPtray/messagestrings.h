/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
BOOL
SimplePGPErrorToString(char *ErrorMessage,
					   unsigned long ErrorMessageLen,
					   int SimpleResult);

BOOL
TranslateSignatureResult(char *SignatureResult,
						  char *Signer,
						  char *SignatureDate,
						  int SignatureCode);
