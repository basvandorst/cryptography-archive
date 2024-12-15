/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
KSERR UrlEncode(char **Dest, char *Source);
void ConvertKeyIDToString(LPSTR sz);
unsigned long AddKeySetWithConfirm(HWND hwnd,
								   PGPKeySet *pSourceKeySet,
								   PGPKeySet *pDestKeySet,
								   unsigned long *NumKeysFound);
