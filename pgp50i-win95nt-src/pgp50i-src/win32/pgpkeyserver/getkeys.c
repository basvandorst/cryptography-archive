/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
#include <windows.h>
#include <winsock.h>
#include <commctrl.h>
#include <process.h>

#include <assert.h>

#define PGPLIB
#define KEYSERVER_DLL

#include "..\include\config.h"
#include "..\include\spgp.h"
#include "..\include\pgpkeydb.h"
#include "..\include\pgpcomdlg.h"
#include "..\include\pgpmem.h"
#include "..\include\pgpkeyserversupport.h"

#include "winids.h"
#include "getkeys.h"
#include "resource.h"
#include "keyserver.h"
#include "ksconfig.h"
#include "utils.h"

struct _error_table
{
	char ServerMessage[1024];
	unsigned long MessageID;
	unsigned long TitleID;
	BOOL IsError;
}ErrorTable[] =
{
	{"Key block corrupt: pubkey decode failed",
			IDS_PUBKEY_DECODE_FAILED, IDS_SERVER_ERROR_TITLE, TRUE},
	{"Key block added to key server database.\n  New public keys added: ",
			IDS_KEYS_ADDED_OK, IDS_SERVER_OK_TITLE, FALSE},
	{"Key block in add request contained no new\nkeys, userids, or signatures.",
			IDS_NO_NEW_KEYS_ADDEDD, IDS_SERVER_OK_TITLE, FALSE},
	{"\0", 0, 0}
};

struct _error_string_table
{
	unsigned long RawCode;
	unsigned long StringID;
}ErrToStringTable[] =
{
/*The commented out errors are internal errors or problems we shouldn't see (ie,
 *if IDS_ERR_UNABLE_TO_LOAD_LIBRARY happened, we wouldn't be loaded to display
 *the error.)
 */
//	{ERR_UNABLE_TO_LOAD_LIBRARY,		IDS_ERR_UNABLE_TO_LOAD_LIBRARY},
//	{ERR_NO_FUNCS_AVAILABLE,			IDS_ERR_NO_FUNCS_AVAILABLE},
//	{ERR_LIBRARY_NOT_LOADED,			IDS_ERR_LIBRARY_NOT_LOADED},
//	{ERR_BAD_PARAM,						IDS_ERR_BAD_PARAM},
//	{ERR_FUNCTION_NOT_IN_LIB,			IDS_ERR_FUNCTION_NOT_IN_LIB},
	{ERR_USER_NOT_FOUND,				IDS_ERR_USER_NOT_FOUND},
	{ERR_USER_CANCEL,					0}, /*No bonus boxes!*/
	{ERR_KEYRING_FAILURE,				IDS_ERR_KEYRING_FAILURE},
	{ERR_MALLOC_FAILURE,				IDS_ERR_MALLOC_FAILURE},
	{ERR_DATA_WAIT_TIMEOUT,				IDS_ERR_DATA_WAIT_TIMEOUT},
	{ERR_SOCKET_ERROR,					IDS_ERR_SOCKET_ERROR},
	{ERR_HOST_NOT_FOUND,				IDS_ERR_HOST_NOT_FOUND},
	{ERR_UNABLE_TO_OPEN_SOCKET,			IDS_ERR_UNABLE_TO_OPEN_SOCKET},
	{ERR_MANY_NOT_FOUND,				IDS_ERR_MANY_NOT_FOUND},
	{ERR_NO_RESPONSE_FROM_SERVER,		IDS_ERR_UNKNOWN_SERVER_RESPONSE},
	{0,									0}
};

#define HIT_CANCEL    (pThreadInfo->CancelPending)

static unsigned long ReceiveBuffer(char **buffer, SOCKET sockfd,
									BOOL *HitCancel);
static unsigned long ProcessSendReturn(HWND hwnd, char *buffer);
void PGPExport DisplayKSError(HWND hwnd, unsigned long ReturnCode);
void PGPExport StartPutKeyInfoThread(WDPLT *pThreadArgs);


