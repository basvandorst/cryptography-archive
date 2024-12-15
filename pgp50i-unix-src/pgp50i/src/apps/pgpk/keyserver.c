/*
 * keyserver.c
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: keyserver.c,v 1.1.2.4.2.2 1997/08/19 21:00:05 quark Exp $
 */

#include "config.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include <netdb.h>

#include "pgpKeyDB.h"
#include "pgpMem.h"
#include "pgpEnv.h"
#include "pgpOutput.h"
#include "pgpUserIO.h"

#include "keyserver.h"

struct _error_table {
    const char *ServerMessage;
    const char *OutputMessage;
    Boolean IsError;
}ErrorTable[] =
{
    {"Key block corrupt: pubkey decode failed",
     "Key block invalid at server.\n", TRUE},
    {"Key block added to key server database.\n  New public keys added: ",
     "%i public key(s) added.\n", FALSE},
    {"Key block in add request contained no new\n\
keys, userid's, or signatures.",
     "The keyserver already contained all the keys, userids and signatures.\n",
     FALSE},
    {"\0", 0, 0}
};

struct _error_string_table {
    unsigned long RawCode;
    unsigned long StringID;
}ErrToStringTable[] =
{
#if 0
/*The commented out errors are internal errors or problems we shouldn't see
 *(ie, if IDS_ERR_UNABLE_TO_LOAD_LIBRARY happened, we wouldn't be loaded to
 *display the error.)
 */
    {ERR_UNABLE_TO_LOAD_LIBRARY,        IDS_ERR_UNABLE_TO_LOAD_LIBRARY},
/*  {ERR_NO_FUNCS_AVAILABLE,            IDS_ERR_NO_FUNCS_AVAILABLE}, */
/*  {ERR_LIBRARY_NOT_LOADED,            IDS_ERR_LIBRARY_NOT_LOADED}, */
/*  {ERR_BAD_PARAM,                     IDS_ERR_BAD_PARAM}, */
/*  {ERR_FUNCTION_NOT_IN_LIB,           IDS_ERR_FUNCTION_NOT_IN_LIB}, */
    {ERR_USER_NOT_FOUND,                IDS_ERR_USER_NOT_FOUND},
    {ERR_USER_CANCEL,                   0}, /*No bonus boxes!*/
    {ERR_KEYRING_FAILURE,               IDS_ERR_KEYRING_FAILURE},
    {ERR_MALLOC_FAILURE,                IDS_ERR_MALLOC_FAILURE},
    {ERR_DATA_WAIT_TIMEOUT,             IDS_ERR_DATA_WAIT_TIMEOUT},
    {ERR_SOCKET_ERROR,                  IDS_ERR_SOCKET_ERROR},
    {ERR_HOST_NOT_FOUND,                IDS_ERR_HOST_NOT_FOUND},
    {ERR_UNABLE_TO_OPEN_SOCKET,         IDS_ERR_UNABLE_TO_OPEN_SOCKET},
    {ERR_MANY_NOT_FOUND,                IDS_ERR_MANY_NOT_FOUND},
    {ERR_NO_RESPONSE_FROM_SERVER,       IDS_ERR_UNKNOWN_SERVER_RESPONSE},
#endif
    {0,                                 0}
};

static unsigned long ReceiveBuffer(char **buffer,
				   int sockfd);
static unsigned long ProcessSendReturn(char *buffer);
static void DisplayKSError(unsigned long ReturnCode);
static int UrlEncode(char **Dest, char *Source);
static void ConvertKeyIDToString (char *sz);
static unsigned long AddKeySetWithConfirm(PGPKeySet *pSourceKeySet,
					  PGPKeySet *pDestKeySet,
					  unsigned long *NumKeysFound);
static int MassageUserId(char **UserId, char *Start, char *End);
static int PGPkeyserverAddKeyBuffer(char *Buffer,
				    PGPKeySet *pKeyRings,
				    unsigned long *NumKeysFound);

