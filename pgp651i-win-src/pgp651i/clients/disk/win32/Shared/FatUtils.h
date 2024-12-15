//////////////////////////////////////////////////////////////////////////////
// FatUtils.h
//
// Declarations for FatUtils.cpp.
//////////////////////////////////////////////////////////////////////////////

// $Id: FatUtils.h,v 1.5 1998/12/14 19:00:58 nryan Exp $

// Copyright (C) 1998 by Network Associates, Inc.
// All rights reserved.

#ifndef Included_FatUtils_h	// [
#define Included_FatUtils_h

// Align to 1.
#pragma pack(push, 1)

//////////////////////////////////////
// Constants common to all FAT volumes
//////////////////////////////////////

const PGPUInt8	kMediaByte			= 0xF8;
const PGPUInt8	kHardDriveId		= 0x80;
const PGPUInt8	kFat12PartId		= 0x01;
const PGPUInt8	kBigFat16PartId		= 0x06;
const PGPUInt8	kSmallFat16PartId	= 0x04;
const PGPUInt8	kFat32PartId		= 0x0B;
const PGPUInt8	kFirstBootSig		= 0x29;
const PGPUInt16	kSecondBootSig		= 0xAA55;


//////////////////////////////////////
// Constants specific to FAT12 volumes
//////////////////////////////////////

static LPCSTR kFat12IdStr = "FAT12   ";

const PGPUInt32	kFat12Offset		= 0x36;
const PGPUInt32	kFat12ClustBias		= 2;
const PGPUInt32	kBitsFat12Clust		= 12;
const PGPUInt32	kBytesFat12DirEnt	= 32;
const PGPUInt16	kMinFat12Clust		= 0x0002;
const PGPUInt16	kMaxFat12Clust		= 0x0FEF;
const PGPUInt32	kFat12Sig			= 0x00FFFFF8;


//////////////////////////////////////
// Constants specific to FAT16 volumes
//////////////////////////////////////

static LPCSTR kFat16IdStr = "FAT16   ";

const PGPUInt32	kFat16Offset		= 0x36;
const PGPUInt32	kFat16ClustBias		= 2;
const PGPUInt32	kBitsFat16Clust		= 16;
const PGPUInt32	kBytesFat16DirEnt	= 32;
const PGPUInt16	kMinFat16Clust		= 0x0002;
const PGPUInt16	kMaxFat16Clust		= 0xFFEF;
const PGPUInt32	kOver16MbFat16Sig	= 0xFFFFFFF8;
const PGPUInt32	kUnder16MbFat16Sig	= 0x00FFFFF8;


//////////////////////////////////////
// Constants specific to FAT32 volumes
//////////////////////////////////////

static LPCSTR kFat32IdStr = "FAT32   ";

const PGPUInt32	kFat32Offset			= 0x52;
const PGPUInt32	kFat32ClustBias			= 2;
const PGPUInt32	kBitsFat32Clust			= 32;
const PGPUInt32	kBigFatSig				= 0x61417272;
const PGPUInt32	kBigFatSecSig			= 0x41615252;
const PGPUInt32	kFat32ClustMask			= 0x00FFFFFF;
const PGPUInt32	kMinFat32Megs			= 260;
const PGPUInt32	kMinFat32Clust			= 0x00000002;
const PGPUInt32	kMaxFat32Clust			= 0x00FFFFEF;
const PGPUInt32	kFat32Clust1			= 0x0FFFFFF8;
const PGPUInt32	kFat32Clust2			= 0x0FFFFFFF;
const PGPUInt32	kFat32Clust3			= 0x0FFFFFF8;
const PGPUInt32	BGBPB_F_ActiveFATMsk	= 0x000F;
const PGPUInt32	BGBPB_F_NoFATMirror		= 0x0080;


//////////////////////////
// Miscellaneous constants
//////////////////////////

const PGPUInt8 kFloppyMediaId = 0xF0;


////////
// Types
////////

enum FileSysId {kFS_FAT12, kFS_FAT16, kFS_FAT32, kFS_NTFS, kFS_Unknown};

#pragma pack(1)

// Geometry is a structure that describes the geometry of a drive.

typedef struct Geometry
{
	PGPUInt8	geHeads;
	PGPUInt8	geCyls;
	PGPUInt8	geSpt;
	PGPUInt16	geBlockSize;
	PGPUInt64	geSecsDisk;

} Geometry, *PGeometry;

// FatData is a PGPdisk-specific structure used to store FAT information about
// a certain file open on a FAT16 or FAT32 host drive.

