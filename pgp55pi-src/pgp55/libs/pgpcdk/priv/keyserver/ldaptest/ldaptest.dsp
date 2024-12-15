# Microsoft Developer Studio Project File - Name="ldaptest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=ldaptest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ldaptest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ldaptest.mak" CFG="ldaptest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ldaptest - Win32 Release" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "ldaptest - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ldaptest - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ../../../win32/pgpcdk/release/pgpsdk.lib ../../../win32/pgpcdkkeyserver/release/pgpcdkkeyserver.lib ../../../../network/ldap/libraries/liblber/release/liblber.lib ../../../../network/ldap/libraries/libldap/debug/libldap.lib ../../../../network/pgpsockets/win/src/release/pgpsockets.lib ..\..\..\..\..\clients\pgp\win32\pgpcmdlg\debug\pgp55cd.lib ..\..\..\..\..\clients\pgp\win32\PGPkm\release\PGP55km.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "ldaptest - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "../../../pub/include" /I "../../../../pfl/common" /I "../../../../pfl/common/prefs" /I "../../../../pfl/win32" /I "../../../win32" /I "../../../../../clients/pgp/win32/pgpphrase" /I "../../../../../clients/shared" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D UNFINISHED_CODE_ALLOWED=1 /D PGP_DEBUG=1 /D PGP_WIN32=1 /D DEBUG=1 /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ../../../win32/pgpcdk/debug/pgpsdk.lib ../../../win32/pgpcdkkeyserver/debug/pgpsdkkeyserver.lib ../../../../network/ldap/libraries/liblber/debug/liblber.lib ../../../../network/ldap/libraries/libldap/debug/libldap.lib ../../../../network/pgpsockets/win/src/debug/pgpsockets.lib ..\..\..\..\..\clients\pgp\win32\pgpcmdlg\debug\pgp55cd.lib ..\..\..\..\..\clients\pgp\win32\PGPkm\debug\PGP55km.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "ldaptest - Win32 Release"
# Name "ldaptest - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "c;cpp"
# Begin Source File

SOURCE=.\ldaptest.c

!IF  "$(CFG)" == "ldaptest - Win32 Release"

!ELSEIF  "$(CFG)" == "ldaptest - Win32 Debug"

# ADD CPP /I "../../include" /I "../../include/opaque" /I "../../utilities" /I ".../../../pub/include" /I "../../../pfl/common" /I "../../../pfl/win32" /I "../../../../network/pgpsockets/pub" /I "../../../../network/ldap/include"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\pfl\common\pgpDebug.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\pfl\common\pgpLeaks.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\pfl\common\pgpMem.c
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\clients\pgp\win32\PGPphrase\PGPphras.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp"
# Begin Source File

SOURCE=..\..\..\..\network\ldap\include\lber.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\network\ldap\include\ldap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\pfl\common\pgpBase.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pub\include\pgpCipherFeedback.h
# End Source File
# Begin Source File

SOURCE=..\..\..\win32\pgpConfig.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\pfl\common\pgpDebug.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pub\include\pgpEncode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pub\include\pgpErrors.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pub\include\pgpHash.h
# End Source File
# Begin Source File

SOURCE=..\..\include\opaque\pgpKeyDBOpaqueStructs.h
# End Source File
# Begin Source File

SOURCE=..\..\include\pgpKeyFilterPriv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pub\include\pgpKeys.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pub\include\pgpKeyServer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\pfl\common\pgpLeaks.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\pfl\common\pgpMem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\pfl\win32\pgpPFLConfig.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\pfl\common\pgpPFLErrors.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pub\include\pgpPubTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\network\pgpsockets\pub\PGPSockets.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pub\include\pgpSymmetricCipher.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\pfl\common\pgpTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\pub\include\pgpUtilities.h
# End Source File
# End Group
# End Target
# End Project
