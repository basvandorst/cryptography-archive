/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
#ifndef __ACTIONS_H
#define __ACTIONS_H

unsigned long DoEncrypt(HWND hwnd, BOOL Encrypt, BOOL Sign);
unsigned long DoDecrypt(HWND hwnd);
unsigned long DoAddKey(HWND hwnd);
unsigned long DoLaunchKeys(HWND hwnd);

#endif /*__ACTIONS_H*/
