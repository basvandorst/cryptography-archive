/*
 * keyserver.h
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: keyserver.h,v 1.1.2.3 1997/06/24 13:25:59 quark Exp $
 */

#ifndef _PGPKEYSERVERSUPPORT_H
#define _PGPKEYSERVERSUPPORT_H

#define KS_ERR_BASE                             4000

#define ERR_UNABLE_TO_LOAD_LIBRARY		KS_ERR_BASE + 0
#define ERR_NO_FUNCS_AVAILABLE			KS_ERR_BASE + 1
#define ERR_LIBRARY_NOT_LOADED			KS_ERR_BASE + 2
#define ERR_BAD_PARAM				KS_ERR_BASE + 3
#define ERR_FUNCTION_NOT_IN_LIB			KS_ERR_BASE + 4
#define	ERR_USER_NOT_FOUND			KS_ERR_BASE + 5
#define ERR_USER_CANCEL				KS_ERR_BASE + 6
#define	ERR_KEYRING_FAILURE			KS_ERR_BASE + 7
#define	ERR_MALLOC_FAILURE			KS_ERR_BASE + 8
#define ERR_DATA_WAIT_TIMEOUT			KS_ERR_BASE + 9
#define ERR_SOCKET_ERROR			KS_ERR_BASE + 10
#define ERR_NO_MEM                              KS_ERR_BASE + 11
#define ERR_HOST_NOT_FOUND			KS_ERR_BASE + 12
#define ERR_UNABLE_TO_OPEN_SOCKET		KS_ERR_BASE + 13
#define ERR_MANY_NOT_FOUND			KS_ERR_BASE + 14
#define ERR_NO_RESPONSE_FROM_SERVER		KS_ERR_BASE + 15

int PGPkeyserverGetAndAddKeys(char *UserId,
			      PGPKeySet *pAddFromKeyRing,
			      PGPKeySet *pKeyRings,
			      unsigned long *NumKeysFound,
			      char *keyserver,
			      unsigned short port);

int PutKeyInfoHkp(char *keyserver,
		  unsigned short port,
		  char *KeyBuffer);
int GetKeyInfoHkp(char *keyserver,
		  unsigned short port,
		  char **KeyBuffer,
		  char *UserId);
int GetKeyInfoHttp(char *keyserver,
		   unsigned short port,
		   char *file,
		   char **KeyBuffer);
int GetKeyInfoFinger(char *keyserver,
		     unsigned short port,
		     char *user,
		     char **KeyBuffer);
int GetKeyInfo(char *protocol,
	       char *server,
	       char *file,
	       int port,
	       char **KeyBuffer);

#endif //#ifdef _PGPKEYSERVERSUPPORT_H
