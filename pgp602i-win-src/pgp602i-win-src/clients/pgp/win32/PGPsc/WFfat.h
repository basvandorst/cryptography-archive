#define VWIN32_DIOC_DOS_IOCTL   1   // DOS ioctl calls 4400h-4411h
#define VWIN32_DIOC_DOS_INT25   2   // absolute disk read, DOS int 25h
#define VWIN32_DIOC_DOS_INT26   3   // absolute disk write, DOS int 26h
#define VWIN32_DIOC_DOS_DRIVEINFO 6 // OEM Service Release 2 
                                    //   Int 21h, 73xxh extensions
typedef struct _DIOC_REGISTERS {
    DWORD reg_EBX;
    DWORD reg_EDX;
    DWORD reg_ECX;
    DWORD reg_EAX;
    DWORD reg_EDI;
    DWORD reg_ESI;
    DWORD reg_Flags;
    }
    DIOC_REGISTERS, *PDIOC_REGISTERS;

#define CARRY_FLAG 1

#define   LEVEL1_LOCK   1
#define   LEVEL2_LOCK   2
#define   LEVEL3_LOCK   3

// We'll allow everything since we just abort if things
// change
#define   LOCK_MAX_PERMISSION  0x001 

/*********************************************************
 **** Note: all MS-DOS data structures must be packed ****
 ****       on a one-byte boundary.                   ****
 *********************************************************/
#pragma pack(1)

typedef struct _DISKIO {
    DWORD diStartSector;    // sector number to start at
    WORD  diSectors;        // number of sectors
    DWORD diBuffer;         // address of buffer
    }
    DISKIO, *PDISKIO;


typedef struct _BOOTSECT {
    BYTE    bsJump[3];      // jmp instruction
    char    bsOemName[8];   // OEM name and version

	  // This portion is the BPB (BIOS Parameter Block)
    WORD    bsBytesPerSec;		// bytes per sector
    BYTE    bsSecPerClust;		// sectors per cluster
    WORD    bsResSectors;		  // number of reserved sectors
    BYTE    bsFATs;				    // number of file allocation tables
    WORD    bsRootDirEnts;		// number of root-directory entries
    WORD    bsSectors;			  // total number of sectors
    BYTE    bsMedia;			    // media descriptor
    WORD    bsFATsecs;			  // number of sectors per FAT
    WORD    bsSecPerTrack;		// number of sectors per track
    WORD    bsHeads;			    // number of read/write heads
    DWORD   bsHiddenSectors;	// number of hidden sectors
    DWORD   bsHugeSectors;		// number of sectors if bsSectors==0    
	  // End of BPB

    BYTE    bsDriveNumber;		  // 80h if first hard drive
    BYTE    bsReserved;     
    BYTE    bsBootSignature;    // 29h if extended boot-signature record
    DWORD   bsVolumeID;			    // volume ID number
    char    bsVolumeLabel[11];  // volume label
    char    bsFileSysType[8];   // file-system type (FAT12 or FAT16)
    }
    BOOTSECTOR, *PBOOTSECTOR;

typedef struct _DIRENTRY {
    char    deName[8];			    // base name
    char    deExtension[3];		  // extension
    BYTE    deAttributes;		    // file or directory attributes
    BYTE    deReserved[6];
    WORD    deLastAccessDate;	  // *New Win95* - last access date 
    WORD    deEAhandle;			    // *New FAT32* - high word of starting cluster
    WORD    deCreateTime;		    // creation or last modification time
    WORD    deCreateDate;		    // creation or last modification date
    WORD    deStartCluster;		  // starting cluster of the file or directory
    DWORD   deFileSize;			    // size of the file in bytes
    }
    DIRENTRY, *PDIRENTRY;

typedef struct _LONGDIRENTRY {
    char    leSequence;			// sequence byte:1,2,3,..., last entry is or'ed with 40h
    wchar_t leName[5];			// Unicode characters of name
    BYTE    leAttributes;		// Attributes: 0fh
    BYTE    leType;				  // Long Entry Type: 0
    BYTE    leChksum;			  // Checksum for matching short name alias
    wchar_t leName2[6];			// More Unicode characters of name
    WORD    leZero;				  // reserved
    wchar_t leName3[2];			// More Unicode characters of name
    }
    LONGDIRENTRY, *PLONGDIRENTRY;

////////////////// FAT32 Structures ///////////////////

// BPB for a Fat32 partition
typedef struct _A_BF_BPB {
    WORD    A_BF_BPB_BytesPerSector;
    BYTE    A_BF_BPB_SectorsPerCluster;
    WORD    A_BF_BPB_ReservedSectors;
    BYTE    A_BF_BPB_NumberOfFATs;
    WORD    A_BF_BPB_RootEntries;
    WORD    A_BF_BPB_TotalSectors;
    BYTE    A_BF_BPB_MediaDescriptor;
    WORD    A_BF_BPB_SectorsPerFAT;
    WORD    A_BF_BPB_SectorsPerTrack;
    WORD    A_BF_BPB_Heads;
    WORD    A_BF_BPB_HiddenSectors;
    WORD    A_BF_BPB_HiddenSectorsHigh;
    WORD    A_BF_BPB_BigTotalSectors;
    WORD    A_BF_BPB_BigTotalSectorsHigh;
    WORD    A_BF_BPB_BigSectorsPerFat;
    WORD    A_BF_BPB_BigSectorsPerFatHi;
    WORD    A_BF_BPB_ExtFlags;
    WORD    A_BF_BPB_FS_Version;
    WORD    A_BF_BPB_RootDirStrtClus;
    WORD    A_BF_BPB_RootDirStrtClusHi;
    WORD    A_BF_BPB_FSInfoSec;
    WORD    A_BF_BPB_BkUpBootSec;
    WORD    A_BF_BPB_Reserved[6];
    }
    A_BF_BPB, PA_BF_BPB;

typedef struct _BOOTSECT32 {
    BYTE    bsJump[3];      // jmp instruction
    char    bsOemName[8];   // OEM name and version

	  // This portion is the FAT32 BPB
	  A_BF_BPB  bpb;

    BYTE    bsDriveNumber;		  // 80h if first hard drive
    BYTE    bsReserved;     
    BYTE    bsBootSignature;    // 29h if extended boot-signature record
    DWORD   bsVolumeID;			    // volume ID number
    char    bsVolumeLabel[11];  // volume label
    char    bsFileSysType[8];   // file-system type (FAT32)
    }
    BOOTSECTOR32, *PBOOTSECTOR32;
 
#pragma pack()
