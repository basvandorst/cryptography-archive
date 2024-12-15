/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPmlNT_Readme.txt - PGPmemlock readme file (WinNT version)
	

	$Id: PGPmlNT_Readme.txt,v 1.1.2.1 1999/06/03 14:31:53 pbj Exp $
____________________________________________________________________________*/

Instructions for the PGPmemlock memory locking driver - Windows NT version


Introduction :
--------------

- memory locking under Windows NT is accomplished using a kernel-mode
  device driver (PGPmemlock.sys).

- if you only want to use the driver (i.e. you do *not* want to build it), 
  skip to the installation instructions (below).
  
- see pgpMemLock.c for an explanation of how to call this driver.


Building :
----------

- due to an idiosyncracy in the WinNT DDK, you can only build this driver
  under Windows NT (i.e. *not* under Windows 95).

- the following tools are needed to build the PGPmemlock.sys :
    - Microsoft Visual C++ compiler (version 6.0, sp2)
	- Microsoft Windows NT 4.0 Device Driver Kit (DDK)

- the driver is built from the PGPmemlock.dsw workspace.  This workspace 
  relies on the NT4DDKROOT environment variable to find the install path of
  the Windows NT DDK.  Make sure that this environment variable is set before
  opening the workspace.
  
- the PGPmemlock workspace and PGPmemlockNT.dsp project will build the
  driver from within the MSVC++ IDE.  All you need to do is click on "Build".


Installing :
------------

- Copy the driver PGPmemlock.sys to the <nt_root>\system32\drivers\ 
  directory, i.e.:

  copy PGPmemlock.sys c:\winnt\system32\drivers\

- Update the registry with REGEDIT.  You should create a registry key called 

	 \HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\PGPmemlock

     in this location, create the following registry entries with the 
     following types and values :

     entry			type		value
     ErrorControl	REG_DWORD 	0x00000001
     Type			REG_DWORD 	0x00000001
     Start			REG_DWORD	0x00000002
     DisplayName	REG_SZ		"PGP secure memory driver"

- Reboot the computer.  This will cause the driver to load automatically.  

- Note:  If you do not want the driver to load automatically everytime 
  the computer boots, you can control the loading and unloading of the 
  driver manually.  To do this, run the "Devices" applet in the Control 
  Panel.  Select the PGPmemlock driver and press the "Startup" button.
  The valid choices for the PGPmemlock driver are "Automatic" (driver
  automatically runs on startup (default)) and "Manual".  If you select 
  "Manual", you must start and stop the driver manually with the "Start"
  and "Stop" buttons in the "Devices" applet.


Uninstalling :
--------------

- Run the "Devices" applet in the control panel.  Select the PGPmemlock
  driver.  Click on "Stop".

- Delete PGPmemlock.sys from the <nt_root>\system32\drivers\ directory.

- Delete the 
  \HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\PGPmemlock 
  key and all its entries.