UINT PGPExport PGPkeyserverGetAndAddKeys(HWND hwnd,
										 char *UserId,
										 PGPKeySet *pAddFromKeyRing,
										 PGPKeySet *pKeyRings,
										 unsigned long *NumKeysFound)
{
	char *KeyBuffer = NULL;
	UINT ReturnCode = SUCCESS;
	char *Start, *End, *IntUserId = NULL;
	unsigned long NumKeys = 0, NumNotFound = 0;

	assert(UserId || pKeyRings);
	assert(NumKeysFound);

	*NumKeysFound = 0;

	if(UserId)
	{
		Start = strchr(UserId, '\n');
		while(Start && *Start)
		{
			++NumKeys;
			Start = strchr(Start, '\n');
			if(Start)
				++Start;
		}

		if(!NumKeys)
			NumKeys = 1;

		Start = End = UserId;
		while(Start && *Start && (ReturnCode == SUCCESS))
		{
			while(End && *End && *End != '\n')
			{
				++End;
			}

			if(MassageUserId(&IntUserId, Start, End) == SUCCESS)
			{
				ReturnCode = PGPkeyserverGetKeys(hwnd, IntUserId, &KeyBuffer);
				if(ReturnCode == SUCCESS)
				{
					ReturnCode = PGPkeyserverAddKeyBuffer(hwnd,
														  KeyBuffer,
														  pKeyRings,
														  NumKeysFound);
					if(KeyBuffer)
						free(KeyBuffer);
				}
				else
				{
					if(ReturnCode != ERR_USER_NOT_FOUND)
						DisplayKSError(hwnd, ReturnCode);
					else
						++NumNotFound;
				}
			}

			if(IntUserId)
			{
				pgpFree(IntUserId);
				IntUserId = NULL;
			}

			while(End && *End && *End == '\n')
				++End;

			Start = End;
		}

		switch(NumNotFound)
		{
			case 0:
				break;

			case 1:
				DisplayKSError(hwnd, ERR_USER_NOT_FOUND);
				break;

			default:
				DisplayKSError(hwnd, ERR_MANY_NOT_FOUND);
				break;
		}
	}
	else
	{
		if(pKeyRings && pAddFromKeyRing)
		{
			PGPKeyIter* pKeyIter = NULL;
			PGPKey* pKey = NULL;
			PGPKeyList* pKeyList = NULL;
			PGPKeySet *pReceivedSet = NULL;
			char KeyIdBuf[64];
			unsigned int KeyIdSize = sizeof(KeyIdBuf);
			char *MultiKeyBuffer = NULL;
			unsigned long NumKeys = 0, NumNotFound = 0;
			static int count = 0;

			pKeyList = pgpOrderKeySet (pAddFromKeyRing, kPGPUserIDOrdering);
			if(pKeyList)
			{
				pKeyIter = pgpNewKeyIter (pKeyList);

				if(pKeyIter)
				{
					pKey = pgpKeyIterNext(pKeyIter);

					while (pKey && (ReturnCode == SUCCESS))
					{
						pgpGetKeyString(pKey, kPGPKeyPropKeyId, KeyIdBuf,
										&KeyIdSize);
						ConvertKeyIDToString(KeyIdBuf);
						ReturnCode = PGPkeyserverGetKeys(hwnd, KeyIdBuf,
										&KeyBuffer);

						if(ReturnCode == SUCCESS)
						{
							if(KeyBuffer)
							{
								if(!MultiKeyBuffer)
								{
									MultiKeyBuffer = malloc((strlen(KeyBuffer)
														+ 1) * sizeof(char));
									*MultiKeyBuffer = '\0';
								}
								else
								{
									MultiKeyBuffer = realloc(MultiKeyBuffer,
											 (strlen(KeyBuffer) +
												strlen(MultiKeyBuffer) + 1) *
												sizeof(char));
								}
								if(MultiKeyBuffer)
									strcat(MultiKeyBuffer, KeyBuffer);

								free(KeyBuffer);
							}
						}
						else
						{
							if(ReturnCode == ERR_USER_NOT_FOUND)
								++NumNotFound;
							else
								DisplayKSError(hwnd, ReturnCode);
						}
						pKey = pgpKeyIterNext(pKeyIter);
					}
					pgpFreeKeyIter(pKeyIter);
				}
				pgpFreeKeyList(pKeyList);

				switch(NumNotFound)
				{
					case 0:
						break;

					case 1:
						DisplayKSError(hwnd, ERR_USER_NOT_FOUND);
						break;

					default:
						DisplayKSError(hwnd, ERR_MANY_NOT_FOUND);
						break;
				}

				if(MultiKeyBuffer)
				{
					pReceivedSet = pgpImportKeyBuffer(
										(unsigned char *) MultiKeyBuffer,
										strlen(MultiKeyBuffer));

					++count;
					if(pReceivedSet)
					{
						ReturnCode = AddKeySetWithConfirm(hwnd,
														  pReceivedSet,
														  pKeyRings,
														  NumKeysFound);
						pgpFreeKeySet(pReceivedSet);
					}
					free(MultiKeyBuffer);
				}
			}
		}
		else
		ReturnCode = ERR_BAD_PARAM;
	}

	return(ReturnCode);
}

