/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: TaskSchl.cpp,v 1.8 1999/03/10 03:03:33 heller Exp $
____________________________________________________________________________*/
#include "precomp.h"
#include "pgpUserInterface.h"
#include <wchar.h>
#include <windows.h>
#include <mbctype.h>
#include <initguid.h>
#include <mstask.h>
#include <msterr.h>

// StartScheduler
//
// Starts task scheduler for Win95 or NT
#define SCHED_CLASS             TEXT("SAGEWINDOWCLASS")
#define SCHED_TITLE             TEXT("SYSTEM AGENT COM WINDOW")
#define SCHED_SERVICE_APP_NAME  TEXT("mstask.exe")
#define SCHED_SERVICE_NAME      TEXT("Schedule")

DWORD 
StartScheduler()
{
	OSVERSIONINFO osver;

	osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	// Determine what version of OS we are running on.
	GetVersionEx(&osver);

	if (osver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
		// Start the Win95 version of TaskScheduler.

		HWND hwnd = FindWindow(SCHED_CLASS, SCHED_TITLE);

		if (hwnd != NULL)
		{
			// It is already running.
			return ERROR_SUCCESS;
		}

		//
		//  Execute the task scheduler process.
		//
		STARTUPINFO         sui;
		PROCESS_INFORMATION pi;

		ZeroMemory(&sui, sizeof(sui));
		sui.cb = sizeof (STARTUPINFO);

		TCHAR szApp[MAX_PATH];
		LPTSTR pszPath;

		DWORD dwRet = SearchPath(NULL,
			SCHED_SERVICE_APP_NAME,
			NULL,
			MAX_PATH,
			szApp,
			&pszPath);

		if (dwRet == 0)
		{
			return GetLastError();
		}

		BOOL fRet = CreateProcess(szApp,
			NULL,
			NULL,
			NULL,
			FALSE,
			CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,
			NULL,
			NULL,
			&sui,
			&pi);

		if (fRet == 0)
		{
			return GetLastError();
		}

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		return ERROR_SUCCESS;
	}
	else
	{
		// If not Win95 then start the NT version as a TaskScheduler service.

		SC_HANDLE   hSC = NULL;
		SC_HANDLE   hSchSvc = NULL;

		hSC = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);

		if (hSC == NULL)
		{
			return GetLastError();
		}

		hSchSvc = OpenService(hSC,
			SCHED_SERVICE_NAME,
			SERVICE_START | SERVICE_QUERY_STATUS);

		CloseServiceHandle(hSC);

		if (hSchSvc == NULL)
		{
			return GetLastError();
		}

		SERVICE_STATUS SvcStatus;

		if (QueryServiceStatus(hSchSvc, &SvcStatus) == FALSE)
		{
			CloseServiceHandle(hSchSvc);
			return GetLastError();
		}

		if (SvcStatus.dwCurrentState == SERVICE_RUNNING)
		{
			// The service is already running.
			CloseServiceHandle(hSchSvc);
			return ERROR_SUCCESS;
		}

		if (StartService(hSchSvc, 0, NULL) == FALSE)
		{
			CloseServiceHandle(hSchSvc);
			return GetLastError();
		}

		CloseServiceHandle(hSchSvc);

		return ERROR_SUCCESS;
	}
}

// Init
//
// Start the schedule and instantiate task scheduler object.
// Return S_OK on success.
HRESULT Init(ITaskScheduler **ppITaskScheduler)
{
    HRESULT hr = S_OK;

	if(StartScheduler()!=ERROR_SUCCESS)
	{
		// wprintf(L"Start failed\n");
		return -1;
	}

    // Bring in the library

    hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        return hr;
    }

    // Create the pointer to Task Scheduler object
    // CLSID from the header file mstask.h

    hr = CoCreateInstance(
        CLSID_CTaskScheduler,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITaskScheduler,
        (void **) ppITaskScheduler);

    // Should we fail, unload the library

    if (FAILED(hr))
    {
        CoUninitialize();
    }

    return hr;
}

// Cleanup
//
// Kill the instance of the task scheduler
void Cleanup(ITaskScheduler *pITaskScheduler)
{
    if (pITaskScheduler)
    {
        pITaskScheduler->Release();
        pITaskScheduler = NULL;
    }
   
    // Unload the library, now that our pointer is freed.

    CoUninitialize();
    return;
}