int PGPkeyserverGetAndAddKeys(char *UserId,
			      PGPKeySet *pAddFromKeyRing,
			      PGPKeySet *pKeyRings,
			      unsigned long *NumKeysFound,
			      char *keyserver,
			      unsigned short port)
{
    char *KeyBuffer = NULL;
    int ReturnCode = PGPERR_OK;
    char *Start, *End, *IntUserId = NULL;
    unsigned long NumKeys = 0, NumNotFound = 0;

    assert(UserId || pKeyRings);
    assert(NumKeysFound);

    *NumKeysFound = 0;

    if(UserId) {
	Start = strchr(UserId, '\n');
	while(Start && *Start) {
	    ++NumKeys;
	    Start = strchr(Start, '\n');
	    if(Start)
		++Start;
	}

	if(!NumKeys)
	    NumKeys = 1;

	Start = End = UserId;
	while(Start && *Start && (ReturnCode == PGPERR_OK)) {
	    while(End && *End && *End != '\n') {
		++End;
	    }

	    if(MassageUserId(&IntUserId, Start, End) == PGPERR_OK) {
	        StatusOutput(TRUE, "GETTING_KEY_FOR", IntUserId);
		ReturnCode = GetKeyInfoHkp(keyserver,
					   port,
					   &KeyBuffer,
					   IntUserId);
		if(ReturnCode == PGPERR_OK) {
		    ReturnCode = PGPkeyserverAddKeyBuffer(KeyBuffer,
							  pKeyRings,
							  NumKeysFound);
		    if(KeyBuffer)
			pgpFree(KeyBuffer);
		}
		else
		{
		    if(ReturnCode != ERR_USER_NOT_FOUND)
			DisplayKSError(ReturnCode);
		    else
			++NumNotFound;
		}
	    }
	
	    if(IntUserId) {
		pgpFree(IntUserId);
		IntUserId = NULL;
	    }

	    while(End && *End && *End == '\n')
		++End;

	    Start = End;
	}

	switch(NumNotFound) {
	    case 0:
		break;
		
	    case 1:
		DisplayKSError(ERR_USER_NOT_FOUND);
		break;

	    default:
		DisplayKSError(ERR_MANY_NOT_FOUND);
		break;
	}
    }
    else
    {
	if(pKeyRings && pAddFromKeyRing) {
	    PGPKeyIter* pKeyIter = NULL;
	    PGPKey* pKey = NULL;
	    PGPKeyList* pKeyList = NULL;
	    PGPKeySet *pReceivedSet = NULL;
	    char KeyIdBuf[64];
	    unsigned int KeyIdSize = sizeof(KeyIdBuf);
	    char *MultiKeyBuffer = NULL;
	    static int count = 0;

	    pKeyList = pgpOrderKeySet (pAddFromKeyRing, kPGPUserIDOrdering);
	    if(pKeyList) {
		pKeyIter = pgpNewKeyIter (pKeyList);
		
		if(pKeyIter) {
		    pKey = pgpKeyIterNext(pKeyIter);
		
		    while (pKey && (ReturnCode == PGPERR_OK)) {
			pgpGetKeyString(pKey,
					kPGPKeyPropKeyId,
					KeyIdBuf,
					(size_t *) &KeyIdSize);
			ConvertKeyIDToString(KeyIdBuf);
			StatusOutput(TRUE, "GETTING_KEY_FOR", KeyIdBuf);
			ReturnCode = GetKeyInfoHkp(keyserver,
						   port,
						   &KeyBuffer,
						   KeyIdBuf);
			if(ReturnCode == PGPERR_OK) {
			    if(KeyBuffer) {
				if(!MultiKeyBuffer) {
				    MultiKeyBuffer =
					pgpAlloc((strlen(KeyBuffer) + 1) *
					       sizeof(char));
				    *MultiKeyBuffer = '\0';
				}
				else {
				    pgpRealloc((void **) &MultiKeyBuffer,
					       (strlen(KeyBuffer) +
						strlen(MultiKeyBuffer) + 1) *
					       sizeof(char));
				}
				if(MultiKeyBuffer)
				    strcat(MultiKeyBuffer, KeyBuffer);
				
				pgpFree(KeyBuffer);
			    }
			}
			else {
			    if(ReturnCode == ERR_USER_NOT_FOUND)
				++NumNotFound;
			    else
				DisplayKSError(ReturnCode);
			}
			pKey = pgpKeyIterNext(pKeyIter);
		    }
		    pgpFreeKeyIter(pKeyIter);
		}
		pgpFreeKeyList(pKeyList);
		
		switch(NumNotFound) {
		    case 0:
			break;

		    case 1:
			DisplayKSError(ERR_USER_NOT_FOUND);
			break;

		    default:
			DisplayKSError(ERR_MANY_NOT_FOUND);
			break;
		}

		if(MultiKeyBuffer) {
		    pReceivedSet =
			pgpImportKeyBuffer((unsigned char *) MultiKeyBuffer,
					   strlen(MultiKeyBuffer));
		
		    ++count;
		    if(pReceivedSet) {
			ReturnCode = AddKeySetWithConfirm(pReceivedSet,
							  pKeyRings,
							  NumKeysFound);
			pgpFreeKeySet(pReceivedSet);
		    }
		    pgpFree(MultiKeyBuffer);
		}
	    }
	}
	else
	    ReturnCode = ERR_BAD_PARAM;
    }

    return(ReturnCode);
}

