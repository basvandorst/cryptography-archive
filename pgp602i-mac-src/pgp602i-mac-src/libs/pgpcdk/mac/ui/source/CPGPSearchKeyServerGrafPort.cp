/*____________________________________________________________________________	Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.		$Id: CPGPSearchKeyServerGrafPort.cp,v 1.3.12.1 1998/11/12 03:20:47 heller Exp $____________________________________________________________________________*/#include <LStaticText.h>#include "MacStrings.h"#include "pgpDialogs.h"#include "pgpMem.h"#include "CPGPSearchKeyServerGrafPort.h"CPGPSearchKeyServerGrafPortView::	CPGPSearchKeyServerGrafPortView(LStream *inStream)		: CPGPKeyServerGrafPortView(inStream){}CPGPSearchKeyServerGrafPortView::~CPGPSearchKeyServerGrafPortView(){}	PGPErrorCPGPSearchKeyServerGrafPortView::SetOptions(	PGPContextRef 						context,	CPGPSearchKeyServerDialogOptions 	*options){	PGPError	err = kPGPError_NoErr;		err = CPGPKeyServerGrafPortView::SetOptions( context, options );	if( IsntPGPError( err ) )	{		mOptions = options;				if( IsntNull( mKeyNameText ) &&			options->mKeyDescription[0] != 0 )		{			Str255	name;						CToPString( options->mKeyDescription, name );			mKeyNameText->SetDescriptor( name );		}	}		return( err );}