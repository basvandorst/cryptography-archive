/*____________________________________________________________________________	Copyright (C) 1996-1998 Network Associates Inc. and affiliated companies.	All rights reserved.		$Id: CDriveList.h,v 1.3 1999/03/10 02:32:57 heller Exp $____________________________________________________________________________*/#pragma onceclass CDrive;	class CDriveList	{private:	static CDriveList *	sDriveList;		ushort				GetDriveIndex( const CDrive *driveToFind );		const CDrive **		mDriveArray;	ulong				mNumDrives;							CDriveList( void );	virtual				~CDriveList( void );	public:	static CDriveList *	sGetList( void );	static OSErr		sCreateList( void );	static void			sDisposeList( void );		ushort				GetNumDrives( void )							{ return( mNumDrives ); }								CDrive *			GetIndDrive( ushort index )							{ return( (CDrive *)( mDriveArray[ index ] ) ); }								void				AddDrive( const CDrive *drive );								void				RemoveDrive( const CDrive *drive );		CDrive *			GetDriveForDriveNumber( short driveNumber );	};