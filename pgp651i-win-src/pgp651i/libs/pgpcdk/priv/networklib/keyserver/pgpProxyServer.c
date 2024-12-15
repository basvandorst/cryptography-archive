/*____________________________________________________________________________
	Copyright (C) 1999 Network Associates, Inc.
	All rights reserved.

	$Id: pgpProxyServer.c,v 1.5.6.1 1999/06/05 04:52:32 dgal Exp $
____________________________________________________________________________*/
#if PGP_WIN32
#include <windows.h>
#include <wininet.h>
#include <stdio.h>
#elif PGP_MACINTOSH
#include <InternetConfig.h>
#include <string.h>
#include <stdlib.h>
#endif

#include "pgpContext.h"
#include "pgpUtilities.h"
#include "pgpMem.h"
#include "pgpProxyServer.h"

#if PGP_WIN32	/* [ */

struct PGPProxyTableEntry_
{
	char *		stringMatch;
	PGPUInt16	defaultPort;
};


struct PGPProxyTableEntry_ proxyTable[] =
{
	{ NULL,		0  },
	{ "http=",	80 }
};


typedef BOOL (CALLBACK * InternetQueryOptionFuncPtr)(HINTERNET, DWORD, LPVOID, LPDWORD);

PGPError pgpGetProxyServer(PGPContextRef context, PGPProxyServerType type,
			char **proxyAddress, PGPUInt16 *proxyPort)
{
	PGPMemoryMgrRef mgr;
	HANDLE hWinInet;
	INTERNET_PROXY_INFO *proxyInfo = NULL;
	DWORD size = 1024;
	char *begin = NULL;
	char *end = NULL;
	PGPError err = kPGPError_NoErr;
	InternetQueryOptionFuncPtr	pfnInternetQueryOptionA;

	mgr = PGPGetContextMemoryMgr(context);

	hWinInet = LoadLibrary("wininet.dll");
	if (IsNull(hWinInet))
	{
		return kPGPError_NoErr;
	}

	pfnInternetQueryOptionA = (InternetQueryOptionFuncPtr) 
						GetProcAddress(hWinInet, "InternetQueryOptionA");

	if (IsNull(pfnInternetQueryOptionA))
	{
		FreeLibrary(hWinInet);
		return kPGPError_UnknownError;
	}

	proxyInfo = (INTERNET_PROXY_INFO *) PGPNewData(mgr, size, 
											kPGPMemoryMgrFlags_Clear);

	if (IsNull(proxyInfo))
		err = kPGPError_OutOfMemory;

	if (IsntPGPError(err))
	{
		if (!(*pfnInternetQueryOptionA)(NULL, INTERNET_OPTION_PROXY, proxyInfo,
				&size))
			err = kPGPError_UnknownError;
	}

	if (IsntPGPError(err))
	{
		if (IsntNull(proxyInfo->lpszProxy))
		{
			*proxyAddress = (char *) PGPNewData(mgr,
										strlen(proxyInfo->lpszProxy) + 1,
										kPGPMemoryMgrFlags_Clear);

			if (IsNull(*proxyAddress))
				err = kPGPError_OutOfMemory;
		}
	}

	if (IsntPGPError(err))
	{
		if (IsntNull(proxyInfo->lpszProxy))
		{
			begin = strstr(proxyInfo->lpszProxy, 
						proxyTable[type].stringMatch);

			if (IsntNull(begin))
			{
				begin += strlen(proxyTable[type].stringMatch);
				end = begin;
				
				while ((*end != '\0') && (*end != ' ') && (*end != ':'))
					end++;
				
				if (*end == ':')
					sscanf(end+1, "%d", proxyPort);
				else
					*proxyPort = proxyTable[type].defaultPort;

				strncpy(*proxyAddress, begin, end - begin);
				(*proxyAddress)[end - begin] = 0;
			}
			else
			{
				end = (char *) proxyInfo->lpszProxy + 
						strlen(proxyInfo->lpszProxy) - 1;

				begin = end;
				
				while ((begin != proxyInfo->lpszProxy) && (*begin != ' '))
				{
					if (*begin == ':')
						end = begin;
					begin--;
				}
				
				if (!strchr(begin, '='))
				{
					if (*begin == ' ')
						begin++;
					
					if (*end == ':')
					{
						sscanf(end+1, "%d", proxyPort);
						end--;
					}
					else
						*proxyPort = proxyTable[type].defaultPort;

					strncpy(*proxyAddress, begin, end - begin + 1);
					(*proxyAddress)[end - begin + 1] = 0;
				}
				else
				{
					PGPFreeData(*proxyAddress);
					*proxyAddress = NULL;
				}
			}
		}
		else if (IsntNull(*proxyAddress))
		{
			PGPFreeData(*proxyAddress);
			*proxyAddress = NULL;
		}
	}

	if (IsntNull(proxyInfo))
		PGPFreeData(proxyInfo);

	if (IsntNull(hWinInet))
		FreeLibrary(hWinInet);

	return err;
}

