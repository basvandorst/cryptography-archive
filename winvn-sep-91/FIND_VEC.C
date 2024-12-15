/*
 * SRCLIB\NETLIB\FIND_VEC.C
 *
 * Copyright (C) 1986,1987,1988 by FTP Software, Inc.
 * 
 * This software is furnished under a license and may be used and copied
 * only in accordance with the terms of such license and with the
 * inclusion of the above copyright notice. This software or any other
 * copies thereof may not be provided or otherwise made available to any
 * other person. No title to and ownership of the software is hereby
 * transferred.
 * 
 * The information in this software is subject to change without notice
 * and should not be construed as a commitment by FTP Software, Inc.
 *
 * 
 * EDIT HISTORY:
 * 05-Feb-88	jog	_find_vec now calls vec_search to do work previous
 * 			 versions of _find_vec did itself. vec_search returns
 * 			 vector # or 0 on failure. Functionality of _find_vec 
 * 			 is not changed.
 * 19-Feb-88	jbvb	Change message for HP 
*/

#include <pctcp/types.h>
#include <pctcp/pctcp.h>

extern int vec_search();   /* routine to search interrupt table */

_find_vec()
    { int vecnum;            /* = vector number or 0(on failure) */
   
      vecnum = vec_search(); /* look for packet driver vector    */
      
      if (vecnum != 0)       /* if found vector, return vector # */ 
          return (vecnum);
  
      /* else, not found */
#ifdef HP
      printf("The ARPA Services/Vectra resident module is not loaded.\n");
      printf("You should run \"netload\" and try the application again.\n");
#else
      printf("PC\TCP resident module is not loaded; aborting program...\n");
#endif
      exit(255);
    }
