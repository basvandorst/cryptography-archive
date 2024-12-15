/*
 *  STAMOD Release 1.1 (GMD)
 */
/********************************************************************
 * Copyright (C) 1991, GMD. All rights reserved.                    *
 *                                                                  *
 *                                                                  *
 *                         NOTICE                                   *
 *                                                                  *
 *    Acquisition, use, and distribution of this module             *
 *    and related materials are subject to restrictions             *
 *    mentioned in each volume of the documentation.                *
 *                                                                  *
 ********************************************************************/

#ifndef _MEMFUNC_
#define _MEMFUNC_

#ifdef _DOS_
#include <memory.h>
#include <string.h>


#define bzero(b,length)     memset((char *)b,0,(unsigned)length)
#define bcopy(b1,b2,length) memmove((char *)b2,(char *)b1,(unsigned)length)

#endif

#endif
