/*____________________________________________________________________________
	EudoraTypes.h
	
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: EudoraTypes.h,v 1.1 1997/07/25 23:47:24 lloyd Exp $
____________________________________________________________________________*/

#ifdef __cplusplus
extern "C" {
#endif

#include "emsapi-mac.h"
#include "usertrans.h"

#ifdef __cplusplus
}
#endif

typedef emsMIMEparam			emsMIMEParam;
typedef emsMIMEParam **			emsMIMEParamHandle;
typedef const emsMIMEParam **	ConstemsMIMEParamHandle;

typedef emsMIMEtype 			emsMIMEType;
typedef emsMIMEType **			emsMIMETypeHandle;
typedef const emsMIMEType **	ConstemsMIMETypeHandle;