BOOL CALLBACK GetKeyDlgProc (HWND hDlg,
							 UINT uMsg,
							 WPARAM wParam,
							 LPARAM lParam)
{
	BOOL ReturnCode = FALSE;
	WDPLT *pThreadArgs;

	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			SendDlgItemMessage (hDlg, IDC_PROGRESS, PBM_SETRANGE, 0,
								MAKELPARAM(0, 100));
			SendDlgItemMessage (hDlg, IDC_PROGRESS, PBM_SETPOS, 0, 0);
			if(lParam)
			{
				char Title[256];

				SetWindowLong(hDlg, GWL_USERDATA, lParam);
				pThreadArgs = (WDPLT *) lParam;
				pThreadArgs->CancelPending = FALSE;
				pThreadArgs->hWorkingDlg = hDlg;
				SetActiveWindow(hDlg);
				SetForegroundWindow(hDlg);

				if(pThreadArgs->IsGet)
				{
					if(pThreadArgs->UserId)
						sprintf(Title,
								"Getting Key For %s",
								pThreadArgs->UserId);
					else
						strcpy(Title, "Getting Key");

					_beginthread(GetKeyInfoThread, 0, (void *) pThreadArgs);
				}
				else
				{
					if(pThreadArgs->UserId)
						sprintf(Title,
								"Putting Key For %s",
								pThreadArgs->UserId);
					else
						strcpy(Title, "Putting Key(s)");

					_beginthread(PutKeyInfoThread, 0, (void *) pThreadArgs);
				}
				SendMessage(hDlg, WM_SETTEXT, 0, (LPARAM) Title);
			}
			break;
		}

		case WM_QUIT:
		case WM_DESTROY:
		{
			EndDialog(hDlg, 0);
			ReturnCode = TRUE;
			break;
		}

		case PGPM_PROGRESS_UPDATE:
		{
			SetDlgItemText(hDlg, IDC_PROGRESS_TEXT, (char *) wParam);
			SendDlgItemMessage(hDlg, IDC_PROGRESS, PBM_SETPOS,
								(WPARAM) lParam, 0);
			ReturnCode = TRUE;
			break;
		}

		case PGPM_DISMISS_PROGRESS_DLG:
		{
			EndDialog(hDlg, TRUE);
			break;
		}

		case WM_COMMAND:
		{
			switch(LOWORD (wParam))
			{
				case IDCANCEL:
				{
					if((pThreadArgs = (WDPLT *) GetWindowLong(hDlg,
															GWL_USERDATA))) {
						pThreadArgs->CancelPending = TRUE;
						SetDlgItemText(hDlg, IDC_PROGRESS_TEXT,
							"Canceling request.  This may take a moment ...");
						SendDlgItemMessage(hDlg, IDC_PROGRESS, PBM_SETPOS,
											(WPARAM) lParam, 0);
						EnableWindow (GetDlgItem (hDlg, IDCANCEL), FALSE);
					}
					else
						EndDialog (hDlg, FALSE);

					ReturnCode = TRUE;
					break;
				}
			}
			ReturnCode = TRUE;
			break;
		}
	}
	return (ReturnCode);
}

UINT PGPExport PGPkeyserverGetKeys(HWND hwnd,
								   char *UserId,
								   char **KeyBuffer)
{
	WDPLT ThreadInfo;

	assert(UserId);
	assert(KeyBuffer);
	*KeyBuffer = NULL;

	ThreadInfo.hParent = hwnd;
	ThreadInfo.UserId = UserId;
	ThreadInfo.KeyBuffer = KeyBuffer;
	ThreadInfo.CancelPending = FALSE;
	ThreadInfo.ReturnCode = ERR_USER_NOT_FOUND;
	ThreadInfo.IsGet = TRUE;
	ThreadInfo.HideMainWindowAtEnd = TRUE;

	DialogBoxParam(	g_hinst,
					MAKEINTRESOURCE(IDD_GET_KEY_DLG),
					ThreadInfo.hParent,
					(DLGPROC) GetKeyDlgProc,
					(LPARAM) &ThreadInfo);

	return(ThreadInfo.ReturnCode);
}

UINT PGPExport PGPkeyserverPutKeys(HWND hwnd, char *UserIds,
									PGPKeySet *pKeyRings)
{
	UINT ReturnCode = SUCCESS;
	char *Start, *End, *IntUserId = NULL;

	assert(UserIds || pKeyRings);

	if(UserIds)
	{
		Start = End = UserIds;
		while(Start && *Start && ReturnCode == SUCCESS)
		{
			while(End && *End && *End != '\n')
			{
				++End;
			}

			if(MassageUserId(&IntUserId, Start, End) == SUCCESS)
			{
				ReturnCode = PGPkeyserverPutKey(hwnd, IntUserId, pKeyRings);
			}

			if(IntUserId)
			{
				pgpFree(IntUserId);
				IntUserId = NULL;
			}

			while(End && *End && *End == '\n')
				++End;

			Start = End;
		}
	}
	else
	{
		if(pKeyRings)
		{
			ReturnCode = PGPkeyserverPutKey(hwnd, NULL, pKeyRings);
		}
		else
			ReturnCode = ERR_BAD_PARAM;
	}

	if(ReturnCode != SUCCESS)
		DisplayKSError(hwnd, ReturnCode);

	return(ReturnCode);
}