typedef struct FatData
{
	FileSysId	fdFsId;					// will be FAT12, FAT16 or FAT32

	PGPUInt8	fdActiveFat;			// index of active FAT
	PGPUInt8	fdFatCount;				// number of FATs

	PGPUInt16	fdBlockSize;			// blocksize of the volume
	PGPUInt16	fdReservedSecs;			// number of reserved sectors
	PGPUInt16	fdRootDirEnts;			// entries in root dir (FAT12/16)
	PGPUInt16	fdSpc;					// sectors per cluster

	PGPUInt32	fdFatSize;				// # of sectors per fat
	PGPUInt32	fdFirstSecFat;			// sector # of first fat
	PGPUInt32	fdFirstClustFile;		// clust # where file's data begins
	PGPUInt32	fdFirstSecData;			// first sector of data area on drive

} FatData, *PFatData;

// FvBlock is used by the system during int21h 440d 42 calls. It specifies
// a sector on a disk to format.

typedef struct FvBlock
{
	PGPUInt8	fvSpecFunc;				// 'special function'
	PGPUInt16	fvHead;					// head
	PGPUInt16	fvCylinder;				// cylinder
	PGPUInt16	fvTracks;				// track

} FvBlock;

// FvBlock is used by the system during int21h 440d 61 calls. It specified
// one or more sectors on a disk to read.

typedef struct RwBlock
{
	PGPUInt8	rwSpecFunc;				// 'special function'
	PGPUInt16	rwHead;					// head
	PGPUInt16	rwCylinder;				// cylinder
	PGPUInt16	rwFirstSector;			// spt
	PGPUInt16	rwSectors;				// number of sectors
	PGPUInt32	rwBuffer;				// output buffer

} RwBlock;

// MID is used by the system during int21h 440d 66 calls.

typedef struct MID
{
	PGPUInt16	midInfoLevel;			// information level
	PGPUInt32	midSerialNum;			// serial number
	PGPUInt8	midVolLabel[11];		// volume label
	PGPUInt8	midFileSysType[8];		// filesystem type

} MID;

// A PartEntry is a structure contained in the MBR (master boot record) of a
// hard disk that describes the geometry for a particular partition. There can
// be up to 4 of these.

typedef struct PartEntry
{
	PGPUInt8	peBootable;				// 80h = bootable, 00h = nonbootable
	PGPUInt8	peBeginHead;			// beginning head
	PGPUInt8	peBeginSector;			// beginning sector
	PGPUInt8	peBeginCylinder;		// beginning cylinder
	PGPUInt8	peFileSystem;			// ID of filesystem
	PGPUInt8	peEndHead;				// ending head
	PGPUInt8	peEndSector;			// ending sector
	PGPUInt8	peEndCylinder;			// ending cylinder
	PGPUInt32	peStartSector;			// starting sector
	PGPUInt32	peSectors;				// total sectors

} PartEntry, *PPartEntry;

// BootSector16 is a structure exactly 512 bytes long that occupies the first
// sector of a FAT66 partition. On real hard disks, there are actually two
// boot blocks: the MBR (master boot record) at the very beginning of the disk
// and a FAT bootsector in each partition. With PGPdisks we simply combine the
// MBR and FAT bootblocks into one big partition.

typedef struct BootSector16
{
	PGPUInt8	bsJump[3];				// jmp to bootstrap instruction
	char		bsOemName[8];			// OEM name and version

	PGPUInt16	bsBytesPerSec;			// bytes per sector
	PGPUInt8	bsSecPerClust;			// sectors per cluster
	PGPUInt16	bsResSectors;			// number of reserved sectors
	PGPUInt8	bsFats;					// number of file allocation tables
	PGPUInt16	bsRootDirEnts;			// number of root-directory entries
	PGPUInt16	bsSectors;				// total number of secs (<= 32 megs)
	PGPUInt8	bsMedia;				// media descriptor
	PGPUInt16	bsFatSecs;				// number of sectors per FAT
	PGPUInt16	bsSecPerTrack;			// sectors per track (head)
	PGPUInt16	bsHeads;				// number of heads
	PGPUInt32	bsHiddenSecs;			// number of hidden sectors
	PGPUInt32	bsHugeSectors;			// total number of sectors (> 32 megs)
	PGPUInt8	bsDriveNumber;			// drive number (80h)

	PGPUInt8	bsReserved1;			// reserved
	PGPUInt8	bsBootSignature;		// extended boot signature (29h)
	PGPUInt32	bsVolumeId;				// volume ID number
	char		bsVolumeLabel[11];		// volume label
	char		bsFileSysType[8];		// filesystem type
	PGPUInt8	padding[384];			// padding to 512 bytes
	PartEntry	bsPartEnts[4];			// partition entry table
	PGPUInt16	bsSignature;			// boot block signature (0xAA55h)

} BootSector16, *PBootSector16;

