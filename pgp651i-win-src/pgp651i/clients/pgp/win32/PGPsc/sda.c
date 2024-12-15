/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPsda.c - Code for creating SDA's
	

	$Id: sda.c,v 1.19 1999/03/31 23:23:37 wjb Exp $
____________________________________________________________________________*/
#include "precomp.h"
#include "sda.h"

// This is the structure that is passed into
// our getc_buffer and putc_buffer routines from
// LZSS. We need enough info here to read and write
// the cancel the file operations.
typedef struct
{
	FILE							*fin;
	FILE							*fout;
	SDAHEADER						*SDAHeader;
	DWORD							blockindex;
	DWORD							from_pointer;
	DWORD							to_pointer;
	DWORD							from_max;
	unsigned char					*outbuffer;
	unsigned char					*inbuffer;
	HWND							hwnd;
	PGPSymmetricCipherContextRef	CASTContext;
	BOOL							CancelOperation;
	MYSTATE							*ms;
	FILELIST						*fl;
	BOOL							bFeedFilename;
	char							FileName[MAX_PATH+1];
	DWORD							FileSize;
	DWORD							FileIndex;
	DWORD							FeedIndex;
	DWORD							PathHead;
	HPRGDLG							hPrgDlg;
} GETPUTINFO;

	void 
CAST5encryptCFBdbl(
	PGPSymmetricCipherContextRef CASTContext,
	PGPUInt32			iv0, 
	PGPUInt32			iv1, 
	PGPUInt32			iv2, 
	PGPUInt32			iv3, 
	const PGPUInt32*	src, 
	PGPUInt32*			dest, 
	PGPUInt32			len)
{
	PGPError err;
	PGPUInt32 iv[4] = {iv0, iv1, iv2, iv3};
	while (len--) 
	{
		err=PGPSymmetricCipherEncrypt( CASTContext,
			(const PGPUInt8*) iv,
			(PGPUInt8*) iv);

//		We've replaced this restaurants original coffee with new
//		SDK coffee...

//		CAST5encrypt((const PGPUInt8*) iv, (PGPUInt8*) iv, xkey);
		*dest++	= iv[0]	^= *src++;
		*dest++	= iv[1]	^= *src++;

		err=PGPSymmetricCipherEncrypt( CASTContext,
			(const PGPUInt8*) (iv+2),
			(PGPUInt8*) (iv+2));

//		We've replaced this restaurants original coffee with new
//		SDK coffee...

//		CAST5encrypt((const PGPUInt8*) (iv+2), (PGPUInt8*) (iv+2), xkey);
		*dest++	= iv[2]	^= *src++;
		*dest++	= iv[3]	^= *src++;
	}
}

	void
EncryptBlock512(
	PGPSymmetricCipherContextRef CASTContext,
	SDAHEADER				*SDAHeader,
	PGPUInt32				blockNumber,
	const PGPUInt32 *		src,
	PGPUInt32 *				dest)
{
	int			i;
	PGPUInt32	sum0, sum1;
	PGPUInt32	iv0, iv1;
			
	sum0	= SDAHeader->Salt.saltLongs[ 0 ] + blockNumber;
	sum1	= SDAHeader->Salt.saltLongs[ 1 ] + blockNumber;

	for (i = 0; i < kBlockSizeInUInt32 - 2; ++i) 
	{
		sum1	+= src[i];
		sum0	+= sum1;		// Simple Fletcher checksum of block
	}
	
	iv0 = sum0 ^ src[ kBlockSizeInUInt32 - 2 ];
	iv1 = sum1 ^ src[ kBlockSizeInUInt32 - 1 ];
	
	CAST5encryptCFBdbl(CASTContext, iv0, iv1, ~iv0,
			~iv1, src, dest, kBlockSize / ( 8 + 8) );

	dest[ kBlockSizeInUInt32 - 2 ] ^= sum0;
	dest[ kBlockSizeInUInt32 - 1 ] ^= sum1;
}

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

			strcpy(gpi->FileName,JustFile(gpi->fl->name));

			SCSetProgressNewFilename(gpi->hPrgDlg,"From '%s'",gpi->fl->name,TRUE);

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
					char szMessage[256];

					LoadString (g_hinst, IDS_COULDNOTOPEN, szMessage, sizeof(szMessage));

					MessageBox(gpi->hwnd,
						szMessage,
						gpi->FileName,MB_OK);

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
		c=gpi->fl->name[gpi->PathHead+gpi->FeedIndex-4];
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

	if(gpi->FileIndex%512==0)
	{
		if(IsPGPError(SCSetProgressBar(gpi->hPrgDlg,
			gpi->FileIndex*100/gpi->FileSize,FALSE)))
			gpi->CancelOperation=TRUE;

		// Trick the encryptor engine here. We'll delete
		// the file later
		if(gpi->CancelOperation)
			return EOF;
	}

	if(gpi->FileIndex==gpi->FileSize)
	{
		fclose(gpi->fin);
		gpi->fin=0;
		gpi->bFeedFilename=TRUE;
		SCSetProgressBar(gpi->hPrgDlg,100,TRUE);
	}

	return c;
}

