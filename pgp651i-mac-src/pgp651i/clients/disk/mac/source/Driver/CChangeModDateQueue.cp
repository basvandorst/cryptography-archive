/*____________________________________________________________________________	Copyright (C) 1996-1998 Network Associates Inc. and affiliated companies.	All rights reserved.		$Id: CChangeModDateQueue.cp,v 1.5 1999/03/10 02:32:03 heller Exp $____________________________________________________________________________*/#pragma once#include "MacFiles.h"#include "CChangeModDateQueue.h"/*_________________________________________________________________________	Change the modification date of the specified file asynchronously_________________________________________________________________________*/	voidCChangeModDateQueue::ChangeModDate(	const FSSpec *	spec,	ulong 			newModificationDate )	{	QueuePB *			qpb;	ModPB *				modPB;		qpb	= GetPB();		modPB	= (ModPB *)QueuePBToCommonPBHeader( qpb );		modPB->spec				= *spec;	modPB->newModificationDate	= newModificationDate;	modPB->isGetCatInfo		= TRUE;		modPB->cpb.hFileInfo.ioVRefNum		= modPB->spec.vRefNum;	modPB->cpb.hFileInfo.ioDirID		= modPB->spec.parID;	modPB->cpb.hFileInfo.ioNamePtr		= modPB->spec.name;	modPB->cpb.hFileInfo.ioFDirIndex	= 0;		(void)PBGetCatInfoAsync( &modPB->cpb );	}/*_________________________________________________________________________	Override standard completion routine_________________________________________________________________________*/	voidCChangeModDateQueue::Completion( QueuePB *qpb )	{	ModPB *	modPB;	modPB	= (ModPB *)QueuePBToCommonPBHeader( qpb );		AssertNoErr( modPB->cpb.hFileInfo.ioResult,			"CChangeModDateQueue::Completion" );		if ( modPB->isGetCatInfo )		{		// this is the completion for PBGetCatInfoAsync		// reuse same PB for PBSetCatInfoAsync		modPB->isGetCatInfo	= false;				modPB->cpb.hFileInfo.ioVRefNum			= modPB->spec.vRefNum;		modPB->cpb.hFileInfo.ioDirID			= modPB->spec.parID;		modPB->cpb.hFileInfo.ioNamePtr			= modPB->spec.name;		cpbModificationDate( &modPB->cpb )		= modPB->newModificationDate;				(void)PBSetCatInfoAsync( &modPB->cpb );		}	else		{		// this is the completion for PBSetCatInfoAsync--all done		inherited::Completion( qpb );		}	}	