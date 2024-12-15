#ifndef __LISTSTRUCTS__
#define __LISTSTRUCTS__

#if defined (__PPCC__) || defined (applec)
#ifndef __FILES__
#include <Files.h>
#endif


#endif


#if defined(powerc)
#pragma options align=mac68k
#endif

// ATTN: LOWEST BYTE OF FLAGS IS THE EXTRACT TO FOLDER (theFile.af_Flags & 0x000000FF)
// **** IMPORTANT - DO NOT MODIFY UPPER THREE BYTES - THESE ARE HEAVILY RELIED UPON DURING INSTALLATION (af_Flags & 0xFFFFFF00)
#define kExtToInstallFolder				0		// Install to Install Folder
#define kExtToSystemFolder				1		// Install to System Folder
#define kExtToExtensionFolder			2		// Install to Extensions Folder
#define kExtToControlPanelsFolder		3		// Install to Control Panels Folder
#define kExtToPreferencesFolder			4		// Install to Preferences Folder
#define kExtToAppleMenuFolder			5		// Install to Apple Menu Folder
#define kExtToDesktopFolder				6		// Install to Desktop Folder
#define kExtToStartupFolder				7		// Install to Startup Folder
#define kExtToTemporaryFolder			8		// Install to Temporary Folder
#define kExtToPrintMonitorFolder		9		// Install to Print Monitor Folder
#define kExtToFontsFolder				10		// Install to Fonts Folder
#define kExtToSystemFile				12		// Install to System Folder
#define kExtToInstallLocFromFonts		251		// Install to Install Folder - was originally to the Fonts Folder
#define kExtToSystemFldrFromFonts		252		// Install to System Folder - was originally to the Fonts Folder
#define kExtToSystemFldrFromAppleMenu	253		// Install to System Folder - was originally to the Apple Menu Folder
#define kExtToSystemFromFonts			254		// Install to System File - was originally to the Fonts Folder
#define kExtToSystemFromAppleMenu		255		// Install to System File - was originally to the Apple Menu Folder

// FAT BINARY FLAGS (Bit Numbers ie. BitTst(&af_FatBinaryFlags,kInstallHascrfgRes))
#define kInstallFatBinary68K	0		// Install 68K portion of application
#define kInstallFatBinaryPPC	1		// Install PowerPC portion of application
#define kInstallFatBinaryAsk	2		// Ask User what to install
#define kInstallFatBinarySnip	4		// Install Snipit of Code telling user this app requires PPC
#define kInstall68K				8		// * Install 68K Portion of application - USE ONLY IF CUSTOM INSTALL
#define kInstallPPC				9		// * Install PPC Portion of application - USE ONLY IF CUSTOM INSTALL
										// ** ATTN: both kInstall68K & kInstallPPC bits CAN be set (means install FAT)
#define kInstallHasPPC			13		// This application is a PPC Only Application
#define kInstallHas68K			14		// 68K App - may contain PPC app
#define kInstallHascfrgRes		15		// Fat App - 68K and PPC


struct ArcFileType {
	Str32			af_Name;				// File Name
	Str32			af_Internal1;			// Internal Use Only
	unsigned long	af_CreateDate;			// Modificaton Date of File
	unsigned long	af_ModDate;				// Modificaton Date of File
	long			af_DirID;				// Directory ID
	long			af_Internal2;			// Internal Use Only
	long			af_Internal3;			// Internal Use Only
	short			af_Internal4;			// Internal Use Only
	short			af_vRefNum;				// Volume Reference Number
	long			af_CompDataFork;		// Size of compressed data fork
	long			af_CompResFork;			// Size of compressed resource fork
	long			af_unCompDataFork;		// Size of uncompressed data fork
	long			af_unCompResFork;		// Size of un compressed resource fork
	long			af_SegNumber;			// Segment Number file is in
	long			af_Internal5;			// Internal Use Only
	long			af_Internal6;			// Internal Use Only
	long			af_Internal7;			// Internal Use Only
	unsigned long	af_CRC;					// Checksum

	FInfo			af_FInfo;				// FileInfo
	long			af_Flags;				// special location flags and replace flags
	long			af_ExtractFlags;		// gestalt calls bit flags (Gestalts to call before installing this file)
	long			af_PackageFlags1;		// packages flags
	long			af_PackageFlags2;		// packages flags
	long			af_PackageFlags3;		// packages flags
	long			af_CodeBeforeFlags;		// external code to call before installing flags
	long			af_CodeAfterFlags;		// external code to call after installing flags
	short			af_FatBinaryFlags;		// when to install fat binary (see FAT BINARY FLAGS above)
	unsigned short	af_UniqueID;			// a unique number used by the installer
	unsigned short	af_MergeIntoID;			// UniqueID of file to merge into
	unsigned short	af_InstallIfID;			// Install this file if Install Action item/fails succeeds
	unsigned long	af_Version;				// 4 bytes 1st part, 2nd & 3rd parts, development stage, prerelease version
	unsigned char	af_InstallWhen;			// Install if Succeeds or Fails
	unsigned char	af_Directory;			// File is a directory (if nonZero)
	unsigned char	af_Internal8;			// Internal Use Only
	unsigned char	af_Internal9;			// Internal Use Only
	unsigned char	af_InstallDisk;			// disk the uncomp. file is supposed to be on 0 = compressed
	unsigned char	af_Locked;				// file - locked - from ioFlAttrib
	unsigned long	af_LanguageBits[2];		// PopMenu Value for "Language" Country Code (1...37)
	unsigned long	af_RegionBits[2];		// PopMenu Value for "Region" Country Code (1...62)
	unsigned char	af_Internal10;			// Internal Use Only
};
typedef struct ArcFileType ArcFileType;
typedef ArcFileType *ArcFilePtr;

struct ArcList {
	ArcFileType 	al_files[1];
};
typedef struct ArcList ArcList ;
typedef ArcList *ArcListPtr, **ArcListHdl;


#if defined(powerc)
#pragma options align=reset
#endif

#endif
