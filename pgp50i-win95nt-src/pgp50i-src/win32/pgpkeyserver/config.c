/*
 * Copyright (C) 1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id$
 */
#include <windows.h>
#include <assert.h>

#include <stdio.h>

#define PGPLIB
#define KEYSERVER_DLL

#include "..\include\config.h"
#include "..\include\pgpkeydb.h"
#include "..\include\pgpkeyserversupport.h"
#include "ksconfig.h"

#define PGPMAIL_KEY  "Software\\PGP\\PGP"
#define VALUE_SERVER "Server"
#define VALUE_PORT   "Port"
#define VALUE_AUTO   "AutoGet"

KSERR PGPExport GetKeyserverValues(char *Keyserver, unsigned short *Port,
                                    BOOL *AutoGet)
{
    KSERR ReturnCode = SUCCESS;
    HKEY hkey;
    BOOL NeedInitialization = TRUE;
    DWORD disposition, dwPort, dwAutoGet;

    assert(Keyserver);
    assert(Port);
    assert(AutoGet);

    if(Keyserver && Port && AutoGet)
    {
        if(RegCreateKeyEx(HKEY_CURRENT_USER,
                          PGPMAIL_KEY,
                          0,
                          "",
                          REG_OPTION_NON_VOLATILE,
                          KEY_ALL_ACCESS,
                          NULL,
                          &hkey,
                          &disposition) == ERROR_SUCCESS)
        {
            DWORD Size,Type;

            Size = 255;
            Type = REG_SZ;

            if(RegQueryValueEx(hkey,
                               VALUE_SERVER,
                               NULL,
                               &Type,
                               (BYTE*) Keyserver,
                               &Size) != ERROR_SUCCESS)
            {
                strcpy(Keyserver, DEFAULT_KEYSERVER);
                /*Assume it's a new key...*/
                RegSetValueEx(hkey,
                              VALUE_SERVER,
                              0,
                              REG_SZ,
                              (BYTE *) Keyserver,
                              255);
            }

            Size = sizeof(dwPort);
            Type = REG_DWORD;
            if(RegQueryValueEx(hkey,
                               VALUE_PORT,
                               NULL,
                               &Type,
                               (BYTE*) &dwPort,
                               &Size) != ERROR_SUCCESS)
            {
                /*Assume it's a new key...*/
                *Port = 11371;
                dwPort = (DWORD) (*Port);

                RegSetValueEx(hkey,
                              VALUE_PORT,
                              0,
                              REG_DWORD,
                              (BYTE *) &dwPort,
                              sizeof(dwPort));
            }
            *Port = (unsigned short) dwPort;

            Size = sizeof(dwAutoGet);
            Type = REG_DWORD;
            if(RegQueryValueEx(hkey,
                               VALUE_AUTO,
                               NULL,
                               &Type,
                               (BYTE*) &dwAutoGet,
                               &Size) != ERROR_SUCCESS)
            {
                /*Assume it's a new key...*/
                *AutoGet = FALSE;
                dwAutoGet = (DWORD) (*AutoGet);

                RegSetValueEx(hkey,
                              VALUE_AUTO,
                              0,
                              REG_DWORD,
                              (BYTE *) &dwAutoGet,
                              sizeof(dwAutoGet));
            }
            *AutoGet = dwAutoGet;

#if 0
            else
            {
                char buffer[1024];
                sprintf(buffer, "%lu", *Port);
                MessageBox(NULL, buffer, "Wubba", MB_OK);
            }
#endif

        }
        RegCloseKey(hkey);
    }

    return(ReturnCode);
}

KSERR SetKeyserverValues(char *Keyserver, unsigned short Port, BOOL AutoGet)
{
    KSERR ReturnCode = SUCCESS;
    HKEY hkey;
    BOOL NeedInitialization = TRUE;
    DWORD disposition, dwPort, dwAutoGet;

    assert(Keyserver);

    if(Keyserver)
    {
        if(RegCreateKeyEx(HKEY_CURRENT_USER,
                          PGPMAIL_KEY,
                          0,
                          "",
                          REG_OPTION_NON_VOLATILE,
                          KEY_ALL_ACCESS,
                          NULL,
                          &hkey,
                          &disposition) == ERROR_SUCCESS)
        {
            RegSetValueEx(hkey,
                          VALUE_SERVER,
                          0,
                          REG_SZ,
                          (BYTE *) Keyserver,
                          lstrlen (Keyserver)+1);

            dwPort = Port;
            RegSetValueEx(hkey,
                          VALUE_PORT,
                          0,
                          REG_DWORD,
                          (BYTE *) &dwPort,
                          sizeof(dwPort));

            dwAutoGet = AutoGet;
            RegSetValueEx(hkey,
                          VALUE_AUTO,
                          0,
                          REG_DWORD,
                          (BYTE *) &dwAutoGet,
                          sizeof(dwAutoGet));
        }
        RegCloseKey(hkey);
    }

    return(ReturnCode);
}
