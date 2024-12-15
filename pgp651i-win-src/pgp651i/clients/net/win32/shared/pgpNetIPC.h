/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	pgpNetIPC.h - PGPnet Win32 inter-process communication header
	
	$Id: pgpNetIPC.h,v 1.32 1999/05/19 15:42:06 pbj Exp $
____________________________________________________________________________*/

#ifndef _PGPNETIPC_H
#define _PGPNETIPC_H

// This is the registry key that modules should use to find	the install
// directory and contained files
#define PGP_REGISTRYKEY				("Software\\Network Associates\\PGP")
#define PGP_INSTALLPATHVALUE		("InstallPath")

// These are files that are located in the install directory 
#define	PGPNET_PREFSFILE			("PGPnet.dat")
#define	PGPNET_SDKPREFSFILE			("PGPnetsdk.dat")
#define PGPNET_LOGFILE				("PGPnetLog.dat")
#define PGPNET_NETAPP				("PGPnet.exe")
#define PGPNET_HELPFILE				("PGPnet.hlp")

// These are values of command-line switches used to select specific
// pages of PGPnet application on startup
#define PGPNET_STATUSPAGE			0
#define PGPNET_LOGPAGE				1
#define PGPNET_HOSTPAGE				2
#define PGPNET_OPTIONSHEET			3

// These are values passed from service to trayapp in WM_COPYDATA 
// message (see below).
#define PGPNET_LOGGEDOFF			0
#define PGPNET_LOGGEDON				1
#define PGPNET_NOLOGONREQUIRED		2
#define PGPNET_LOGONINPROCESS		3

// These are values passed from service to app as return value to 
// PGPNET_QUERYDRIVERSTATUS message (see below).
#define PGPNET_SERVICENOTRESPONDING	0
#define PGPNET_SERVICENOTAVAILABLE	1
#define PGPNET_DRIVERENABLED		2
#define PGPNET_DRIVERDISABLED		3
#define PGPNET_DRIVERNOTAVAILABLE	4

// This is the name that the Service should use for its window
// class and title.  
#define PGPNET_SERVICEWINDOWNAME	("PGPnet IKE service")
#define PGPNET_SERVICECOMMWINDOWNAME	("PGPnet GUI-IKE Communication")

// These are the names that the main GUI app should use for its window
// class and title.  
#define PGPNET_APPWINDOWCLASS		("PGPnet Application Window Class")
#define PGPNET_APPWINDOWTITLE		("PGPnet")

// the PGPnet service, and main and tray applications send this 
// message for any of a variety of reasons.  The WPARAM of
// this message indicates the type of the request.  The LPARAM 
// indicates the HWND of the requesting window.
#define PGPNET_M_APPMESSAGE					WM_APP+100

// these are the values that can be passed in the WPARAM of the
// PGPNET_M_APPMESSAGE message.

// request a one-time full status message
#define PGPNET_REQUESTCURRENTSTATUS			0x0000

// turn ON and OFF status reporting via WM_COPYDATA messages from service
#define PGPNET_ENABLESTATUSMESSAGES			0x0001	
#define PGPNET_DISABLESTATUSMESSAGES		0x0002	

// turn ON and OFF log update notifications via WM_COPYDATA messages
// from service
#define PGPNET_ENABLELOGMESSAGES			0x0003
#define PGPNET_DISABLELOGMESSAGES			0x0004

// The PGPnet main application sends this when the configuration has
// been updated on disk
#define PGPNET_CONFIGUPDATED				0x0005

// The PGPtray application sends these when the the user wants to 
// logon or logoff
#define PGPNET_LOGON						0x0007
#define PGPNET_LOGOFF						0x0008

// The PGPtray application sends this to request a the logon status
// of the service.  The service should respond with a WM_COPYDATA
// message.
#define PGPNET_QUERYLOGONSTATUS				0x0009

// The PGPnet main application sends this to request that the 
// service zeroes the log file.
#define PGPNET_CLEARLOGFILE					0x000A

// The PGPnet main or tray applications send this to request the current
// status of the driver.  The service should return ONE of 
// PGPNET_DRIVERENABLED, PGPNET_DRIVERDISABLED, or
// PGPNET_DRIVERNOTAVAILABLE
#define PGPNET_QUERYDRIVERSTATUS			0x000B

// The PGPnet service sends the apps these messages when it wants to
// disable user interaction with the apps (e.g. when it puts up the
// passphrase dialog).
#define PGPNET_DISABLEGUI					0x000C
#define PGPNET_ENABLEGUI					0x000D

// Used to get the service to attempt IKE negotiations with a particular
// machine, without needing the driver. Send the IP address of the machine
// as the LPARAM of the message
#define PGPNET_ATTEMPTIKE					0x000E

// Used by the service to notify the app that host has been added to the 
// end of the list
#define PGPNET_PROMISCUOUSHOSTADDED			0x000F

// The PGPnet main application sends this when the configuration and the
// keyrings have been updated
#define PGPNET_CONFIGANDKEYRINGSUPDATED		0x0010

// these are the values that can be passed by the Service in the dwData 
// parameter of the COPYDATASTRUCT of the WM_COPYDATA message.

// lpData points to PGPnetLogEvent structure that service wants app
// to display
#define PGPNET_LOGUPDATE					0x0001

// lpData points to an array of PGPikeSA structs representing current
// active list of SAs
#define PGPNET_STATUSFULL					0x0002

// lpData points to ???
#define PGPNET_STATUSUPDATE					0x0003

// lpData points to a PGPikeSA structure containing new SA to add to list
#define PGPNET_STATUSNEWSA					0x0004

// lpData points to a PGPipsecSPI structure containing SPI of SA to remove
// (NB: also sent from main app to service to request that SA be removed)
#define PGPNET_STATUSREMOVESA				0x0005

// The PGPnet service sends this when the user logs on or off, or when
// the GUI requests it (via PGPNET_QUERYLOGONSTATUS).  The data buffer
// should be a DWORD which contains *one* of PGPNET_LOGGEDOFF, 
// PGPNET_LOGGEDON, PGPNET_NOLOGONREQUIRED.
#define PGPNET_LOGONSTATUS					0x0006

#endif //_PGPNETIPC_H