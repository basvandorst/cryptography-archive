#include "precomp.h"

void WritePattern(void* buf, DWORD* pattern, long size)
{
	char* buffer = (char*)buf;
	int patternSize = sizeof(DWORD) * 256;

	if(size <= patternSize)
	{
		memcpy(buffer, pattern, size);
	}
	else
	{
		int passes = size / patternSize;
		int remainder = size % patternSize;
		int i;

		for(i = 0; i < passes; i++)
		{
			memcpy(buffer + (i * patternSize), pattern, patternSize);
		}

		memcpy(buffer + (i * patternSize), pattern, remainder);
	}
}

DWORD ClusterToSector(VOLINFO *vi,DWORD Cluster)
{
	return ((Cluster-0x2)*vi->sectors_per_clus)+vi->dwStartSector;
}

int ReadAbsoluteSectors( HANDLE hDev,int volume, DWORD start_sector, 
                         WORD num_sectors, LPBYTE pBuf ) 
{
	DIOC_REGISTERS reg;
	DISKIO di;
	BOOL bResult;
	DWORD cb;

	reg.reg_EAX      = volume-1;  // zero-based volume number
	reg.reg_EBX      = (DWORD)&di;
	reg.reg_ECX      = 0xffff;  // use DISKIO structure
	reg.reg_Flags    = 1;       // preset the carry flag
	di.diStartSector = start_sector;
	di.diSectors     = num_sectors;
	di.diBuffer      = (DWORD)pBuf;
	bResult = DeviceIoControl( hDev, VWIN32_DIOC_DOS_INT25,
              &reg, sizeof( reg ), &reg, sizeof( reg ), &cb, 0 ); 

	if ( !bResult || (reg.reg_Flags & 1) ) return -1;
	return 0;
}

BOOL WriteAbsoluteSectors(HANDLE hDev,
                          int    bDrive,
                          DWORD  dwStartSector,
                          WORD   wSectors,
                          LPBYTE lpSectBuff)
{
	BOOL           fResult;
	DWORD          cb;
	DIOC_REGISTERS reg = {0};
	DISKIO         di = {0};

//	if(bDrive!=1)
//		return FALSE;

	di.diStartSector = dwStartSector;
	di.diSectors      = wSectors;
	di.diBuffer      = (DWORD)lpSectBuff;

	reg.reg_EAX = bDrive - 1;    // Int 26h drive numbers are 0-based.
	reg.reg_EBX = (DWORD)&di;
	reg.reg_ECX = 0xFFFF;        // use DISKIO struct

	fResult = DeviceIoControl(hDev, VWIN32_DIOC_DOS_INT26,
                                &reg, sizeof(reg),
                                &reg, sizeof(reg), &cb, 0);

      // Determine if the DeviceIoControl call and the write succeeded.
	fResult = fResult && !(reg.reg_Flags & CARRY_FLAG);

	return fResult;
}

int ReadAbsoluteSectors32( HANDLE hDev,int volume, DWORD start_sector, 
                           WORD num_sectors, LPBYTE pBuf ) 
{
	DIOC_REGISTERS reg;
	DISKIO di;
	BOOL bResult;
	DWORD cb;

	reg.reg_EAX      = 0x7305;  // Ext_ABSDiskReadWrite
	reg.reg_EBX      = (DWORD)&di;
	reg.reg_ECX      = 0xffff;  // use DISKIO structure
	reg.reg_EDX      = volume;  // one-based volume number
	reg.reg_ESI      = 0;       // READ operation
	reg.reg_Flags    = 0;       // preset the carry flag
	di.diStartSector = start_sector;
	di.diSectors     = num_sectors;
	di.diBuffer      = (DWORD)pBuf;

	bResult = DeviceIoControl( hDev, VWIN32_DIOC_DOS_DRIVEINFO,
              &reg, sizeof( reg ), &reg, sizeof( reg ), &cb, 0 ); 

	if ( /*!bResult ||*/ (reg.reg_Flags & 1) ) return -1;
	return 0;
}

BOOL WriteAbsoluteSectors32(HANDLE hDev,
                            int    bDrive,
                            DWORD  dwStartSector,
                            WORD   wSectors,
                            LPBYTE lpSectBuff)
{
	BOOL           fResult;
	DWORD          cb;
	DIOC_REGISTERS reg = {0};
	DISKIO         di;

//	if(bDrive!=1)
//		return FALSE;

	di.diStartSector = dwStartSector;
	di.diSectors      = wSectors;
	di.diBuffer       = (DWORD)lpSectBuff;

	reg.reg_EAX = 0x7305;   // Ext_ABSDiskReadWrite
	reg.reg_EBX = (DWORD)&di;
	reg.reg_ECX = -1;
	reg.reg_EDX = bDrive;   // Int 21h, fn 7305h drive numbers are 1-based

	reg.reg_ESI = 0x6001;   // Normal file data (See function
                             // documentation for other values)


	fResult = DeviceIoControl(hDev, VWIN32_DIOC_DOS_DRIVEINFO,
                               &reg, sizeof(reg),
                               &reg, sizeof(reg), &cb, 0);

	// Determine if the DeviceIoControl call and the write succeeded.
	fResult = fResult && !(reg.reg_Flags & CARRY_FLAG);

	return fResult;
}

