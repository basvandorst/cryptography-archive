/*
* pgpTextFilt.h -- Text Filter Module
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by: Derek Atkins <warlord@MIT.EDU>
*
* This is a Public API Function Header.
*
* $Id: pgpTextFilt.h,v 1.4.2.1 1997/06/07 09:51:09 mhw Exp $
*/

#ifndef PGPTEXTFILT_H
#define PGPTEXTFILT_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPipeline;
#ifndef TYPE_PGPPIPELINE
#define TYPE_PGPPIPELINE 1
typedef struct PgpPipeline PgpPipeline;
#endif

/*
* Create a text filtering module. It will use the appropriate
* character map to map input to output characters. It will optionally
* strip ending spaces off the end of lines, if stripspace is non-zero.
* It will also convert line-endings to the appropriate type if
* crlf is non-zero. Use TEXTFILT_* to designate the line-ending
* required.
*/
struct PgpPipeline PGPExport **
pgpTextFiltCreate (struct PgpPipeline **head, byte const *map, int stripspace,
						 int crlf);

	#define PGP_TEXTFILT_NONE	0
	#define PGP_TEXTFILT_LF	1
	#define PGP_TEXTFILT_CR	2
	#define PGP_TEXTFILT_CRLF	3

#ifdef __cplusplus
}
#endif

#endif /* PGPTEXTFILT_H */