int GetKeyInfoHkp(char *keyserver,
		  unsigned short port,
		  char **KeyBuffer,
		  char *UserId)
{
    struct hostent *pHostEnt;
    struct sockaddr_in server_info;
    struct in_addr ip;
    int sockfd;
    char buffer[8192];
    int ReturnCode = PGPERR_OK;

    StatusOutput(TRUE, "LOOKING_UP_HOST", keyserver);

    if((pHostEnt = gethostbyname(keyserver))) {
	StatusOutput(TRUE, "ESTABLISHING_CONNECTION");
	memcpy(&ip, pHostEnt->h_addr, sizeof(ip));
	server_info.sin_addr = ip;
	server_info.sin_port = htons(port);
	server_info.sin_family = pHostEnt->h_addrtype;
#if 0
	if((sockfd =
	    socket(pHostEnt->h_addrtype, SOCK_STREAM, 0)))
#endif
	if((sockfd =
	    socket(server_info.sin_family, SOCK_STREAM, 0)) >= 0) {
	    if(!connect(sockfd,
			(struct sockaddr *) & server_info,
			sizeof(server_info))) {
		StatusOutput(TRUE, "SENDING_REQUEST");
		sprintf(buffer,
			"GET /pks/lookup?op=get&exact=on&search=%s HTTP/1.0\n",
			UserId);
		strcat(buffer, "User-Agent: PGP/5.0 (Unix)\n\r\n\r");
		send(sockfd, buffer, strlen(buffer), 0);
		StatusOutput(TRUE, "RECEIVING_DATA");
		ReturnCode =
		    ReceiveBuffer(KeyBuffer, sockfd);
		StatusOutput(TRUE, "CLEANING_UP");

		if(ReturnCode == PGPERR_OK) {
		    if(strstr(*(KeyBuffer), "BEGIN PGP"))
			ReturnCode = PGPERR_OK;
		    else
			ReturnCode = ERR_USER_NOT_FOUND;
		    StatusOutput(TRUE, "COMPLETE");
		}
	    }
	    else
		ReturnCode = ERR_UNABLE_TO_OPEN_SOCKET;
	    close(sockfd);
	}
	else
	    ReturnCode = ERR_UNABLE_TO_OPEN_SOCKET;
    }
    else
	ReturnCode = ERR_HOST_NOT_FOUND;

    return(ReturnCode);
}