UINT PGPExport PGPkeyserverPutKey(HWND hwnd, char *UserId, PGPKeySet *pKeyRings)
{
	WDPLT ThreadInfo;
	PGPKeySet *DefaultKeys, *ThisKey = NULL;
	char *Buffer = NULL;
	unsigned int BufSize = 0, RetryCount = 0;
	UINT ExportReturnCode;

	assert(UserId || pKeyRings);

	pgpLibInit();

	ThreadInfo.ReturnCode = SUCCESS;

	if(UserId)
	{
		if(!pKeyRings)
		{
			DefaultKeys = pgpOpenDefaultKeyRings(FALSE, NULL);
		}
		else
		{
			DefaultKeys = pKeyRings;
		}

		if(DefaultKeys)
		{
			if(*UserId == '0' && *(UserId + 1) == 'x')
			{
				PGPKey *pSingleKey;

				if(pSingleKey = pgpGetKeyByKeyID(DefaultKeys, UserId,
												strlen(UserId)))
				{
					ThisKey = pgpNewSingletonKeySet(pSingleKey);
					pgpFreeKey(pSingleKey);
				}
			}
			else
			{
				ThisKey = pgpFilterKeySetUserID(DefaultKeys, UserId,
												strlen(UserId));
			}

			if(ThisKey)
			{
//				BufSize = 1;

//				Buffer = malloc(BufSize * sizeof(char));

//				ExportReturnCode = pgpExportKeyBuffer(ThisKey, Buffer,
//														&BufSize);
				ExportReturnCode = pgpExportKeyBuffer(ThisKey, NULL, &BufSize);
				if(ExportReturnCode == PGPERR_OK)
				{
					/*Everything is probably OK, but BufSize
					 *can't possibly be alright, so we need to
					 *allocate the proper ammount of space.*/

					unsigned int NewBufSize;
//					free(Buffer);
					++BufSize; /*Allow for a null*/
					Buffer = malloc(BufSize * sizeof(char));
					NewBufSize = BufSize;
					ExportReturnCode = pgpExportKeyBuffer(ThisKey, Buffer,
															&NewBufSize);
					*(Buffer + BufSize) = '\0';
					Buffer[NewBufSize] = '\0';
//					assert(NewBufSize == BufSize);
				}
			}
		}
	}
	else
	{
		if(pKeyRings)
		{
//			BufSize = 1;

//			Buffer = malloc(BufSize * sizeof(char));

//			ExportReturnCode = pgpExportKeyBuffer(pKeyRings, Buffer, &BufSize);
			ExportReturnCode = pgpExportKeyBuffer(pKeyRings, NULL, &BufSize);
			if(ExportReturnCode == PGPERR_OK)
			{
				/*Everything is probably OK, but BufSize
				 *can't possibly be alright, so we need to
				 *allocate the proper ammount of space.*/

				unsigned int NewBufSize;
//				free(Buffer);
				++BufSize; /*Allow for a null*/
				Buffer = malloc(BufSize * sizeof(char));
				NewBufSize = BufSize;
				ExportReturnCode = pgpExportKeyBuffer(pKeyRings,
													  Buffer, &NewBufSize);
				Buffer[NewBufSize] = '\0';
//				*(Buffer + BufSize) = '\0';
//				assert(NewBufSize == BufSize);
			}
			else
				BufSize = 1;
		}
		else
			ThreadInfo.ReturnCode = ERR_BAD_PARAM;
	}
					
	if(ThreadInfo.ReturnCode == SUCCESS &&
	   ExportReturnCode == SUCCESS &&
	   SimplePGPAnalyzeBuffer(Buffer, BufSize - 1) == SIMPLEANALYZE_KEY)
	{
		*(Buffer + BufSize) = '\0';

		ThreadInfo.hParent = hwnd;
		ThreadInfo.UserId = UserId;
		ThreadInfo.KeyBuffer = &Buffer;
		ThreadInfo.CancelPending = FALSE;
		ThreadInfo.ReturnCode = ERR_USER_NOT_FOUND;
		ThreadInfo.IsGet = FALSE;
		ThreadInfo.HideMainWindowAtEnd = TRUE;

		DialogBoxParam(	g_hinst,
						MAKEINTRESOURCE(IDD_GET_KEY_DLG),
						ThreadInfo.hParent,
						(DLGPROC) GetKeyDlgProc,
						(LPARAM) &ThreadInfo);
	}

	if(ThisKey)
	{
		pgpFreeKeySet(ThisKey);
	}

	if(!pKeyRings)
	{
		pgpFreeKeySet(DefaultKeys);
	}

	pgpLibCleanup();

	return(ThreadInfo.ReturnCode);
}

