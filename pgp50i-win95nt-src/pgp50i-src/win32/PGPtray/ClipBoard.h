/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
void* RetrieveClipboardData(HWND hwnd, UINT* pClipboardFormat,
							DWORD* pDataSize);
void StoreClipboardData(HWND hwnd, void* pData, DWORD dwDataSize);