// putc_buffer takes output from compression engine,
// encodes it, and writes it to disk
int putc_buffer(int invoer,void *pUserInfo)
{
	GETPUTINFO *gpi;

	gpi=(GETPUTINFO *)pUserInfo;

	gpi->inbuffer[gpi->to_pointer++] = (unsigned char) invoer;
	gpi->SDAHeader->CompLength++;

	if(gpi->to_pointer==kBlockSize)
	{
		EncryptBlock512(gpi->CASTContext,
			gpi->SDAHeader,
			gpi->blockindex,
			(const PGPUInt32 *)gpi->inbuffer,
			(PGPUInt32 *)gpi->outbuffer);

		fwrite(gpi->outbuffer,1,gpi->to_pointer,gpi->fout);

		gpi->blockindex++;
		memset(gpi->inbuffer,0x00,kBlockSize);
		gpi->to_pointer=0;
	}

	return 1;
}

PGPError SDAEncryptFile(MYSTATE *ms,
		PGPSymmetricCipherContextRef CASTContext)
{
	SDAHEADER *SDAHeader;
	FILE *fout;
	char inbuffer[kBlockSize];
	char outbuffer[kBlockSize];
	char szOutput[MAX_PATH+1];
	PGPFileSpecRef filespec;
	char *fullPathPtr;
	BOOL CancelOperation;
	GETPUTINFO gpi;
	DWORD dwStubSize;
	HRSRC hRCStub;
	HGLOBAL hGBStub;
	char *pStubData;
	FILELIST *flnext,*fl;
	char *lastslash;
	BOOL OneDirectory;
	char szCaption[256];

	// Reverse list since we need directories first
	fl=NULL;

	while(ms->ListHead!=NULL)
	{
		flnext=ms->ListHead->next;
		
		ms->ListHead->next=fl;
		fl=ms->ListHead;

		ms->ListHead=flnext;
	}

	// Now same, but reversed
	ms->ListHead=fl;

	// Check to see if we only have one directory with contents
	OneDirectory=ms->ListHead->IsDirectory;

	SDAHeader=ms->SDAHeader;

	strcpy(szOutput,ms->ListHead->name);
	strcat(szOutput,".sda.exe");

	if(OneDirectory)
	{
		LoadString (g_hinst, IDS_FILECONFLICT, szCaption, sizeof(szCaption));

		// Do automatically if possible
		CancelOperation=SaveOutputFile(ms->context,
			ms->hwndWorking, 
			szCaption,
			szOutput, 
			&filespec,
			FALSE);
	}
	else
	{
		LoadString (g_hinst, IDS_CONFIRMSDANAME, szCaption, sizeof(szCaption));

		// Always check with the user first
		CancelOperation=SaveOutputFile(ms->context,
			ms->hwndWorking, 
			szCaption,
			szOutput, 
			&filespec,
			TRUE);
	}

	if(CancelOperation)
		return kPGPError_UserAbort;

	PGPGetFullPathFromFileSpec( filespec,
		&fullPathPtr);

	strcpy(szOutput,fullPathPtr);
	PGPFreeData(fullPathPtr);
	PGPFreeFileSpec(filespec);

	fout=fopen(szOutput,"wb");

	if(fout==0)
	{
		PGPscMessageBox (ms->hwndWorking,IDS_PGPERROR,IDS_COULDNOTOPENFILE,
				MB_OK|MB_ICONSTOP);

		return kPGPError_UserAbort;
	}

	// Copy SDA.exe prefix executable into SDA file and
	// set header offset so we can find data again.
	// Write out stub to disk from resources
	hRCStub=FindResource(g_hinst,
		MAKEINTRESOURCE(IDR_SDASTUB),
		RT_RCDATA);

	dwStubSize=SizeofResource(g_hinst,hRCStub);

	hGBStub=LoadResource(g_hinst,hRCStub);
	pStubData=(char *)LockResource(hGBStub);

	fwrite(pStubData,1,dwStubSize,fout);
	SDAHeader->offset=dwStubSize;

	// Initialize variables for compression call
	memset(&gpi,0x00,sizeof(GETPUTINFO));

	gpi.fout=fout;
	gpi.SDAHeader=SDAHeader;
	gpi.blockindex=0;
	gpi.outbuffer=outbuffer;
	gpi.inbuffer=inbuffer;
	gpi.hwnd=ms->hwndWorking;
	gpi.CASTContext=CASTContext;
	gpi.from_pointer=0;
	gpi.CancelOperation=FALSE;
	gpi.ms=ms;
	gpi.bFeedFilename=TRUE;
	gpi.fl=fl;
	gpi.hPrgDlg=(HPRGDLG)GetWindowLong(ms->hwndWorking, GWL_USERDATA);

	// Find beginning of SDA directory tree
	lastslash=strrchr(gpi.fl->name,'\\');
	if(lastslash==NULL)
		lastslash=gpi.fl->name;
	else
		lastslash++;

	gpi.PathHead=lastslash-gpi.fl->name;

	Deflate_Compress(&gpi);

	if(gpi.fin)
		fclose(gpi.fin);

	fl=NULL;

	// Re-reverse list since we may need to delete
	while(ms->ListHead!=NULL)
	{
		flnext=ms->ListHead->next;
		
		ms->ListHead->next=fl;
		fl=ms->ListHead;

		ms->ListHead=flnext;
	}

	// Now same, but reversed
	ms->ListHead=fl;

	// Write out the last block since compress doesn't
	// know how we are delaying writes
	if(gpi.to_pointer!=0)
	{
		EncryptBlock512(gpi.CASTContext,
			gpi.SDAHeader,
			gpi.blockindex,
			(const PGPUInt32 *)gpi.inbuffer,
			(PGPUInt32 *)gpi.outbuffer);

		fwrite(gpi.outbuffer,1,kBlockSize,gpi.fout);
	}

	fwrite(SDAHeader,1,sizeof(SDAHEADER),fout);

	memset(inbuffer,0x00,kBlockSize);
	memset(outbuffer,0x00,kBlockSize);

	fclose(fout);

	if(gpi.CancelOperation)
	{
		remove(szOutput);
		return kPGPError_UserAbort;
	}

	return kPGPError_NoErr;
}

