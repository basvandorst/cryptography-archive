/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	$Id: DecodeStub.c,v 1.7 1999/03/10 02:41:58 heller Exp $
____________________________________________________________________________*/
#include "DecodeStub.h"

HINSTANCE g_hinst;

int getc_buffer(void *pUserValue)
{
	GETPUTINFO *gpi;

	gpi=(GETPUTINFO *)pUserValue;

	if(gpi->CancelOperation)
		return EOF;

	if((gpi->from_pointer==kBlockSize)||
	   (gpi->blockindex==0))
	{
		if(gpi->blockindex%10==0)
		{
			if(IsPGPError(SCSetProgressBar(gpi->hPrgDlg,
				gpi->blockindex*kBlockSize*100/gpi->from_max,FALSE)))
				gpi->CancelOperation=TRUE;

			if(gpi->CancelOperation)
				return EOF;
		}

		memset(gpi->inbuffer,0x00,kBlockSize);
		fread(gpi->inbuffer,1,kBlockSize,gpi->fin);

#if IS_SDA
		DecryptBlock512(gpi->SDAHeader,
			gpi->ExpandedKey,
			gpi->blockindex,
			(const PGPUInt32 *)gpi->inbuffer,
			(PGPUInt32 *)gpi->outbuffer);
#else
		memcpy(gpi->outbuffer,gpi->inbuffer,kBlockSize);
#endif // IS_SDA

		gpi->blockindex++;
		gpi->from_pointer=0;
	}

	if((gpi->blockindex-1)*kBlockSize+
		gpi->from_pointer==gpi->from_max)
	{
		return EOF;
	}
	
	return(gpi->outbuffer[gpi->from_pointer++]);
}

// putc_buffer handles the output from the compression
// engine
int putc_buffer(int c,void *pUserValue)
{
	GETPUTINFO *gpi;
	char Message[100];

	gpi=(GETPUTINFO *)pUserValue;

	if(gpi->CancelOperation)
		return 1;

	if(gpi->bFeedFilename)
	{
		// Grab New File
		// Feed in the File Size
		if(gpi->FeedIndex<4)
		{
			((char *)&(gpi->FileSize))[gpi->FeedIndex]=
				(unsigned char)c;
			gpi->FeedIndex++;
			return 1;
		}

		// Feed in the File Name
		gpi->FileName[gpi->FeedIndex-4]=
			(unsigned char) c;
		gpi->FeedIndex++;

		// We have the complete file name
		if(c==0)
		{
			// Create directory and get next file
			if(gpi->FileSize==0xFFFFFFFF)
			{
				char dirname[MAX_PATH];

				strcpy(dirname,gpi->szPrefixPath);
				strcat(dirname,gpi->FileName);

#if IS_SDA
				SCSetProgressNewFilename(gpi->hPrgDlg,"To '%s'",dirname,FALSE);
#endif // IS_SDA
				_mkdir(dirname);

#if !IS_SDA
				// Keep track of it so we can delete it later
				FileListFromFile(&gpi->fl,dirname,NULL);
				gpi->fl->IsDirectory=TRUE;
#endif
				gpi->bFeedFilename=TRUE;
			}
			// Create zero length file and get next
			else if (gpi->FileSize==0)
			{
				char inname[MAX_PATH];
				char outname[MAX_PATH];

				strcpy(inname,gpi->szPrefixPath);
				strcat(inname,gpi->FileName);
#if IS_SDA
				SCSetProgressNewFilename(gpi->hPrgDlg,"To '%s'",inname,FALSE);

				LoadString (g_hinst, IDS_SELECTFILENAME, Message, sizeof(Message));

				gpi->CancelOperation=SaveOutputFile(gpi->hwnd, 
					 Message,
					 inname, 
					 outname);

				if(gpi->CancelOperation)
					return 1;
#else
				strcpy(outname,inname);
#endif // IS_SDA
				gpi->fout=fopen(outname,"wb");

				if(gpi->fout==0)
				{
					LoadString (g_hinst, IDS_CANTCREATEFILE, Message, sizeof(Message));

					MessageBox(gpi->hwnd,Message,
					outname,
					MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);

					gpi->CancelOperation=TRUE;

					return 1;
				}
#if !IS_SDA
				// Keep track of it so we can delete it later
				FileListFromFile(&gpi->fl,outname,NULL);
#endif

				fclose(gpi->fout);
				gpi->fout=NULL;
				gpi->bFeedFilename=TRUE;
			}
			// Prepare for real data for file
			else
			{
				char inname[MAX_PATH];
				char outname[MAX_PATH];

				strcpy(inname,gpi->szPrefixPath);
				strcat(inname,gpi->FileName);
#if IS_SDA
				SCSetProgressNewFilename(gpi->hPrgDlg,"To '%s'",inname,FALSE);

				LoadString (g_hinst, IDS_SELECTFILENAME, Message, sizeof(Message));

				gpi->CancelOperation=SaveOutputFile(gpi->hwnd, 
					 Message,
					 inname, 
					 outname);

				if(gpi->CancelOperation)
					return 1;
#else
				strcpy(outname,inname);
#endif // IS_SDA

				gpi->bFeedFilename=FALSE;
				gpi->fout=fopen(outname,"wb");
			
				if(gpi->fout==0)
				{
					LoadString (g_hinst, IDS_CANTCREATEFILE, Message, sizeof(Message));

					MessageBox(gpi->hwnd,Message,
					outname,
					MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);

					gpi->CancelOperation=TRUE;

					return 1;
				}

#if !IS_SDA
				// Keep track of it so we can delete it later
				FileListFromFile(&gpi->fl,outname,NULL);
#endif
				gpi->FileIndex=0;
			}
			gpi->FeedIndex=0;
		}

		return 1;
	}

	// Feed in the File!
	fputc(c,gpi->fout);
	gpi->FileIndex++;

	if(gpi->FileIndex==gpi->FileSize)
	{
		fclose(gpi->fout);
		gpi->fout=NULL;
		gpi->bFeedFilename=TRUE;
	}

	return 1;
}


