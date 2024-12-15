/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: WFnt.c,v 1.11 1999/04/01 20:49:59 wjb Exp $
____________________________________________________________________________*/
#include "precomp.h"

//
// Invalid longlong number
//
#define LLINVALID		((ULONGLONG) -1)

//
// Size of the buffer we read file mapping information into.
// The buffer is big enough to hold the 16 bytes that 
// come back at the head of the buffer (the number of entries 
// and the starting virtual cluster), as well as 512 pairs
// of [virtual cluster, logical cluster] pairs.
//
#define	FILEMAPSIZE		(512+2)

//
// Size of the bitmap buffer we pass in. Its large enough to
// hold information for the 16-byte header that's returned
// plus the indicated number of bytes, each of which has 8 bits 
// (imagine that!)
//
#define BITMAPBYTES		4096
#define BITMAPSIZE		(BITMAPBYTES+2*sizeof(ULONGLONG))

//
// Bit shifting array for efficient processing of the bitmap
//
BYTE		BitShift[] = { 1, 2, 4, 8, 16, 32, 64, 128 };

// StatusToError
//
// Takes the status messages and turns them into our
// native wipe free codes.
DWORD StatusToError( NTSTATUS Status )
{
	switch( Status ) 
	{
	case STATUS_SUCCESS:
		return WFE_NOERROR;
	case STATUS_INVALID_PARAMETER:
		return WFE_INVALPARAM; // Must be the metafile
	case STATUS_BUFFER_TOO_SMALL:
		return WFE_OUTOFMEMORY;
	case STATUS_ALREADY_COMMITTED:
		return WFE_ALREADYCOMMIT; // NTFS won't let us. Try anyway
	case STATUS_INVALID_DEVICE_REQUEST:
		return WFE_DRIVENOTSUPPORTED;
	case STATUS_DEVICELOCKED:
		return WFE_COULDNOTLOCK;
	default:
		return WFE_CLUSTERERROR;
	}		  
}

// OpenVolume
//
// Grabs the handle of our logical volume
DWORD OpenVolume( VOLINFO *vi ) 
{
	static char			volumeName[] = "\\\\.\\A:";

	//
	// open the volume
	//
	volumeName[4] = (char)vi->vol0 + 'A'; 
	vi->VolHandle = CreateFile( volumeName, GENERIC_READ, 
					FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 
					0, 0 );
	if( vi->VolHandle == INVALID_HANDLE_VALUE )	{

		return GetLastError();
	}

	return ERROR_SUCCESS;
}

// WipeCluster
//
// WipeCluster uses the patterns stored in filename to 
// eradicate the data in the target cluster of the logical
// drive specified.