PGPError SDAEncryptStub(MYSTATE *ms)
{
	SDAHEADER *SDAHeader;
	// SHA is 160 bits (20*8)
	byte HashedPassphrase[20];
	PGPError err;
	PGPMemoryMgrRef memMgr;
	PGPHashContextRef SHAContext;
	PGPSymmetricCipherContextRef CASTContext;
	PGPUInt64 endTicks;
	PGPUInt8		j;
	PGPUInt16		i;

	memMgr=PGPGetContextMemoryMgr(ms->context);

	SDAHeader=ms->SDAHeader;

	memcpy((char *)&(SDAHeader->szPGPSDA),"PGPSDA",6);

	// Get random bits for salt and IV's
	do
	{
		err=PGPContextGetRandomBytes (ms->context, 
			(char *)&(SDAHeader->Salt.saltBytes), 8);

		if(err==kPGPError_OutOfEntropy)
		{
			err=PGPclRandom(ms->context,ms->hwndWorking, 8*8);

			if(IsPGPError(err))
				return err;
		}
	} while(err==kPGPError_OutOfEntropy);

	// Hash the passphrase
	err=PGPNewHashContext(memMgr,kPGPHashAlgorithm_SHA,&SHAContext);

	// Salt it to eliminate common passphrases across files
	err=PGPContinueHash(SHAContext,
		SDAHeader->Salt.saltBytes, 
		sizeof(SDAHeader->Salt.saltBytes));

	// We hash the passphrase in with a rotating counter byte 
	// an arbitrary number of times based on the processing 
	// power of the computer we're running on up to a maximum of 
	// 16000.

	endTicks = clock() + CLOCKS_PER_SEC/2;	// 500 ms

	for (i=0, j=0; (clock() < endTicks) && (i < 16000); 
		i++, j++)
	{
		err=PGPContinueHash(SHAContext,
			ms->ConvPassPhrase,strlen(ms->ConvPassPhrase));
		err=PGPContinueHash(SHAContext,&j, 1);
	}

	SDAHeader->hashReps = i;

	err=PGPFinalizeHash(SHAContext,HashedPassphrase);
	err=PGPFreeHashContext(SHAContext);

	err=PGPNewSymmetricCipherContext(memMgr,kPGPCipherAlgorithm_CAST5,
		128/8,&CASTContext);

	err=PGPInitSymmetricCipher( CASTContext,HashedPassphrase);

	memcpy(&(SDAHeader->CheckBytes),HashedPassphrase,8);

	err=PGPSymmetricCipherEncrypt( CASTContext,
		(BYTE *)&(SDAHeader->CheckBytes),
		(BYTE *)&(SDAHeader->CheckBytes));

	err=SDAEncryptFile(ms,CASTContext);

	PGPFreeSymmetricCipherContext(CASTContext);

	memset(HashedPassphrase,0x00,20);

	return err;
}

PGPError SDA(MYSTATE *ms)
{
	SDAHEADER SDAHeader;

	memset(&SDAHeader,0x00,sizeof(SDAHEADER));
//	strcpy(SDAHeader.filename,JustFile(ms->fileName));
	ms->SDAHeader=&SDAHeader;

	return SDAEncryptStub(ms);
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
