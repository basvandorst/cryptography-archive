#ifndef __INSTALLERDEFINES__
#define __INSTALLERDEFINES__

#ifndef __LISTSTRUCTS__
#include "ListStructs.h"
#endif

#if defined(powerc) || defined(__powerc)
#pragma options align=mac68k
#endif

// ITEM NUMBERS IN THE INSTALLATION DIALOG
#define kEasyInstallBtn				1		// Install Button
#define kEasyHelpBtn				2		// Help Button - Hidden if no help
#define kEasyCustomizeBtn			3		// 
#define kEasyQuitBtn				4		// Quit Button
#define kEasySwitchDiskBtn			5		// Switch Disk Button - Switches Destination disk of install
#define kEasyInstallOutlineItem		6		// Outline item for Install Button
#define kCustomListItem				7		// List Item (Custom Install)
#define kEasyLocTitleItem			8		// "Install Location" item
#define kEasyInstallSwitchPMItem	9		// Disk Popmenu User Item
#define kEasySelectFolderBtn		10		// Select Folder Button (visible if only one drive)
#define kEasyInstallFolderItem		11		// Frame for "Install Location"
#define kEasyInstallIconItem		12		// Disk & Folder Icon Item
#define kEasyInstallFolderNameItem	13		// Disk & Folder Name Item
#define kEasyInstallReadMeItem		14		// Read Me Button Item
#define kEasySelectedSizeTextItem	15		// "Selected Size" (Custom Install)
#define kEasySelectedSizeNumberItem	16		// Calculated Selected Size of Checked Packages (Custom Install)
#define kEasyDiskSpaceTextItem		17		// "Space Available"
#define kEasyDiskSpaceNumberItem	18		// Free Disk Space of current selected disk
#define kEasyItems2InstallItem		19		// Text for what is going to be installed (Easy Install)
#define kEasyMessageItem			20		// Disk Is Locked message item
#define kEasyMessageIconItem		21		// Alert Icon that goes with kEasyMessageItem


#define kMaxPackages	96					// Maximum number of packages

typedef unsigned char	Str9[10];

struct IPackageType {						// Internal Structure for packages
	Str63			ipk_Name;				// Package Name
	Str255			ipk_Description;		// Package Description
	Str9			ipk_Version;			// Package Version
	short			ipk_IconID;				// Package Icon ID#
	unsigned short	ipk_UniqueID;			// Unique ID Assigned to each package
	long			ipk_Size;				// Package Size
	Handle			ipk_BillBoard;			// PicHandle for the billboard
	unsigned char	ipk_NeedRestart;		// Need Restart Flag 0 = No Restart, 1 = Need Restart
	unsigned char	ipk_Index;				// internal index
	unsigned char	ipk_Checked;			// Package Checkbox is on=true or off=false
	Boolean			ipk_UseFilesSetting;	// FAT Package - Use the Files Setting
	Boolean			ipk_68K;				// FAT Package - install 68K Version of Fat Apps
	Boolean			ipk_PPC;				// FAT Package - install PPC Version of Fat Apps
											// if both ipk_68K and ipk_PPC then install Fat Version of App
	Boolean			ipk_MEP;				// Mutual Exclusive Package
	Boolean			ipk_SubPackage;			// Package is a Sub-Archive Install Package
	Boolean			ipk_ListPackage;		// If this package is a List Package
	Boolean			ipk_ChildPackage;		// If this package has a Parent Package (Hiararchial packages)
	Boolean			ipk_DefaultPkgOn;		// If this package checkbox defaults to on
	unsigned char	ipk_ExpandStatus;		// Whether or Not the Parent Package is expanded
	Handle			ipk_ListHelp;			// Handle to a PICT resource for the List Package Help
	Str255			ipk_ListHeader;			// A Pascal String for the List Package Header Message
	Str255			ipk_ListFooter;			// A Pascal String for the List Package Footer Message
	long			ipk_Gestalts;			// Bits for which Gestalts To Check
};
typedef struct IPackageType IPackageType;
typedef IPackageType *IPackageTypePtr;


struct IPackageArray {
	IPackageType		ipl_List[kMaxPackages];	// Array of Packages
};
typedef struct IPackageArray IPackageArray;
typedef IPackageArray *IPackageArrayPtr, **IPackageArrayHdl;


#endif