void GetKeyInfoThread(void *pArgs)
{
	HWND hDlg;
	WSADATA wsaData;
	struct hostent *pHostEnt;
	struct in_addr ip;
	struct sockaddr_in server_info;
	SOCKET sockfd;
	char buffer[8192];
	WDPLT* pThreadInfo = (WDPLT *) pArgs;
	char Keyserver[255];
	unsigned short port;
	BOOL AutoGet;
	BOOL bResolved;
	ULONG ulAddr;

	hDlg = pThreadInfo->hWorkingDlg;

	SendMessage(hDlg, PGPM_PROGRESS_UPDATE, (WPARAM) "Initializing Winsock",
				(LPARAM) 0);

	WSAStartup(MAKEWORD(1, 1), &wsaData);

	SendMessage(hDlg, PGPM_PROGRESS_UPDATE, (WPARAM) "Looking Up Host",
				(LPARAM) 10);
	GetKeyserverValues(Keyserver, &port, &AutoGet);

	bResolved = FALSE;
	ulAddr = inet_addr (Keyserver);
	if ((ulAddr != INADDR_NONE) ||
		(lstrcmp (Keyserver, "255.255.255.255") == 0))
	{
		memcpy (&ip, &ulAddr, sizeof(ip));
		server_info.sin_addr = ip;
		server_info.sin_port = htons(port);
		server_info.sin_family = AF_INET;
		bResolved = TRUE;
	}
	else {
		if (!HIT_CANCEL) {
			if((pHostEnt = gethostbyname(Keyserver)) && !HIT_CANCEL)
			{
				memcpy(&ip, pHostEnt->h_addr, sizeof(ip));
				server_info.sin_addr = ip;
				server_info.sin_port = htons(port);
				server_info.sin_family = pHostEnt->h_addrtype;
				bResolved = TRUE;
			}
		}
	}

	if(bResolved && !HIT_CANCEL)
	{
		SendMessage(hDlg, PGPM_PROGRESS_UPDATE,
					(WPARAM) "Establishing Connection", (LPARAM) 20);

		if((sockfd = socket(server_info.sin_family, SOCK_STREAM, 0))
				&& !HIT_CANCEL)
		{
			if(!connect(sockfd,
						(struct sockaddr *) & server_info,
						sizeof(server_info)) && !HIT_CANCEL)
			{
				SendMessage(hDlg, PGPM_PROGRESS_UPDATE,
							(WPARAM) "Sending Request", (LPARAM) 35);
				sprintf(buffer,
						"GET /pks/lookup?op=get&exact=on&search=%s HTTP/1.0\n",
						pThreadInfo->UserId);
				strcat(buffer, "User-Agent: PGP/5.0 (Win32)\n\r\n\r");
				send(sockfd, buffer, strlen(buffer), 0);
				SendMessage(hDlg, PGPM_PROGRESS_UPDATE,
							(WPARAM) "Receiving Data", (LPARAM) 60);
				if(!HIT_CANCEL)
				{
					pThreadInfo->ReturnCode = ReceiveBuffer(
								pThreadInfo->KeyBuffer, sockfd, &HIT_CANCEL);

					SendMessage(hDlg, PGPM_PROGRESS_UPDATE,
								(WPARAM) "Cleaning Up", (LPARAM) 95);

					if(!HIT_CANCEL && pThreadInfo->ReturnCode == SUCCESS)
                      {
						if(SimplePGPAnalyzeBuffer(*(pThreadInfo->KeyBuffer),
										strlen(*(pThreadInfo->KeyBuffer)))
									== SIMPLEANALYZE_KEY)
							pThreadInfo->ReturnCode = SUCCESS;
						else
							pThreadInfo->ReturnCode = ERR_USER_NOT_FOUND;
						SendMessage(hDlg, PGPM_PROGRESS_UPDATE,
									(WPARAM) "Complete", (LPARAM) 100);
					}
				}
			}
			else
				pThreadInfo->ReturnCode = ERR_UNABLE_TO_OPEN_SOCKET;
			closesocket(sockfd);
		}
		else
			pThreadInfo->ReturnCode = ERR_UNABLE_TO_OPEN_SOCKET;
	}
	else
		pThreadInfo->ReturnCode = ERR_HOST_NOT_FOUND;


	WSACleanup();

	PostMessage(hDlg, PGPM_DISMISS_PROGRESS_DLG, 0, 0);

	if(HIT_CANCEL)
		pThreadInfo->ReturnCode = ERR_USER_CANCEL;

	_endthread();
}

void PGPExport StartPutKeyInfoThread(WDPLT *pThreadArgs)
{
	_beginthread(PutKeyInfoThread, 0, (void *) pThreadArgs);
}