PGPError SDADecryptStub(HPRGDLG hPrgDlg,void *pUserValue)
{
	HWND hwnd;
	SDAHEADER SDAHeader;
	PGPUInt32 *ExpandedKey;
	FILE *fin;
	char inbuffer[kBlockSize];
	char outbuffer[kBlockSize];
	char filename[MAX_PATH+1];
	GETPUTINFO gpi;
	char Caption[100];
	char Message[100];
	SDAWORK *SDAWork;
	PGPError err;

	SDAWork=(SDAWORK *)pUserValue;
	SDAWork->hwndWorking=SCGetProgressHWND(hPrgDlg);

	LoadString (g_hinst, IDS_PGPERROR, Caption, sizeof(Caption));

	ExpandedKey=NULL;
	hwnd=SDAWork->hwndWorking;

	GetModuleFileName(NULL, filename, MAX_PATH);

	fin=fopen(filename,"rb");

	fseek(fin, -(int)(sizeof(SDAHEADER)), SEEK_END);
	fread(&SDAHeader,1,sizeof(SDAHEADER),fin);

#if IS_SDA
	if(memcmp(SDAHeader.szPGPSDA,"PGPSDA",6)!=0)
	{
		LoadString (g_hinst, IDS_NOTVALIDSDA, Message, sizeof(Message));

		MessageBox(hwnd,Message,
			Caption,
			MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
		return kPGPError_UserAbort;
	}

	// Fetch the passphrase from the user
	err=SDAGetPassphrase(SDAWork,&SDAHeader,&ExpandedKey);

	if(err!=kPGPError_NoErr)
		return err;
#else
	if(memcmp(SDAHeader.szPGPSDA,"PGPSEA",6)!=0)
	{
		LoadString (g_hinst, IDS_NOTVALIDSEA, Message, sizeof(Message));

		MessageBox(hwnd,Message,
			Caption,
			MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);
		return kPGPError_UserAbort;
	}
#endif // IS_SDA

	fseek(fin, SDAHeader.offset, SEEK_SET );

	memset(&gpi,0x00,sizeof(GETPUTINFO));

	gpi.fin=fin;
	gpi.NumFiles=SDAHeader.NumFiles;
	gpi.SDAHeader=&SDAHeader;
	gpi.SDAWork=SDAWork;
	gpi.ExpandedKey=ExpandedKey;
	gpi.blockindex=0;
	gpi.outbuffer=outbuffer;
	gpi.inbuffer=inbuffer;
	gpi.hwnd=hwnd;
	gpi.CancelOperation=FALSE;
	gpi.from_max=SDAHeader.CompLength;
	gpi.bFeedFilename=TRUE;
	gpi.hPrgDlg=hPrgDlg;

#if !IS_SDA
	err=SEAGetTempPath(&gpi);

	if(err!=kPGPError_NoErr)
		return err;
#else
	GetPrefixPath(&gpi);
#endif

	Deflate_Decompress(&gpi);

	// In case of cancellation or unknown abort
	if(gpi.fout!=NULL)
		fclose(gpi.fout);

	memset(inbuffer,0x00,kBlockSize);
	memset(outbuffer,0x00,kBlockSize);

	SCSetProgressBar(hPrgDlg,100,TRUE);	

#if IS_SDA
	memset(ExpandedKey,0x00,sizeof(PGPUInt32)*32);	
	free(ExpandedKey);	
	fclose(fin);
#else
	SEACreateExecDelete(fin,&gpi);
#endif

	if(gpi.CancelOperation)
		return kPGPError_UserAbort;

	return kPGPError_NoErr;
}

void SDA(HWND hwnd)
{
	SDAWORK SDAWork;
	PGPError err;

	memset(&SDAWork,0x00,sizeof(SDAWORK));

	err=SCProgressDialog(hwnd,SDADecryptStub,&SDAWork,
		  0,NULL,
		  NULL,NULL,0);
}

LRESULT CALLBACK HiddenWndProc (HWND hwnd, UINT iMsg, 
								WPARAM wParam, LPARAM lParam)
{
	 RECT rc;
	 
     switch (iMsg)
     {
         case WM_CREATE :

  			GetWindowRect (hwnd, &rc);
			SetWindowPos (hwnd, NULL,
				(GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
				(GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2,
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
			                
            return 0 ;
	 }
 
    return DefWindowProc (hwnd, iMsg, wParam, lParam) ;
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
		    LPSTR szCmdLine, int iCmdShow)
{
	HWND hwnd;
	WNDCLASS wndclass; 

	g_hinst = hInstance ;
	
	InitCommonControls();

	wndclass.style			= 0;
	wndclass.lpfnWndProc	= (WNDPROC)HiddenWndProc;
	wndclass.cbClsExtra		= 0;
	wndclass.cbWndExtra		= 0;
	wndclass.hInstance		= hInstance;
	wndclass.hIcon			= 0;
	wndclass.hCursor		= 0;
	wndclass.hbrBackground	= 0;
	wndclass.lpszMenuName	= 0;
	wndclass.lpszClassName	= "PGPsda_Hidden_Window";

	RegisterClass(&wndclass);

	hwnd = CreateWindow("PGPsda_Hidden_Window", 
		"PGPsda_Hidden_Window", WS_OVERLAPPEDWINDOW, 
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, NULL, NULL, hInstance, NULL );

	ShowWindow(hwnd, SW_HIDE);
	UpdateWindow(hwnd);

	SDA(hwnd);

#if !IS_SDA
	SEADoInstallerCommand();
#endif

	return TRUE ;
}     

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/