/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	sea.c - Code for creating SEA's
	

	$Id: sea.c,v 1.7 1999/03/24 20:54:35 wjb Exp $
____________________________________________________________________________*/
#include "sea.h"

// This is the structure that is passed into
// our getc_buffer and putc_buffer routines from
// LZSS. We need enough info here to read and write
// the cancel the file operations.
typedef struct
{
	FILE							*fin;
	FILE							*fout;
	SDAHEADER						*SDAHeader;
	BOOL							CancelOperation;
	FILELIST						*fl;
	BOOL							bFeedFilename;
	DWORD							PathHead;
	char							FileName[MAX_PATH+1];
	DWORD							FileSize;
	DWORD							FileIndex;
	DWORD							FeedIndex;
} GETPUTINFO;

// getc_buffer feeds characters into the compression
// engine
int getc_buffer(void *pUserInfo)
{
	int c;
	GETPUTINFO *gpi;

	gpi=(GETPUTINFO *)pUserInfo;

	if(gpi->CancelOperation)
		return EOF;

	if(gpi->bFeedFilename)
	{
		if(gpi->fl==NULL)
			return EOF;

		// Generate New File
		if(gpi->FeedIndex==0)
		{
			gpi->SDAHeader->NumFiles++;

			strcpy(gpi->FileName,&(gpi->fl->name[gpi->PathHead]));

			printf("Adding %s\n",gpi->FileName);

			if(gpi->fl->IsDirectory)
			{
				gpi->FileSize=0xFFFFFFFF;
			}
			else
			{
				struct _stat buf;

				_stat(gpi->fl->name, &buf );
				gpi->FileSize=buf.st_size;

				gpi->fin=fopen(gpi->fl->name,"rb");

				if(gpi->fin==0)
				{
					printf("Could not open file for encoding\n");
					gpi->CancelOperation=TRUE;
					return EOF;
				}

				gpi->FileIndex=0;
			}
		}

		// Feed in the File Size
		if(gpi->FeedIndex<4)
		{
			c=(unsigned char)((char *)&(gpi->FileSize))[gpi->FeedIndex];
			gpi->FeedIndex++;
			return c;
		}

		// Feed in the File Name
		c=gpi->FileName[gpi->FeedIndex-4];
		gpi->FeedIndex++;

		if(c==0)
		{
			// If directory or empty just repeat for next
			if((gpi->fl->IsDirectory)||(gpi->FileSize==0))
			{
				gpi->bFeedFilename=TRUE;
			}
			else
			{
				gpi->bFeedFilename=FALSE;
			}
			gpi->FeedIndex=0;
			gpi->fl=gpi->fl->next;
		}

		return c;
	}

	// Feed in the File!
	c=fgetc(gpi->fin);
	gpi->FileIndex++;

	if(gpi->FileIndex==gpi->FileSize)
	{
		fclose(gpi->fin);
		gpi->bFeedFilename=TRUE;
	}

	return c;
}

// putc_buffer takes output from compression engine,
// encodes it, and writes it to disk
int putc_buffer(int invoer,void *pUserInfo)
{
	GETPUTINFO *gpi;

	gpi=(GETPUTINFO *)pUserInfo;

	fputc(invoer,gpi->fout);
	gpi->SDAHeader->CompLength++;

	return 1;
}


// Fake routine since we aren't using compression
void Deflate_Compress(void *gpi)
{
	int c;

	while((c=getc_buffer(gpi))!=EOF)
	{
		putc_buffer(c,gpi);
	}
}


int main(int argc, char* argv[])
{
	SDAHEADER SDAHeader;
	FILE *fout;
	char szOutput[MAX_PATH+1];
	char szDirectory[MAX_PATH+1];
	FILELIST *flindex,*fl,*flnext;
	GETPUTINFO gpi;
	DWORD dwStubSize;
	HRSRC hRCStub;
	HGLOBAL hGBStub;
	char *pStubData;
	HMODULE hModule;

	if(argc!=3)
	{
		printf("MakeSEA <output filename> <input directory>\n");
		return 0;
	}

	strcpy(szOutput,argv[1]);
	strcpy(szDirectory,argv[2]);

	fout=fopen(szOutput,"wb");

	if(fout==0)
	{
		printf("Could not open output file for writing\n");
		return 0;
	}

	memset(&SDAHeader,0x00,sizeof(SDAHEADER));
	memcpy(&(SDAHeader.szPGPSDA),"PGPSEA",6);

	// Write out stub to disk from resources
	hModule=GetModuleHandle(NULL);

	hRCStub=FindResource(hModule,
		MAKEINTRESOURCE(IDR_SEASTUB),
		RT_RCDATA);

	dwStubSize=SizeofResource(hModule,hRCStub);

	hGBStub=LoadResource(hModule,hRCStub);
	pStubData=(char *)LockResource(hGBStub);

	fwrite(pStubData,1,dwStubSize,fout);
	SDAHeader.offset=dwStubSize;

	// Get files from directory parameter
	fl=flindex=NULL;

	AddToFileList(&flindex,szDirectory,NULL);

	// Reverse list since we need directories first
	while(flindex!=NULL)
	{
		flnext=flindex->next;
		
		flindex->next=fl;
		fl=flindex;

		flindex=flnext;
	}

	memset(&gpi,0x00,sizeof(GETPUTINFO));

	gpi.fout=fout;
	gpi.SDAHeader=&SDAHeader;
	gpi.fl=fl;
	gpi.bFeedFilename=TRUE;

	// Find beginning of SEA directory tree
	gpi.PathHead=strlen(gpi.fl->name);
	if(gpi.fl->name[gpi.PathHead]!='\\')
		gpi.PathHead++;
	gpi.fl=gpi.fl->next;

	Deflate_Compress(&gpi);

	FreeFileList(fl);

	fwrite(&SDAHeader,1,sizeof(SDAHEADER),fout);

	fclose(fout);

	if(gpi.CancelOperation)
	{
		remove(szOutput);
	}

	return 0;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