DWORD NTWipeCluster( VOLINFO *vi, char *fileName, ULONGLONG Vcn, ULONGLONG target)
{
	DWORD						status;
	IO_STATUS_BLOCK				ioStatus;
	HANDLE						sourceFile;
	LARGE_INTEGER				startVcn, targetLcn;
	DWORD						numClusters;
	MOVEFILE_DESCRIPTOR			moveFile;

	DWORD RetVal;

	RetVal=WFE_NOERROR;

	if(vi->bCountClusters)
	{
		vi->dwClusterCount++;

		if(vi->dwFS==FS_NTFS)
			// NTFS works with 16 cluster segments
			vi->dwClusterCount=vi->dwClusterCount+15; 

		if(UserPressedCancel(vi))
			RetVal=WFE_USERCANCEL;
		return RetVal;
	}

	// 
	// Get numeric parameters
	//
	startVcn.QuadPart=Vcn;
	targetLcn.QuadPart=target;
	numClusters=1;

	if(vi->dwFS==FS_NTFS)
		numClusters=16; // NTFS works with 16 cluster segments
	//
	// Open the file
	//
	sourceFile = CreateFile( fileName, GENERIC_READ, 
					FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 
					FILE_FLAG_NO_BUFFERING, 0 );
	if( sourceFile == INVALID_HANDLE_VALUE ) 
	{
		// Could not open file
		return WFE_COULDNOTREAD ;
	}

	//
	// Setup movefile descriptor and make the call
	//
	memset(&moveFile,0x00,sizeof(MOVEFILE_DESCRIPTOR));
	memset(&ioStatus,0x00,sizeof(IO_STATUS_BLOCK));

	moveFile.FileHandle = sourceFile;
	moveFile.StartVcn = startVcn;
	moveFile.TargetLcn = targetLcn;
	moveFile.NumVcns = numClusters;

	status = NtFsControlFile( vi->VolHandle, NULL, NULL, 0, &ioStatus,
						FSCTL_MOVE_FILE,
						&moveFile, sizeof( moveFile ),
						NULL, 0 );

	RetVal=StatusToError(status);

	// 
	// If the operation is pending, wait for it to finish
	//
	if( status == STATUS_PENDING ) 
	{
		WaitForSingleObject( sourceFile, INFINITE ); 

		RetVal=StatusToError(ioStatus.Status);
	}

	CloseHandle(sourceFile);
	
	// Handle our progress bars and look out for user cancel
	{
		char StrRes[500];
		char statusmsg[500];

		vi->dwClustersWritten=vi->dwClustersWritten+1;
		
		if(vi->dwFS==FS_NTFS)
			// NTFS works with 16 cluster segments
			vi->dwClustersWritten=vi->dwClustersWritten+15; 

		if(vi->dwClustersWritten%10==0)
		{
			float pos;

			LoadString (g_hinst, IDS_PERCENTWRITTEN, StrRes, sizeof(StrRes));

			if(RetVal==WFE_NOERROR)
			{
				sprintf(statusmsg,
					StrRes,
					vi->dwClustersWritten,vi->dwClusterCount);
			}
			else
			{
				sprintf(statusmsg,
					"An error occurred... %d of %d",
					vi->dwClustersWritten,vi->dwClusterCount);
			}

			if(vi->dwFS==FS_NTFS)
			{
				if(RetVal==WFE_INVALPARAM)
				{
					sprintf(statusmsg,
						"Skipping NTFS metadata. %d of %d.",
						vi->dwClustersWritten,vi->dwClusterCount);
				}
				else if(RetVal==WFE_ALREADYCOMMIT)
				{
					sprintf(statusmsg,
						"NTFS won't release cluster. %d of %d.",
						vi->dwClustersWritten,vi->dwClusterCount);
				}
			}

			StatusMessage(statusmsg, FALSE);

			pos = 10 + 
				( (float)vi->dwClustersWritten / 
				  (float)vi->dwClusterCount )
				* 90;

			WipeProgress((int)pos); 
		}
	}

	if(UserPressedCancel(vi))
		RetVal=WFE_USERCANCEL;

	return RetVal;
}

// WipeFreeClusters
//
// Given a logical drive number, routine iterates through free
// clusters and calls WipeCluster to zap the information stored
// there. Note that if changes take place on the drive while
// routine is executes, some clusters may be missed.

