/*
 * pgpBlockPrefs.h -- Definitions of internal functions and structures for
 * pgpBlockPrefs.c.
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * Written by:	Brett A. Thomas <quark@baz.com>, <bat@pgp.com>
 *
 * $Id: pgpBlockPrefs.h,v 1.1.2.1.2.1 1997/07/01 20:12:29 quark Exp $
 */

typedef struct _Value
{
    char *Name;
    char *Contents;
}VALUE;

typedef struct _Block

{
    char *BlockName;
    VALUE *Value;
    unsigned long NumValues;
}BLOCK;

typedef struct _Prefs
{
    BLOCK *Block;
    unsigned long NumBlocks;
}PREFS;

unsigned long pgpBlockPrefsLoadFile(PREFS **ppPrefs, const char *Filename);
unsigned long pgpBlockPrefsLoadBuffer(PREFS **ppPrefs, const char *Buffer);
unsigned long pgpBlockPrefsFreePrefs(PREFS **ppPrefs);
unsigned long pgpBlockPrefsGetValue(PREFS *pPrefs,
				    const char *BlockName,
				    const char *ValueName,
				    char **Value);
unsigned long pgpBlockPrefsFreeValue(char **Value);