// DeleteJob
//
// Erase the task entry specified by lpwszJobName
HRESULT DeleteJob(ITaskScheduler *pITaskScheduler,LPWSTR lpwszJobName)
{
    HRESULT hr = S_OK;

    hr = pITaskScheduler->Delete(lpwszJobName);

	return hr;
}

// EditTaskSettings
//
// Shows user the current run schedule so they can change 
// settings in the task scheduler property sheet.
VOID EditTaskSettings(HWND hwnd,ITask *pTask)
{
	IProvideTaskPage   *ptp;
	PROPSHEETHEADER     psh;
	HPROPSHEETPAGE      hpage;
       
	ptp=NULL;
	memset(&psh,0x00,sizeof(PROPSHEETHEADER));
	memset(&hpage,0x00,sizeof(HPROPSHEETPAGE));

	// Display the "Settings" property page.
	pTask->QueryInterface(IID_IProvideTaskPage, (VOID**)(IProvideTaskPage**)&ptp);
       
	// The settings will be saved to the task object on release.
	ptp->GetPage(TASKPAGE_SCHEDULE, TRUE, &hpage);
        
	// Display the page in its own property sheet.
	psh.dwSize=sizeof(PROPSHEETHEADER);
	psh.dwFlags=PSH_DEFAULT;
	psh.hwndParent=hwnd;
	psh.hInstance=NULL;
	psh.pszCaption=TEXT("Please review/edit the schedule for this job");
	psh.phpage=&hpage;
	psh.nPages=1;
       
	PropertySheet(&psh);
}

// AddJob
//
// Create new command in jobname
// Returns S_OK on success. Note NT prompts for password of
// current user.

