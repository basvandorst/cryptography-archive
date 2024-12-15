/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.

	

	$Id: Define.h,v 1.24.4.1 1999/06/14 05:26:54 philipn Exp $
____________________________________________________________________________*/
#pragma message( "Compiling " __FILE__ ) 
#pragma message( "Last modified on " __TIMESTAMP__ ) 
#pragma once


#define BINDREVIEWOCCURED		"Because of these network settings changes, after rebooting you must run PGPnet Set Adapter to secure a network card."
#define STDREBOOTMSG			"You must shut down and restart your computer before the new settings will take effect.\n\nDo you want to restart your computer now?"
#define BINDREVIEWREBOOT		"Due to the sytem bind review that just occured you need to reboot your system.\nAfter rebooting you will need run PGPnet Set Adapter and select a network card\nto secure."
#define REMOVEDNEEDREBOOT		"You need to reboot your machine.\nAfter reboot you can run PGPnet Set Adapter to	secure a new network card."
#define NEEDREBOOT				"You need to reboot your machine.\nNew settings will take effect after system is rebooted."
#define	NCPA					"SOFTWARE\\Microsoft\\Ncpa\\CurrentVersion"
#define	RUNONCE					"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce"
#define RUNKEY					"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
#define TITLE					"PGPnet Set Adapter"
#define NETA_KEY				"SOFTWARE\\Network Associates"
#define	ROOTNET					"Enum\\Root\\PGPnet"
#define	CLASSNET				"System\\CurrentControlSet\\Services\\Class\\Net"
#define	CLASSNETTRANS			"System\\CurrentControlSet\\Services\\Class\\NetTrans"
#define DELREG					"delreg"
#define BIND					"Bind"
#define FULLPATHTOPGPNETREGKEY	"HKEY_LOCAL_MACHINE\\SOFTWARE\\Network Associates\\PGP\\PGPnet\\Settings"
#define PATHTOPGPNETREGKEY		"SOFTWARE\\Network Associates\\PGP\\PGPnet"
#define PATHTOADAPTERLIST		"SOFTWARE\\Network Associates\\PGP\\PGPnet\\Adapters"
#define KEYTCPIPDISABLE			"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Linkage\\Disabled"
#define KEYNETBTDISABLE			"SYSTEM\\CurrentControlSet\\Services\\NetBT\\Linkage\\Disabled"
#define SERVICESKEY				"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services"
#define PGPMACMPKEY				"SYSTEM\\CurrentControlSet\\Services\\PGPMacMP"
#define SERVICESSUBKEY			"SYSTEM\\CurrentControlSet\\Services"
#define TCPIPKEYSTUB			"Parameters\\Tcpip"
#define KEYSTUB					"\\Device\\"
#define INFNAME					"oemsetup.inf"
#define TEMPDIR					"\\pgpnettmp"
#define ADAPTERNAME				"PGPMACMP"
#define ADAPTERDESCRIPTION		"PGPnet Vpn Driver Virtual Adapter"
#define ADDINF					"addinf"
#define REMOVEINF				"removeinf"
#define INSTALL					"install"
#define INSTALLSILENT			"installs"
#define UNINSTALL				"uninstall"
#define BINDREVIEW				"review"
#define SZNETREVIEW				"NetSetupReviewBindings"
#define SZNETINSTALL			"NetSetupComponentInstall"
#define SZNETREMOVE				"NetSetupComponentRemove"
#define NETWORKCARDTITLE		"Title"
#define NETWORKCARDS_KEY		"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards"
#define FULLNETWORKCARDS_KEY	"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards\\"
#define NETWORKCARDS_KEYVALUE	"ServiceName"
#define REALSERVICENAMEKEY		"ProductName"
#define TCPIP_SERVICES_KEY		"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Linkage"
#define BINDINGS				"Bind"
#define ERR_OPENREG				"Unable to open registry.  Reinstall may fix this."
#define ERR_READREG				"Unable to read registry. Reinstall may fix this."
#define ERR_GETDLGITEM			"Unexpected error getting window handle. Reinstall may fix this."
#define ERR_IMGLISTCREATE		"Unable to create image list"
#define MAX_KEY_LENGTH          256
#define NUM_BITMAPS				2
#define	NETCFGDLL				"netcfg.dll"
#define WRONGOS					"Incorrect OS"
#define USAGEERROR				"This application is used by the PGPnet install and uninstall only."
#define	CONFIRM					"Are you sure you want to cancel the\ninstall of PGPnet?"
#define PGPNETCANCELED			"PGPnet will not be installed."
//WIN9598
#define WIN95NETLIST			"System\\CurrentControlSet\\Services\\Class\\Net"
#define NETWORKCARDTITLE95		"DriverDesc"
#define ERRREMOVE95				"Unable to delete PGPnet Bindings. User not logged in with proper permissions"
#define GENERRREMOVE			"Unable to remove PGPnet components. To reconfigure, uninstall and reinstall."
#define BACKUP95BINDINGSKEY		"Software\\Network Associates\\PGP\\PGPnet\\PGPnetBackup"
#define FULLBACKUP95BINDINGSKEY	"HKEY_LOCAL_MACHINE\\Software\\Network Associates\\PGP\\PGPnet\\PGPnetBackup"