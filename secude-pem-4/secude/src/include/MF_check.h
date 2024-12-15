/*
 *  SecuDE Release 4.3 (GMD)
 */
/********************************************************************
 * Copyright (C) 1994, GMD. All rights reserved.                    *
 *                                                                  *
 *                                                                  *
 *                         NOTICE                                   *
 *                                                                  *
 *    Acquisition, use, and distribution of this module             *
 *    and related materials are subject to restrictions             *
 *    mentioned in each volume of the documentation.                *
 *                                                                  *
 ********************************************************************/

#ifndef __MFCHECK
#define __MFCHECK

#ifdef __STDC__
char	*aux_malloc	(char *proc, unsigned size);
char	*aux_calloc	(char *proc, unsigned elem, unsigned size);
char	*aux_realloc	(char *proc, void *ptr, unsigned size);
void	aux_free	(char *proc, void *ptr);
#else
char	*aux_malloc	();
char	*aux_calloc	();
char	*aux_realloc	();
void	aux_free	();
#endif


#define malloc(x) aux_malloc(proc, x)
#define calloc(x, y) aux_calloc(proc, x, y)
#define realloc(x, y) aux_realloc(proc, x, y)
#define free(x) aux_free(proc, x)
extern char *proc;
extern char MF_check;
extern int sec_debug;
#endif