HRESULT AddJob(HWND hwnd,
			   PGPContextRef context,
			   ITaskScheduler *pITaskScheduler,
			   LPWSTR lpwszJobName, 
			   LPSYSTEMTIME lptTime,
               LPWSTR lpwszAppName,
			   LPWSTR lpwszCommand)
{
	HRESULT hr = S_OK;
	IUnknown *pIU;
	IPersistFile *pIPF;
	ITask *pITask;
	ITaskTrigger *pITaskTrig;
	DWORD dwTaskFlags, dwTrigFlags;
	WORD wTrigNumber;
	TASK_TRIGGER TaskTrig;
	WCHAR lpwszUserName[255];
	WCHAR *lpwszPassword=NULL; // SM? -wjb
	char *pszPassword=NULL;
	int PasswordLen;
	char Prompt[500];
    
	PGPMemoryMgrRef	memmgr;
	memmgr = PGPGetContextMemoryMgr (context);

	memset(&TaskTrig,0x00,sizeof(TASK_TRIGGER));

	memset(lpwszUserName,0x00,255*sizeof(WCHAR));

	// Create dummy password
	PasswordLen=10;
	lpwszPassword=(WCHAR *)PGPNewSecureData (memmgr, sizeof(WCHAR)*PasswordLen, 0);
	memset(lpwszPassword,0x00,sizeof(WCHAR)*PasswordLen);

	wcscpy(lpwszUserName,L"");
	wcscpy(lpwszPassword,L"");

	// Add the task.  Most likely failure is that work item already exists.
	// Uses ITaskScheduler::NewWorkItem

	hr = pITaskScheduler->NewWorkItem(lpwszJobName, 
                                        CLSID_CTask, 
                                        IID_ITask, 
                                        &pIU);
	if (FAILED(hr))
	{
		// wprintf(L"Error:  Create New Task failure\n");
		return hr;
	}

	// We now have an IUnknown pointer.  This is queried for an ITask
	// pointer on the work item we just added.
    
	hr = pIU->QueryInterface(IID_ITask, (void **) &pITask);
    if (FAILED(hr))
    {
        // wprintf(L"Error: IUnknown failed to yield ITask\n");
        pIU->Release();
        return hr;
    }
    
    // Cleanup pIUnknown, as we are done with it.
    
    pIU->Release();
    pIU = NULL;

    //
    // We need to see if we support security, and we
    // do this by calling ITask::SetAccountInformation
    // and checking if the failure code is SCHED_E_NO_SECURITY_SERVICES
    //

    hr = pITask->SetAccountInformation(lpwszUserName, lpwszPassword);

    if (hr != SCHED_E_NO_SECURITY_SERVICES)
    {
		char User[256];
		DWORD UserSize=256;
		UINT uCodePage;
		PGPError err;

		GetUserName(User,&UserSize);

		uCodePage = _getmbcp();
		MultiByteToWideChar(uCodePage, 0, User,
			UserSize, lpwszUserName,
			255);

		strcpy(Prompt,"Enter passphrase for NT user: ");
		strcat(Prompt,User);

		err=PGPConventionalEncryptionPassphraseDialog(context,
			PGPOUIDialogPrompt( context, Prompt ),
			PGPOUIShowPassphraseQuality(context,FALSE),
			PGPOUIOutputPassphrase( context, &pszPassword ),
			PGPOUIParentWindowHandle( context, hwnd ),
			PGPOLastOption( context ) );

		PGPFreeData(lpwszPassword);

		if(IsPGPError(err))
			return S_FALSE;

		PasswordLen=strlen(pszPassword)+1; // Add one for NULL

		// Replace dummy password with real one
		lpwszPassword=(WCHAR *)PGPNewSecureData (memmgr, sizeof(WCHAR)*PasswordLen, 0);
		memset(lpwszPassword,0x00,sizeof(WCHAR)*PasswordLen);

   		MultiByteToWideChar(uCodePage, 0, pszPassword,
			PasswordLen, 
			lpwszPassword,
			PasswordLen);

		PGPFreeData(pszPassword);
    }

    // Set the account information using ITask::SetAccountInformation
    // This fails for Win9x, but we ignore the failure.
    hr = pITask->SetAccountInformation(lpwszUserName, lpwszPassword);;
	memset(lpwszPassword,0x00,sizeof(WCHAR)*PasswordLen);
	PGPFreeData(lpwszPassword);

    if ((FAILED(hr)) && (hr != SCHED_E_NO_SECURITY_SERVICES))
    {
        // wprintf(L"Error: Failed to set credentials on task object %x\n",hr);
        pITask->Release();
        return hr;
    }

    // Set command name with ITask::SetApplicationName

    hr = pITask->SetApplicationName(lpwszAppName);
    if (FAILED(hr))
    {
        // wprintf(L"Error: Failed to set command name (with parms)\n"); 
        pITask->Release();
        return hr;
    }


    // Set task parameters with ITask::SetParameters

    hr = pITask->SetParameters(lpwszCommand);
    if (FAILED(hr))
    {
        // wprintf(L"Error: Failed to set parameters\n");
        pITask->Release();
        return hr;
    }
   
    // Set the comment, so we know how this job go there
    // Uses ITask::SetComment

    hr = pITask->SetComment(L"This job was added by the PGP free space wiper.");
    if (FAILED(hr))
    {
        // wprintf(L"Error: Task comment could not be set\n");
        pITask->Release();
        return hr;
    }

    // Set the flags on the task object
    // Use ITask::SetFlags

    dwTaskFlags = TASK_FLAG_DELETE_WHEN_DONE;
    hr = pITask->SetFlags(dwTaskFlags);
    if (FAILED(hr))
    {
        // wprintf(L"Error: Could not set task flags\n");
        pITask->Release();
        return hr;
    }

    // Now, create a trigger to run the task at our specified time.
    // Uses ITask::CreateTrigger()

    hr = pITask->CreateTrigger(&wTrigNumber, &pITaskTrig);
    if (FAILED(hr))
    {
        // wprintf(L"Error: Could not create a new trigger\n");
        pITask->Release();
        return hr;
    }

    // Now, fill in the trigger as necessary.
    dwTrigFlags = 0;

    TaskTrig.cbTriggerSize = sizeof(TASK_TRIGGER);
    TaskTrig.Reserved1 = 0;
    TaskTrig.wBeginYear = lptTime->wYear;
    TaskTrig.wBeginMonth = lptTime->wMonth;
    TaskTrig.wBeginDay = lptTime->wDay;
    TaskTrig.wEndYear = 0;
    TaskTrig.wEndMonth = 0;
    TaskTrig.wEndDay = 0;
    TaskTrig.wStartHour = lptTime->wHour;
    TaskTrig.wStartMinute = lptTime->wMinute;
    TaskTrig.MinutesDuration = 0;
    TaskTrig.MinutesInterval = 0;
    TaskTrig.rgFlags = dwTrigFlags;
    TaskTrig.TriggerType = TASK_TIME_TRIGGER_DAILY;
	TaskTrig.Type.Daily.DaysInterval=1;
    TaskTrig.wRandomMinutesInterval = 0;
    TaskTrig.Reserved2 = 0;

    // Add this trigger to the task using ITaskTrigger::SetTrigger

    hr = pITaskTrig->SetTrigger(&TaskTrig);
    if (FAILED(hr))
    {
        // wprintf(L"Error: Failed to set trigger to desired values\n");
        pITaskTrig->Release();
        pITask->Release();
        return hr;
    }

    // Make the changes permanent
    // Requires using IPersistFile::Save()

    hr = pITask->QueryInterface(IID_IPersistFile, (void **) &pIPF); 
    if (FAILED(hr))
    {
        // wprintf(L"Error: Could not get IPersistFile on task\n");
        pITaskTrig->Release();
        pITask->Release();
        return hr;
    }

    hr = pIPF->Save(NULL, FALSE);
    if (FAILED(hr))
    {
        // wprintf(L"Error: Could not save object\n");
        pITaskTrig->Release();
        pITask->Release();
        pIPF->Release();
        return hr;
    }

    // We no longer need ITask
    
	EditTaskSettings(hwnd,pITask);

    pITask->Release();
    pITask = NULL;

    // Done with ITaskTrigger pointer
    
    pITaskTrig->Release();
    pITaskTrig = NULL;

    // Done with IPersistFile
    
    pIPF->Release();
    pIPF = NULL;

    return hr;
} 

