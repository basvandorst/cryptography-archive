/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	Translators.h,v 1.3 1997/10/22 23:06:00 elrod Exp
____________________________________________________________________________*/

#ifndef Included_Translators_h	/* [ */
#define Included_Translators_h

#include "PGPPlugTypes.h"

PluginError CanPerformTranslation(
	short trans_id, 
	emsMIMEtypeP in_mime, 
	long context
	);

PluginError PerformTranslation(
	short trans_id, 
	char* in_file, 
	char* out_file, 
	char** addresses, 
	emsMIMEtypeP  in_mime,
	emsMIMEtypeP* out_mime
	);


#endif /* ] Included_Translators_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