void PutKeyInfoThread(void *pArgs)
{
	HWND hDlg;
	WSADATA wsaData;
	struct hostent *pHostEnt;
	struct in_addr ip;
	struct sockaddr_in server_info;
	SOCKET sockfd;
	char *buffer, *ErrBuffer = NULL;
	WDPLT* pThreadInfo = (WDPLT *) pArgs;
	char Keyserver[255], *TempBuffer;
	unsigned short port;
	BOOL AutoGet;
	ULONG ulAddr;
	BOOL bResolved;

	pThreadInfo->ReturnCode = SUCCESS;		//assume no errors
	hDlg = pThreadInfo->hWorkingDlg;

	SendMessage(hDlg, PGPM_PROGRESS_UPDATE, (WPARAM) "Initializing Winsock",
				(LPARAM) 0);

	WSAStartup(MAKEWORD(1, 1), &wsaData);

	SendMessage(hDlg, PGPM_PROGRESS_UPDATE, (WPARAM) "Looking Up Host",
				(LPARAM) 10);
	GetKeyserverValues(Keyserver, &port, &AutoGet);

	bResolved = FALSE;
	ulAddr = inet_addr (Keyserver);
	if ((ulAddr != INADDR_NONE) ||
		(lstrcmp (Keyserver, "255.255.255.255") == 0))
	{
		memcpy (&ip, &ulAddr, sizeof(ip));
		server_info.sin_addr = ip;
		server_info.sin_port = htons(port);
		server_info.sin_family = AF_INET;
		bResolved = TRUE;
	}
	else {
		if (!HIT_CANCEL) {
			if((pHostEnt = gethostbyname(Keyserver)) && !HIT_CANCEL)
			{
				memcpy(&ip, pHostEnt->h_addr, sizeof(ip));
				server_info.sin_addr = ip;
				server_info.sin_port = htons(port);
				server_info.sin_family = pHostEnt->h_addrtype;
				bResolved = TRUE;
			}
		}
	}

	if (bResolved && !HIT_CANCEL)
	{
		SendMessage(hDlg, PGPM_PROGRESS_UPDATE,
					(WPARAM) "Establishing Connection", (LPARAM) 20);

		if((sockfd = socket(server_info.sin_family, SOCK_STREAM, 0))
				&& !HIT_CANCEL)
		{
			if(!connect(sockfd,
						(struct sockaddr *) & server_info,
						sizeof(server_info)) && !HIT_CANCEL)
			{
				SendMessage(hDlg, PGPM_PROGRESS_UPDATE,
							(WPARAM) "Sending Key", (LPARAM) 35);
				UrlEncode(&TempBuffer, *(pThreadInfo->KeyBuffer));
				if(TempBuffer)
				{
					buffer = malloc((strlen(TempBuffer) + 512) * sizeof(char));
					if(buffer)
					{
						sprintf(buffer, "POST /pks/add HTTP/1.0\n"
								"User-Agent: PGP/5.0 (Win32)\n"
								"Content-length: %i\n\nkeytext=",
							    strlen(TempBuffer) + 8);
						strcat(buffer, TempBuffer);
						send(sockfd, buffer, strlen(buffer), 0);
						SendMessage(hDlg, PGPM_PROGRESS_UPDATE,
								(WPARAM) "Receiving Response", (LPARAM) 60);
						free(buffer);
						if(!HIT_CANCEL)
						{
							pThreadInfo->ReturnCode = ReceiveBuffer(
											&ErrBuffer, sockfd, &HIT_CANCEL);
							SendMessage(hDlg, PGPM_PROGRESS_UPDATE,
										(WPARAM) "Cleaning Up", (LPARAM) 95);
						}
					}
					free(TempBuffer);
				}
			}
			else
				pThreadInfo->ReturnCode = ERR_UNABLE_TO_OPEN_SOCKET;
			closesocket(sockfd);
		}
		else
			pThreadInfo->ReturnCode = ERR_UNABLE_TO_OPEN_SOCKET;
	}
	else
		pThreadInfo->ReturnCode = ERR_HOST_NOT_FOUND;


	WSACleanup();

	if(HIT_CANCEL)
	{
		pThreadInfo->ReturnCode = ERR_USER_CANCEL;
		if (ErrBuffer)
			free (ErrBuffer);
	}
	else
	{
		if(ErrBuffer)
		{
			if(pThreadInfo->HideMainWindowAtEnd)
				ShowWindow(hDlg, SW_HIDE);
			ProcessSendReturn(hDlg, ErrBuffer);
			free(ErrBuffer);
		}
		else if (pThreadInfo->ReturnCode == SUCCESS) {
			pThreadInfo->ReturnCode = ERR_NO_RESPONSE_FROM_SERVER;
		}
	}

	PostMessage(hDlg, PGPM_DISMISS_PROGRESS_DLG, 0, 0);


	_endthread();
}