extern "C" HRESULT SchedulePGPwipe(HWND hwnd,PGPContextRef context,char *Drive)
{
	HRESULT hRes;
	ITaskScheduler *pITaskScheduler;
	WCHAR wszJobName[255];
	WCHAR wszAppName[255];
	WCHAR wszCommand[255];
	UINT uCodePage;
	SYSTEMTIME SysTime;
	int result;
	char szJobName[255];
	char szAppName[255];
	char szCommand[255];

	strcpy(szJobName,"PGPwipeVolume");
	szJobName[strlen(szJobName)+1]=0;
	szJobName[strlen(szJobName)]=Drive[0];
	strcat(szJobName,".job");

	PGPclGetPGPPath (szAppName, sizeof(szAppName));
	strcat(szAppName, "PGPtools.exe");

	strcpy(szCommand,"/w ");
	strcat(szCommand,Drive);

	memset(wszJobName,0x00,255*sizeof(WCHAR));
	memset(wszAppName,0x00,255*sizeof(WCHAR));
	memset(wszCommand,0x00,255*sizeof(WCHAR));

	uCodePage = _getmbcp();
	result=MultiByteToWideChar(uCodePage, 0, 
		szJobName,
		strlen(szJobName),
		wszJobName,255);

	result=MultiByteToWideChar(uCodePage, 0, 
		szAppName,
		strlen(szAppName),
		wszAppName,255);

	result=MultiByteToWideChar(uCodePage, 0, 
		szCommand,
		strlen(szCommand),
		wszCommand,255);

	hRes=Init(&pITaskScheduler);

	if(hRes!=S_OK)
	{
		PGPscMessageBox (hwnd,IDS_PGPERROR,IDS_NOTASKSCHL,
			MB_OK|MB_ICONSTOP);
	}
	else
	{
		if(PGPscMessageBox (hwnd,IDS_SCHLTITLE,IDS_SCHLINFO,
			MB_OKCANCEL|MB_ICONQUESTION)==IDCANCEL)
		{
			hRes=S_FALSE;
		}
		else
		{
			hRes=DeleteJob(pITaskScheduler,wszJobName);

			GetLocalTime(&SysTime);

			hRes=AddJob(hwnd,
				context,
				pITaskScheduler,
				wszJobName, 
				&SysTime,
				wszAppName,
				wszCommand);

			if(hRes!=S_OK)
			{
				PGPscMessageBox (hwnd,IDS_PGPERROR,IDS_NOADDJOB,
					MB_OK|MB_ICONSTOP);
			}
			else
			{
				PGPscMessageBox (hwnd,IDS_SCHLTITLE,IDS_SCHLOK,
					MB_OK|MB_ICONINFORMATION);
			}
		}
		Cleanup(pITaskScheduler);
	}

	return hRes;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
