/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	
	
	$Id: sharedmem.h,v 1.2.10.1 1997/12/05 22:15:19 mhw Exp $
____________________________________________________________________________*/

#pragma once 

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif



// Gestalt selector
enum {
	kSharedMemSelector	=	'PVTL'
};

// Possible errors
enum {
	err_Unknown			=	'Uknw',
	err_AllocationError	=	'Aloc',
	err_BlockNotFound	=	'BkNF',
	err_DuplicateName	=	'DpNm'
};

// Memory block structure
typedef struct NamedBlock{
	Str31				name;
	struct NamedBlock *	link;
	Byte				data[1];
} NamedBlockS, *NamedBlockP;


// Function declarations
SInt32 NewNamedBlock(const Str31 inName, const UInt32 inSize,
Ptr * outAddress);
SInt32 GetNamedBlock(const Str31 inName, Ptr * outAddress);
SInt32 ReleaseNamedBlock(const Str31 inName);




#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif
