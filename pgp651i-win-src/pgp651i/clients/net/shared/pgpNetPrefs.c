/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: pgpNetPrefs.c,v 1.7 1999/03/28 11:49:10 wprice Exp $
____________________________________________________________________________*/

#include "pgpNetPrefs.h"
#include "pflPrefTypes.h"
#include "pgpMem.h"
#include "pgpMemoryMgr.h"
#include "pgpEndianConversion.h"

#include <string.h>
#include <stdio.h>

	PGPError
PGPGetNetHostPrefs(		
		PGPPrefRef				prefRef,
		PGPNetPrefHostEntry		**netHostList,
		PGPUInt32				*netHostCount)
{
	PGPError				err			= kPGPError_NoErr;
	PGPSize					dataLength;

	PGPValidatePtr(netHostList);
	PGPValidatePtr(netHostCount);
	*netHostList = NULL;

	err = PGPGetPrefNumber (prefRef, 
							kPGPNetPrefHostListCount, 
							netHostCount);

	if( IsntPGPError( err ) && ( *netHostCount > 0 ) )
	{
		err = PGPGetPrefData(	prefRef, 
								kPGPNetPrefHostList, 
								&dataLength, 
								netHostList);
		if( IsntPGPError( err ) &&
			( ( ( dataLength % sizeof( PGPNetPrefHostEntry ) ) != 0 ) ||
			( ( dataLength / sizeof( PGPNetPrefHostEntry ) ) != *netHostCount ) ) )
		{
			*netHostCount = 0;
		}
	}

	return err;

}


	PGPError
PGPSetNetHostPrefs(
	PGPPrefRef				prefRef,
	PGPNetPrefHostEntry		*netHostList,
	PGPUInt32				netHostCount )
{
	PGPError	err				= kPGPError_NoErr;
	PGPSize		dataLength;

	err = PGPSetPrefNumber (prefRef, 
					kPGPNetPrefHostListCount, 
					netHostCount); 

	if (IsntPGPError (err))
	{
		if ((netHostList == NULL) || (netHostCount == 0))
			PGPRemovePref (prefRef, kPGPNetPrefHostList);
		else
		{
			dataLength = netHostCount * sizeof(PGPNetPrefHostEntry);
			err = PGPSetPrefData(	prefRef, 
									kPGPNetPrefHostList, 
									dataLength, 
									netHostList);
		}
	}
	return err;
}


	PGPError
PGPGetNetIKEProposalPrefs(	
		PGPPrefRef					prefRef,
		PGPNetPrefIKEProposalEntry	**proposalList,
		PGPUInt32					*proposalCount)
{
	PGPError						err;
	PGPSize							dataLength;

	PGPValidatePtr(proposalList);
	PGPValidatePtr(proposalCount);

	err = PGPGetPrefData(	prefRef, 
							kPGPNetPrefIKEProposalList, 
							&dataLength, 
							proposalList);

	*proposalCount = dataLength / sizeof(PGPNetPrefIKEProposalEntry);

	return err;
}


	PGPError
PGPSetNetIKEProposalPrefs(
	PGPPrefRef					prefRef,
	PGPNetPrefIKEProposalEntry	*proposalList,
	PGPUInt32					proposalCount )
{
	PGPError	err;
	PGPSize		dataLength;

	PGPValidatePtr(proposalList);

	dataLength = proposalCount * sizeof(PGPNetPrefIKEProposalEntry);

	err = PGPSetPrefData(	prefRef, 
							kPGPNetPrefIKEProposalList, 
							dataLength, 
							proposalList);
	return err;
}


	PGPError
PGPGetNetIPSECProposalPrefs(	
		PGPPrefRef						prefRef,
		PGPNetPrefIPSECProposalEntry	**proposalList,
		PGPUInt32						*proposalCount)
{
	PGPError						err;
	PGPSize							dataLength;

	PGPValidatePtr(proposalList);
	PGPValidatePtr(proposalCount);

	err = PGPGetPrefData(	prefRef, 
							kPGPNetPrefIPSECProposalList, 
							&dataLength, 
							proposalList);

	*proposalCount = dataLength / sizeof(PGPNetPrefIPSECProposalEntry);

	return err;
}


		PGPError
PGPSetNetIPSECProposalPrefs(
	PGPPrefRef						prefRef,
	PGPNetPrefIPSECProposalEntry	*proposalList,
	PGPUInt32						proposalCount )
{
	PGPError	err;
	PGPSize		dataLength;

	PGPValidatePtr(proposalList);

	dataLength = proposalCount * sizeof(PGPNetPrefIPSECProposalEntry);

	err = PGPSetPrefData(	prefRef, 
							kPGPNetPrefIPSECProposalList, 
							dataLength, 
							proposalList);
	return err;
}