int PutKeyInfoHkp(char *keyserver,
	       unsigned short port,
	       char *KeyBuffer)
{
    struct hostent *pHostEnt;
    struct in_addr ip;
    struct sockaddr_in server_info;
    int sockfd;
    char *buffer, *ErrBuffer = NULL;
    char *TempBuffer;
    int ReturnCode = PGPERR_OK;

    StatusOutput(TRUE, "LOOKING_UP_HOST", keyserver);

    if((pHostEnt = gethostbyname(keyserver))) {
	StatusOutput(TRUE, "ESTABLISHING_CONNECTION");
	memcpy(&ip, pHostEnt->h_addr, sizeof(ip));
	server_info.sin_addr = ip;
	server_info.sin_port = htons(port);
	server_info.sin_family = pHostEnt->h_addrtype;
	
	if((sockfd = socket(server_info.sin_family, SOCK_STREAM, 0)) >= 0) {
	    if(!connect(sockfd,
			(struct sockaddr *) & server_info,
			sizeof(server_info))) {
		StatusOutput(TRUE, "SENDING_KEY");
		UrlEncode(&TempBuffer, KeyBuffer);
		if(TempBuffer) {
		    buffer =
			pgpAlloc((strlen(TempBuffer) + 512) * sizeof(char));
		    if(buffer) {
			sprintf(buffer,
				"POST /pks/add HTTP/1.0\nUser-Agent:\
PGP/5.0 (Unix)\nContent-length: %i\n\nkeytext=",
				strlen(TempBuffer) + 8);
			strcat(buffer, TempBuffer);
			send(sockfd, buffer, strlen(buffer), 0);
			StatusOutput(TRUE, "RECEIVING_RESPONSE");
			pgpFree(buffer);
			ReturnCode =
			    ReceiveBuffer(&ErrBuffer, sockfd);
			StatusOutput(TRUE, "CLEANING_UP");
		    }
		    pgpFree(TempBuffer);
		}
	    }
	    else
		ReturnCode = ERR_UNABLE_TO_OPEN_SOCKET;
	    close(sockfd);
	}
	else
	    ReturnCode = ERR_UNABLE_TO_OPEN_SOCKET;
    }
    else {
	ReturnCode = ERR_HOST_NOT_FOUND;
    }

    if(ErrBuffer) {
	ProcessSendReturn(ErrBuffer);
	pgpFree(ErrBuffer);
    }
    else {
	if (ReturnCode == PGPERR_OK) {
	    ReturnCode = ERR_NO_RESPONSE_FROM_SERVER;
	}
    }

    if(ReturnCode != PGPERR_OK)
	DisplayKSError(ReturnCode);

    return(ReturnCode);
}

int GetKeyInfoHttp(char *keyserver,
		   unsigned short port,
		   char *file,
		   char **KeyBuffer)
{
    struct hostent *pHostEnt;
    struct sockaddr_in server_info;
    struct in_addr ip;
    int sockfd;
    char buffer[8192];
    int ReturnCode = PGPERR_OK;

    StatusOutput(TRUE, "LOOKING_UP_HOST", keyserver);

    if((pHostEnt = gethostbyname(keyserver))) {
	StatusOutput(TRUE, "ESTABLISHING_CONNECTION");
	memcpy(&ip, pHostEnt->h_addr, sizeof(ip));
	server_info.sin_addr = ip;
	server_info.sin_port = htons(port);
	server_info.sin_family = pHostEnt->h_addrtype;
#if 0
	if((sockfd =
	    socket(pHostEnt->h_addrtype, SOCK_STREAM, 0)))
#endif
	if((sockfd =
	    socket(server_info.sin_family, SOCK_STREAM, 0)) >= 0) {
	    if(!connect(sockfd,
			(struct sockaddr *) & server_info,
			sizeof(server_info))) {
		StatusOutput(TRUE, "SENDING_REQUEST");
		sprintf(buffer,
			"GET %s HTTP/1.0\n",
			file);
		strcat(buffer, "User-Agent: PGP/5.0 (Unix)\n\r\n\r");
		send(sockfd, buffer, strlen(buffer), 0);
		StatusOutput(TRUE, "RECEIVING_DATA");
		ReturnCode =
		    ReceiveBuffer(KeyBuffer, sockfd);
		StatusOutput(TRUE, "CLEANING_UP");

		if(ReturnCode == PGPERR_OK) {
		    if(strstr(*(KeyBuffer), "BEGIN PGP"))
			ReturnCode = PGPERR_OK;
		    else
			ReturnCode = ERR_USER_NOT_FOUND;
		    StatusOutput(TRUE, "COMPLETE");
		}
	    }
	    else
		ReturnCode = ERR_UNABLE_TO_OPEN_SOCKET;
	    close(sockfd);
	}
	else
	    ReturnCode = ERR_UNABLE_TO_OPEN_SOCKET;
    }
    else
	ReturnCode = ERR_HOST_NOT_FOUND;

    return(ReturnCode);
}


