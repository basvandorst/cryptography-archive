/*
 *  Filename: file1847.h
 *
 *  Last Edited: Wednesday, October 2, 1996
 *
 *  Author: Scott Manjourides
 *
 *  Copyright 1995, 1996 QUALCOMM Inc.
 *
 *  Send comments and questions to <emsapi-info@qualcomm.com>
 */

#ifndef EMS_FILE1847_H
#define EMS_FILE1847_H

#include <windows.h> /* Must be included for ems-win.h */
#include "ems-win.h"

#include "encoding.h"
#include "rfc1847.h"

enum { FILE1847_FAIL = (-1), FILE1847_ABORT = (0), FILE1847_OK = (1) };

/* ------------------------------------------------------------------------- */

/*  CreateTmpFile
 *
 *  Creates a empty, unique, temporary file in the users 'TEMP'
 *    directory.
 *
 *  No arguments.
 *
 *  Returns a pointer to the filename string. This should be
 *    considered temporary data, and should never be free()'d.
 */
char *CreateTmpFile();

/* ------------------------------------------------------------------------ */

/*  rfc1847_file_create
 *
 *  Creates RFC1847 MIME structure.
 *
 *  Args:
 *    pOutFilename     [IN]     Output filename ** WRITE-ONLY **
 *    OutMimePtr       [IN/OUT] MIME type of output -- boundary param will be added
 *    InPart1MimePtr   [IN]     MIME type of part 1
 *    InPart1CTE       [IN]     Content-transfer-encoding for part 1
 *    pInPart1Filename [IN]     Input filename for part 1 ** READ-ONLY **
 *    InPart2MimePtr   [IN]     MIME type of part 2
 *    InPart2CTE       [IN]     Content-transfer-encoding for part 2
 *    pInPart2Filename [IN]     Input filename for part 2 ** READ-ONLY **
 *    progress         [IN]     EMS progress callback function
 *
 *  Returns boolean. TRUE for success, FALSE for failure.
 */
int rfc1847_file_create(const char *pOutFilename,
				   emsMIMEtypeP OutMimePtr,

				   emsMIMEtypeP InPart1MimePtr,
				   TrEncType InPart1CTE,
				   const char *pInPart1Filename,

				   emsMIMEtypeP InPart2MimePtr,
				   TrEncType InPart2CTE,
				   const char *pInPart2Filename,

				   emsProgress progress);

/* ------------------------------------------------------------------------ */

/*  rfc1847_file_parse
 *
 *  Parses RFC1847 MIME structure.
 *
 *  Args:
 *    pInFilename       [IN]  Input filename of file to parse ** READ-ONLY **
 *    InMimePtr         [OUT] MIME type of input file
 *    pOutPart1Filename [IN]  Output filename for part 1 ** WRITE-ONLY **
 *    OutPart1MimePtr   [OUT] MIME type of created part 1
 *    bPart1DeMime      [IN]  Boolean: Remove header and transfer encoding, part 1?
 *    pOutPart2Filename [IN]  Output filename for part 2 ** WRITE-ONLY **
 *    OutPart2MimePtr   [OUT] MIME type of created part 2
 *    bPart2DeMime      [IN]  Boolean: Remove header and transfer encoding, part 2?
 *    progress          [IN]  EMS progress callback function
 *
 *  Returns boolean. TRUE for success, FALSE for failure.
 */
int rfc1847_file_parse(const char *pInFilename,
				  emsMIMEtypeP *InMimePtr,

				  const char *pOutPart1Filename,
				  emsMIMEtypeP *OutPart1MimePtr,
				  int bPart1DeMime,

				  const char *pOutPart2Filename,
				  emsMIMEtypeP *OutPart2MimePtr,
				  int bPart2DeMime,
				  
				  emsProgress progress);

#endif /* EMS_FILE1847_H */