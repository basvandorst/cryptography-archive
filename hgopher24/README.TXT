		  README for HGOPHER  (Hampson's Gopher) 

	A Gopher Client For Windows 3.1  Version 2.4


Welcome

This is a Gopher+ Client for windows 3.1.

Send comments or problems on this product to m.hampson@ic.ac.uk

For information on winsock consult alt.winsock
For information on gopher  consult comp.infosystems.gopher
For information on PC NFS  consult com.protocols.nfs

HGopher now has online help. Please read the on line help section 
called  "read this first" at least.

If you are a 2.3 user read the section "What's new in Version 2.4" to
discover the bug fixes and enhencements.


Requirements
	
    WINSOCK VERSION

	* MS Windows 3.1  (not tested with Windows 3.0)
	  Or Windows NT.
	* Winsock DLL and TCP/IP+Packet support layer.
	* See later for a list of Winsock tested versions.

    PC NFS VERSION
	
	* MS Windows 3.1 (not tested with Windows 3.0)
	* PC NFS Version 4 or version 5 (may work with other versions)
          NOTE: you can also use the Winsock version with PC NFS 5.0
                and I suggest you do, as support for the native PC NFS
                version may be withdrawn at some point.


Installation

	Available for ANON FTP from lister.cc.ic.ac.uk
        
	Change to directory pub/wingopher and collect one of the files

	hgopher2.4.zip   [Winsock Version]      (Binary mode required)
	hngopher2.4.zip  [Sun PC NFS Version]   (Binary mode required)


        Unzip the package you have chosen to get 
        hgopher.exe / hngopher.exe		Gopher program
        hgopher.hlp				Online help
        hgcso.exe / hngcso.exe			CSO Viewer
        hgopher.ini				All the defaults
        default.gbm				Some book marks
        readme.txt				This file


        Install these files in a directory of you choice. You should
        make this the working directory when you run HGopher from
        program manager.

        If you have an existing 2.2/2.3 hgopher.ini in your windows directory
        HGopher 2.4 will use that. Else it will use the one in the working
        directory. Old Version 1.0 and 1.1 ini files will cause HGopher 2.4
        a major stress attack - get rid of them.

	You should also be able to get this product
        from the following archives

	sunsite.unc.edu:<somewhere>		(Probably a day or two delay)
        microdyne.com:/pub/winsock/apps	               ( or look in incoming)
        boombox.micro.umn.edu:/pub/gopher/Windows       (or via their Gopher)
        ftp.york.ac.uk:<somewhere>

        But make sure that you are getting the new 2.4 version (ie hgopher2.4.zip)

        You can also get HGopher from the "HGopher Information Centre", here's the
        bookmark

	#
        Type=1
        Host=gopher.ic.ac.uk
        Port=71
        Path=1/HGopher Information
        #
        # Note the port - 71 NOT 70.


Troubleshooting
    
    WINSOCK VERSION

	The first time you start it hgopher may not be able to 
	connect to a server or will probably connect to the wrong
	server
	Use the Options  menu to config it as follows

	Gopher Options
	
	Set the name and path of your favorite gopher server

	Select a directory for temp files that already exists

	Hit the save button to save these setting between sessions
	Hit the OK button to use these setting now

	use "go home" from the command menu to get the main menu 
	from your default gopher.

	Network setup
	
	Set whether you are using DNS or the Vendor look up
	service - If your vendor supports DNS lookup themselves
	I suggest you use their service as the inbuilt DNS has
	been shown not to work on some (most) Winsock versions. 

	If you are using the Winsock version with PC NFS you must
        use the inbuilt DNS as Sun have not only not bothered to
	implement DNS support but their own NIS does not function
	with their Winsock either (at least at the release level
	I have).
	
	If using the inbuilt DNS remember to use IP addresses and
	not host names.

        WINSOCK USERS PLEASE NOTE - The file WINSOCK.DLL is required by this
        program - if you don't have that file you are not running winsock.
        WINSOCK.DLL does not come with HGopher, it is part of the network
        package you are using. WINSOCK.DLL are NOT interchangable you must
        use the correct one for your network stack. Public domain/ Shareware
        WINSOCK stacks are beginning to appear - tune to alt.winsock to stay
        in touch with what is going on in this field.


    PC NFS VERSION

	Make sure at least RTM is loaded before you start Windows

	The first time you start it hngopher may not be able to 
	connect to a server or will probably connect to the wrong
	server
	Use the Options menu to config it as follows

	Gopher Options
	
	Set the name and path of your favorite gopher server

	Select a directory for temp files that already exists

	Hit the save button to save these setting between sessions
	Hit the OK button to use these setting now

	use "go home" from the command menu to get the main menu 
	from your default gopher.

	Network setup
	
	Set whether you are using DNS or the Vendor name lookup
	- with Sun this is NIS  - DNS is better.
	Add up to 3 local DNS servers and your local domain
	Hit OK. Remember to use IP addresses not hostnames.


Usage

        For further usage information please read the online manual.
        If you have any problems read the "read this first" section
        of the manual.

	
Winsock implementations tested against

  HGopher should work with any winsock complaint software. Previous versions
  are known to work with the following.


	Lanera		- Verified to work
	Distinct	- Verified to work
	Netmange	- Believed to work
	PC NFS 5.0      - Verified to work
	PCTCP 2.11	- Verified to work
	PCTCP 2.2	- Verified to work - However if you get a message
			  like PCTCPAPI052: get_kernel_info: Bad net_dope
                          structure 0x1ae01197, try getting a newer version
		          of their winsoc dll via ANON ftp
			  vax.ftp.com:/pub/winsockapi/FTPSoftware/winsock.dll.
	SuperTcp 3.56   - Verified to work
	Windows NT      - Verified to work under Beta version
        Wintrumpet Socket version 1.00 alpha #12  Verified to work
	Wollongong Pathway for DOS Runtime Release 1.2.0.1 with fix c42992a.exe

        Novell  Lan WorkPlace for DOS Ver 4.1 (Patch level N)
        ( Special thanks to Brian Meek of Novell for making this software
          available to me).

	Persoft's SMARTTERM 420 for Windows
        ( Special thanks to Scott Meeker of Persoft for making this software
          available to me).



----------------------------------------------------------------------------
	CopyRight notices 

	This version of gopher is not available in source code
	form, however future versions maybe.
----------------------------------------------------------------------------

 * Copyright (c) 1993  Martyn Hampson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Martyn Hampson
 * 4. The name Martyn Hampson
 *    may NOT be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Martyn Hampson  BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

Some sections of the gopher code are modified versions of existing
software. I include the copyright notices attached to that software below.

* The Resolver routines *

/*
 * Copyright (c) 1985, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

* Misc utility routines (from Xgopher) *

* Allan Tuchman, University of Illinois at Urbana-Champaign     
*                Computing and Communications Services Office   
* Copyright 1992 by                                             
*           the Board of Trustees of the University of Illinois 
* Permission is granted to freely copy and redistribute this    
* software with the copyright notice intact.                    