BOOL UserPressedCancel(VOLINFO *vi)
{
	MSG msg;

	while(PeekMessage(&msg,vi->hwnd,0,0,PM_REMOVE))
	{
		if(!IsDialogMessage(vi->hwnd,&msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	if(bGlobalCancel)
		return TRUE;

	return FALSE;
}

DWORD ReadSectors(VOLINFO *vi,DWORD start_sector, 
                 WORD num_sectors,LPBYTE pBuf) 
{
	DWORD RetVal;

	RetVal=WFE_NOERROR;

	if(vi->dwFS==FS_FAT32) 
	{
		if(ReadAbsoluteSectors32(vi->hVWin32,vi->vol1,
				start_sector,num_sectors,pBuf)!=0)
			RetVal=WFE_COULDNOTREAD;
	}
	else
	{
		if(ReadAbsoluteSectors(vi->hVWin32,vi->vol1,
				start_sector,num_sectors,pBuf)<0)
			RetVal=WFE_COULDNOTREAD;
	}

	if(UserPressedCancel(vi))
		RetVal=WFE_USERCANCEL;

	return RetVal;
}

int GetLockFlagState( HANDLE hDev,int volume,BOOL bFlatFAT ) 
{
	DIOC_REGISTERS reg;
	BOOL bResult,fResult;
	DWORD cb;

	reg.reg_EAX = 0x440d; // generic IOCTL
	reg.reg_ECX = bFlatFAT ? 0x486c : 0x086c; // lock logical volume 
	reg.reg_EBX = volume; // 1 based
	reg.reg_Flags = 1; // preset the carry flag

	bResult = DeviceIoControl( hDev, VWIN32_DIOC_DOS_IOCTL,
		&reg, sizeof( reg ), &reg, sizeof( reg ), &cb, 0 ); 

	fResult = bResult && !(reg.reg_Flags & CARRY_FLAG);

	if(!fResult)
		return -1;

	return (reg.reg_EAX & 0xffff);
}

int LockLogicalVolume( HANDLE hDev,int volume, int lock_level, 
                       int permissions, BOOL bFlatFAT ) 
{
	DIOC_REGISTERS reg;
	BOOL bResult;
	DWORD cb;

	reg.reg_EAX = 0x440d; // generic IOCTL
	reg.reg_ECX = bFlatFAT ? 0x484a : 0x084a; // lock logical volume 
	reg.reg_EBX = volume | (lock_level << 8);
	reg.reg_EDX = permissions;
	reg.reg_Flags = 1; // preset the carry flag

	bResult = DeviceIoControl( hDev, VWIN32_DIOC_DOS_IOCTL,
              &reg, sizeof( reg ), &reg, sizeof( reg ), &cb, 0 ); 

	if(bResult)
		return 0;

	return (reg.reg_EAX & 0xffff);

//	if ( !bResult || (reg.reg_Flags & 1) ) 
//		return (reg.reg_EAX & 0xffff);

	return 0;
}

int UnlockLogicalVolume( HANDLE hDev,int volume, BOOL bFlatFAT ) 
{
	DIOC_REGISTERS reg;
	BOOL bResult;
	DWORD cb;

	reg.reg_EAX = 0x440d;
	reg.reg_ECX = bFlatFAT ? 0x486a : 0x086a; // lock logical volume 
	reg.reg_EBX = volume;
	reg.reg_Flags = 1; // preset the carry flag

	bResult = DeviceIoControl( hDev, VWIN32_DIOC_DOS_IOCTL,
              &reg, sizeof( reg ), &reg, sizeof( reg ), &cb, 0 ); 

	if ( !bResult || (reg.reg_Flags & 1) ) return -1;
	return 0;
}

DWORD WriteSectors(VOLINFO *vi,DWORD start_sector, 
                 WORD num_sectors,LPBYTE pBuf) 
{
	DWORD RetVal;
	BOOL bFat32;
    
	RetVal=WFE_NOERROR;
	bFat32=(vi->dwFS==FS_FAT32);

	if(vi->bCountClusters)
	{
		char StrRes[500];

		LoadString (g_hinst, IDS_SCANNINGDISK, StrRes, sizeof(StrRes));

		if(vi->dwClusterCount==0)
			StatusMessage(StrRes, FALSE);

		vi->dwClusterCount++;
		if(UserPressedCancel(vi))
			RetVal=WFE_USERCANCEL;
		return RetVal;
	}

	// Get Level2 Exclusive Volume Lock
	if(LockLogicalVolume(vi->hVWin32,vi->vol1, 
		LEVEL2_LOCK, 
		0,
		bFat32)!=0)
	{
		RetVal=WFE_COULDNOTLOCK;
	}
	else
	{
		// Get Level3 Exclusive Volume Lock
		if(LockLogicalVolume(vi->hVWin32,vi->vol1, 
			LEVEL3_LOCK, 
			0,
			bFat32)!=0)
		{
			RetVal=WFE_COULDNOTLOCK;
		}
		else
		{
			// Make sure nobody has been writing to the disk.
			if(GetLockFlagState(vi->hVWin32,vi->vol1,bFat32)!=0)
			{
				RetVal=WFE_DISKMODIFIED;
			}
			else
			{
				if(bFat32)
				{
					if(!WriteAbsoluteSectors32(vi->hVWin32,
						vi->vol1,start_sector,num_sectors,pBuf))
					{
						RetVal=WFE_COULDNOTWRITE;
					}
				}
				else
				{
					if(!WriteAbsoluteSectors(vi->hVWin32,
						vi->vol1,start_sector,num_sectors,pBuf))
					{
						RetVal=WFE_COULDNOTWRITE;
					}
				}
			}
			// Free Level 3 lock
			UnlockLogicalVolume(
				vi->hVWin32, 
				vi->vol1, 
				bFat32);
		}
		// Free Level 2 lock
		UnlockLogicalVolume(
			vi->hVWin32, 
			vi->vol1, 
			bFat32);
	}

	if(RetVal==WFE_NOERROR)
	{
		char StrRes[500];
		char statusmsg[500];

		vi->dwClustersWritten=vi->dwClustersWritten+1;
		
		if(vi->dwClustersWritten%10==0)
		{
			float pos;

			LoadString (g_hinst, IDS_PERCENTWRITTEN, StrRes, sizeof(StrRes));

			sprintf(statusmsg,
				StrRes,
				vi->dwClustersWritten,vi->dwClusterCount);

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

DWORD Fat32Item(unsigned char *FATbuffer,DWORD FlatIndex)
{
	DWORD dwFat32index;
	DWORD dwFat32value;
	DWORD *pFat;

	dwFat32index=FlatIndex*32/8;
	pFat = (DWORD *)(&(FATbuffer[dwFat32index]));

	// upper 4 bits are masked off
	dwFat32value=0x0fffffff & (*pFat);

	return dwFat32value;
}

DWORD Fat16Item(unsigned char *FATbuffer,DWORD FlatIndex)
{
	DWORD dwFat16index;
	DWORD dwFat16value;
	WORD *pFat;

	dwFat16index=FlatIndex*16/8;
	pFat = (WORD *)(&(FATbuffer[dwFat16index]));

	dwFat16value=0xffff & (DWORD)(*pFat);

	return dwFat16value;
}

DWORD Fat12Item(unsigned char *FATbuffer,DWORD FlatIndex)
{
	DWORD dwFat12index;
	DWORD dwFat12value;
	DWORD nib;
	unsigned char* pFat;

	// Depending if we're odd or even, we may
	// get need 8 bits then 4 bits, or 4 bits
	// then 8 bits.
	dwFat12index=FlatIndex*12/8;
	pFat = &(FATbuffer[dwFat12index]);

	if(FlatIndex%2==0) // Even
	{   // Get the 8 bits
		dwFat12value = 0xff & (DWORD)(*pFat);
		pFat++;
		// Get the 4 bits
		nib = 0x0f & (DWORD)(*pFat);
		// Shift the 4 bit portion as MSB.
		nib <<= 8;
		dwFat12value |= nib;
	}
	else // Odd
	{   // Get the 4 bits.
		nib = 0xf0 & (DWORD)(*pFat);
		pFat++;
		// Get the 8 bits
		dwFat12value = 0xff & (DWORD)(*pFat);
		// Shift the 8 bit portion MSB.
		dwFat12value <<= 4;
		// Shift the 4 bit portion LSB.
		nib >>= 4;
		dwFat12value |= nib;
	}

	return dwFat12value;
}

DWORD FatItem(VOLINFO *vi,DWORD index)
{
	switch(vi->dwFS)
	{
		case FS_FAT12:
			return(Fat12Item(vi->pFatBuf,index));

		case FS_FAT16:
			return(Fat16Item(vi->pFatBuf,index));

		case FS_FAT32:
			return(Fat32Item(vi->pFatBuf,index));
	}

	return 0;
}

BOOL IsntEnd(VOLINFO *vi,DWORD dwIndexCluster)
{
	switch(vi->dwFS)
	{
		case FS_FAT12:
			if(dwIndexCluster<0xff0)
				return TRUE;
			break;

		case FS_FAT16:
			if(dwIndexCluster<0xfff0)
				return TRUE;
			break;

		case FS_FAT32:
			if(dwIndexCluster<0x0ffffff0)
				return TRUE;
			break;
	}
	return FALSE;
}

BOOL IsClusterError(VOLINFO *vi,DWORD dwIndexCluster)
{
	switch(vi->dwFS)
	{
		case FS_FAT12:
			if(dwIndexCluster==0xff7)
				return TRUE;
			break;

		case FS_FAT16:
			if(dwIndexCluster==0xfff7)
				return TRUE;
			break;

		case FS_FAT32:
			if(dwIndexCluster==0x0ffffff7)
				return TRUE;
			break;
	}
	return FALSE;
}

BOOL PushDirStack(DIRSTACK **ds,
				  DWORD dwStartCluster)
{
	DIRSTACK *newds;

	newds=(DIRSTACK *)malloc(sizeof(DIRSTACK));
	if(newds)
	{
		memset(newds,0x00,sizeof(DIRSTACK));
		newds->dwStartCluster=dwStartCluster;
		newds->next=*ds;
		*ds=newds;
		return TRUE;
	}
	return FALSE;
}

BOOL PopDirStack(DIRSTACK **ds,
				 DWORD *dwStartCluster)
{
	DIRSTACK *FreeAtLast;

	if(*ds==0)
		return FALSE;

	FreeAtLast=*ds;

	*dwStartCluster=(*ds)->dwStartCluster;
	*ds=(*ds)->next;

	free(FreeAtLast);

	return TRUE;
}

DWORD DoFileSlack(VOLINFO *vi,
				 DWORD dwCluster,
				 DWORD dwLength)
{
	DWORD dwLastCluster;
	DWORD RetVal,err;

	RetVal=WFE_NOERROR;

	// Starting cluster is invalid? Something wrong
	if(!IsntEnd(vi,dwCluster))
		return WFE_CLUSTERERROR;

	dwLastCluster=dwCluster;
	dwCluster=FatItem(vi,dwCluster);

	// Find the last cluster and the remainder length
	while(IsntEnd(vi,dwCluster))
	{
		dwLastCluster=dwCluster;
		dwLength-=vi->dwClusterSize;
		dwCluster=FatItem(vi,dwCluster);
	}

	// Cluster with an error is in the chain
	if(IsClusterError(vi,dwCluster))
		return WFE_CLUSTERERROR;

	// File length and cluster chain don't agree
	// Since we're unsigned, less than zero is accounted for
	if(dwLength>vi->dwClusterSize)
		return WFE_CLUSTERERROR;

	memset(vi->pClusterBuf,0x00,vi->dwClusterSize);

	if(vi->bCountClusters)
	{
		// Since we're just counting, no need to read in
		// the last sector.... it won't be written anyway.
		// Speeds things up greatly.
		err=WFE_NOERROR;
	}
	else
	{
		// Read in last cluster of file
		err=ReadSectors(vi,
			ClusterToSector(vi,dwLastCluster), 
			vi->sectors_per_clus,
			vi->pClusterBuf);
	}

	if(err!=WFE_NOERROR)
	{
		RetVal=err;
	}
	else
	{
		// wipe off the slack
		WritePattern(	&((vi->pClusterBuf)[dwLength]),
						vi->pattern_buffer,
						vi->dwClusterSize-dwLength);

		//memset(&((vi->pClusterBuf)[dwLength]),0x00,
		//	vi->dwClusterSize-dwLength);

		// Write out last cluster of file
		err=WriteSectors(vi,
			ClusterToSector(vi,dwLastCluster), 
			vi->sectors_per_clus,
			vi->pClusterBuf);

		if(err!=WFE_NOERROR)
			RetVal=err;
	}

	return RetVal;
}

DWORD DoDirAndSlack(VOLINFO *vi,
				   char *pBuf,int bufsize) 
{
	unsigned char *pDir = pBuf;
	unsigned char *pEnd = pDir+bufsize;
	DWORD dwStartCluster;
	DWORD dwTotalLength,err,RetVal;
	PDIRENTRY pd;

	RetVal=WFE_NOERROR;

	while((*pDir)&&(RetVal==WFE_NOERROR)) 
	{
		// Skip over and erase deleted directory entries
		if (*pDir == 0xe5)
		{
//			WritePattern(&(pDir[1]),vi->pattern_buffer,sizeof(DIRENTRY)-1);
//			Maybe this is a problem... yah.. long filenames are mislinked.				
			memset(&(pDir[1]),0x00,sizeof(DIRENTRY)-1);
			pDir += sizeof( DIRENTRY );
		}
		else if (*(pDir+0x0b) == 0xf && *(pDir+0x0c) == 0 ) 
		{
			// Type 0 "long" directory entry
			// We don't really care about these. leave em be.
			pDir += sizeof( DIRENTRY );
		}
		// Other Type "long" directory entry
	    else if (*(pDir+0x0b) == 0xf && *(pDir+0x0c) != 0 ) 
		{
			// Other Type "long" directory entry
			// We don't care about these either.
			pDir += sizeof( DIRENTRY );
		}
		// "Short" directory entry
		else 
		{
			// "Short" directory entry
			// Every file has one of these. It has our info.
			pd = (PDIRENTRY)pDir;

			dwTotalLength=pd->deFileSize;

    		if (vi->dwFS!=FS_FAT32)
			{
				dwStartCluster=((DWORD)(pd->deStartCluster))&0xfffff;
			}
			else
			{
				dwStartCluster=((DWORD)(pd->deEAhandle))&0xffff;
				dwStartCluster<<=16;
				dwStartCluster|=((DWORD)(pd->deStartCluster))&0xffff;
			}

			if((!strncmp(".       ",pd->deName,8))||
			   (!strncmp("..      ",pd->deName,8)))
			{
				// same and parent directory
			}
			// Check d attrib to see if directory
			else if(pd->deAttributes & 0x10)
			{
				// Store dir on stack for later processing...
				PushDirStack(&(vi->ds),dwStartCluster);
			}
			else if(dwStartCluster>=2) // 0 and 1 are weird stuff
			{
				char eightthree[12];

				memset(eightthree,0x00,12);
				memcpy(eightthree,pd->deName,11);

				// Wipe slack space for this file...
//				vErrorOut(fg_yellow,"%s %8x %d\n",
//					eightthree,dwStartCluster,dwTotalLength);
				err=DoFileSlack(vi,dwStartCluster,dwTotalLength);

				if(err!=WFE_NOERROR)
					RetVal=err;
			}

			pDir += sizeof( DIRENTRY );
		}

		if(pDir>=pEnd)   // Just in case it's completely full
			break;       // and there is no terminating zero
	}

	// We must keep trailing zero to show end of directory,
	// unless of course cluster is completely full
	pDir++;

	// Clear off slack at end of directory chain unless full
	if(pDir<pEnd)
	{
//		Patterns seem to be messing up directory structure
//		WritePattern(pDir,vi->pattern_buffer,pEnd-pDir);
		memset(pDir,0x00,pEnd-pDir);
	}

	return RetVal;
}

DWORD DoOldFatRootDirectory(VOLINFO *vi)
{
	DWORD bufsize,err,RetVal;
	unsigned char *pReadBuf;

	RetVal=WFE_NOERROR;

	bufsize = vi->FAT_DirSectorCount * vi->sector_size;
	pReadBuf = (BYTE *)malloc( bufsize );
	if(!pReadBuf)
	{
		RetVal=WFE_OUTOFMEMORY;
	}
	else
	{
//		This is just clearing the buffer
//		WritePattern(pReadBuf,vi->pattern_buffer, bufsize);
		memset( pReadBuf, 0x00, bufsize );

		err=ReadSectors(vi,vi->FAT_DirSector, 
			vi->FAT_DirSectorCount,pReadBuf);
		if(err!=WFE_NOERROR)
		{
			RetVal=err;
		}
		else
		{
			err=DoDirAndSlack(vi,pReadBuf,bufsize);
			if(err!=WFE_NOERROR)
			{
				RetVal=err;
			}
			else
			{
				err=WriteSectors(vi,vi->FAT_DirSector, 
					vi->FAT_DirSectorCount,pReadBuf);
				if(err!=WFE_NOERROR)
				{
					RetVal=err;
				}
			}
		}
		free(pReadBuf);
	}

	return RetVal;
}

DWORD GetBufferSize(VOLINFO *vi,
					DWORD dwIndexCluster,
					DWORD *bufsize)
{
	*bufsize=0;
	
	while(IsntEnd(vi,dwIndexCluster))
	{
		*bufsize+=vi->dwClusterSize;
		dwIndexCluster=FatItem(vi,dwIndexCluster);
	}

	if(IsClusterError(vi,dwIndexCluster))
		return WFE_CLUSTERERROR;

	return WFE_NOERROR;
}

DWORD ReadChainIntoBuffer(VOLINFO *vi,DWORD dwIndexCluster,
						  unsigned char *pReadBuf)
{
	DWORD dwIndexBuffer,RetVal,err;

	RetVal=WFE_NOERROR;
	dwIndexBuffer=0;

	while((IsntEnd(vi,dwIndexCluster))&&(RetVal==WFE_NOERROR))
	{
		err=ReadSectors(vi,
			ClusterToSector(vi,dwIndexCluster), 
			vi->sectors_per_clus,
			&(pReadBuf[dwIndexBuffer]));

		if(err!=WFE_NOERROR)
		{
			RetVal=err;
		}
		else
		{
			dwIndexBuffer+=vi->dwClusterSize;
			dwIndexCluster=FatItem(vi,dwIndexCluster);
		}
	}

	return RetVal;
}

DWORD WriteChainFromBuffer(VOLINFO *vi,DWORD dwIndexCluster,
						  unsigned char *pReadBuf)
{
	DWORD dwIndexBuffer,RetVal,err;

	RetVal=WFE_NOERROR;
	dwIndexBuffer=0;

	while((IsntEnd(vi,dwIndexCluster))&&(RetVal==WFE_NOERROR))
	{
		err=WriteSectors(vi,
			ClusterToSector(vi,dwIndexCluster), 
			vi->sectors_per_clus,
			&(pReadBuf[dwIndexBuffer]));

		if(err!=WFE_NOERROR)
		{
			RetVal=err;
		}
		else
		{
			dwIndexBuffer+=vi->dwClusterSize;
			dwIndexCluster=FatItem(vi,dwIndexCluster);
		}
	}

	return RetVal;
}

DWORD DoDirectories(VOLINFO *vi)
{
	DWORD bufsize,err,RetVal;
	unsigned char *pReadBuf;
	DWORD dwStartCluster;

	RetVal=WFE_NOERROR;
	vi->ds=0;

	if(vi->dwFS!=FS_FAT32)
	{
	// Do the nonstandard FAT16/FAT12 root directory
		err=DoOldFatRootDirectory(vi);
		if(err!=WFE_NOERROR)
			RetVal=err;
	}
	else
	{
	// FAT32 root is normal.. just push on stack.
		PushDirStack(&(vi->ds),vi->FAT32_DirCluster);
	}

	// If no errors so far, do operations on the stack
	while((PopDirStack(&(vi->ds),&dwStartCluster))&&
		  (RetVal==WFE_NOERROR))
	{
		// Starting cluster is invalid? Something wrong
		if(!IsntEnd(vi,dwStartCluster))
			return WFE_CLUSTERERROR;

		// Get the size of the buffer we'll need
		err=GetBufferSize(vi,dwStartCluster,&bufsize);

		if(err!=WFE_NOERROR)
		{
			RetVal=err;
		}
		else
		{
			pReadBuf = (BYTE*)malloc( bufsize );
			if(!pReadBuf)
			{
				RetVal=WFE_OUTOFMEMORY;
			}
			else
			{
//				This is just clearing the buffer
//				WritePattern(pReadBuf,vi->pattern_buffer, bufsize);
				memset( pReadBuf, 0x00, bufsize );

				// Read it in
				err=ReadChainIntoBuffer(vi,
					dwStartCluster,pReadBuf);
	
				if(err!=WFE_NOERROR)
				{
					RetVal=err;
				}
				else
				{
					// Clear entries and wipe slack
					err=DoDirAndSlack(vi,pReadBuf,
						bufsize); 
					if(err!=WFE_NOERROR)
					{
						RetVal=err;
					}
					else
					{
						// Write it out again.
						err=WriteChainFromBuffer(vi,
							dwStartCluster,pReadBuf);
						if(err!=WFE_NOERROR)
							RetVal=err;
					}
				}
			}
			free( pReadBuf );
		}
	}

	return RetVal;
}

BOOL IsFATOK(VOLINFO *vi)
{
	DWORD dwClusterValue,dwIndex;

	// Two is the first valid cluster
	for(dwIndex=2;dwIndex<vi->no_of_clusters;dwIndex++)
	{
		dwClusterValue=FatItem(vi,dwIndex);

		switch(vi->dwFS)
		{
			case FS_FAT12:
				if(!(((dwClusterValue>=0xff0)&&
				      (dwClusterValue<=0xfff))||
				      (dwClusterValue<=vi->no_of_clusters)))
					return FALSE;
				break;

			case FS_FAT16:
				if(!(((dwClusterValue>=0xfff0)&&
			 	      (dwClusterValue<=0xffff))||
				      (dwClusterValue<=vi->no_of_clusters)))
					return FALSE;
				break;

			case FS_FAT32:
				if(!(((dwClusterValue>=0x0ffffff0)&&
				      (dwClusterValue<=0x0fffffff))||
				      (dwClusterValue<=vi->no_of_clusters)))
					return FALSE;
				break;
		}
	}
	return TRUE;
}

DWORD ReadFAT(VOLINFO *vi,DWORD start_sector)
{
	DWORD bufsize,RetVal,err;

	RetVal=WFE_NOERROR;

	bufsize=vi->sector_size*vi->sectors_per_fat;
	vi->pFatBuf = (BYTE*)malloc(bufsize);

	if(!vi->pFatBuf)
	{
		RetVal=WFE_OUTOFMEMORY;
	}
	else
	{
//		This is just clearing the buffer
//		WritePattern(vi->pFatBuf,vi->pattern_buffer, bufsize);
		memset(vi->pFatBuf,0x00,bufsize);

		// Read in raw data
		err=ReadSectors(vi,start_sector, 
			vi->sectors_per_fat,vi->pFatBuf);

		if(err!=WFE_NOERROR)
		{
			free(vi->pFatBuf);
			RetVal=err;
		}
		else
		{
			if(!IsFATOK(vi))
			{
				RetVal=WFE_FATFAILED;
			}
		}
	}
	return RetVal;
}

DWORD WipeFreeClusters(VOLINFO *vi)
{
	DWORD i,err;
	
	WritePattern(vi->pClusterBuf,vi->pattern_buffer, vi->dwClusterSize);
	//memset(vi->pClusterBuf,0x00,vi->dwClusterSize);

	// Two is the first valid cluster
	for(i=2;i<vi->no_of_clusters;i++)
	{
		if(FatItem(vi,i)==0)
		{
			err=WriteSectors(vi,
				ClusterToSector(vi,i),
				vi->sectors_per_clus,
				vi->pClusterBuf);
			if(err!=WFE_NOERROR)
				return err;
		}
	}

	return WFE_NOERROR;
}

BOOL GetFileSystem(VOLINFO *vi,char *szRoot)
{
	char szFSName[256],szVolName[256];
	DWORD dwVolSerial,dwFileLen,dwFSFlags;

	if(!GetVolumeInformation(szRoot,
		szVolName,256,
		&dwVolSerial,
		&dwFileLen,
		&dwFSFlags,
		szFSName,256))
		return FALSE;

	vi->dwFS=FS_NOFS;

	// NTFS is also supported under WinNT
	// HPFS went away with 3.51, so we'll
	// forget about it
	if(vi->WinNT)
	{
		if(!strcmp(szFSName,"NTFS"))
			vi->dwFS=FS_NTFS;
	}

	if(!strcmp(szFSName,"FAT32"))
		vi->dwFS=FS_FAT32;
	else if(!strcmp(szFSName,"FAT"))
		vi->dwFS=FS_FAT;
	
	if(vi->dwFS==FS_NOFS)
		return FALSE;

	return TRUE;
}

void GetFatInfo(VOLINFO *vi,BOOTSECTOR *bs)
{
	vi->dwReservedSectors = bs->bsResSectors; 
	vi->sector_size = bs->bsBytesPerSec;
	vi->sectors_per_fat = bs->bsFATsecs;
	vi->fat_count=bs->bsFATs;
	vi->sectors_per_clus = bs->bsSecPerClust;
	 
	vi->sectors_total= bs->bsSectors ;
	if (vi->sectors_total==0)
		vi->sectors_total = bs->bsHugeSectors ;

	// Root direcory info
	vi->FAT_DirSectorCount=
		((32 * bs->bsRootDirEnts)+vi->sector_size - 1)/
		vi->sector_size;
	vi->FAT_DirSector=vi->dwReservedSectors+ 
		vi->sectors_per_fat*vi->fat_count;
	vi->dwStartSector=
		vi->FAT_DirSector+vi->FAT_DirSectorCount;
}

void GetFat32Info(VOLINFO *vi,BOOTSECTOR32 *bs32)
{
	vi->dwReservedSectors=bs32->bpb.A_BF_BPB_ReservedSectors;
	vi->sector_size=bs32->bpb.A_BF_BPB_BytesPerSector;
	vi->sectors_per_fat=
		(bs32->bpb.A_BF_BPB_BigSectorsPerFatHi<<16)+ 
		bs32->bpb.A_BF_BPB_BigSectorsPerFat;
	vi->fat_count=bs32->bpb.A_BF_BPB_NumberOfFATs;
	vi->sectors_per_clus=bs32->bpb.A_BF_BPB_SectorsPerCluster;
	vi->sectors_total=bs32->bpb.A_BF_BPB_TotalSectors;
	if(vi->sectors_total==0)
	{
		vi->sectors_total=
			(bs32->bpb.A_BF_BPB_BigTotalSectorsHigh<<16)+
			bs32->bpb.A_BF_BPB_BigTotalSectors;
	}

	// Root directory info
	vi->FAT32_DirCluster=
		(bs32->bpb.A_BF_BPB_RootDirStrtClusHi<<16)+ 
		bs32->bpb.A_BF_BPB_RootDirStrtClus;
	vi->dwStartSector = vi->dwReservedSectors + 
		vi->sectors_per_fat*vi->fat_count;
}

DWORD DoWipe(VOLINFO *vi)
{
	DWORD RetVal,err;
	char StrRes[500];

	RetVal=WFE_NOERROR;

	vi->pClusterBuf=
		(unsigned char *)malloc(vi->dwClusterSize);

	if(!vi->pClusterBuf)
	{
		RetVal=WFE_OUTOFMEMORY;
	}
	else
	{
		LoadString (g_hinst, IDS_READINGFAT, StrRes, sizeof(StrRes));

		StatusMessage(StrRes, FALSE);

		err=ReadFAT(vi,vi->dwReservedSectors);

		if(err!=WFE_NOERROR)
		{
			RetVal=err;
		}
		else
		{
			LoadString (g_hinst, IDS_WIPINGDELANDSLACK, StrRes, sizeof(StrRes));

			StatusMessage(StrRes, FALSE);
			err=DoDirectories(vi);

			WipeProgress(5);

			if(err!=WFE_NOERROR)
			{
				RetVal=err;
			}
			else
			{
				LoadString (g_hinst, IDS_WIPINGCLUSTERS, StrRes, sizeof(StrRes));

				StatusMessage(StrRes, FALSE);
				err=WipeFreeClusters(vi);

				WipeProgress(10); 

				if(err!=WFE_NOERROR)
				{
					RetVal=err;
				}
			}
			free(vi->pFatBuf);
		}

		free(vi->pClusterBuf);
	}

	return RetVal;
}

DWORD NinetyFiveWipeFree(HWND hwnd,VOLINFO *vi,char *szRoot,
			   DWORD* pattern, BOOL StartWipe)
{
	BYTE* pReadBuf;
	DWORD RetVal,err;
	char StrRes[500];

	// One-based volume, 'A'=1, 'B'=2, 'C'=3, etc.
	vi->vol1=toupper(szRoot[0])-'A'+1;
 
	// Open VWIN32
	vi->hVWin32 = CreateFile( "\\\\.\\vwin32",
		0,0,NULL,0,FILE_FLAG_DELETE_ON_CLOSE,NULL );

	// Get Level1 Exclusive Volume Lock
	if(LockLogicalVolume(vi->hVWin32,vi->vol1, 
		LEVEL1_LOCK, 
		LOCK_MAX_PERMISSION,
		(vi->dwFS==FS_FAT32))!=0)
	{
		RetVal=WFE_COULDNOTLOCK;
	}
	else
	{
		LoadString (g_hinst, IDS_READINGBOOT, StrRes, sizeof(StrRes));

		StatusMessage(StrRes, FALSE);

		// Read in boot sector
		pReadBuf = (BYTE*)malloc( 2048 );

		memset(pReadBuf,0x00,2048);

		err=ReadSectors(vi,0,1,pReadBuf);

		if(err!=WFE_NOERROR)
		{
			RetVal=err;
		}
		else
		{
			// Store some disk parameters
			if(vi->dwFS==FS_FAT32)
				GetFat32Info(vi,
					(BOOTSECTOR32 *)pReadBuf);
			else
				GetFatInfo(vi,
					(BOOTSECTOR *)pReadBuf);

			free(pReadBuf);

			if(vi->sectors_per_clus==0)
				vi->no_of_clusters=0;
			else
				vi->no_of_clusters=
					(vi->sectors_total-vi->dwStartSector)/
					vi->sectors_per_clus;

			vi->dwClusterSize=vi->sectors_per_clus*
				vi->sector_size;

			vi->fTotalCapacity=(float)vi->no_of_clusters*
				(float)vi->dwClusterSize;

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

			DisplayDiskStats(vi);

			// Do wipe if so. Otherwise report stats.
			if(StartWipe)
			{
				vi->bCountClusters=TRUE;
				RetVal=DoWipe(vi);
				if(RetVal==WFE_NOERROR)
				{
					vi->bCountClusters=FALSE;
					RetVal=DoWipe(vi);
				}
			}
		}
		// Free Level 1 lock
		UnlockLogicalVolume(
			vi->hVWin32, 
			vi->vol1, 
			(vi->dwFS==FS_FAT32));
	} 
	CloseHandle(vi->hVWin32);

	return RetVal;
}

// WipeFree
//
// Main entry point for Win95 and WinNT wiper. Does
// idiot checking first, then branches off
DWORD WipeFree(HWND hwnd,VOLINFO *vi,char *szRoot,
			   DWORD* pattern, BOOL StartWipe)
{
	DWORD RetVal;
	OSVERSIONINFO osid;
	UINT DriveType;

	bGlobalCancel=FALSE;
	RetVal=WFE_NOERROR;

	memset(vi,0x00,sizeof(VOLINFO));
	vi->hwnd=hwnd;
	
	if(pattern)
	{
		memcpy(vi->pattern_buffer, pattern, 256 * sizeof(DWORD));
	}

	if(*szRoot==0)
		return WFE_NODRIVESELECTED;

	DriveType=GetDriveType(szRoot);

	if((DriveType==DRIVE_REMOTE)||
		(DriveType==DRIVE_CDROM)||
		(DriveType==DRIVE_UNKNOWN)||
		(DriveType==DRIVE_NO_ROOT_DIR)||
		(DriveType==DRIVE_RAMDISK))
		return WFE_DRIVENOTSUPPORTED;

	osid.dwOSVersionInfoSize = sizeof (osid);
	GetVersionEx (&osid);   
	vi->WinNT=(osid.dwPlatformId == VER_PLATFORM_WIN32_NT);

	if(!GetFileSystem(vi,szRoot))
		return WFE_FSNOTSUPPORTED;

	// End of idiot testing. Time to do work
	if(vi->WinNT)
		return NTWipeFree(hwnd,vi,szRoot,
			   pattern, StartWipe);
	else
		return NinetyFiveWipeFree(hwnd,vi,szRoot,
			   pattern, StartWipe);
}
