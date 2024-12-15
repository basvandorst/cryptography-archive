# Microsoft Developer Studio Project File - Name="ipsec" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ipsec - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ipsec.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ipsec.mak" CFG="ipsec - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ipsec - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ipsec - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ipsec - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "." /I "..\minisdk" /I "..\ike" /I "..\..\..\shared\ipsec" /I "..\..\..\shared\minisdk\include" /I "..\..\..\shared\compress" /I "..\shared\ndis" /D "NDEBUG" /D PGP_DEBUG=0 /D "WIN32" /D "_WINDOWS" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_WIN32=1 /D USERLAND_TEST=1 /D FRAGMENT_TEST=1 /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "ipsec - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "." /I "..\minisdk" /I "..\ike" /I "..\..\..\shared\ipsec" /I "..\..\..\shared\minisdk\include" /I "..\..\..\shared\compress" /I "..\shared\ndis" /D "_DEBUG" /D PGP_DEBUG=1 /D "WIN32" /D "_WINDOWS" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_WIN32=1 /D USERLAND_TEST=1 /D FRAGMENT_TEST=1 /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "ipsec - Win32 Release"
# Name "ipsec - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=..\..\..\..\..\libs\pfl\common\util\pgpEndianConversion.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\ipsec\pgpIPheader.c
# End Source File
# Begin Source File

SOURCE=.\pgpIPsec.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\ipsec\pgpIPsecAH.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\ipsec\pgpIPsecBuffer.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\ipsec\pgpIPsecComp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\ipsec\pgpIPsecContext.c
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\ipsec\pgpIPsecESP.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=..\..\..\shared\ipsec\pgpIPheader.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\ipsec\pgpIPsecAH.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\ipsec\pgpIPsecBuffer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\ipsec\pgpIPsecComp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\ipsec\pgpIPsecContext.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\ipsec\pgpIPsecContextPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\ipsec\pgpIPsecErrors.h
# End Source File
# Begin Source File

SOURCE=..\..\..\shared\ipsec\pgpIPsecESP.h
# End Source File
# End Group
# End Target
# End Project