int GetKeyInfoFinger(char *keyserver,
		     unsigned short port,
		     char *user,
		     char **KeyBuffer)
{
    struct hostent *pHostEnt;
    struct sockaddr_in server_info;
    struct in_addr ip;
    int sockfd;
    char buffer[8192];
    int ReturnCode = PGPERR_OK;

    StatusOutput(TRUE, "LOOKING_UP_HOST", keyserver);

    if((pHostEnt = gethostbyname(keyserver))) {
	StatusOutput(TRUE, "ESTABLISHING_CONNECTION");
	memcpy(&ip, pHostEnt->h_addr, sizeof(ip));
	server_info.sin_addr = ip;
	server_info.sin_port = htons(port);
	server_info.sin_family = pHostEnt->h_addrtype;
#if 0
	if((sockfd =
	    socket(pHostEnt->h_addrtype, SOCK_STREAM, 0)))
#endif
	if((sockfd =
	    socket(server_info.sin_family, SOCK_STREAM, 0)) >= 0) {
	    if(!connect(sockfd,
			(struct sockaddr *) & server_info,
			sizeof(server_info))) {
		StatusOutput(TRUE, "SENDING_REQUEST");
		sprintf(buffer,
			"%s\n",
			user);
		send(sockfd, buffer, strlen(buffer), 0);
		StatusOutput(TRUE, "RECEIVING_DATA");
		ReturnCode =
		    ReceiveBuffer(KeyBuffer, sockfd);
		StatusOutput(TRUE, "CLEANING_UP");

		if(ReturnCode == PGPERR_OK) {
		    if(strstr(*(KeyBuffer), "BEGIN PGP"))
			ReturnCode = PGPERR_OK;
		    else
			ReturnCode = ERR_USER_NOT_FOUND;
		    StatusOutput(TRUE, "COMPLETE");
		}
	    }
	    else
		ReturnCode = ERR_UNABLE_TO_OPEN_SOCKET;
	    close(sockfd);
	}
	else
	    ReturnCode = ERR_UNABLE_TO_OPEN_SOCKET;
    }
    else
	ReturnCode = ERR_HOST_NOT_FOUND;

    return(ReturnCode);
}

static int PGPkeyserverAddKeyBuffer(char *Buffer,
				    PGPKeySet *pKeyRings,
				    unsigned long *NumKeysFound)
{
    unsigned long ReturnCode = ERR_KEYRING_FAILURE;
    PGPKeySet *lRingSet;
    PGPKeySet *NewKeySet;
    assert(Buffer);

    assert(NumKeysFound);

    pgpLibInit();

    if(!pKeyRings) {
	lRingSet = pgpOpenDefaultKeyRings(TRUE, NULL);
    }
    else {
	lRingSet = pKeyRings;
    }

    if(lRingSet) {
	NewKeySet =
	    pgpImportKeyBuffer((unsigned char *) Buffer, strlen(Buffer));

	if(NewKeySet) {
	    ReturnCode =
		AddKeySetWithConfirm(NewKeySet, lRingSet, NumKeysFound);
	    pgpFreeKeySet(NewKeySet);
	}
	if(!pKeyRings) {
	    pgpFreeKeySet(lRingSet);
	}
    }
    pgpLibCleanup();

    return(ReturnCode);
}

static int MassageUserId(char **UserId, char *Start, char *End) {
    int ReturnCode = PGPERR_OK;
    char *LeftAngle = NULL, *RightAngle = NULL, *p;

    assert(UserId);
    assert(Start);
    assert(End);

    if(UserId && Start && End) {
	/*Alright, is it a "John Doe <jdoe@pgp.com>" kinda address?*/
	*UserId = pgpAlloc(End - Start + 1);
	strncpy(*UserId, Start, End - Start);
	*(*UserId + (End - Start)) = '\0';

	if((LeftAngle = strchr(*UserId, '<')))
	    RightAngle = strchr(LeftAngle, '>');
	
	if(LeftAngle && RightAngle) {
	    *RightAngle = '\0';
	    memmove(*UserId, LeftAngle + 1, RightAngle - LeftAngle);
	}

	p = *UserId;
	while(p && *p) {
	    if(*p == ' ')
		*p = '+';

	    ++p;
	}
    }
    else
	ReturnCode = ERR_BAD_PARAM;

	return(ReturnCode);
}

