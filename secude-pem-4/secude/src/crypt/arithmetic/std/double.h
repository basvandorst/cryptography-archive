/*
 *  SecuDE Release 4.3 (GMD)
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

/* get byte order routines ntohl, ntohs, htonl, htons from <netinet/in.h> */
#include <sys/types.h>
#include <netinet/in.h>

typedef union {
	struct {
#if defined(ntohl)
		unsigned short  h_part, l_part;
#else
		unsigned short  l_part, h_part;
#endif
	}               hw;
	L_NUMBER        ln;
}               Word;

#define HSW(x)  (x).hw.h_part
#define LSW(x)  (x).hw.l_part
#define W(x)    (x).ln
