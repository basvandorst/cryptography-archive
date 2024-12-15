/*
 * pgpBlockPrefs.c -- Functions to allow reading of "block preferences" files,
 * similar in format to Windows INI files.  Primarily used by 5.0 for
 * supprot of multiple languages.
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * Written by:	Brett A. Thomas <quark@baz.com>, <bat@pgp.com>
 *
 * $Id: pgpBlockPrefs.c,v 1.1.2.1.2.3 1997/07/16 05:07:33 quark Exp $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <ctype.h>

#include "pgpUsuals.h"
#include "pgpErr.h"
#include "pgpDebug.h"
#include "pgpMem.h"

#include "pgpBlockPrefs.h"

static unsigned long ProcessLine(PREFS **ppPrefs,
				 char *LineStart,
				 char *LineEnd);
static unsigned long pgpBlockPrefsProcessEscapes(char *buffer);

unsigned long pgpBlockPrefsLoadFile(PREFS **ppPrefs, const char *Filename)
{
    FILE *inf;
    unsigned long ReturnCode = PGPERR_OK;

    pgpAssert(ppPrefs);
    pgpAssert(Filename);

    *ppPrefs = NULL;

    if((inf = fopen(Filename, "r"))) {
	unsigned long FileSize = 0;
	char *buffer = NULL;

	fseek(inf, 0L, SEEK_END);
	FileSize = ftell(inf);
	if((buffer = pgpAlloc(sizeof(char) * (FileSize + 1)))) {
	    fseek(inf, 0L, SEEK_SET);
	    fread(buffer, sizeof(char), FileSize, inf);
	    *(buffer + FileSize) = '\0';
	    pgpBlockPrefsLoadBuffer(ppPrefs, buffer);
	    pgpFree(buffer);
	}
	else
	    ReturnCode = PGPERR_NOMEM;
    }
    else
	ReturnCode = PGPERR_NO_FILE;

    return(ReturnCode);
}

unsigned long pgpBlockPrefsLoadBuffer(PREFS **ppPrefs, const char *Buffer)
{
    unsigned long ReturnCode = PGPERR_OK;
    char *LineStart, *LineEnd, *tmp, *TmpBuffer = NULL;

    pgpAssert(Buffer);
    pgpAssert(ppPrefs);

    *ppPrefs = NULL;

    if((TmpBuffer = pgpAlloc((strlen(Buffer) + 1) * sizeof(char)))) {
	strcpy(TmpBuffer, Buffer);
	LineStart = TmpBuffer;

	do {
	    LineEnd = LineStart;

	    while(*LineEnd && *LineEnd != '\n') {
		++LineEnd;
		if(*LineEnd == '\\') {
		    tmp = LineEnd + 1;
		    while(*tmp && isspace(*tmp))
			++tmp;
		    if(tmp > LineStart && *(tmp - 1) == '\n') {
			memmove(LineEnd, tmp, strlen(tmp));
		    }
		}
	    }

	    ReturnCode = ProcessLine(ppPrefs, LineStart, LineEnd);
	    if(*LineEnd) {
		LineStart = LineEnd + 1;
		while(*LineStart && isspace(*LineStart))
		    ++LineStart;
	    }
	    else
		LineStart = NULL;
	
	} while(ReturnCode == PGPERR_OK && LineStart && *LineStart);
	pgpFree(TmpBuffer);
    }
    else
	ReturnCode = PGPERR_NOMEM;

    return(ReturnCode);
}

unsigned long pgpBlockPrefsGetValue(PREFS *pPrefs,
				    const char *pBlockName,
				    const char *pValueName,
				    char **ppValue)
{
    unsigned long BlockCursor = 0, ValueCursor = 0, ReturnCode = PGPERR_OK;
    unsigned short FoundIt = 0;

    pgpAssert(pPrefs);
    pgpAssert(pBlockName);
    pgpAssert(pValueName);
    pgpAssert(ppValue);


    for(BlockCursor = 0;
	BlockCursor < pPrefs->NumBlocks && !FoundIt;
	++BlockCursor) {
	if(strcmp((pPrefs->Block + BlockCursor)->BlockName,
		  pBlockName) == 0) {
	    FoundIt = 1;
	}
    }

    if(FoundIt) {
	BLOCK *pCurrentBlock;
	--BlockCursor;
	pCurrentBlock = (pPrefs->Block + BlockCursor);
	for(ValueCursor = 0, FoundIt = 0;
	    ValueCursor < pCurrentBlock->NumValues && !FoundIt;
	    ++ValueCursor) {
	    if(strcmp((pCurrentBlock->Value + ValueCursor)->Name,
		      pValueName) == 0) {
		FoundIt = 1;
	    }
	}

	if(FoundIt) {
	    char *Contents;
	    --ValueCursor;
	    Contents = (pCurrentBlock->Value + ValueCursor)->Contents;
	    if((*ppValue = pgpAlloc(sizeof(char) *
				    strlen(Contents) + 1))) {
		strcpy(*ppValue, Contents);
	    }
	    else
		ReturnCode = PGPERR_NOMEM;
	}
	else
	    ReturnCode = PGPERR_STRING_NOT_IN_LANGUAGE;
    }
    else
	ReturnCode = PGPERR_STRING_NOT_FOUND;

    return(ReturnCode);
}

unsigned long pgpBlockPrefsFreePrefs(PREFS **ppPrefs)
{
    unsigned long BlockCursor, ValueCursor, ReturnCode = PGPERR_OK;
    short FoundIt = 0;
    VALUE *pCurrentValue;
    BLOCK *pCurrentBlock;

    pgpAssert(ppPrefs);
    pgpAssert(*ppPrefs);

    for(BlockCursor = 0;
	BlockCursor < (*ppPrefs)->NumBlocks && !FoundIt;
	++BlockCursor) {
	pCurrentBlock = ((*ppPrefs)->Block + BlockCursor);
	pgpAssert(pCurrentBlock);
	pgpAssert(pCurrentBlock->BlockName);
	pgpAssert(pCurrentBlock->Value);

	for(ValueCursor = 0;
	    ValueCursor < pCurrentBlock->NumValues;
	    ++ValueCursor) {
	    pCurrentValue = pCurrentBlock->Value + ValueCursor;
	    pgpAssert(pCurrentValue);
	    pgpAssert(pCurrentValue->Name);
	    pgpAssert(pCurrentValue->Contents);

	    pgpFree(pCurrentValue->Name);
	    pgpFree(pCurrentValue->Contents);
	}
	pgpFree(pCurrentBlock->BlockName);
	pgpFree(pCurrentBlock->Value);
    }
    pgpFree((*ppPrefs)->Block);
    pgpFree((*ppPrefs));
    *ppPrefs = NULL;

    return(ReturnCode);
}

unsigned long pgpBlockPrefsFreeValue(char **Value)
{
    unsigned long ReturnCode = PGPERR_OK;

    pgpAssert(Value);
    pgpAssert(*Value);

    if(Value && *Value) {
	pgpFree(*Value);
	*Value = NULL;
    }

    return(ReturnCode);
}

static unsigned long ProcessLine(PREFS **ppPrefs,
				 char *LineStart,
				 char *LineEnd)
{
    BLOCK *CurrentBlock = NULL;
    unsigned long ReturnCode = PGPERR_OK;
    char *TempLine = NULL;

    pgpAssert(ppPrefs);
    pgpAssert(LineStart);
    pgpAssert(LineEnd);

    while(*LineStart && isspace(*LineStart))
	++LineStart;

    while(*LineEnd && isspace(*LineEnd))
	--LineEnd;

    if((TempLine = pgpAlloc(sizeof(char) * ((LineEnd - LineStart) + 2)))) {
	strncpy(TempLine, LineStart, (LineEnd - LineStart) + 1);
	*(TempLine + ((LineEnd - LineStart) + 1)) = '\0';
	pgpBlockPrefsProcessEscapes(TempLine);
	LineStart = LineEnd = TempLine;
	while(*LineEnd)
	    ++LineEnd;
	--LineEnd;
    }
    else
	ReturnCode = PGPERR_NOMEM;


    if(ReturnCode == PGPERR_OK) {
	if(*LineStart == '[') {/*It's a block*/
	    char *BlockEnd, *BlockStart;
	    if((BlockEnd = strchr(LineStart, ']')))
		BlockEnd--;
	    else
		BlockEnd = LineEnd - 1;
	
	    BlockStart = LineStart + 1;
	
	    if(!*ppPrefs) {
		if((*ppPrefs = pgpAlloc(sizeof(PREFS)))) {
		    if(((*ppPrefs)->Block = pgpAlloc(sizeof(BLOCK)))) {
			(*ppPrefs)->NumBlocks = 1;
		    }
		    else {
			pgpFree(*ppPrefs);
			*ppPrefs = NULL;
			ReturnCode = PGPERR_NOMEM;
		    }
		}
		else
		    ReturnCode = PGPERR_NOMEM;
	    }
	    else {
		(*ppPrefs)->NumBlocks++;
		ReturnCode = pgpRealloc((void **) &((*ppPrefs)->Block),
					(sizeof(BLOCK) *
					 (*ppPrefs)->NumBlocks));
	    }
	
	    if(ReturnCode == PGPERR_OK && *ppPrefs && (*ppPrefs)->Block) {
		CurrentBlock = ((*ppPrefs)->Block +
				((*ppPrefs)->NumBlocks - 1));
		if((CurrentBlock->BlockName =
		    pgpAlloc(sizeof(char) * ((BlockEnd - BlockStart) + 2)))) {
		    strncpy(CurrentBlock->BlockName,
			    BlockStart,
			    (BlockEnd - BlockStart) + 1);
		    *(CurrentBlock->BlockName +
		      (BlockEnd - BlockStart) + 1) = '\0';
		    CurrentBlock->Value = NULL;
		    CurrentBlock->NumValues = 0;
		}
		else {
		    pgpFree(CurrentBlock);
		    ReturnCode = PGPERR_NOMEM;
		}
	    }
	    else
		ReturnCode = PGPERR_NOMEM;
	}
	else { /*It's a value!*/
	    if(*ppPrefs && (*ppPrefs)->Block && (*ppPrefs)->NumBlocks) {
		char *NameStart = NULL,
		    *NameEnd = NULL,
		    *ValueStart = NULL,
		    *ValueEnd = NULL;
		
		CurrentBlock = ((*ppPrefs)->Block + (*ppPrefs)->NumBlocks - 1);
		
		pgpAssert((*ppPrefs)->NumBlocks != 1 ||
			  CurrentBlock == (*ppPrefs)->Block);
		
		NameStart = LineStart;
		while(NameStart < LineEnd && isspace(*NameStart))
		    ++NameStart;
		
		if((NameEnd = strchr(NameStart, '='))) {
		    ValueStart = NameEnd + 1;
		    --NameEnd;
		    ValueEnd = LineEnd;

		    if(*ValueStart == '\"' &&
		       *ValueEnd == '\"' &&
		       ValueEnd > ValueStart) {
			++ValueStart;
			--ValueEnd;
		    }
		
		    while(NameEnd > NameStart && isspace(*NameEnd))
			--NameEnd;
		
		    while(ValueStart > ValueEnd && isspace(*ValueStart))
			++ValueStart;
		
		    if(!CurrentBlock->NumValues || !CurrentBlock->Value) {
			CurrentBlock->NumValues = 1;
			if(!(CurrentBlock->Value = pgpAlloc(sizeof(VALUE)))) {
			    ReturnCode = PGPERR_NOMEM;
			}
		    }
		    else {
			++(CurrentBlock->NumValues);
			ReturnCode =
			    pgpRealloc((void **) &(CurrentBlock->Value),
				       CurrentBlock->NumValues *
				       sizeof(VALUE));
		    }
		
		    if(ReturnCode == PGPERR_OK) {
			VALUE *CurrentValue;
			
			CurrentValue = (CurrentBlock->Value +
					(CurrentBlock->NumValues - 1));
			pgpAssert(CurrentBlock->NumValues != 1 ||
				  CurrentValue == CurrentBlock->Value);
			CurrentValue->Name = CurrentValue->Contents = NULL;
			
			if((CurrentValue->Name =
			    pgpAlloc((NameEnd - NameStart + 2) *
				     sizeof(char)))) {
			    if((CurrentValue->Contents =
				pgpAlloc((ValueEnd - ValueStart + 2) *
					 sizeof(char)))) {
				strncpy(CurrentValue->Name,
					NameStart,
					(NameEnd - NameStart) + 1);
				*(CurrentValue->Name +
				  (NameEnd - NameStart) + 1) = '\0';
				strncpy(CurrentValue->Contents,
					ValueStart,
					(ValueEnd - ValueStart) + 1);
				*(CurrentValue->Contents +
				  (ValueEnd - ValueStart) + 1) = '\0';
			    }
			}
		    }
		}
		else
		    ReturnCode = PGPERR_OK; /*XXX Add a new code for bad file*/
	    }
	    else
		ReturnCode = PGPERR_OK; /*XXX Add a new code for bad file*/
	}
    }

    if(TempLine)
	pgpFree(TempLine);

    return(ReturnCode);
}