typedef BootSector16 BootSector12;
typedef BootSector12 *PBootSector12;

// The BigFatBootFSInfo structure (yes, this is the official Microsoft name
// for it) contains extra info for FAT32 drives and typically occupies its own
// sector.

typedef struct BigFatBootFSInfo
{
	PGPUInt32	bfSecSig;						// sector sig (kBigFatSecSig)
	PGPUInt8	bfPadding1[480];				// padding to 512 bytes

	PGPUInt32	bfFSInf_Sig;					// signature (kBigFatSig)
    PGPUInt32	bfFSInf_free_clus_cnt;			// number of free clusters
	PGPUInt32	bfFSInf_next_free_clus;			// next free cluster
    PGPUInt32	bfFSInf_resvd;
	PGPUInt32	bfReserved[2];
	PGPUInt8	bfPadding2[2];					// padding to 512 bytes
	PGPUInt16	bsSignature;					// 0xAA55h as in boot block

} BigFatBootFSInfo, *PBigFatBootFSInfo;

// BootSector32 is a structure that is two sectors long and represents the
// boot blocks for FAT32 drives. The first sector is similar to that for
// FAT16, but the second sector contains some additional information about
// free sectors, etc.

typedef struct BootSector32
{
	PGPUInt8	bsJump[3];				// jmp to bootstrap instruction
	char		bsOemName[8];			// OEM name and version

	PGPUInt16	bsBytesPerSec;			// bytes per sector
	PGPUInt8	bsSecPerClust;			// sectors per cluster
	PGPUInt16	bsResSectors;			// number of reserved sectors
	PGPUInt8	bsFats;					// number of file allocation tables
	PGPUInt16	bsRootDirEnts;			// number of root-directory entries
	PGPUInt16	bsSectors;				// total number of secs (<= 32 megs)
	PGPUInt8	bsMedia;				// media descriptor
	PGPUInt16	bsFatSecs;				// number of sectors per FAT
	PGPUInt16	bsSecPerTrack;			// sectors per track (head)
	PGPUInt16	bsHeads;				// number of heads
	PGPUInt32	bsHiddenSecs;			// number of hidden sectors
	PGPUInt32	bsHugeSectors;			// total number of sectors (> 32 megs)
	PGPUInt32	bsBigSectorsPerFat;		// total sectors per FAT
	PGPUInt16	bsExtFlags;				// extended flags
	PGPUInt16	bsFS_Version;			// filesystem version
	PGPUInt32	bsRootDirStrtClus;		// first cluster of root dir
	PGPUInt16	bsFsInfoSec;			// sector of FSInfo sec
	PGPUInt16	bsBkUpBootSec;			// sector of backup boot sec
	PGPUInt16	bsReserved2[6];			// reserved
	
	PGPUInt8	bsDriveNumber;			// drive number (80h)
	PGPUInt8	bsReserved1;			// reserved
	PGPUInt8	bsBootSignature;		// extended boot signature (29h)
	PGPUInt32	bsVolumeId;				// volume ID number
	char		bsVolumeLabel[11];		// volume label
	char		bsFileSysType[8];		// filesystem type
	PGPUInt8	padding1[356];			// padding to fill out first sector
	PartEntry	bsPartEnts[4];			// partition entry table
	PGPUInt16	bsSignature;			// boot block signature (0xAA55h)

} BootSector32, *PBootSector32;

// In order to gather the necessary information about the structure of the
// host FAT drive, the app makes simulated DOS IOCTL calls. If the host is
// FAT16, it makes a call to retrieve a FAT16 DevParams structure. If the
// host is FAT32, it makes a call to retrieve a FAT32 DevParams structure.