UINT PGPExport PGPkeyserverAddKeyBuffer(HWND hwnd,
										char *Buffer,
										PGPKeySet *pKeyRings,
										unsigned long *NumKeysFound)
{
	unsigned long ReturnCode = ERR_KEYRING_FAILURE;
	PGPKeySet *RingSet;
	PGPKeySet *NewKeySet;
	assert(Buffer);

	assert(NumKeysFound);

    pgpLibInit();

	if(!pKeyRings)
	{
		RingSet = pgpOpenDefaultKeyRings(TRUE, NULL);
	}
	else
	{
		RingSet = pKeyRings;
	}

	if(RingSet)
	{
		NewKeySet = pgpImportKeyBuffer((unsigned char *) Buffer,
										strlen(Buffer));

		if(NewKeySet)
		{
			ReturnCode = AddKeySetWithConfirm(hwnd, NewKeySet, RingSet,
												NumKeysFound);

			pgpFreeKeySet(NewKeySet);
		}
		if(!pKeyRings)
		{
			pgpFreeKeySet(RingSet);
		}
	}
	pgpLibCleanup();

	return(ReturnCode);
}

KSERR MassageUserId(char **UserId, char *Start, char *End)
{
	KSERR ReturnCode = SUCCESS;
	char *LeftAngle = NULL, *RightAngle = NULL, *p;

	assert(UserId);
	assert(Start);
	assert(End);

	if(UserId && Start && End)
	{
		/*Alright, is it a "John Doe <jdoe@pgp.com>" kinda address?*/
		*UserId = pgpMemAlloc(End - Start + 1);
		strncpy(*UserId, Start, End - Start);
		*(*UserId + (End - Start)) = '\0';

		if((LeftAngle = strchr(*UserId, '<')))
			RightAngle = strchr(LeftAngle, '>');

		if(LeftAngle && RightAngle)
		{
			*RightAngle = '\0';
			memmove(*UserId, LeftAngle + 1, RightAngle - LeftAngle);
		}
		
		p = *UserId;
		while(p && *p)
		{
			if(*p == ' ')
				*p = '+';

			++p;
		}
	}
	else
		ReturnCode = ERR_BAD_PARAM;

	return(ReturnCode);
}

LRESULT WINAPI GetEmailDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam,
								LPARAM lParam)
{
	static BOOL IsFirstPaint = TRUE;

	switch (uMsg)
	{
		case WM_INITDIALOG:
			SetWindowLong(hDlg, GWL_USERDATA, lParam);
			SendMessage(GetDlgItem(hDlg, IDC_EMAIL), EM_SETLIMITTEXT,
						(WPARAM) 255, 0);
			SetFocus(GetDlgItem(hDlg, IDC_EMAIL));
			IsFirstPaint = TRUE;
			return TRUE;

		case WM_PAINT:
		{
			if(IsFirstPaint)
			{
				IsFirstPaint = FALSE;
				SetFocus(GetDlgItem(hDlg, IDC_EMAIL));
			}
		}

		case WM_COMMAND:
		{
			switch (LOWORD (wParam))
			{
				case IDOK:
				{
					char *Email;
					if((Email = (char *) GetWindowLong(hDlg, GWL_USERDATA)))
					{
						GetDlgItemText(hDlg, IDC_EMAIL, Email, 255);
					}
					EndDialog(hDlg, TRUE);
					break;
				}

				case IDCANCEL:
				{
					EndDialog(hDlg, FALSE);
					break;
				}
			}
			break;
		}
	}
	return FALSE;
}

static unsigned long ReceiveBuffer(char **buffer, SOCKET sockfd,
									BOOL *CancelFlag)
{
	fd_set set;
	unsigned long CharsReceived = 0, bufferSize = 1, ReturnCode = SUCCESS;
	char TempBuffer[64];
	struct timeval timeout = {120, 0}; /*Timeout the select after 120 seconds*/
	int SelectReturn;

	assert(buffer);

	*buffer = NULL;

	FD_ZERO(&set);
	FD_SET(sockfd, &set);

	SelectReturn = select(0, &set, NULL, NULL, &timeout);

	switch(SelectReturn)
	{
		case 0:
			ReturnCode = ERR_DATA_WAIT_TIMEOUT;
			break;

		case SOCKET_ERROR:
			ReturnCode = ERR_SOCKET_ERROR;
			break;

		default:
			do
			{
				CharsReceived = recv(sockfd, TempBuffer,
									sizeof(TempBuffer) - 1, 0);
				if(CharsReceived)
				{
					*(TempBuffer + CharsReceived) = '\0';
					bufferSize += CharsReceived;
					if(*buffer)
					{
						*buffer =
							realloc(*buffer,
									bufferSize * sizeof(char));
					}
					else
					{
						*buffer =
							malloc(bufferSize * sizeof(char));
						**buffer = '\0';
					}
					strcat(*buffer, TempBuffer);
				}
			}while(CharsReceived && ((CancelFlag && *CancelFlag == FALSE)
										||  !CancelFlag));
			if(*CancelFlag)
				ReturnCode = ERR_USER_CANCEL;
			break;
	}

	return(ReturnCode);
}