static unsigned long ReceiveBuffer(char **buffer,
				   int sockfd)
{
    unsigned long CharsReceived = 0, bufferSize = 1, ReturnCode = PGPERR_OK;
    char TempBuffer[64];

    assert(buffer);

    *buffer = NULL;

    do {
	CharsReceived =
	    recv(sockfd, TempBuffer, sizeof(TempBuffer) - 1, 0);
	if(CharsReceived) {
	    *(TempBuffer + CharsReceived) = '\0';
	    bufferSize += CharsReceived;
	    if(*buffer) {
		pgpRealloc((void **) buffer,
			   bufferSize * sizeof(char));
	    }
	    else {
		*buffer =
		    pgpAlloc(bufferSize * sizeof(char));
		**buffer = '\0';
	    }
	    strcat(*buffer, TempBuffer);
	}
    }while(CharsReceived);

    return(ReturnCode);
}

static unsigned long ProcessSendReturn(char *buffer)
{
    char *p;
    int x;
    const char *Message = NULL;
    char OutputMessage[1024];
    char *TempBuffer;
    Boolean IsError = TRUE, FoundError = FALSE;
    unsigned long ReturnCode = PGPERR_OK;

    assert(buffer);

    TempBuffer = pgpAlloc((strlen(buffer) + 1) * sizeof(char));
    if(TempBuffer) {
	strcpy(TempBuffer, buffer);
	p = TempBuffer;
	/*The first step is to skip over the HTTP headers.*/
	p = strstr(buffer, "\r\n\r\n");
	if(p) {
	    /*Next, skip the title*/
	    p = strstr(p, "<p>\r\n");
	    if(p) {
		++p;
		/*Next, skip the header (<h1>)*/
		p = strstr(p, "<p>\r\n");
		if(p) {
		    /*Last, jump the <p>\r\n we just found*/
		    p += 5;
		    /*Sometimes, it has a "<pre>...</pre>" tag, so let's
		     *clean those up.  Ideally, if we ever have to show this
		     *to the user, we don't want the raw HTML to pop out.
		     */
		    if(strncmp(p, "<pre>", 5) == 0) {
			char *p2;

			p += 5;
			p2 = strstr(p, "</pre>");
			if(p2)
			    *p2 = '\0';
		    }
		    while(isspace(*p))
			++p;
		    /*God willing and the creek don't rise, p now points to
		     *the actual error message.  See if we can look it up...*/

		    for(x = 0;
			ErrorTable[x].ServerMessage[0] && !FoundError;
			++x) {
			if(strncmp(ErrorTable[x].ServerMessage,
				   p,
				   strlen(ErrorTable[x].ServerMessage)) == 0) {
			    FoundError = TRUE;
			    Message = ErrorTable[x].OutputMessage;
			    IsError = ErrorTable[x].IsError;
			}
		    }

		    /*It's a known return; look it up*/
		    if(Message) {
			if(*Message == '%') {
			    int NumAdded = 0;

			    while(p && *p && !isdigit(*p))
				++p;
			
			    NumAdded = atoi(p);
			
			    sprintf(OutputMessage, Message, NumAdded);
			}
			else
			    strcpy(OutputMessage, Message);
		    }
		    else {
			if(*p)
			    strcpy(OutputMessage, p);
			else {
			    strcpy(OutputMessage,
				   "Unknown server response.\n");
			}
		    }
		}

		ErrorOutputString(TRUE, LEVEL_SEVERE, OutputMessage);
		pgpFree(TempBuffer);
	    }
	}
	else
	    ReturnCode = ERR_MALLOC_FAILURE;
    }
    else
	ReturnCode = ERR_MALLOC_FAILURE;

    return(ReturnCode);
}

static void DisplayKSError(unsigned long ReturnCode)
{
    int x;
    unsigned long StringToLoad = 0;

    for(x = 0; ErrToStringTable[x].RawCode && !StringToLoad; ++x) {
	if(ErrToStringTable[x].RawCode == ReturnCode)
	    StringToLoad = ErrToStringTable[x].StringID;
    }

    if(StringToLoad) {
/*XXX This needs to be fixed*/
#if 0
	xLoadString(g_hinst, StringToLoad, OutputBuf, sizeof(OutputBuf));
	xLoadString(g_hinst, IDS_GENERAL_ERROR, Title, sizeof(Title));
	MessageBox(hwnd, OutputBuf, Title, MB_OK | MB_ICONERROR);
#endif
    }
}