DWORD NTWipeFreeClusters( VOLINFO *vi)
{
	DWORD						status,wipestatus;
	PBITMAP_DESCRIPTOR			bitMappings;
	ULONGLONG					startLcn;
	ULONGLONG					nextLcn;
	IO_STATUS_BLOCK				ioStatus;
	ULONGLONG					i;
	BYTE						BitMap[ BITMAPSIZE ];
	char						fileName[MAX_PATH];
	
	//
	// Make the name into a real pathname
	//
	sprintf(fileName, "%C:\\%s", vi->vol0+'A', "PGPwipepattern.tmp" );

	//
	// Start scanning at the cluster offset the user specifies
	//
	bitMappings = (PBITMAP_DESCRIPTOR) BitMap;
	nextLcn = 0;

	while( !(status = NtFsControlFile( vi->VolHandle, NULL, NULL, 0, &ioStatus,
						FSCTL_GET_VOLUME_BITMAP,
						&nextLcn, sizeof( ULONGLONG ),
						bitMappings, BITMAPSIZE )) ||
			 status == STATUS_BUFFER_OVERFLOW ||
			 status == STATUS_PENDING ) 
	{

		// 
		// If the operation is pending, wait for it to finish
		//
		if( status == STATUS_PENDING ) 
		{
			
			WaitForSingleObject( vi->VolHandle, INFINITE );
			
			//
			// Get the status from the status block
			//
			if( ioStatus.Status != STATUS_SUCCESS && 
				ioStatus.Status != STATUS_BUFFER_OVERFLOW ) 
			{
				// Error out
				return StatusToError(ioStatus.Status);
			}
		}

		//
		// Scan through the returned bitmap info, looking for empty clusters
		//
		startLcn = bitMappings->StartLcn;
	
		for( i = 0; i < min( bitMappings->ClustersToEndOfVol, 8*BITMAPBYTES); i++ ) 
		{

			if( !(bitMappings->Map[ i/8 ] & BitShift[ i % 8 ])) 
			{
				wipestatus=NTWipeCluster( vi, fileName, 0, startLcn + i);

				if((wipestatus==WFE_NOERROR)&&(vi->SlackMove))
				{
					// Hey, we were able to do it. Save this cluster for 
					// later slack movement operations since we know its
					// free
					if(vi->PatternBuffer==0)
						vi->PatternBuffer=startLcn+i;
					else if(vi->SlackBufferIndex<SLACKBUFFER)
					{
						vi->SlackBuffer[vi->SlackBufferIndex]=startLcn+i;
						vi->SlackBufferIndex=vi->SlackBufferIndex+1;
					}
				}

				if(((wipestatus==WFE_ALREADYCOMMIT)||(wipestatus==WFE_INVALPARAM))&&(vi->dwFS==FS_NTFS))
				{
					// Couldn't do the move because of MFT, or user is mucking
					// around.... just skip it and we'll get it on the next pass.
					// Only applies to NTFS
					wipestatus=WFE_NOERROR;
				}

				if(wipestatus!=WFE_NOERROR)
				{
					return wipestatus;
				}
			} 

			if(vi->dwFS==FS_NTFS)
				i=i+15; // NTFS works with 16 cluster segments
		}
	
		//
		// End of volume?
		//
		if( status != STATUS_BUFFER_OVERFLOW ) 
		{
		
			return StatusToError(STATUS_SUCCESS);
		}				

		//
		// Move to the next block
		//
		nextLcn = bitMappings->StartLcn + i;
	}

	//
	// We only get here when there's an error
	//

	return StatusToError(status);
}



// GetLastClusterOfFile
//
// Loops through till it finds the last cluster in the chain

DWORD GetLastClusterOfFile(VOLINFO *vi, char *fileName, SLACKMOVE *sm)
{
	DWORD						status;
	int							i;
	HANDLE						sourceFile;
	IO_STATUS_BLOCK				ioStatus;
	ULONGLONG					startVcn;
	PGET_RETRIEVAL_DESCRIPTOR	fileMappings;
	int							lines = 0;
	ULONGLONG					FileMap[ FILEMAPSIZE ];

	// Save name of file for later
	sm->name=fileName;

	//
	// Open the file
	//
	sourceFile = CreateFile( fileName, GENERIC_READ, 
					FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 
					FILE_FLAG_NO_BUFFERING, 0 );

	if( sourceFile == INVALID_HANDLE_VALUE ) 
		return WFE_COULDNOTREAD;

	//
	// Start dumping the mapping information. Go until we hit the end of the
	// file.
	//
	startVcn = 0;
	fileMappings = (PGET_RETRIEVAL_DESCRIPTOR) FileMap;
	while( !(status = NtFsControlFile( sourceFile, NULL, NULL, 0, &ioStatus,
						FSCTL_GET_RETRIEVAL_POINTERS,
						&startVcn, sizeof( startVcn ),
						fileMappings, FILEMAPSIZE * sizeof(LARGE_INTEGER) ) ) ||
			 status == STATUS_BUFFER_OVERFLOW ||
			 status == STATUS_PENDING ) 
	{
		// 
		// If the operation is pending, wait for it to finish
		//
		if( status == STATUS_PENDING ) {
			
			WaitForSingleObject( sourceFile, INFINITE ); 

			//
			// Get the status from the status block
			//
			if( ioStatus.Status != STATUS_SUCCESS && 
				ioStatus.Status != STATUS_BUFFER_OVERFLOW ) 
			{
				// Error out
				return StatusToError(ioStatus.Status);
			}
		}

		//
		// Loop through the buffer of number/cluster pairs, printing them
		// out.
		//
		startVcn = fileMappings->StartVcn;
		for( i = 0; i < (ULONGLONG) fileMappings->NumberOfPairs; i++ ) 
		{
			//
			// On NT 4.0, a compressed virtual run (0-filled) is 
			// identified with a cluster offset of -1
			//
			if( fileMappings->Pair[i].Lcn == LLINVALID ) 
			{
				// Ignore virtual runs since OS is supposed to fill slack
				// with zeros
			} 
			else 
			{
				// This gives us the last cluster Vcn (First+Length-1)
				sm->Vcn=startVcn+(fileMappings->Pair[i].Vcn - startVcn )-1;
		
				if(vi->dwFS==FS_NTFS)
				{
					// We need to 16 cluster align it if NTFS
					sm->Vcn=(sm->Vcn/16)*16;
				}

				// This gives us the last cluster Lcn (Start Location + offset)
				sm->Orig=fileMappings->Pair[i].Lcn+(sm->Vcn-startVcn);
			}
			startVcn = fileMappings->Pair[i].Vcn;
		}

		//
		// If the buffer wasn't overflowed, then we're done
		//
		if( !status ) break;
	}

	CloseHandle( sourceFile );

	return StatusToError(status);
}

