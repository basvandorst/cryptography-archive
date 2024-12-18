/*____________________________________________________________________________	Copyright (C) 1997 Network Associates Inc. and affiliated companies.	All rights reserved.				$Id: sharedmem.c,v 1.4 1999/04/19 03:23:43 jason Exp $____________________________________________________________________________*/#include <Gestalt.h>#include <string.h>#include "sharedmem.h"	SInt32							// ErrorNewNamedBlock(	const Str31		inName,		// Name of block	const UInt32	inSize,		// Size of block	Ptr *			outAddress)	// Address of returned block{	SInt32			err	= noErr;	NamedBlockP *	gNamedBlocksP;		// Get the head of the list	err = Gestalt(	kSharedMemSelector,					(long *) &gNamedBlocksP);	if (err == noErr) {		NamedBlockP		theNamedBlock = *gNamedBlocksP;		// Check to make sure that a block with this name doesn't already		// exist		while (theNamedBlock != nil) {			if (memcmp(inName, theNamedBlock->name, inName[0] + 1) == 0) {				break;			} else {				theNamedBlock = theNamedBlock->link;			}		}				// Allocate new block and put it into the list		if (theNamedBlock == nil) {			theNamedBlock =				(NamedBlockP) NewPtrSys(inSize + sizeof(NamedBlockS));			if (theNamedBlock == nil) {				err = err_AllocationError;			} else {				memcpy(theNamedBlock->name, inName, inName[0] + 1);				theNamedBlock->link = *gNamedBlocksP;				*gNamedBlocksP = theNamedBlock;				if (outAddress != nil) {					*outAddress = (Ptr) theNamedBlock->data;				}			}		} else {			err = err_DuplicateName;		}	}	return err;}	SInt32						// ErrorGetNamedBlock(	const Str31	inName,		// Name of block	Ptr *		outAddress)	// Address of named block{	SInt32			err	= noErr;	NamedBlockP *	gNamedBlocksP;		// Get the head of the list	err = Gestalt(	kSharedMemSelector,					(long *) &gNamedBlocksP);		// Search for the block	if (err == noErr) {		NamedBlockP		theNamedBlock = *gNamedBlocksP;				while (theNamedBlock != nil) {			if (memcmp(inName, theNamedBlock->name, inName[0] + 1) == 0) {				break;			} else {				theNamedBlock = theNamedBlock->link;			}		}		if (theNamedBlock != nil) {			if (outAddress != nil) {				*outAddress = (Ptr) theNamedBlock->data;			}		} else {			err = err_BlockNotFound;		}	}	return err;}	SInt32					// ErrorReleaseNamedBlock(	const Str31 inName)	// Name of block{	SInt32			err	= noErr;	NamedBlockP *	gNamedBlocksP;		// Get head of list	err = Gestalt(	kSharedMemSelector,					(long *) &gNamedBlocksP);						// Release block	if (err == noErr) {		NamedBlockP		theNamedBlock = *gNamedBlocksP;		NamedBlockP		prevNamedBlock = nil;				while (theNamedBlock != nil) {			if (memcmp(inName, theNamedBlock->name, inName[0] + 1) == 0) {				break;			} else {				prevNamedBlock = theNamedBlock;				theNamedBlock = theNamedBlock->link;			}		}		if (theNamedBlock != nil) {			if (prevNamedBlock == nil) {				*gNamedBlocksP = theNamedBlock->link;			} else {				prevNamedBlock->link = theNamedBlock->link;			}			DisposePtr((Ptr) theNamedBlock);		} else {			err = err_BlockNotFound;		}	}	return err;}