static int UrlEncode(char **Dest, char *Source)
{
    int ReturnCode = PGPERR_OK;
    char *pSource, *pDest;

    /*Absolute worst case scenario is three times the source size.  Rather
     *than getting too precise, we'll allocate that much initially, and then
     *realloc it down to actuality later.
     */

    assert(Source);
    assert(Dest);

    if(Source && Dest) {
	if((*Dest = pgpAlloc((sizeof(char)) * ((strlen(Source) * 3) + 1)))) {
	    pSource = Source;
	    pDest = *Dest;
	
	    while(pSource && *pSource) {
		/*Zeroth case:  it's an alphabetic or numeric character*/
		if(!isalpha(*pSource) &&
		   !isdigit(*pSource) &&
		   *pSource != '-') {
		    /*First case:  Turn spaces into pluses.*/
		    if(*pSource == ' ') {
			*pDest = '+';
		    }
		    else {
			/*This is overkill, but works for our purposes*/
			*pDest = '%';
			++pDest;
			sprintf(pDest, "%02X", *pSource);
			++pDest;
		    }
		}
		else
		    *pDest = *pSource;
		
		++pDest;
		++pSource;
	    }

	    *pDest = '\0';
	
	}
	else
	    ReturnCode = ERR_MALLOC_FAILURE;
    }
    else
	ReturnCode = ERR_BAD_PARAM;

    return(ReturnCode);
}

/*Stolen from PBJ*/
static void ConvertKeyIDToString(char *sz)
{
    int i;
    unsigned long ulSecond;

    /*convert second half of keyid to little endian*/
    ulSecond = 0;
    for (i=0; i<4; i++) {
	ulSecond <<= 8;
	ulSecond |= ((unsigned long)sz[i+4] & 0xFF);
    }
    /* convert to string */
    sprintf(sz, "0x%08lX", ulSecond);
}

static unsigned long AddKeySetWithConfirm(PGPKeySet *pSourceKeySet,
					  PGPKeySet *pDestKeySet,
					  unsigned long *NumKeysFound)
{
    unsigned long ReturnCode = ERR_KEYRING_FAILURE;
    long NumKeys;
    PGPError error;

    assert(NumKeysFound);

    /*We assume NumKeysFound is set to zero on the first call*/
    NumKeys = pgpCountKeys(pSourceKeySet);
    *NumKeysFound += NumKeys;
			
    if(NumKeys == 1) {
	InformationOutput(TRUE, "ONE_KEY_RECEIVED");
    }
    else {
	InformationOutput(TRUE, "MANY_KEYS_RECEIVED", NumKeys);
    }

    /* XXX Probably aught to actually ask the user, here...*/

    error = pgpAddKeys(pDestKeySet, pSourceKeySet);
    if(error == PGPERR_OK) {
	error = pgpCommitKeyRingChanges(pDestKeySet);
			
	if(error == PGPERR_OK) {
	    ReturnCode = PGPERR_OK;
	}
    }

    return(ReturnCode);
}

int GetKeyInfo(char *protocol,
	       char *server,
	       char *file,
	       int port,
	       char **KeyBuffer)
{
    int GetKeyReturn = PGPERR_OK;
    /*This should probably be table-based...*/

    if(strcasecmp(protocol, "hkp") == 0) {
	GetKeyReturn =
	    GetKeyInfoHkp(server, port, KeyBuffer, file + 1);
    }
    else {
	if(strcasecmp(protocol, "http") == 0) {
	    GetKeyReturn =
		GetKeyInfoHttp(server, port, file, KeyBuffer);
	}
	else {
	    if(strcasecmp(protocol, "finger") == 0) {
		GetKeyReturn =
		    GetKeyInfoFinger(server, port, file + 1, KeyBuffer);
	    }
	    else {
		ErrorOutput(TRUE,
			    LEVEL_CRITICAL,
			    "UNKNOWN_PROTOCOL",
			    protocol);
		GetKeyReturn = -1;
	    }
	}
    }

    return(GetKeyReturn);
}