DWORD GetOrigForFileList(VOLINFO *vi)
{
	DWORD count;
	FILELIST *index;
	DWORD RetVal;

	index=vi->FileList;
	count=0;
	RetVal=WFE_NOERROR;

	while(index!=NULL)
	{
		if(!index->IsDirectory)
		{
			RetVal=GetLastClusterOfFile(vi,
				index->name,
				&(vi->SlackMove[count]));

			// Probably somebody has a lock on the file. ignore
			//if(RetVal!=WFE_NOERROR)
			//	return RetVal;

			count++;
		}
		index=index->next;
	}

	if(((RetVal==WFE_ALREADYCOMMIT)||(RetVal==WFE_INVALPARAM))&&(vi->dwFS==FS_NTFS))
	{
		// Couldn't do the move because of MFT, or user is mucking
		// around.... just skip it and we'll get it on the next pass.
		// Only applies to NTFS
		RetVal=WFE_NOERROR;
	}

	return RetVal;
}

DWORD CountFileList(FILELIST *index)
{
	DWORD count;

	count=0;

	while(index!=NULL)
	{
		if(!index->IsDirectory)
			count++;
		index=index->next;
	}

	return count;
}

DWORD AbortSlackMovements(VOLINFO *vi,DWORD RetVal,BOOL bSafe)
{
	DWORD rst;

	if(RetVal==WFE_USERCANCEL)
	{
		// Turn off the cancel button cancel
		bGlobalCancel=FALSE;

		// We're just counting, not moving. Go ahead and abort.
		if(vi->bCountClusters)
			return WFE_USERCANCEL;

		if(!bSafe)
		{
			rst=PGPscMessageBox (vi->hwnd,IDS_PGPWARNING,IDS_ABORTWILLFRAG,
					MB_YESNO|MB_DEFBUTTON2|MB_ICONEXCLAMATION);	

			if(rst==IDNO)
				return WFE_NOERROR;
		}

		// It's safe to abort here.
		return WFE_USERCANCEL;
	}

	// Couldn't do the move because of MFT, or user is mucking
	// around.... just skip it and we'll get it on the next pass.
	// Only applies to NTFS
	if(((RetVal==WFE_ALREADYCOMMIT)||(RetVal==WFE_INVALPARAM))&&(vi->dwFS==FS_NTFS))
		return WFE_NOERROR;

	return RetVal;
}