typedef struct DevParams16
{
	PGPUInt8	dpSpecFunc;				// special functions
	PGPUInt8	dpDevType;				// device type
	PGPUInt16	dpDevAttr;				// device attributes
	PGPUInt16	dpCylinders;			// number of cylinders
	PGPUInt8	dpMediaType;			// media type

	// FAT16 BPB begins here.
	PGPUInt16	dpBytesPerSec;			// bytes per sector
	PGPUInt8	dpSecPerClust;			// sectors per cluster
	PGPUInt16	dpResSectors;			// number of reserved sectors
	PGPUInt8	dpFats;					// number of FATs
	PGPUInt16	dpRootDirEnts;			// number of root-directory entries
	PGPUInt16	dpSectors;				// total # of sectors (<= 32 megs)
	PGPUInt8	dpMedia;				// media descriptor
	PGPUInt16	dpFatSecs;				// number of sectors per FAT
	PGPUInt16	dpSecPerTrack;			// sectors per track
	PGPUInt16	dpHeads;				// number of heads
	PGPUInt32	dpHiddenSecs;			// number of hidden sectors
	PGPUInt32	dpHugeSectors;			// total # of sectors (> 32 megs)

} DevParams16, *PDevParams16;

typedef struct DevParams32
{
	PGPUInt8	dpSpecFunc;				// special functions
	PGPUInt8	dpDevType;				// device type
	PGPUInt16	dpDevAttr;				// device attributes
	PGPUInt16	dpCylinders;			// number of cylinders
	PGPUInt8	dpMediaType;			// media type

	// FAT32 BPB begins here
	PGPUInt16	dpBytesPerSector;		// bytes per sector	
	PGPUInt8	dpSectorsPerClust;		// sectors per cluster
	PGPUInt16	dpReservedSectors;		// number of reserved sectors
	PGPUInt8	dpNumberOfFats;			// number of FATs
	PGPUInt16	dpRootEntries;			// field ignored on FAT32
	PGPUInt16	dpTotalSectors;			// total number sectors
	PGPUInt8	dpMediaDescriptor;		// media descriptor
	PGPUInt16	dpSectorsPerFat;		// field ignored in FAT32
	PGPUInt16	dpSectorsPerTrack;		// sectors per track (head)
	PGPUInt16	dpHeads;				// number of heads
	PGPUInt32	dpHiddenSectors;		// number of hidden sectors
	PGPUInt32	dpBigTotalSectors;		// total sectors on drive
	PGPUInt32	dpBigSectorsPerFat;		// total sectors per FAT
	PGPUInt16	dpExtFlags;				// extended flags
	PGPUInt16	dpFS_Version;			// filesystem version
	PGPUInt32	dpRootDirStrtClus;		// first cluster of root dir
	PGPUInt16	dpFsInfoSec;			// sector of FSInfo sec
	PGPUInt16	dpBkUpBootSec;			// sector of backup boot sec
	PGPUInt16	dpReserved;				// reserved
	// FAT 32 BPB ends here

	PGPUInt8	dpPadding[32];

} DevParams32, *PDevParams32;

// Restore alignment.
#pragma pack(pop)


///////////////////////////
// Inline utility functions
///////////////////////////

// IsFatVolume returns TRUE if the passed file system ID refers to a FAT
// volume.

inline 
PGPBoolean
IsFatVolume(FileSysId fsId)
{
	return ((fsId == kFS_FAT12) || (fsId == kFS_FAT16) || 
		(fsId == kFS_FAT32));
}


/////////////////////
// Exported functions
/////////////////////

void		CalcGeometry(PGPUInt64 blocksDisk, PGPUInt16 blockSize, 
				Geometry *geom);

PGPUInt16	CalcFat16Spc(PGPUInt64 blocksData);
PGPUInt16	CalcFat32Spc(PGPUInt64 blocksData);
void		CalcFatSize(FatData *fat, PGPUInt64 blocksDisk);
void		CalcFatDataSec(FatData *fat);

void		InitFatData(FatData *fat, PGPUInt64 blocksDisk);

PGPUInt32	ClusterToSector(PGPUInt32 clustNum, PGPUInt32 firstSecData, 
				PGPUInt16 spc, PGPUInt16 bias);
PGPUInt32	ExtractCluster(PGPUInt8 *buf, PGPUInt32 bitOffset, 
				PGPUInt32 curClust, FileSysId fsId);
void		GetClusterInfo(PGPUInt32 firstFatSec, PGPUInt32 clustNum, 
				PGPUInt32 bitsClust, PGPUInt16 blockSize, PGPUInt32 *clustSec, 
				PGPUInt32 *bitOffset);
PGPUInt32	TotalClusters(PGPUInt32 fatSize, PGPUInt32 bitsClust, 
				PGPUInt16 blockSize);

#endif	// ] Included_FatUtils_h
