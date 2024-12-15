/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: Utils.c,v 1.2 1999/03/10 03:04:34 heller Exp $
____________________________________________________________________________*/

#include "sea.h"

UINT FileListFromFile(FILELIST **filelist,char *filename,BOOL *UserCancel)
{
	FILELIST *Current;

	if(UserCancel!=NULL)
	{
		if(*UserCancel)
		{
			return FALSE;
		}
	}

	Current=(FILELIST *)malloc(sizeof(FILELIST));
    memset(Current,0x00,sizeof(FILELIST));

	strcpy(Current->name,filename);
	Current->next=*filelist;
	*filelist=Current;

	return TRUE;
}

UINT FileListFromDirectory(FILELIST **filelist,char *directory,BOOL *UserCancel)
{
	HANDLE hFindFile;
	WIN32_FIND_DATA FindFileData;
	BOOL RetVal=FALSE;
	BOOL KeepGoing;
	char search[MAX_PATH];
	char filename[MAX_PATH];

	if(directory[strlen(directory)]!='\\')
		strcat(directory,"\\");

	strcpy(search,directory);

	strcat(search,"*.*");

	hFindFile=FindFirstFile(search,&FindFileData);

	if(hFindFile!=INVALID_HANDLE_VALUE)
	{
		RetVal=TRUE;

		while(RetVal==TRUE)
		{
			// Our quick fix callback to allow user to back out
			// of operation
			if(UserCancel!=NULL)
			{
				if(*UserCancel)
				{
					FindClose(hFindFile);
					return FALSE;
				}
			}

			strcpy(filename,directory);
			strcat(filename,FindFileData.cFileName);

			if((FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
				!=FILE_ATTRIBUTE_DIRECTORY)
			{
				FileListFromFile(filelist,filename,UserCancel);
			}
			else
			{
				if((strcmp(FindFileData.cFileName,"."))&&
					(strcmp(FindFileData.cFileName,"..")))
				{
					KeepGoing=FileListFromFile(filelist,filename,UserCancel);
					if(!KeepGoing)
					{
						FindClose(hFindFile);
						return FALSE;
					}

					(*filelist)->IsDirectory=TRUE;
					KeepGoing=FileListFromDirectory(filelist,filename,UserCancel);

					if(!KeepGoing)
					{
						FindClose(hFindFile);
						return FALSE;
					}
				}
			}
			RetVal=FindNextFile(hFindFile,&FindFileData); 
		}

		FindClose(hFindFile);
	}

	return TRUE;
} 

BOOL AddToFileList(FILELIST **filelist,char *filename,BOOL *UserCancel)
{
	DWORD dwFileAttributes;
	UINT RetVal;

	dwFileAttributes=GetFileAttributes(filename);

	if(dwFileAttributes==0xFFFFFFFF)
		return TRUE;

	if((dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
		==FILE_ATTRIBUTE_DIRECTORY)
	{
		RetVal=FileListFromFile(filelist,filename,UserCancel);
		if(!RetVal)
			return FALSE;

		(*filelist)->IsDirectory=TRUE;
		RetVal=FileListFromDirectory(filelist,filename,UserCancel);
	}
	else
	{
		RetVal=FileListFromFile(filelist,filename,UserCancel);
	}

	return RetVal;
}

BOOL FreeFileList(FILELIST *FileList)
{
	FILELIST *FreeAtLast;

	while(FileList!=0)
	{
		FreeAtLast=FileList;
		FileList=FileList->next;
		memset(FreeAtLast,0x00,sizeof(FILELIST));
		free(FreeAtLast);
	}

	return TRUE;
}


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