DWORD DoSlackMovements(VOLINFO *vi)
{
	char fileName[MAX_PATH];
	DWORD i;
	DWORD RetVal;
	DWORD BufferLoop;
	DWORD Start,End,ModAmt;
	BOOL bAbortASAP;

	bAbortASAP=FALSE;

	RetVal=WFE_NOERROR;
	sprintf(fileName, "%C:\\%s", vi->vol0+'A', "PGPwipepattern.tmp" );

	ModAmt=SLACKBUFFER;

	if(vi->SlackBufferIndex<SLACKBUFFER)
		ModAmt=vi->SlackBufferIndex;

	if(ModAmt==0)
		return WFE_COULDNOTWRITE;

	// Move Pattern out of the way
	// in case its in the last temp
	RetVal=NTWipeCluster( vi, 
				fileName,
				0,
				vi->PatternBuffer);

	if(RetVal==WFE_USERCANCEL)
		bAbortASAP=TRUE;

	RetVal=AbortSlackMovements(vi,RetVal,TRUE);

	if(RetVal!=WFE_NOERROR)
		return RetVal;

	// Reuse our temp buffer in ModAmt increments
	for(BufferLoop=0;BufferLoop<(vi->SlackMoveCount+ModAmt-1)/ModAmt;BufferLoop++)
	{
		Start=ModAmt*BufferLoop;
		End=Start+ModAmt;

		if(vi->SlackMoveCount<End)
			End=vi->SlackMoveCount;

		// Move end of files to temp
		for(i=Start;i<End;i++)
		{
			// Don't do bogus files
			if(vi->SlackMove[i].Orig==0)
			{
			}
			else
			{
				RetVal=NTWipeCluster( vi, 
					vi->SlackMove[i].name,
					vi->SlackMove[i].Vcn,
					vi->SlackBuffer[i%ModAmt]);

				if(RetVal==WFE_USERCANCEL)
					bAbortASAP=TRUE;

				RetVal=AbortSlackMovements(vi,RetVal,FALSE);

				if(RetVal!=WFE_NOERROR)
					return RetVal;
			}
		}

		// Wipe former location
		for(i=Start;i<End;i++)
		{
			// Don't do bogus files
			if(vi->SlackMove[i].Orig==0)
			{
			}
			else
			{
				RetVal=NTWipeCluster( vi, 
					fileName,
					0,
					vi->SlackMove[i].Orig);

					if(RetVal==WFE_USERCANCEL)
						bAbortASAP=TRUE;

					RetVal=AbortSlackMovements(vi,RetVal,FALSE);

					if(RetVal!=WFE_NOERROR)
						return RetVal;
			}
		}

		// Move Pattern out of the way
		// since its in the last orig
		RetVal=NTWipeCluster( vi, 
					fileName,
					0,
					vi->PatternBuffer);

		if(RetVal==WFE_USERCANCEL)
			bAbortASAP=TRUE;

		RetVal=AbortSlackMovements(vi,RetVal,FALSE);

		if(RetVal!=WFE_NOERROR)
			return RetVal;

		// Move back. Not necessary, but prevents fragmentation
		for(i=Start;i<End;i++)
		{
			// Don't do bogus files
			if(vi->SlackMove[i].Orig==0)
			{
			}
			else
			{
				RetVal=NTWipeCluster( vi, 
					vi->SlackMove[i].name,
					vi->SlackMove[i].Vcn,
					vi->SlackMove[i].Orig);

				if(RetVal==WFE_USERCANCEL)
					bAbortASAP=TRUE;

				RetVal=AbortSlackMovements(vi,RetVal,FALSE);

				if(RetVal!=WFE_NOERROR)
					return RetVal;
			}
		}

		// We've completed a pass with fragmentation issues figured
		// out. Its safe to abort.
		if(bAbortASAP)
			return WFE_USERCANCEL;
	}

	// Not necessary, but wipe temp spots too
	for(i=0;i<ModAmt;i++)
	{
		RetVal=NTWipeCluster( vi, 
			fileName,
			0,
			vi->SlackBuffer[i]);

		if(RetVal==WFE_USERCANCEL)
				bAbortASAP=TRUE;

		RetVal=AbortSlackMovements(vi,RetVal,TRUE); // safe to get out here

		if(RetVal!=WFE_NOERROR)
			return RetVal;
	}

	return WFE_NOERROR;
}

