/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
#ifndef __WORDWRAP_H
#define __WORDWRAP_H

unsigned long WrapBuffer(char **WrappedBuffer,
						 char *OriginalBuffer,
						 unsigned short WordWrapThreshold);

#endif //__WORDWRAP_H