static unsigned long pgpBlockPrefsProcessEscapes(char *buffer)
{
    char *p, *DeleteStart = NULL, *DeleteEnd = NULL;
    unsigned long ReturnCode = PGPERR_OK;

    p = buffer;

    while(*p) {
	if(*p == '\\') {
	    if(!isdigit(*(p + 1))) {
		if(*(p + 1)) {
		    DeleteStart = DeleteEnd = p + 1;
		    switch(*(p + 1)) {
			case 'n':
			    *p = '\n';
			    break;
			
			case 'r':
			    *p = '\r';
			    break;
			
			case 't':
			    *p = '\t';
			    break;
			
			case 'a':
			    *p = '\a';
			    break;
			
			case '\n': /*It's a line continuation*/
			    DeleteStart = p;
			    DeleteEnd = p + 1;
			    break;

			default:
			    *p = *(p + 1);
			    break;
		    }
		}
	    }
	    else { /*It's a numeric escape*/
		long int TempValue;
		DeleteStart = DeleteEnd = p + 1;
		while(*DeleteEnd && isdigit(*DeleteEnd))
		    ++DeleteEnd;

		if(!*DeleteEnd)
		    --DeleteEnd;

		TempValue = strtol(DeleteStart, &DeleteEnd, 8);
		*p = (char) TempValue;
	    }

	    if(DeleteStart && DeleteEnd) {
		memmove(DeleteStart, DeleteEnd + 1, strlen(DeleteEnd));
	    }
	}
	++p;
    }

    return(ReturnCode);
}