DWORD NTDoWipe(VOLINFO *vi)
{
	DWORD status,RetVal;
	char drivestr[50];
	DWORD memamt;
	char StrRes[500];

	RetVal=WFE_NOERROR;

	LoadString (g_hinst, IDS_SCANNINGDISK, StrRes, sizeof(StrRes));
	StatusMessage(StrRes, FALSE);

	sprintf(drivestr,"%c:\\",vi->vol0+'A');

	// Get the NtFsControlFile entry point since undocumented
	if( !(NtFsControlFile = (void *) GetProcAddress( GetModuleHandle("ntdll.dll"),
			"NtFsControlFile" )) )
	{
		return WFE_FSNOTSUPPORTED;
	}

	vi->FileList=NULL;

	// Enumerate the files on the drive
	RetVal=AddToFileList(&(vi->FileList),drivestr,&bGlobalCancel); 

	if(!RetVal)
	{
		FreeFileList(vi->FileList);
		return WFE_USERCANCEL;
	}
	
	// Find the number of files
	vi->SlackMoveCount=CountFileList(vi->FileList); 

	// Allocate a slack movement array
	memamt=vi->SlackMoveCount*sizeof(SLACKMOVE); 
	vi->SlackMove=(SLACKMOVE *)malloc(memamt);
	memset(vi->SlackMove,0x00,memamt);

	// Open the volume
	status = OpenVolume(vi);
	
	if( status != ERROR_SUCCESS ) 
	{
		return StatusToError(status);
	}

	// Get the last cluster for each file (Orig)
	RetVal=GetOrigForFileList(vi);

	if(RetVal==WFE_NOERROR)
	{
		// Now, wipe the free clusters, grabbing Temp along the way
		RetVal=NTWipeFreeClusters(vi);

		if(RetVal==WFE_NOERROR)
		{
			// With this information, wipe the slack space
			RetVal=DoSlackMovements(vi);
		}

	}

	// Done with File List... Free it
	FreeFileList(vi->FileList);

	// Done with slack space info.. Free it
	if(vi->SlackMove)
		free(vi->SlackMove);

	CloseHandle(vi->VolHandle);

	return RetVal;
}

void NTGetDriveInfo(VOLINFO *vi)
{
	int rst;
	DWORD NumFreeClusters;
	DWORD SectorsPerCluster;
	DWORD BytesPerSector;
	DWORD NumClusters;
	char drivestr[50];

	sprintf(drivestr,"%c:\\",vi->vol0+'A');

	rst=GetDiskFreeSpace(drivestr,
		&SectorsPerCluster,
		&BytesPerSector,
		&NumFreeClusters,
		&NumClusters);

	vi->no_of_clusters=NumClusters;
	vi->sectors_per_clus=(USHORT)SectorsPerCluster;
	vi->sector_size=(USHORT)BytesPerSector;

	vi->fTotalCapacity=(FLOAT)
		vi->no_of_clusters*vi->sectors_per_clus*vi->sector_size;

	if(vi->dwFS==FS_FAT)
	{
		// Microsoft says 4086 here
		// FIPs says 4079. 4079 makes more sense,
		// but probably won't matter either way
		if(vi->no_of_clusters <= 4079)
			vi->dwFS=FS_FAT12;
		else
			vi->dwFS=FS_FAT16;
	}
}

DWORD NTWipeFree(HWND hwnd,VOLINFO *vi,char *szRoot,
			   DWORD* pattern, BOOL StartWipe)
{
	DWORD RetVal;
	char filename[MAX_PATH];
	char *PatternBuf;
	int memamt;
	FILE *fin;

	// For clusters greater than 4K, defrag API is not supported
	// because of msoft compression goof
	if((vi->WinNT)&&(vi->dwFS==FS_NTFS)&&(vi->sectors_per_clus*vi->sector_size>4096))
		return WFE_NTFSGREATER4K;

	// Zero-based volume, 'A'=0, 'B'=1, 'C'=2, etc.
	vi->vol0=toupper(szRoot[0])-'A';

	NTGetDriveInfo(vi);

	DisplayDiskStats(vi);

	// Do wipe if so. Otherwise report stats.
	if(StartWipe)
	{
		memamt=vi->sectors_per_clus*vi->sector_size;
		if(vi->dwFS==FS_NTFS)
			memamt=memamt*16; // NTFS works with 16 cluster segments

		PatternBuf=(char *)malloc(memamt);

		if(PatternBuf==0)
		{
			RetVal=WFE_OUTOFMEMORY;
		}
		else
		{
		//	memset(PatternBuf,'o',memamt); // For testing

			WritePattern(PatternBuf,vi->pattern_buffer,memamt);

			// Create our pattern file
			sprintf(filename,"%c:\\",vi->vol0+'A');
			strcat(filename,"PGPwipepattern.tmp");

			fin=fopen(filename,"wb");

			if(fin==0)
			{
				RetVal=WFE_COULDNOTWRITE;
			}
			else
			{
				fwrite(PatternBuf,1,memamt,fin);
				fclose(fin);

				free(PatternBuf);

				vi->bCountClusters=TRUE;

				RetVal=NTDoWipe(vi);

				if(RetVal==WFE_NOERROR)
				{
					vi->bCountClusters=FALSE;
					RetVal=NTDoWipe(vi);
				}

				_unlink (filename);
			}
		}
	}
	return RetVal;
}

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
