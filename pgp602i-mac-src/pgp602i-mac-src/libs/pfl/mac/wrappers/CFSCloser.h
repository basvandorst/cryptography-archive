/*____________________________________________________________________________	CFSCloser.h		Copyright (C) 1997 Network Associates, Inc. and its affiliates.	All rights reserved.			$Id: CFSCloser.h,v 1.2.34.1 1998/11/12 03:20:05 heller Exp $____________________________________________________________________________*/#pragma onceclass CFSCloser{protected:	short	mFileRef;	public:	CFSCloser( short fileRef )			{ mFileRef	= fileRef; }				~CFSCloser()			{ if ( mFileRef > 0 )	FSClose( mFileRef ); }};