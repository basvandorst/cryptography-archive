/*____________________________________________________________________________	Copyright (C) 1996-1998 Network Associates, Inc. and its affiliates.	All rights reserved.		$Id: CChangeModDateQueue.h,v 1.3.8.1 1998/11/12 03:05:35 heller Exp $____________________________________________________________________________*/#pragma once#include "CPBQueue.h"class CChangeModDateQueue : public CPBQueue	{protected:	typedef CPBQueue	inherited;		typedef struct		{		CInfoPBRec	cpb;	// must be first		FSSpec		spec;		ulong		newModificationDate;		Boolean		isGetCatInfo;		} ModPB;			virtual ulong	GetPBSize( void )	{ return( sizeof( ModPB ) ); }		virtual void	Completion( QueuePB *pb );	public:	void		ChangeModDate( const FSSpec *spec, ulong newModDate );	};