#endif	/* ] PGP_WIN32 */

#if PGP_MACINTOSH /* [ */

static
PGPError pgpGetProxyServer(PGPContextRef context, PGPProxyServerType type,
			char **proxyAddress, PGPUInt16 *proxyPort)
{
	PGPMemoryMgrRef	mgr = PGPGetContextMemoryMgr(context);
	PGPError			err = kPGPError_NoErr;
	ICError				icErr;
	ICInstance			icInstance;
	Str255				proxy;
	SInt32				bufSize;
	ICAttr				icAttr;
	Boolean				useProxy;
	static PGPUInt32	kDefaultPort = 80;
	char *				host = (char *) proxy;
	char *				port = NULL;
	
	if (type != kPGPProxyServerType_HTTP) {
		return kPGPError_BadParams;
	}
	if (ICStart != (void *) kUnresolvedCFragSymbolAddress) {
		icErr = ICStart(&icInstance, '????');
		if (icErr == noErr) {
			icErr = ICFindConfigFile(icInstance, 0, NULL);
			if (icErr == noErr) {
				bufSize = sizeof(Boolean);
				icErr = ICGetPref(icInstance, kICUseHTTPProxy, &icAttr, (Ptr) &useProxy, &bufSize);
				if ((icErr == noErr) && useProxy) {
					bufSize = sizeof(Str255);
					icErr = ICGetPref(icInstance, kICHTTPProxyHost, &icAttr, (Ptr) proxy, &bufSize);
					if (icErr == noErr) {
						p2cstr(proxy);
						port = strchr(host, ':');
						if (port == NULL) {
							port = strchr(host, ' ');
						}
						if (port == NULL) {
							*proxyPort = kDefaultPort;
						} else {
							*port++ = 0;
							*proxyPort = atoi(port);
						}
						*proxyAddress = (char *) PGPNewData(mgr, strlen(host) + 1,
													kPGPMemoryMgrFlags_None);
						if (*proxyAddress == NULL) {
							err = kPGPError_OutOfMemory;
							*proxyPort = 0;
						} else {
							strcpy(*proxyAddress, host);
						}
					}
				}
			}
			ICStop(icInstance);
		}
	}
	
	return err;
}

#endif /* ] PGP_MACINTOSH */



PGPError PGPGetProxyServer(PGPContextRef context, PGPProxyServerType type,
			char **proxyAddress, PGPUInt16 *proxyPort)
{
	PGPError err = kPGPError_NoErr;

	if (IsntNull(proxyPort))
		*proxyPort = 0;
	if (IsntNull(proxyAddress))
		*proxyAddress = NULL;

	PGPValidateContext(context);
	PGPValidatePtr(proxyAddress);
	PGPValidatePtr(proxyPort);

	if (type == kPGPProxyServerType_Invalid)
		return kPGPError_BadParams;

#if PGP_WIN32 || PGP_MACINTOSH
	err = pgpGetProxyServer(context, type, proxyAddress, proxyPort);
#endif

	return err;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
