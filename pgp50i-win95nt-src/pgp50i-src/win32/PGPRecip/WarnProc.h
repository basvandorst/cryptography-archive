/*
 * WarnProc.h  Main message loop for the warning dialog
 *
 * This message loop handles all the operations of the warning
 * dialog. Its a bit trickey, since it includes some routines to tack
 * on our "global" variables to the USERDATA portion of the window
 * (for reentrancy)
 *
 * Copyright (C) 1996, 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 */

int RSADSARecWarning(HWND hwnd);
