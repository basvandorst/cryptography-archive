/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: pflTypes.h,v 1.4.16.1 1998/11/12 03:17:48 heller Exp $
____________________________________________________________________________*/
#ifndef Included_pflTypes_h	/* [ */
#define Included_pflTypes_h



/* opaque declarations */
typedef struct PFLFileSpec *		PFLFileSpecRef;
typedef struct PFLFileSpec const *	PFLConstFileSpecRef;

typedef struct PFLDirectoryIter *		PFLDirectoryIterRef;
typedef struct PFLDirectoryIter const *	PFLConstDirectoryIterRef;

/* Validity checks */
#define	kInvalidPFLFileSpecRef			((PFLFileSpecRef) NULL)
#define	kInvalidPFLDirectoryIterRef		((PFLDirectoryIterRef) NULL)

#define PFLFileSpecRefIsValid( ref )		( (ref) != kInvalidPFLFileSpecRef )
#define PFLDirectoryIterRefIsValid( ref )	\
			( (ref) != kInvalidPFLDirectoryIterRef )

#endif /* ] Included_pflTypes_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/