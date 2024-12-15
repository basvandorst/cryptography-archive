/*____________________________________________________________________________
	Copyright (C) 1998 Network Associates, Inc.
	All rights reserved.
	
	PGPml95_Readme.txt - PGPmemlock readme file (Win95 version)
	

	$Id: PGPml95_Readme.txt,v 1.1.2.1 1999/06/03 14:31:53 pbj Exp $
____________________________________________________________________________*/

Instructions for the PGPmemlock memory locking driver - Windows 95/98 version


Introduction :
--------------

- memory locking under Windows 95/98 is accomplished using a dynamic
  virtual device driver (PGPmemlock.vxd).

- if you only want to use the driver (i.e. you do *not* want to build it), 
  skip to the installation instructions (below).
  
- see pgpMemLock.c for an explanation of how to call this driver.


Building :
----------

- the following tools are needed to build the PGPmemlock.vxd :
    - Microsoft Visual C++ compiler (version 6.0, sp2)
	- Microsoft Windows 98 Device Driver Kit (DDK)

- the driver is built from the PGPmemlock.dsw workspace.  This workspace 
  relies on the 98DDKROOT environment variable to find the install path of
  the Windows 98 DDK.  Make sure that this environment variable is set before
  opening the workspace.
  
- the PGPmemlock workspace and PGPmemlock9x.dsp project will build the
  driver from within the MSVC++ IDE.  The project includes a pre-link step
  to assemble pgpMLdevdcl.asm, and post-link steps to add the VERSIONINFO
  resource into the VxD.  This is all automated -- all you need to do is
  click on "Build".

- the linker will produce a few warnings about code sections.  This is normal.


Installing :
------------

- copy PGPmemlock.vxd to your \Windows directory.


Uninstalling :
--------------

- delete PGPmemlock.vxd from your <Windows> directory.	
