/****************************************************************************
 *
 * Copyright (c) 1998, Network Associates, Inc. and its affiliated Companies
 *
 ****************************************************************************/

#ifndef _str2_h_
#define _str2_h_

/* file neede??? */

/*
IA5String *str2IA5String( char *str );

OCTET_STRING *str2OCTET_STRING( char *str );
*/

#define _IA5STRING(f)		(IA5String *)f
#define _OCTET_STRING(f)	(OCTET_STRING *)f

#define str2IA5String 		_IA5STRING( str2vbl_blk )
#define str2OCTET_STRING 	_OCTET_STRING( str2vbl_blk )

#endif
