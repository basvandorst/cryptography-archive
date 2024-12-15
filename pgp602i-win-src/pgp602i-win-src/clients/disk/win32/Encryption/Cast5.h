//////////////////////////////////////////////////////////////////////////////
// Cast5.h
//
// Contains declarations Cast5.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: Cast5.h,v 1.1.2.6 1998/07/06 08:56:33 nryan Exp $

// Portions Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

/*****************************************************************************
 *					C A S T 5   S Y M M E T R I C   C I P H E R
 *		Copyright (c) 1996 Northern Telecom Ltd. All rights reserved.
 *****************************************************************************
 *
 * FILE:		cast5.h
 *
 * AUTHOR(S):	R.T.Lockhart, Dept. 9C40, Nortel Ltd.
 *				C. Adams, Dept 9C21, Nortel Ltd.
 *
 * DESCRIPTION: CAST5 header file. This file defines the interface for
 * the CAST5 symmetric-key encryption/decryption/MACing code. The code
 * supports key lengths from 8 through 128 bits, in multiples of 8.
 *
 *
 * Error handling:
 * Most functions return an int that indicates the success or failure of the
 * operation. See the C5E_xxx #defines for a list of error conditions.
 *	
 * Data size assumptions: 	BYTE	- unsigned 8 bits
 *				UINT32	- unsigned 32 bits
 *
 ****************************************************************************/

#ifndef Included_Cast5_h	// [
#define Included_Cast5_h


////////
// Types
////////

// Align to 1.
#pragma pack(push, 1)

typedef union ExpandedCASTKey
{
	PGPUInt8  keyBytes[32*sizeof(PGPUInt32)];
	PGPUInt32 keyDWords[32];

} ExpandedCASTKey;
	
typedef struct CASTCFBContext
{
	ExpandedCASTKey expandedKey;

} CASTCFBContext;

typedef union CASTKey
{
	PGPUInt8	keyBytes[16];
	PGPUInt32	keyDWords[16/sizeof(PGPUInt32)];

} CASTKey, EncryptedCASTKey;

// PassphraseSalt contains salt bytes for use in a CipherContext.

typedef union PassphraseSalt
{
	PGPUInt8	saltBytes[8];
	PGPUInt32	saltDWords[8/sizeof(PGPUInt32)];

} PassphraseSalt;

// Restore alignment.
#pragma pack(pop)


/////////////////////
// Exported Functions
/////////////////////

void	CAST5schedule(PGPUInt32 xkey[32], const PGPUInt8 *k);

void	CAST5encrypt(const PGPUInt8 *in, PGPUInt8 *out, 
			const PGPUInt32 *xkey);
void	CAST5decrypt(const PGPUInt8 *in, PGPUInt8 *out, 
			const PGPUInt32 *xkey);

void	CAST5encryptCFBdbl(const PGPUInt32 *xkey, PGPUInt32 iv0, 
			PGPUInt32 iv1, PGPUInt32 iv2, PGPUInt32 iv3, 
			const PGPUInt32 *src, PGPUInt32 *dest, PGPUInt32 len);
void	CAST5decryptCFBdbl(const PGPUInt32 *xkey, PGPUInt32 iv0, 
			PGPUInt32 iv1, PGPUInt32 iv2, PGPUInt32 iv3, 
			const PGPUInt32 *src, PGPUInt32 *dest, PGPUInt32 len);

#if defined(PGP_INTEL)

void __cdecl	CAST5encryptAsm(const PGPUInt8 *in, PGPUInt8 *out, 
					const PGPUInt32 *xkey);

void __cdecl	CAST5encryptCFBdblAsm(const PGPUInt32 *xkey, PGPUInt32 iv0, 
					PGPUInt32 iv1, PGPUInt32 iv2, PGPUInt32 iv3, 
					const PGPUInt32 *src, PGPUInt32 *dest, PGPUInt32 len);
void __cdecl	CAST5decryptCFBdblAsm(const PGPUInt32 *xkey, PGPUInt32 iv0, 
					PGPUInt32 iv1, PGPUInt32 iv2, PGPUInt32 iv3, 
					const PGPUInt32 *src, PGPUInt32 *dest, PGPUInt32 len);

#endif	// PGP_INTEL

PGPBoolean	VerifyCastSBoxesValid();

#endif	// ] Included_Cast5_h