static unsigned long ProcessSendReturn(HWND hwnd, char *buffer)
{
	char *p;
	int x;
	unsigned long MessageID = 0, TitleID = 0;
	char OutputMessage[1024];
	char OutputTitle[1024];
	char *TempBuffer;
	BOOL IsError = TRUE, FoundError = FALSE;
	UINT MBType = MB_ICONERROR | MB_OK;
	unsigned long ReturnCode = SUCCESS;

	assert(buffer);

	TempBuffer = malloc((strlen(buffer) + 1) * sizeof(char));

	if(TempBuffer)
	{
		strcpy(TempBuffer, buffer);
		p = TempBuffer;
		/*The first step is to skip over the HTTP headers.*/
		p = strstr(buffer, "\r\n\r\n");
		if(p)
		{
			/*Next, skip the title*/
			p = strstr(p, "<p>\r\n");
			if(p)
			{
				++p;
				/*Next, skip the header (<h1>)*/
				p = strstr(p, "<p>\r\n");
				if(p)
				{
					/*Last, jump the <p>\r\n we just found*/
					p += 5;

					/*Sometimes, it has a "<pre>...</pre>" tag, so let's clean
					 *those up. Ideally, if we ever have to show this to the
					 *user, we don't want the raw HTML to pop out.
					 */
					if(strncmp(p, "<pre>", 5) == 0)
					{
						char *p2;

						p += 5;
						p2 = strstr(p, "</pre>");
						if(p2)
							*p2 = '\0';
					}
					while(isspace(*p))
						++p;
					/*God willing and the creek don't rise, p now points to the
					 *actual error message.  See if we can look it up...*/

					for(x = 0; ErrorTable[x].ServerMessage[0] && !FoundError;
							++x)
					{
						if(strncmp(ErrorTable[x].ServerMessage, p,
									strlen(ErrorTable[x].ServerMessage)) == 0)
						{
							FoundError = TRUE;
							MessageID = ErrorTable[x].MessageID;
							TitleID = ErrorTable[x].TitleID;
							IsError = ErrorTable[x].IsError;
						}
					}

					/*It's a known return; look it up*/
					if(MessageID)
					{
						LoadString(g_hinst, MessageID, OutputMessage,
									sizeof(OutputMessage));
						if(MessageID == IDS_KEYS_ADDED_OK)
						{
							char TempBuf[1024];
							int NumAdded = 0;

							while(p && *p && !isdigit(*p))
								++p;

							NumAdded = atoi(p);

							sprintf(TempBuf, OutputMessage, NumAdded);
							strcpy(OutputMessage, TempBuf);
						}
					}
					else
					{
						if (*p) strcpy(OutputMessage, p);
						else {
							LoadString(g_hinst, IDS_ERR_UNKNOWN_SERVER_RESPONSE,
								OutputMessage, sizeof(OutputMessage));
							TitleID = IDS_SERVER_ERROR_TITLE;
						}
					}

					/*Get the title*/
					if(TitleID)
					{
						LoadString(g_hinst, TitleID, OutputTitle,
									sizeof(OutputTitle));
						/*It may be a successful message, in which case, we
						 *don't want to display the error message icon, we
						 *want the informational one.
						 */
						if(!IsError)
							MBType = MB_ICONINFORMATION | MB_OK;
					}
					else
					{
						strcpy(OutputTitle, "Server Response");
						MBType = MB_ICONEXCLAMATION | MB_OK;
					}
				}
			}
		}

		if (!p) {
			LoadString(g_hinst, IDS_ERR_UNKNOWN_SERVER_RESPONSE,
				OutputMessage, sizeof(OutputMessage));
			strcpy(OutputTitle, "Unrecognized Server Response");
		}
		MessageBox(hwnd, OutputMessage, OutputTitle, MBType);

		free (TempBuffer);
	}
	else
		ReturnCode = ERR_MALLOC_FAILURE;

	return(ReturnCode);
}

void PGPExport DisplayKSError(HWND hwnd, unsigned long ReturnCode)
{
	char OutputBuf[1024], Title[1024];
	int x;
	unsigned long StringToLoad = 0;

	for(x = 0; ErrToStringTable[x].RawCode && !StringToLoad; ++x)
	{
		if(ErrToStringTable[x].RawCode == ReturnCode)
			StringToLoad = ErrToStringTable[x].StringID;
	}

	if(StringToLoad)
	{
		LoadString(g_hinst, StringToLoad, OutputBuf, sizeof(OutputBuf));
		LoadString(g_hinst, IDS_GENERAL_ERROR, Title, sizeof(Title));
		MessageBox(hwnd, OutputBuf, Title, MB_OK | MB_ICONERROR);
	}
}
