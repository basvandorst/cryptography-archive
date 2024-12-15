# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=pgp - Win32 NO_RSA Debug
!MESSAGE No configuration specified.  Defaulting to pgp - Win32 NO_RSA Debug.
!ENDIF 

!IF "$(CFG)" != "pgp - Win32 Release" && "$(CFG)" != "pgp - Win32 Debug" &&\
 "$(CFG)" != "pgp - Win32 RSAREF Debug" && "$(CFG)" !=\
 "pgp - Win32 RSAREF Release" && "$(CFG)" != "pgp - Win32 NO_RSA Debug" &&\
 "$(CFG)" != "pgp - Win32 NO_RSA Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "pgp.mak" CFG="pgp - Win32 NO_RSA Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pgp - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pgp - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pgp - Win32 RSAREF Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pgp - Win32 RSAREF Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pgp - Win32 NO_RSA Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "pgp - Win32 NO_RSA Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "pgp - Win32 Debug"
RSC=rc.exe
MTL=mktyplib.exe
CPP=cl.exe

!IF  "$(CFG)" == "pgp - Win32 Release"

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
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\pgp.dll"

CLEAN : 
	-@erase "$(INTDIR)\pgpAddHdr.obj"
	-@erase "$(INTDIR)\pgpAltRSAGlu.obj"
	-@erase "$(INTDIR)\pgpAnnotate.obj"
	-@erase "$(INTDIR)\pgpArmor.obj"
	-@erase "$(INTDIR)\pgpArmrFil.obj"
	-@erase "$(INTDIR)\pgpBufMod.obj"
	-@erase "$(INTDIR)\pgpByteFIFO.obj"
	-@erase "$(INTDIR)\pgpCAST5.obj"
	-@erase "$(INTDIR)\pgpCFB.obj"
	-@erase "$(INTDIR)\pgpCharMap.obj"
	-@erase "$(INTDIR)\pgpCipher.obj"
	-@erase "$(INTDIR)\pgpCiphrMod.obj"
	-@erase "$(INTDIR)\pgpCompMod.obj"
	-@erase "$(INTDIR)\pgpConf.obj"
	-@erase "$(INTDIR)\pgpConvMod.obj"
	-@erase "$(INTDIR)\pgpCopyMod.obj"
	-@erase "$(INTDIR)\pgpCRC.obj"
	-@erase "$(INTDIR)\pgpDebug.obj"
	-@erase "$(INTDIR)\pgpDecPipe.obj"
	-@erase "$(INTDIR)\pgpDefMod.obj"
	-@erase "$(INTDIR)\pgpDES3.obj"
	-@erase "$(INTDIR)\pgpDevNull.obj"
	-@erase "$(INTDIR)\pgpDSAKey.obj"
	-@erase "$(INTDIR)\pgpElGKey.obj"
	-@erase "$(INTDIR)\pgpEncPipe.obj"
	-@erase "$(INTDIR)\pgpEnv.obj"
	-@erase "$(INTDIR)\pgpErr.obj"
	-@erase "$(INTDIR)\pgpESK.obj"
	-@erase "$(INTDIR)\pgpFIFO.obj"
	-@erase "$(INTDIR)\pgpFile.obj"
	-@erase "$(INTDIR)\pgpFileFIFO.obj"
	-@erase "$(INTDIR)\pgpFileMod.obj"
	-@erase "$(INTDIR)\pgpFileNames.obj"
	-@erase "$(INTDIR)\pgpFileRef.obj"
	-@erase "$(INTDIR)\pgpFileType.obj"
	-@erase "$(INTDIR)\pgpFixedKey.obj"
	-@erase "$(INTDIR)\pgpFlexFIFO.obj"
	-@erase "$(INTDIR)\pgpGlobals.obj"
	-@erase "$(INTDIR)\pgpHash.obj"
	-@erase "$(INTDIR)\pgpHashMod.obj"
	-@erase "$(INTDIR)\pgpHeader.obj"
	-@erase "$(INTDIR)\pgpIDEA.obj"
	-@erase "$(INTDIR)\pgpInfMod.obj"
	-@erase "$(INTDIR)\pgpJoin.obj"
	-@erase "$(INTDIR)\pgpKeyMisc.obj"
	-@erase "$(INTDIR)\pgpKeySpec.obj"
	-@erase "$(INTDIR)\pgpLeaks.obj"
	-@erase "$(INTDIR)\pgpLiteral.obj"
	-@erase "$(INTDIR)\pgpMakePKE.obj"
	-@erase "$(INTDIR)\pgpMakeSig.obj"
	-@erase "$(INTDIR)\pgpMD5.obj"
	-@erase "$(INTDIR)\pgpMem.obj"
	-@erase "$(INTDIR)\pgpMemFile.obj"
	-@erase "$(INTDIR)\pgpMemMod.obj"
	-@erase "$(INTDIR)\pgpMemPool.obj"
	-@erase "$(INTDIR)\pgpMsg.obj"
	-@erase "$(INTDIR)\pgpPassCach.obj"
	-@erase "$(INTDIR)\pgpPipeFile.obj"
	-@erase "$(INTDIR)\pgpPKEMod.obj"
	-@erase "$(INTDIR)\pgpPktList.obj"
	-@erase "$(INTDIR)\pgpPrsAsc.obj"
	-@erase "$(INTDIR)\pgpPrsBin.obj"
	-@erase "$(INTDIR)\pgpPubKey.obj"
	-@erase "$(INTDIR)\pgpRadix64.obj"
	-@erase "$(INTDIR)\pgpReadAnn.obj"
	-@erase "$(INTDIR)\pgpRIPEMD160.obj"
	-@erase "$(INTDIR)\pgpRndEvnt.obj"
	-@erase "$(INTDIR)\pgpRndom.obj"
	-@erase "$(INTDIR)\pgpRndPool.obj"
	-@erase "$(INTDIR)\pgpRndSeed.obj"
	-@erase "$(INTDIR)\pgpRndWin32.obj"
	-@erase "$(INTDIR)\pgpRngMnt.obj"
	-@erase "$(INTDIR)\pgpRngPars.obj"
	-@erase "$(INTDIR)\pgpRngPkt.obj"
	-@erase "$(INTDIR)\pgpRngPriv.obj"
	-@erase "$(INTDIR)\pgpRngPub.obj"
	-@erase "$(INTDIR)\pgpRngRead.obj"
	-@erase "$(INTDIR)\pgpRSAGlue1.obj"
	-@erase "$(INTDIR)\pgpRSAGlue2.obj"
	-@erase "$(INTDIR)\pgpRSAKey.obj"
	-@erase "$(INTDIR)\pgpRSAKeyGen.obj"
	-@erase "$(INTDIR)\pgpSHA.obj"
	-@erase "$(INTDIR)\pgpSig.obj"
	-@erase "$(INTDIR)\pgpSigMod.obj"
	-@erase "$(INTDIR)\pgpSigPipe.obj"
	-@erase "$(INTDIR)\pgpSigSpec.obj"
	-@erase "$(INTDIR)\pgpSplit.obj"
	-@erase "$(INTDIR)\pgpStr2Key.obj"
	-@erase "$(INTDIR)\pgpTextFilt.obj"
	-@erase "$(INTDIR)\pgpTimeDate.obj"
	-@erase "$(INTDIR)\pgpTrstPkt.obj"
	-@erase "$(INTDIR)\pgpTrust.obj"
	-@erase "$(INTDIR)\pgpVerifyRa.obj"
	-@erase "$(INTDIR)\pgpVrfySig.obj"
	-@erase "$(INTDIR)\pgpZBits.obj"
	-@erase "$(INTDIR)\pgpZDeflate.obj"
	-@erase "$(INTDIR)\pgpZInflate.obj"
	-@erase "$(INTDIR)\pgpZTrees.obj"
	-@erase "$(OUTDIR)\pgp.dll"
	-@erase "$(OUTDIR)\pgp.exp"
	-@erase "$(OUTDIR)\pgp.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../.." /I "../../include" /I "../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D UNFINISHED_CODE_ALLOWED=1 /D HAVE_CONFIG_H=1 /D "PGPLIB" /D PGPTRUSTMODEL=0 /D DEBUG=0 /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "../.." /I "../../include" /I "../include"\
 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D UNFINISHED_CODE_ALLOWED=1 /D\
 HAVE_CONFIG_H=1 /D "PGPLIB" /D PGPTRUSTMODEL=0 /D DEBUG=0\
 /Fp"$(INTDIR)/pgp.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/pgp.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\bn\release\bn.lib /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib ..\bn\release\bn.lib /nologo /subsystem:windows /dll\
 /incremental:no /pdb:"$(OUTDIR)/pgp.pdb" /machine:I386 /out:"$(OUTDIR)/pgp.dll"\
 /implib:"$(OUTDIR)/pgp.lib" 
LINK32_OBJS= \
	"$(INTDIR)\pgpAddHdr.obj" \
	"$(INTDIR)\pgpAltRSAGlu.obj" \
	"$(INTDIR)\pgpAnnotate.obj" \
	"$(INTDIR)\pgpArmor.obj" \
	"$(INTDIR)\pgpArmrFil.obj" \
	"$(INTDIR)\pgpBufMod.obj" \
	"$(INTDIR)\pgpByteFIFO.obj" \
	"$(INTDIR)\pgpCAST5.obj" \
	"$(INTDIR)\pgpCFB.obj" \
	"$(INTDIR)\pgpCharMap.obj" \
	"$(INTDIR)\pgpCipher.obj" \
	"$(INTDIR)\pgpCiphrMod.obj" \
	"$(INTDIR)\pgpCompMod.obj" \
	"$(INTDIR)\pgpConf.obj" \
	"$(INTDIR)\pgpConvMod.obj" \
	"$(INTDIR)\pgpCopyMod.obj" \
	"$(INTDIR)\pgpCRC.obj" \
	"$(INTDIR)\pgpDebug.obj" \
	"$(INTDIR)\pgpDecPipe.obj" \
	"$(INTDIR)\pgpDefMod.obj" \
	"$(INTDIR)\pgpDES3.obj" \
	"$(INTDIR)\pgpDevNull.obj" \
	"$(INTDIR)\pgpDSAKey.obj" \
	"$(INTDIR)\pgpElGKey.obj" \
	"$(INTDIR)\pgpEncPipe.obj" \
	"$(INTDIR)\pgpEnv.obj" \
	"$(INTDIR)\pgpErr.obj" \
	"$(INTDIR)\pgpESK.obj" \
	"$(INTDIR)\pgpFIFO.obj" \
	"$(INTDIR)\pgpFile.obj" \
	"$(INTDIR)\pgpFileFIFO.obj" \
	"$(INTDIR)\pgpFileMod.obj" \
	"$(INTDIR)\pgpFileNames.obj" \
	"$(INTDIR)\pgpFileRef.obj" \
	"$(INTDIR)\pgpFileType.obj" \
	"$(INTDIR)\pgpFixedKey.obj" \
	"$(INTDIR)\pgpFlexFIFO.obj" \
	"$(INTDIR)\pgpGlobals.obj" \
	"$(INTDIR)\pgpHash.obj" \
	"$(INTDIR)\pgpHashMod.obj" \
	"$(INTDIR)\pgpHeader.obj" \
	"$(INTDIR)\pgpIDEA.obj" \
	"$(INTDIR)\pgpInfMod.obj" \
	"$(INTDIR)\pgpJoin.obj" \
	"$(INTDIR)\pgpKeyMisc.obj" \
	"$(INTDIR)\pgpKeySpec.obj" \
	"$(INTDIR)\pgpLeaks.obj" \
	"$(INTDIR)\pgpLiteral.obj" \
	"$(INTDIR)\pgpMakePKE.obj" \
	"$(INTDIR)\pgpMakeSig.obj" \
	"$(INTDIR)\pgpMD5.obj" \
	"$(INTDIR)\pgpMem.obj" \
	"$(INTDIR)\pgpMemFile.obj" \
	"$(INTDIR)\pgpMemMod.obj" \
	"$(INTDIR)\pgpMemPool.obj" \
	"$(INTDIR)\pgpMsg.obj" \
	"$(INTDIR)\pgpPassCach.obj" \
	"$(INTDIR)\pgpPipeFile.obj" \
	"$(INTDIR)\pgpPKEMod.obj" \
	"$(INTDIR)\pgpPktList.obj" \
	"$(INTDIR)\pgpPrsAsc.obj" \
	"$(INTDIR)\pgpPrsBin.obj" \
	"$(INTDIR)\pgpPubKey.obj" \
	"$(INTDIR)\pgpRadix64.obj" \
	"$(INTDIR)\pgpReadAnn.obj" \
	"$(INTDIR)\pgpRIPEMD160.obj" \
	"$(INTDIR)\pgpRndEvnt.obj" \
	"$(INTDIR)\pgpRndom.obj" \
	"$(INTDIR)\pgpRndPool.obj" \
	"$(INTDIR)\pgpRndSeed.obj" \
	"$(INTDIR)\pgpRndWin32.obj" \
	"$(INTDIR)\pgpRngMnt.obj" \
	"$(INTDIR)\pgpRngPars.obj" \
	"$(INTDIR)\pgpRngPkt.obj" \
	"$(INTDIR)\pgpRngPriv.obj" \
	"$(INTDIR)\pgpRngPub.obj" \
	"$(INTDIR)\pgpRngRead.obj" \
	"$(INTDIR)\pgpRSAGlue1.obj" \
	"$(INTDIR)\pgpRSAGlue2.obj" \
	"$(INTDIR)\pgpRSAKey.obj" \
	"$(INTDIR)\pgpRSAKeyGen.obj" \
	"$(INTDIR)\pgpSHA.obj" \
	"$(INTDIR)\pgpSig.obj" \
	"$(INTDIR)\pgpSigMod.obj" \
	"$(INTDIR)\pgpSigPipe.obj" \
	"$(INTDIR)\pgpSigSpec.obj" \
	"$(INTDIR)\pgpSplit.obj" \
	"$(INTDIR)\pgpStr2Key.obj" \
	"$(INTDIR)\pgpTextFilt.obj" \
	"$(INTDIR)\pgpTimeDate.obj" \
	"$(INTDIR)\pgpTrstPkt.obj" \
	"$(INTDIR)\pgpTrust.obj" \
	"$(INTDIR)\pgpVerifyRa.obj" \
	"$(INTDIR)\pgpVrfySig.obj" \
	"$(INTDIR)\pgpZBits.obj" \
	"$(INTDIR)\pgpZDeflate.obj" \
	"$(INTDIR)\pgpZInflate.obj" \
	"$(INTDIR)\pgpZTrees.obj"

"$(OUTDIR)\pgp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"

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
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\pgp.dll"

CLEAN : 
	-@erase "$(INTDIR)\pgpAddHdr.obj"
	-@erase "$(INTDIR)\pgpAltRSAGlu.obj"
	-@erase "$(INTDIR)\pgpAnnotate.obj"
	-@erase "$(INTDIR)\pgpArmor.obj"
	-@erase "$(INTDIR)\pgpArmrFil.obj"
	-@erase "$(INTDIR)\pgpBufMod.obj"
	-@erase "$(INTDIR)\pgpByteFIFO.obj"
	-@erase "$(INTDIR)\pgpCAST5.obj"
	-@erase "$(INTDIR)\pgpCFB.obj"
	-@erase "$(INTDIR)\pgpCharMap.obj"
	-@erase "$(INTDIR)\pgpCipher.obj"
	-@erase "$(INTDIR)\pgpCiphrMod.obj"
	-@erase "$(INTDIR)\pgpCompMod.obj"
	-@erase "$(INTDIR)\pgpConf.obj"
	-@erase "$(INTDIR)\pgpConvMod.obj"
	-@erase "$(INTDIR)\pgpCopyMod.obj"
	-@erase "$(INTDIR)\pgpCRC.obj"
	-@erase "$(INTDIR)\pgpDebug.obj"
	-@erase "$(INTDIR)\pgpDecPipe.obj"
	-@erase "$(INTDIR)\pgpDefMod.obj"
	-@erase "$(INTDIR)\pgpDES3.obj"
	-@erase "$(INTDIR)\pgpDevNull.obj"
	-@erase "$(INTDIR)\pgpDSAKey.obj"
	-@erase "$(INTDIR)\pgpElGKey.obj"
	-@erase "$(INTDIR)\pgpEncPipe.obj"
	-@erase "$(INTDIR)\pgpEnv.obj"
	-@erase "$(INTDIR)\pgpErr.obj"
	-@erase "$(INTDIR)\pgpESK.obj"
	-@erase "$(INTDIR)\pgpFIFO.obj"
	-@erase "$(INTDIR)\pgpFile.obj"
	-@erase "$(INTDIR)\pgpFileFIFO.obj"
	-@erase "$(INTDIR)\pgpFileMod.obj"
	-@erase "$(INTDIR)\pgpFileNames.obj"
	-@erase "$(INTDIR)\pgpFileRef.obj"
	-@erase "$(INTDIR)\pgpFileType.obj"
	-@erase "$(INTDIR)\pgpFixedKey.obj"
	-@erase "$(INTDIR)\pgpFlexFIFO.obj"
	-@erase "$(INTDIR)\pgpGlobals.obj"
	-@erase "$(INTDIR)\pgpHash.obj"
	-@erase "$(INTDIR)\pgpHashMod.obj"
	-@erase "$(INTDIR)\pgpHeader.obj"
	-@erase "$(INTDIR)\pgpIDEA.obj"
	-@erase "$(INTDIR)\pgpInfMod.obj"
	-@erase "$(INTDIR)\pgpJoin.obj"
	-@erase "$(INTDIR)\pgpKeyMisc.obj"
	-@erase "$(INTDIR)\pgpKeySpec.obj"
	-@erase "$(INTDIR)\pgpLeaks.obj"
	-@erase "$(INTDIR)\pgpLiteral.obj"
	-@erase "$(INTDIR)\pgpMakePKE.obj"
	-@erase "$(INTDIR)\pgpMakeSig.obj"
	-@erase "$(INTDIR)\pgpMD5.obj"
	-@erase "$(INTDIR)\pgpMem.obj"
	-@erase "$(INTDIR)\pgpMemFile.obj"
	-@erase "$(INTDIR)\pgpMemMod.obj"
	-@erase "$(INTDIR)\pgpMemPool.obj"
	-@erase "$(INTDIR)\pgpMsg.obj"
	-@erase "$(INTDIR)\pgpPassCach.obj"
	-@erase "$(INTDIR)\pgpPipeFile.obj"
	-@erase "$(INTDIR)\pgpPKEMod.obj"
	-@erase "$(INTDIR)\pgpPktList.obj"
	-@erase "$(INTDIR)\pgpPrsAsc.obj"
	-@erase "$(INTDIR)\pgpPrsBin.obj"
	-@erase "$(INTDIR)\pgpPubKey.obj"
	-@erase "$(INTDIR)\pgpRadix64.obj"
	-@erase "$(INTDIR)\pgpReadAnn.obj"
	-@erase "$(INTDIR)\pgpRIPEMD160.obj"
	-@erase "$(INTDIR)\pgpRndEvnt.obj"
	-@erase "$(INTDIR)\pgpRndom.obj"
	-@erase "$(INTDIR)\pgpRndPool.obj"
	-@erase "$(INTDIR)\pgpRndSeed.obj"
	-@erase "$(INTDIR)\pgpRndWin32.obj"
	-@erase "$(INTDIR)\pgpRngMnt.obj"
	-@erase "$(INTDIR)\pgpRngPars.obj"
	-@erase "$(INTDIR)\pgpRngPkt.obj"
	-@erase "$(INTDIR)\pgpRngPriv.obj"
	-@erase "$(INTDIR)\pgpRngPub.obj"
	-@erase "$(INTDIR)\pgpRngRead.obj"
	-@erase "$(INTDIR)\pgpRSAGlue1.obj"
	-@erase "$(INTDIR)\pgpRSAGlue2.obj"
	-@erase "$(INTDIR)\pgpRSAKey.obj"
	-@erase "$(INTDIR)\pgpRSAKeyGen.obj"
	-@erase "$(INTDIR)\pgpSHA.obj"
	-@erase "$(INTDIR)\pgpSig.obj"
	-@erase "$(INTDIR)\pgpSigMod.obj"
	-@erase "$(INTDIR)\pgpSigPipe.obj"
	-@erase "$(INTDIR)\pgpSigSpec.obj"
	-@erase "$(INTDIR)\pgpSplit.obj"
	-@erase "$(INTDIR)\pgpStr2Key.obj"
	-@erase "$(INTDIR)\pgpTextFilt.obj"
	-@erase "$(INTDIR)\pgpTimeDate.obj"
	-@erase "$(INTDIR)\pgpTrstPkt.obj"
	-@erase "$(INTDIR)\pgpTrust.obj"
	-@erase "$(INTDIR)\pgpVerifyRa.obj"
	-@erase "$(INTDIR)\pgpVrfySig.obj"
	-@erase "$(INTDIR)\pgpZBits.obj"
	-@erase "$(INTDIR)\pgpZDeflate.obj"
	-@erase "$(INTDIR)\pgpZInflate.obj"
	-@erase "$(INTDIR)\pgpZTrees.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\pgp.dll"
	-@erase "$(OUTDIR)\pgp.exp"
	-@erase "$(OUTDIR)\pgp.ilk"
	-@erase "$(OUTDIR)\pgp.lib"
	-@erase "$(OUTDIR)\pgp.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "../.." /I "../../include" /I "../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D UNFINISHED_CODE_ALLOWED=1 /D HAVE_CONFIG_H=1 /D "PGPLIB" /D PGPTRUSTMODEL=0 /D DEBUG=0 /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "../.." /I "../../include" /I\
 "../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D UNFINISHED_CODE_ALLOWED=1\
 /D HAVE_CONFIG_H=1 /D "PGPLIB" /D PGPTRUSTMODEL=0 /D DEBUG=0\
 /Fp"$(INTDIR)/pgp.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/pgp.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\bn\debug\bn.lib /nologo /subsystem:windows /dll /debug /machine:I386
# SUBTRACT LINK32 /pdb:none /incremental:no /nodefaultlib
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib ..\bn\debug\bn.lib /nologo /subsystem:windows /dll\
 /incremental:yes /pdb:"$(OUTDIR)/pgp.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/pgp.dll" /implib:"$(OUTDIR)/pgp.lib" 
LINK32_OBJS= \
	"$(INTDIR)\pgpAddHdr.obj" \
	"$(INTDIR)\pgpAltRSAGlu.obj" \
	"$(INTDIR)\pgpAnnotate.obj" \
	"$(INTDIR)\pgpArmor.obj" \
	"$(INTDIR)\pgpArmrFil.obj" \
	"$(INTDIR)\pgpBufMod.obj" \
	"$(INTDIR)\pgpByteFIFO.obj" \
	"$(INTDIR)\pgpCAST5.obj" \
	"$(INTDIR)\pgpCFB.obj" \
	"$(INTDIR)\pgpCharMap.obj" \
	"$(INTDIR)\pgpCipher.obj" \
	"$(INTDIR)\pgpCiphrMod.obj" \
	"$(INTDIR)\pgpCompMod.obj" \
	"$(INTDIR)\pgpConf.obj" \
	"$(INTDIR)\pgpConvMod.obj" \
	"$(INTDIR)\pgpCopyMod.obj" \
	"$(INTDIR)\pgpCRC.obj" \
	"$(INTDIR)\pgpDebug.obj" \
	"$(INTDIR)\pgpDecPipe.obj" \
	"$(INTDIR)\pgpDefMod.obj" \
	"$(INTDIR)\pgpDES3.obj" \
	"$(INTDIR)\pgpDevNull.obj" \
	"$(INTDIR)\pgpDSAKey.obj" \
	"$(INTDIR)\pgpElGKey.obj" \
	"$(INTDIR)\pgpEncPipe.obj" \
	"$(INTDIR)\pgpEnv.obj" \
	"$(INTDIR)\pgpErr.obj" \
	"$(INTDIR)\pgpESK.obj" \
	"$(INTDIR)\pgpFIFO.obj" \
	"$(INTDIR)\pgpFile.obj" \
	"$(INTDIR)\pgpFileFIFO.obj" \
	"$(INTDIR)\pgpFileMod.obj" \
	"$(INTDIR)\pgpFileNames.obj" \
	"$(INTDIR)\pgpFileRef.obj" \
	"$(INTDIR)\pgpFileType.obj" \
	"$(INTDIR)\pgpFixedKey.obj" \
	"$(INTDIR)\pgpFlexFIFO.obj" \
	"$(INTDIR)\pgpGlobals.obj" \
	"$(INTDIR)\pgpHash.obj" \
	"$(INTDIR)\pgpHashMod.obj" \
	"$(INTDIR)\pgpHeader.obj" \
	"$(INTDIR)\pgpIDEA.obj" \
	"$(INTDIR)\pgpInfMod.obj" \
	"$(INTDIR)\pgpJoin.obj" \
	"$(INTDIR)\pgpKeyMisc.obj" \
	"$(INTDIR)\pgpKeySpec.obj" \
	"$(INTDIR)\pgpLeaks.obj" \
	"$(INTDIR)\pgpLiteral.obj" \
	"$(INTDIR)\pgpMakePKE.obj" \
	"$(INTDIR)\pgpMakeSig.obj" \
	"$(INTDIR)\pgpMD5.obj" \
	"$(INTDIR)\pgpMem.obj" \
	"$(INTDIR)\pgpMemFile.obj" \
	"$(INTDIR)\pgpMemMod.obj" \
	"$(INTDIR)\pgpMemPool.obj" \
	"$(INTDIR)\pgpMsg.obj" \
	"$(INTDIR)\pgpPassCach.obj" \
	"$(INTDIR)\pgpPipeFile.obj" \
	"$(INTDIR)\pgpPKEMod.obj" \
	"$(INTDIR)\pgpPktList.obj" \
	"$(INTDIR)\pgpPrsAsc.obj" \
	"$(INTDIR)\pgpPrsBin.obj" \
	"$(INTDIR)\pgpPubKey.obj" \
	"$(INTDIR)\pgpRadix64.obj" \
	"$(INTDIR)\pgpReadAnn.obj" \
	"$(INTDIR)\pgpRIPEMD160.obj" \
	"$(INTDIR)\pgpRndEvnt.obj" \
	"$(INTDIR)\pgpRndom.obj" \
	"$(INTDIR)\pgpRndPool.obj" \
	"$(INTDIR)\pgpRndSeed.obj" \
	"$(INTDIR)\pgpRndWin32.obj" \
	"$(INTDIR)\pgpRngMnt.obj" \
	"$(INTDIR)\pgpRngPars.obj" \
	"$(INTDIR)\pgpRngPkt.obj" \
	"$(INTDIR)\pgpRngPriv.obj" \
	"$(INTDIR)\pgpRngPub.obj" \
	"$(INTDIR)\pgpRngRead.obj" \
	"$(INTDIR)\pgpRSAGlue1.obj" \
	"$(INTDIR)\pgpRSAGlue2.obj" \
	"$(INTDIR)\pgpRSAKey.obj" \
	"$(INTDIR)\pgpRSAKeyGen.obj" \
	"$(INTDIR)\pgpSHA.obj" \
	"$(INTDIR)\pgpSig.obj" \
	"$(INTDIR)\pgpSigMod.obj" \
	"$(INTDIR)\pgpSigPipe.obj" \
	"$(INTDIR)\pgpSigSpec.obj" \
	"$(INTDIR)\pgpSplit.obj" \
	"$(INTDIR)\pgpStr2Key.obj" \
	"$(INTDIR)\pgpTextFilt.obj" \
	"$(INTDIR)\pgpTimeDate.obj" \
	"$(INTDIR)\pgpTrstPkt.obj" \
	"$(INTDIR)\pgpTrust.obj" \
	"$(INTDIR)\pgpVerifyRa.obj" \
	"$(INTDIR)\pgpVrfySig.obj" \
	"$(INTDIR)\pgpZBits.obj" \
	"$(INTDIR)\pgpZDeflate.obj" \
	"$(INTDIR)\pgpZInflate.obj" \
	"$(INTDIR)\pgpZTrees.obj"

"$(OUTDIR)\pgp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "pgp___Wi"
# PROP BASE Intermediate_Dir "pgp___Wi"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "RSAREF_Debug"
# PROP Intermediate_Dir "RSAREF_Debug"
# PROP Target_Dir ""
OUTDIR=.\RSAREF_Debug
INTDIR=.\RSAREF_Debug

ALL : "$(OUTDIR)\pgp.dll"

CLEAN : 
	-@erase "$(INTDIR)\pgpAddHdr.obj"
	-@erase "$(INTDIR)\pgpAltRSAGlu.obj"
	-@erase "$(INTDIR)\pgpAnnotate.obj"
	-@erase "$(INTDIR)\pgpArmor.obj"
	-@erase "$(INTDIR)\pgpArmrFil.obj"
	-@erase "$(INTDIR)\pgpBufMod.obj"
	-@erase "$(INTDIR)\pgpByteFIFO.obj"
	-@erase "$(INTDIR)\pgpCAST5.obj"
	-@erase "$(INTDIR)\pgpCFB.obj"
	-@erase "$(INTDIR)\pgpCharMap.obj"
	-@erase "$(INTDIR)\pgpCipher.obj"
	-@erase "$(INTDIR)\pgpCiphrMod.obj"
	-@erase "$(INTDIR)\pgpCompMod.obj"
	-@erase "$(INTDIR)\pgpConf.obj"
	-@erase "$(INTDIR)\pgpConvMod.obj"
	-@erase "$(INTDIR)\pgpCopyMod.obj"
	-@erase "$(INTDIR)\pgpCRC.obj"
	-@erase "$(INTDIR)\pgpDebug.obj"
	-@erase "$(INTDIR)\pgpDecPipe.obj"
	-@erase "$(INTDIR)\pgpDefMod.obj"
	-@erase "$(INTDIR)\pgpDES3.obj"
	-@erase "$(INTDIR)\pgpDevNull.obj"
	-@erase "$(INTDIR)\pgpDSAKey.obj"
	-@erase "$(INTDIR)\pgpElGKey.obj"
	-@erase "$(INTDIR)\pgpEncPipe.obj"
	-@erase "$(INTDIR)\pgpEnv.obj"
	-@erase "$(INTDIR)\pgpErr.obj"
	-@erase "$(INTDIR)\pgpESK.obj"
	-@erase "$(INTDIR)\pgpFIFO.obj"
	-@erase "$(INTDIR)\pgpFile.obj"
	-@erase "$(INTDIR)\pgpFileFIFO.obj"
	-@erase "$(INTDIR)\pgpFileMod.obj"
	-@erase "$(INTDIR)\pgpFileNames.obj"
	-@erase "$(INTDIR)\pgpFileRef.obj"
	-@erase "$(INTDIR)\pgpFileType.obj"
	-@erase "$(INTDIR)\pgpFixedKey.obj"
	-@erase "$(INTDIR)\pgpFlexFIFO.obj"
	-@erase "$(INTDIR)\pgpGlobals.obj"
	-@erase "$(INTDIR)\pgpHash.obj"
	-@erase "$(INTDIR)\pgpHashMod.obj"
	-@erase "$(INTDIR)\pgpHeader.obj"
	-@erase "$(INTDIR)\pgpIDEA.obj"
	-@erase "$(INTDIR)\pgpInfMod.obj"
	-@erase "$(INTDIR)\pgpJoin.obj"
	-@erase "$(INTDIR)\pgpKeyMisc.obj"
	-@erase "$(INTDIR)\pgpKeySpec.obj"
	-@erase "$(INTDIR)\pgpLeaks.obj"
	-@erase "$(INTDIR)\pgpLiteral.obj"
	-@erase "$(INTDIR)\pgpMakePKE.obj"
	-@erase "$(INTDIR)\pgpMakeSig.obj"
	-@erase "$(INTDIR)\pgpMD5.obj"
	-@erase "$(INTDIR)\pgpMem.obj"
	-@erase "$(INTDIR)\pgpMemFile.obj"
	-@erase "$(INTDIR)\pgpMemMod.obj"
	-@erase "$(INTDIR)\pgpMemPool.obj"
	-@erase "$(INTDIR)\pgpMsg.obj"
	-@erase "$(INTDIR)\pgpPassCach.obj"
	-@erase "$(INTDIR)\pgpPipeFile.obj"
	-@erase "$(INTDIR)\pgpPKEMod.obj"
	-@erase "$(INTDIR)\pgpPktList.obj"
	-@erase "$(INTDIR)\pgpPrsAsc.obj"
	-@erase "$(INTDIR)\pgpPrsBin.obj"
	-@erase "$(INTDIR)\pgpPubKey.obj"
	-@erase "$(INTDIR)\pgpRadix64.obj"
	-@erase "$(INTDIR)\pgpReadAnn.obj"
	-@erase "$(INTDIR)\pgpRIPEMD160.obj"
	-@erase "$(INTDIR)\pgpRndEvnt.obj"
	-@erase "$(INTDIR)\pgpRndom.obj"
	-@erase "$(INTDIR)\pgpRndPool.obj"
	-@erase "$(INTDIR)\pgpRndSeed.obj"
	-@erase "$(INTDIR)\pgpRndWin32.obj"
	-@erase "$(INTDIR)\pgpRngMnt.obj"
	-@erase "$(INTDIR)\pgpRngPars.obj"
	-@erase "$(INTDIR)\pgpRngPkt.obj"
	-@erase "$(INTDIR)\pgpRngPriv.obj"
	-@erase "$(INTDIR)\pgpRngPub.obj"
	-@erase "$(INTDIR)\pgpRngRead.obj"
	-@erase "$(INTDIR)\pgpRSAGlue1.obj"
	-@erase "$(INTDIR)\pgpRSAGlue2.obj"
	-@erase "$(INTDIR)\pgpRSAKey.obj"
	-@erase "$(INTDIR)\pgpRSAKeyGen.obj"
	-@erase "$(INTDIR)\pgpSHA.obj"
	-@erase "$(INTDIR)\pgpSig.obj"
	-@erase "$(INTDIR)\pgpSigMod.obj"
	-@erase "$(INTDIR)\pgpSigPipe.obj"
	-@erase "$(INTDIR)\pgpSigSpec.obj"
	-@erase "$(INTDIR)\pgpSplit.obj"
	-@erase "$(INTDIR)\pgpStr2Key.obj"
	-@erase "$(INTDIR)\pgpTextFilt.obj"
	-@erase "$(INTDIR)\pgpTimeDate.obj"
	-@erase "$(INTDIR)\pgpTrstPkt.obj"
	-@erase "$(INTDIR)\pgpTrust.obj"
	-@erase "$(INTDIR)\pgpVerifyRa.obj"
	-@erase "$(INTDIR)\pgpVrfySig.obj"
	-@erase "$(INTDIR)\pgpZBits.obj"
	-@erase "$(INTDIR)\pgpZDeflate.obj"
	-@erase "$(INTDIR)\pgpZInflate.obj"
	-@erase "$(INTDIR)\pgpZTrees.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\pgp.dll"
	-@erase "$(OUTDIR)\pgp.exp"
	-@erase "$(OUTDIR)\pgp.ilk"
	-@erase "$(OUTDIR)\pgp.lib"
	-@erase "$(OUTDIR)\pgp.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "../.." /I "../../include" /I "../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D UNFINISHED_CODE_ALLOWED=1 /D HAVE_CONFIG_H=1 /D "PGPLIB" /D PGPTRUSTMODEL=0 /D DEBUG=0 /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "../.." /I "../../include" /I "../include" /I "../../../rsaref/source" /I "../../../rsaref/install" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D UNFINISHED_CODE_ALLOWED=1 /D HAVE_CONFIG_H=1 /D "PGPLIB" /D PGPTRUSTMODEL=0 /D DEBUG=0 /D "USERSAREF" /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "../.." /I "../../include" /I\
 "../include" /I "../../../rsaref/source" /I "../../../rsaref/install" /D\
 "_DEBUG" /D "WIN32" /D "_WINDOWS" /D UNFINISHED_CODE_ALLOWED=1 /D\
 HAVE_CONFIG_H=1 /D "PGPLIB" /D PGPTRUSTMODEL=0 /D DEBUG=0 /D "USERSAREF"\
 /Fp"$(INTDIR)/pgp.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\RSAREF_Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/pgp.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 ../../../rsaref/debug/rsaref.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# SUBTRACT BASE LINK32 /pdb:none /incremental:no /nodefaultlib
# ADD LINK32 ../../../rsaref/debug/rsaref.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\bn\debug\bn.lib /nologo /subsystem:windows /dll /debug /machine:I386
# SUBTRACT LINK32 /pdb:none /incremental:no /nodefaultlib
LINK32_FLAGS=../../../rsaref/debug/rsaref.lib kernel32.lib user32.lib gdi32.lib\
 winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib\
 uuid.lib odbc32.lib odbccp32.lib ..\bn\debug\bn.lib /nologo /subsystem:windows\
 /dll /incremental:yes /pdb:"$(OUTDIR)/pgp.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/pgp.dll" /implib:"$(OUTDIR)/pgp.lib" 
LINK32_OBJS= \
	"$(INTDIR)\pgpAddHdr.obj" \
	"$(INTDIR)\pgpAltRSAGlu.obj" \
	"$(INTDIR)\pgpAnnotate.obj" \
	"$(INTDIR)\pgpArmor.obj" \
	"$(INTDIR)\pgpArmrFil.obj" \
	"$(INTDIR)\pgpBufMod.obj" \
	"$(INTDIR)\pgpByteFIFO.obj" \
	"$(INTDIR)\pgpCAST5.obj" \
	"$(INTDIR)\pgpCFB.obj" \
	"$(INTDIR)\pgpCharMap.obj" \
	"$(INTDIR)\pgpCipher.obj" \
	"$(INTDIR)\pgpCiphrMod.obj" \
	"$(INTDIR)\pgpCompMod.obj" \
	"$(INTDIR)\pgpConf.obj" \
	"$(INTDIR)\pgpConvMod.obj" \
	"$(INTDIR)\pgpCopyMod.obj" \
	"$(INTDIR)\pgpCRC.obj" \
	"$(INTDIR)\pgpDebug.obj" \
	"$(INTDIR)\pgpDecPipe.obj" \
	"$(INTDIR)\pgpDefMod.obj" \
	"$(INTDIR)\pgpDES3.obj" \
	"$(INTDIR)\pgpDevNull.obj" \
	"$(INTDIR)\pgpDSAKey.obj" \
	"$(INTDIR)\pgpElGKey.obj" \
	"$(INTDIR)\pgpEncPipe.obj" \
	"$(INTDIR)\pgpEnv.obj" \
	"$(INTDIR)\pgpErr.obj" \
	"$(INTDIR)\pgpESK.obj" \
	"$(INTDIR)\pgpFIFO.obj" \
	"$(INTDIR)\pgpFile.obj" \
	"$(INTDIR)\pgpFileFIFO.obj" \
	"$(INTDIR)\pgpFileMod.obj" \
	"$(INTDIR)\pgpFileNames.obj" \
	"$(INTDIR)\pgpFileRef.obj" \
	"$(INTDIR)\pgpFileType.obj" \
	"$(INTDIR)\pgpFixedKey.obj" \
	"$(INTDIR)\pgpFlexFIFO.obj" \
	"$(INTDIR)\pgpGlobals.obj" \
	"$(INTDIR)\pgpHash.obj" \
	"$(INTDIR)\pgpHashMod.obj" \
	"$(INTDIR)\pgpHeader.obj" \
	"$(INTDIR)\pgpIDEA.obj" \
	"$(INTDIR)\pgpInfMod.obj" \
	"$(INTDIR)\pgpJoin.obj" \
	"$(INTDIR)\pgpKeyMisc.obj" \
	"$(INTDIR)\pgpKeySpec.obj" \
	"$(INTDIR)\pgpLeaks.obj" \
	"$(INTDIR)\pgpLiteral.obj" \
	"$(INTDIR)\pgpMakePKE.obj" \
	"$(INTDIR)\pgpMakeSig.obj" \
	"$(INTDIR)\pgpMD5.obj" \
	"$(INTDIR)\pgpMem.obj" \
	"$(INTDIR)\pgpMemFile.obj" \
	"$(INTDIR)\pgpMemMod.obj" \
	"$(INTDIR)\pgpMemPool.obj" \
	"$(INTDIR)\pgpMsg.obj" \
	"$(INTDIR)\pgpPassCach.obj" \
	"$(INTDIR)\pgpPipeFile.obj" \
	"$(INTDIR)\pgpPKEMod.obj" \
	"$(INTDIR)\pgpPktList.obj" \
	"$(INTDIR)\pgpPrsAsc.obj" \
	"$(INTDIR)\pgpPrsBin.obj" \
	"$(INTDIR)\pgpPubKey.obj" \
	"$(INTDIR)\pgpRadix64.obj" \
	"$(INTDIR)\pgpReadAnn.obj" \
	"$(INTDIR)\pgpRIPEMD160.obj" \
	"$(INTDIR)\pgpRndEvnt.obj" \
	"$(INTDIR)\pgpRndom.obj" \
	"$(INTDIR)\pgpRndPool.obj" \
	"$(INTDIR)\pgpRndSeed.obj" \
	"$(INTDIR)\pgpRndWin32.obj" \
	"$(INTDIR)\pgpRngMnt.obj" \
	"$(INTDIR)\pgpRngPars.obj" \
	"$(INTDIR)\pgpRngPkt.obj" \
	"$(INTDIR)\pgpRngPriv.obj" \
	"$(INTDIR)\pgpRngPub.obj" \
	"$(INTDIR)\pgpRngRead.obj" \
	"$(INTDIR)\pgpRSAGlue1.obj" \
	"$(INTDIR)\pgpRSAGlue2.obj" \
	"$(INTDIR)\pgpRSAKey.obj" \
	"$(INTDIR)\pgpRSAKeyGen.obj" \
	"$(INTDIR)\pgpSHA.obj" \
	"$(INTDIR)\pgpSig.obj" \
	"$(INTDIR)\pgpSigMod.obj" \
	"$(INTDIR)\pgpSigPipe.obj" \
	"$(INTDIR)\pgpSigSpec.obj" \
	"$(INTDIR)\pgpSplit.obj" \
	"$(INTDIR)\pgpStr2Key.obj" \
	"$(INTDIR)\pgpTextFilt.obj" \
	"$(INTDIR)\pgpTimeDate.obj" \
	"$(INTDIR)\pgpTrstPkt.obj" \
	"$(INTDIR)\pgpTrust.obj" \
	"$(INTDIR)\pgpVerifyRa.obj" \
	"$(INTDIR)\pgpVrfySig.obj" \
	"$(INTDIR)\pgpZBits.obj" \
	"$(INTDIR)\pgpZDeflate.obj" \
	"$(INTDIR)\pgpZInflate.obj" \
	"$(INTDIR)\pgpZTrees.obj"

"$(OUTDIR)\pgp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "pgp___W0"
# PROP BASE Intermediate_Dir "pgp___W0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "RSAREF_Release"
# PROP Intermediate_Dir "RSAREF_Release"
# PROP Target_Dir ""
OUTDIR=.\RSAREF_Release
INTDIR=.\RSAREF_Release

ALL : "$(OUTDIR)\pgp.dll"

CLEAN : 
	-@erase "$(INTDIR)\pgpAddHdr.obj"
	-@erase "$(INTDIR)\pgpAltRSAGlu.obj"
	-@erase "$(INTDIR)\pgpAnnotate.obj"
	-@erase "$(INTDIR)\pgpArmor.obj"
	-@erase "$(INTDIR)\pgpArmrFil.obj"
	-@erase "$(INTDIR)\pgpBufMod.obj"
	-@erase "$(INTDIR)\pgpByteFIFO.obj"
	-@erase "$(INTDIR)\pgpCAST5.obj"
	-@erase "$(INTDIR)\pgpCFB.obj"
	-@erase "$(INTDIR)\pgpCharMap.obj"
	-@erase "$(INTDIR)\pgpCipher.obj"
	-@erase "$(INTDIR)\pgpCiphrMod.obj"
	-@erase "$(INTDIR)\pgpCompMod.obj"
	-@erase "$(INTDIR)\pgpConf.obj"
	-@erase "$(INTDIR)\pgpConvMod.obj"
	-@erase "$(INTDIR)\pgpCopyMod.obj"
	-@erase "$(INTDIR)\pgpCRC.obj"
	-@erase "$(INTDIR)\pgpDebug.obj"
	-@erase "$(INTDIR)\pgpDecPipe.obj"
	-@erase "$(INTDIR)\pgpDefMod.obj"
	-@erase "$(INTDIR)\pgpDES3.obj"
	-@erase "$(INTDIR)\pgpDevNull.obj"
	-@erase "$(INTDIR)\pgpDSAKey.obj"
	-@erase "$(INTDIR)\pgpElGKey.obj"
	-@erase "$(INTDIR)\pgpEncPipe.obj"
	-@erase "$(INTDIR)\pgpEnv.obj"
	-@erase "$(INTDIR)\pgpErr.obj"
	-@erase "$(INTDIR)\pgpESK.obj"
	-@erase "$(INTDIR)\pgpFIFO.obj"
	-@erase "$(INTDIR)\pgpFile.obj"
	-@erase "$(INTDIR)\pgpFileFIFO.obj"
	-@erase "$(INTDIR)\pgpFileMod.obj"
	-@erase "$(INTDIR)\pgpFileNames.obj"
	-@erase "$(INTDIR)\pgpFileRef.obj"
	-@erase "$(INTDIR)\pgpFileType.obj"
	-@erase "$(INTDIR)\pgpFixedKey.obj"
	-@erase "$(INTDIR)\pgpFlexFIFO.obj"
	-@erase "$(INTDIR)\pgpGlobals.obj"
	-@erase "$(INTDIR)\pgpHash.obj"
	-@erase "$(INTDIR)\pgpHashMod.obj"
	-@erase "$(INTDIR)\pgpHeader.obj"
	-@erase "$(INTDIR)\pgpIDEA.obj"
	-@erase "$(INTDIR)\pgpInfMod.obj"
	-@erase "$(INTDIR)\pgpJoin.obj"
	-@erase "$(INTDIR)\pgpKeyMisc.obj"
	-@erase "$(INTDIR)\pgpKeySpec.obj"
	-@erase "$(INTDIR)\pgpLeaks.obj"
	-@erase "$(INTDIR)\pgpLiteral.obj"
	-@erase "$(INTDIR)\pgpMakePKE.obj"
	-@erase "$(INTDIR)\pgpMakeSig.obj"
	-@erase "$(INTDIR)\pgpMD5.obj"
	-@erase "$(INTDIR)\pgpMem.obj"
	-@erase "$(INTDIR)\pgpMemFile.obj"
	-@erase "$(INTDIR)\pgpMemMod.obj"
	-@erase "$(INTDIR)\pgpMemPool.obj"
	-@erase "$(INTDIR)\pgpMsg.obj"
	-@erase "$(INTDIR)\pgpPassCach.obj"
	-@erase "$(INTDIR)\pgpPipeFile.obj"
	-@erase "$(INTDIR)\pgpPKEMod.obj"
	-@erase "$(INTDIR)\pgpPktList.obj"
	-@erase "$(INTDIR)\pgpPrsAsc.obj"
	-@erase "$(INTDIR)\pgpPrsBin.obj"
	-@erase "$(INTDIR)\pgpPubKey.obj"
	-@erase "$(INTDIR)\pgpRadix64.obj"
	-@erase "$(INTDIR)\pgpReadAnn.obj"
	-@erase "$(INTDIR)\pgpRIPEMD160.obj"
	-@erase "$(INTDIR)\pgpRndEvnt.obj"
	-@erase "$(INTDIR)\pgpRndom.obj"
	-@erase "$(INTDIR)\pgpRndPool.obj"
	-@erase "$(INTDIR)\pgpRndSeed.obj"
	-@erase "$(INTDIR)\pgpRndWin32.obj"
	-@erase "$(INTDIR)\pgpRngMnt.obj"
	-@erase "$(INTDIR)\pgpRngPars.obj"
	-@erase "$(INTDIR)\pgpRngPkt.obj"
	-@erase "$(INTDIR)\pgpRngPriv.obj"
	-@erase "$(INTDIR)\pgpRngPub.obj"
	-@erase "$(INTDIR)\pgpRngRead.obj"
	-@erase "$(INTDIR)\pgpRSAGlue1.obj"
	-@erase "$(INTDIR)\pgpRSAGlue2.obj"
	-@erase "$(INTDIR)\pgpRSAKey.obj"
	-@erase "$(INTDIR)\pgpRSAKeyGen.obj"
	-@erase "$(INTDIR)\pgpSHA.obj"
	-@erase "$(INTDIR)\pgpSig.obj"
	-@erase "$(INTDIR)\pgpSigMod.obj"
	-@erase "$(INTDIR)\pgpSigPipe.obj"
	-@erase "$(INTDIR)\pgpSigSpec.obj"
	-@erase "$(INTDIR)\pgpSplit.obj"
	-@erase "$(INTDIR)\pgpStr2Key.obj"
	-@erase "$(INTDIR)\pgpTextFilt.obj"
	-@erase "$(INTDIR)\pgpTimeDate.obj"
	-@erase "$(INTDIR)\pgpTrstPkt.obj"
	-@erase "$(INTDIR)\pgpTrust.obj"
	-@erase "$(INTDIR)\pgpVerifyRa.obj"
	-@erase "$(INTDIR)\pgpVrfySig.obj"
	-@erase "$(INTDIR)\pgpZBits.obj"
	-@erase "$(INTDIR)\pgpZDeflate.obj"
	-@erase "$(INTDIR)\pgpZInflate.obj"
	-@erase "$(INTDIR)\pgpZTrees.obj"
	-@erase "$(OUTDIR)\pgp.dll"
	-@erase "$(OUTDIR)\pgp.exp"
	-@erase "$(OUTDIR)\pgp.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "../.." /I "../../include" /I "../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D UNFINISHED_CODE_ALLOWED=1 /D HAVE_CONFIG_H=1 /D "PGPLIB" /D PGPTRUSTMODEL=0 /D DEBUG=0 /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../.." /I "../../include" /I "../include" /I "../../../rsaref/source" /I "../../../rsaref/install" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D UNFINISHED_CODE_ALLOWED=1 /D HAVE_CONFIG_H=1 /D "PGPLIB" /D PGPTRUSTMODEL=0 /D DEBUG=0 /D "USERSAREF" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "../.." /I "../../include" /I "../include"\
 /I "../../../rsaref/source" /I "../../../rsaref/install" /D "NDEBUG" /D "WIN32"\
 /D "_WINDOWS" /D UNFINISHED_CODE_ALLOWED=1 /D HAVE_CONFIG_H=1 /D "PGPLIB" /D\
 PGPTRUSTMODEL=0 /D DEBUG=0 /D "USERSAREF" /Fp"$(INTDIR)/pgp.pch" /YX\
 /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\RSAREF_Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/pgp.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 ../../../rsaref/release/rsaref.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\bn\release\bn.lib /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=../../../rsaref/release/rsaref.lib kernel32.lib user32.lib\
 gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib\
 oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\bn\release\bn.lib /nologo\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)/pgp.pdb" /machine:I386\
 /out:"$(OUTDIR)/pgp.dll" /implib:"$(OUTDIR)/pgp.lib" 
LINK32_OBJS= \
	"$(INTDIR)\pgpAddHdr.obj" \
	"$(INTDIR)\pgpAltRSAGlu.obj" \
	"$(INTDIR)\pgpAnnotate.obj" \
	"$(INTDIR)\pgpArmor.obj" \
	"$(INTDIR)\pgpArmrFil.obj" \
	"$(INTDIR)\pgpBufMod.obj" \
	"$(INTDIR)\pgpByteFIFO.obj" \
	"$(INTDIR)\pgpCAST5.obj" \
	"$(INTDIR)\pgpCFB.obj" \
	"$(INTDIR)\pgpCharMap.obj" \
	"$(INTDIR)\pgpCipher.obj" \
	"$(INTDIR)\pgpCiphrMod.obj" \
	"$(INTDIR)\pgpCompMod.obj" \
	"$(INTDIR)\pgpConf.obj" \
	"$(INTDIR)\pgpConvMod.obj" \
	"$(INTDIR)\pgpCopyMod.obj" \
	"$(INTDIR)\pgpCRC.obj" \
	"$(INTDIR)\pgpDebug.obj" \
	"$(INTDIR)\pgpDecPipe.obj" \
	"$(INTDIR)\pgpDefMod.obj" \
	"$(INTDIR)\pgpDES3.obj" \
	"$(INTDIR)\pgpDevNull.obj" \
	"$(INTDIR)\pgpDSAKey.obj" \
	"$(INTDIR)\pgpElGKey.obj" \
	"$(INTDIR)\pgpEncPipe.obj" \
	"$(INTDIR)\pgpEnv.obj" \
	"$(INTDIR)\pgpErr.obj" \
	"$(INTDIR)\pgpESK.obj" \
	"$(INTDIR)\pgpFIFO.obj" \
	"$(INTDIR)\pgpFile.obj" \
	"$(INTDIR)\pgpFileFIFO.obj" \
	"$(INTDIR)\pgpFileMod.obj" \
	"$(INTDIR)\pgpFileNames.obj" \
	"$(INTDIR)\pgpFileRef.obj" \
	"$(INTDIR)\pgpFileType.obj" \
	"$(INTDIR)\pgpFixedKey.obj" \
	"$(INTDIR)\pgpFlexFIFO.obj" \
	"$(INTDIR)\pgpGlobals.obj" \
	"$(INTDIR)\pgpHash.obj" \
	"$(INTDIR)\pgpHashMod.obj" \
	"$(INTDIR)\pgpHeader.obj" \
	"$(INTDIR)\pgpIDEA.obj" \
	"$(INTDIR)\pgpInfMod.obj" \
	"$(INTDIR)\pgpJoin.obj" \
	"$(INTDIR)\pgpKeyMisc.obj" \
	"$(INTDIR)\pgpKeySpec.obj" \
	"$(INTDIR)\pgpLeaks.obj" \
	"$(INTDIR)\pgpLiteral.obj" \
	"$(INTDIR)\pgpMakePKE.obj" \
	"$(INTDIR)\pgpMakeSig.obj" \
	"$(INTDIR)\pgpMD5.obj" \
	"$(INTDIR)\pgpMem.obj" \
	"$(INTDIR)\pgpMemFile.obj" \
	"$(INTDIR)\pgpMemMod.obj" \
	"$(INTDIR)\pgpMemPool.obj" \
	"$(INTDIR)\pgpMsg.obj" \
	"$(INTDIR)\pgpPassCach.obj" \
	"$(INTDIR)\pgpPipeFile.obj" \
	"$(INTDIR)\pgpPKEMod.obj" \
	"$(INTDIR)\pgpPktList.obj" \
	"$(INTDIR)\pgpPrsAsc.obj" \
	"$(INTDIR)\pgpPrsBin.obj" \
	"$(INTDIR)\pgpPubKey.obj" \
	"$(INTDIR)\pgpRadix64.obj" \
	"$(INTDIR)\pgpReadAnn.obj" \
	"$(INTDIR)\pgpRIPEMD160.obj" \
	"$(INTDIR)\pgpRndEvnt.obj" \
	"$(INTDIR)\pgpRndom.obj" \
	"$(INTDIR)\pgpRndPool.obj" \
	"$(INTDIR)\pgpRndSeed.obj" \
	"$(INTDIR)\pgpRndWin32.obj" \
	"$(INTDIR)\pgpRngMnt.obj" \
	"$(INTDIR)\pgpRngPars.obj" \
	"$(INTDIR)\pgpRngPkt.obj" \
	"$(INTDIR)\pgpRngPriv.obj" \
	"$(INTDIR)\pgpRngPub.obj" \
	"$(INTDIR)\pgpRngRead.obj" \
	"$(INTDIR)\pgpRSAGlue1.obj" \
	"$(INTDIR)\pgpRSAGlue2.obj" \
	"$(INTDIR)\pgpRSAKey.obj" \
	"$(INTDIR)\pgpRSAKeyGen.obj" \
	"$(INTDIR)\pgpSHA.obj" \
	"$(INTDIR)\pgpSig.obj" \
	"$(INTDIR)\pgpSigMod.obj" \
	"$(INTDIR)\pgpSigPipe.obj" \
	"$(INTDIR)\pgpSigSpec.obj" \
	"$(INTDIR)\pgpSplit.obj" \
	"$(INTDIR)\pgpStr2Key.obj" \
	"$(INTDIR)\pgpTextFilt.obj" \
	"$(INTDIR)\pgpTimeDate.obj" \
	"$(INTDIR)\pgpTrstPkt.obj" \
	"$(INTDIR)\pgpTrust.obj" \
	"$(INTDIR)\pgpVerifyRa.obj" \
	"$(INTDIR)\pgpVrfySig.obj" \
	"$(INTDIR)\pgpZBits.obj" \
	"$(INTDIR)\pgpZDeflate.obj" \
	"$(INTDIR)\pgpZInflate.obj" \
	"$(INTDIR)\pgpZTrees.obj"

"$(OUTDIR)\pgp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "pgp___Wi"
# PROP BASE Intermediate_Dir "pgp___Wi"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "NO_RSA_Debug"
# PROP Intermediate_Dir "NO_RSA_Debug"
# PROP Target_Dir ""
OUTDIR=.\NO_RSA_Debug
INTDIR=.\NO_RSA_Debug

ALL : "$(OUTDIR)\pgp.dll"

CLEAN : 
	-@erase "$(INTDIR)\pgpAddHdr.obj"
	-@erase "$(INTDIR)\pgpAltRSAGlu.obj"
	-@erase "$(INTDIR)\pgpAnnotate.obj"
	-@erase "$(INTDIR)\pgpArmor.obj"
	-@erase "$(INTDIR)\pgpArmrFil.obj"
	-@erase "$(INTDIR)\pgpBufMod.obj"
	-@erase "$(INTDIR)\pgpByteFIFO.obj"
	-@erase "$(INTDIR)\pgpCAST5.obj"
	-@erase "$(INTDIR)\pgpCFB.obj"
	-@erase "$(INTDIR)\pgpCharMap.obj"
	-@erase "$(INTDIR)\pgpCipher.obj"
	-@erase "$(INTDIR)\pgpCiphrMod.obj"
	-@erase "$(INTDIR)\pgpCompMod.obj"
	-@erase "$(INTDIR)\pgpConf.obj"
	-@erase "$(INTDIR)\pgpConvMod.obj"
	-@erase "$(INTDIR)\pgpCopyMod.obj"
	-@erase "$(INTDIR)\pgpCRC.obj"
	-@erase "$(INTDIR)\pgpDebug.obj"
	-@erase "$(INTDIR)\pgpDecPipe.obj"
	-@erase "$(INTDIR)\pgpDefMod.obj"
	-@erase "$(INTDIR)\pgpDES3.obj"
	-@erase "$(INTDIR)\pgpDevNull.obj"
	-@erase "$(INTDIR)\pgpDSAKey.obj"
	-@erase "$(INTDIR)\pgpElGKey.obj"
	-@erase "$(INTDIR)\pgpEncPipe.obj"
	-@erase "$(INTDIR)\pgpEnv.obj"
	-@erase "$(INTDIR)\pgpErr.obj"
	-@erase "$(INTDIR)\pgpESK.obj"
	-@erase "$(INTDIR)\pgpFIFO.obj"
	-@erase "$(INTDIR)\pgpFile.obj"
	-@erase "$(INTDIR)\pgpFileFIFO.obj"
	-@erase "$(INTDIR)\pgpFileMod.obj"
	-@erase "$(INTDIR)\pgpFileNames.obj"
	-@erase "$(INTDIR)\pgpFileRef.obj"
	-@erase "$(INTDIR)\pgpFileType.obj"
	-@erase "$(INTDIR)\pgpFixedKey.obj"
	-@erase "$(INTDIR)\pgpFlexFIFO.obj"
	-@erase "$(INTDIR)\pgpGlobals.obj"
	-@erase "$(INTDIR)\pgpHash.obj"
	-@erase "$(INTDIR)\pgpHashMod.obj"
	-@erase "$(INTDIR)\pgpHeader.obj"
	-@erase "$(INTDIR)\pgpIDEA.obj"
	-@erase "$(INTDIR)\pgpInfMod.obj"
	-@erase "$(INTDIR)\pgpJoin.obj"
	-@erase "$(INTDIR)\pgpKeyMisc.obj"
	-@erase "$(INTDIR)\pgpKeySpec.obj"
	-@erase "$(INTDIR)\pgpLeaks.obj"
	-@erase "$(INTDIR)\pgpLiteral.obj"
	-@erase "$(INTDIR)\pgpMakePKE.obj"
	-@erase "$(INTDIR)\pgpMakeSig.obj"
	-@erase "$(INTDIR)\pgpMD5.obj"
	-@erase "$(INTDIR)\pgpMem.obj"
	-@erase "$(INTDIR)\pgpMemFile.obj"
	-@erase "$(INTDIR)\pgpMemMod.obj"
	-@erase "$(INTDIR)\pgpMemPool.obj"
	-@erase "$(INTDIR)\pgpMsg.obj"
	-@erase "$(INTDIR)\pgpPassCach.obj"
	-@erase "$(INTDIR)\pgpPipeFile.obj"
	-@erase "$(INTDIR)\pgpPKEMod.obj"
	-@erase "$(INTDIR)\pgpPktList.obj"
	-@erase "$(INTDIR)\pgpPrsAsc.obj"
	-@erase "$(INTDIR)\pgpPrsBin.obj"
	-@erase "$(INTDIR)\pgpPubKey.obj"
	-@erase "$(INTDIR)\pgpRadix64.obj"
	-@erase "$(INTDIR)\pgpReadAnn.obj"
	-@erase "$(INTDIR)\pgpRIPEMD160.obj"
	-@erase "$(INTDIR)\pgpRndEvnt.obj"
	-@erase "$(INTDIR)\pgpRndom.obj"
	-@erase "$(INTDIR)\pgpRndPool.obj"
	-@erase "$(INTDIR)\pgpRndSeed.obj"
	-@erase "$(INTDIR)\pgpRndWin32.obj"
	-@erase "$(INTDIR)\pgpRngMnt.obj"
	-@erase "$(INTDIR)\pgpRngPars.obj"
	-@erase "$(INTDIR)\pgpRngPkt.obj"
	-@erase "$(INTDIR)\pgpRngPriv.obj"
	-@erase "$(INTDIR)\pgpRngPub.obj"
	-@erase "$(INTDIR)\pgpRngRead.obj"
	-@erase "$(INTDIR)\pgpRSAGlue1.obj"
	-@erase "$(INTDIR)\pgpRSAGlue2.obj"
	-@erase "$(INTDIR)\pgpRSAKey.obj"
	-@erase "$(INTDIR)\pgpRSAKeyGen.obj"
	-@erase "$(INTDIR)\pgpSHA.obj"
	-@erase "$(INTDIR)\pgpSig.obj"
	-@erase "$(INTDIR)\pgpSigMod.obj"
	-@erase "$(INTDIR)\pgpSigPipe.obj"
	-@erase "$(INTDIR)\pgpSigSpec.obj"
	-@erase "$(INTDIR)\pgpSplit.obj"
	-@erase "$(INTDIR)\pgpStr2Key.obj"
	-@erase "$(INTDIR)\pgpTextFilt.obj"
	-@erase "$(INTDIR)\pgpTimeDate.obj"
	-@erase "$(INTDIR)\pgpTrstPkt.obj"
	-@erase "$(INTDIR)\pgpTrust.obj"
	-@erase "$(INTDIR)\pgpVerifyRa.obj"
	-@erase "$(INTDIR)\pgpVrfySig.obj"
	-@erase "$(INTDIR)\pgpZBits.obj"
	-@erase "$(INTDIR)\pgpZDeflate.obj"
	-@erase "$(INTDIR)\pgpZInflate.obj"
	-@erase "$(INTDIR)\pgpZTrees.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\pgp.dll"
	-@erase "$(OUTDIR)\pgp.exp"
	-@erase "$(OUTDIR)\pgp.ilk"
	-@erase "$(OUTDIR)\pgp.lib"
	-@erase "$(OUTDIR)\pgp.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "../.." /I "../../include" /I "../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D UNFINISHED_CODE_ALLOWED=1 /D HAVE_CONFIG_H=1 /D "PGPLIB" /D PGPTRUSTMODEL=0 /D DEBUG=0 /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "../.." /I "../../include" /I "../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D UNFINISHED_CODE_ALLOWED=1 /D HAVE_CONFIG_H=1 /D "PGPLIB" /D PGPTRUSTMODEL=0 /D DEBUG=0 /D NO_RSA=1 /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "../.." /I "../../include" /I\
 "../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D UNFINISHED_CODE_ALLOWED=1\
 /D HAVE_CONFIG_H=1 /D "PGPLIB" /D PGPTRUSTMODEL=0 /D DEBUG=0 /D NO_RSA=1\
 /Fp"$(INTDIR)/pgp.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\NO_RSA_Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/pgp.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# SUBTRACT BASE LINK32 /pdb:none /incremental:no /nodefaultlib
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\bn\debug\bn.lib /nologo /subsystem:windows /dll /debug /machine:I386
# SUBTRACT LINK32 /pdb:none /incremental:no /nodefaultlib
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib ..\bn\debug\bn.lib /nologo /subsystem:windows /dll\
 /incremental:yes /pdb:"$(OUTDIR)/pgp.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/pgp.dll" /implib:"$(OUTDIR)/pgp.lib" 
LINK32_OBJS= \
	"$(INTDIR)\pgpAddHdr.obj" \
	"$(INTDIR)\pgpAltRSAGlu.obj" \
	"$(INTDIR)\pgpAnnotate.obj" \
	"$(INTDIR)\pgpArmor.obj" \
	"$(INTDIR)\pgpArmrFil.obj" \
	"$(INTDIR)\pgpBufMod.obj" \
	"$(INTDIR)\pgpByteFIFO.obj" \
	"$(INTDIR)\pgpCAST5.obj" \
	"$(INTDIR)\pgpCFB.obj" \
	"$(INTDIR)\pgpCharMap.obj" \
	"$(INTDIR)\pgpCipher.obj" \
	"$(INTDIR)\pgpCiphrMod.obj" \
	"$(INTDIR)\pgpCompMod.obj" \
	"$(INTDIR)\pgpConf.obj" \
	"$(INTDIR)\pgpConvMod.obj" \
	"$(INTDIR)\pgpCopyMod.obj" \
	"$(INTDIR)\pgpCRC.obj" \
	"$(INTDIR)\pgpDebug.obj" \
	"$(INTDIR)\pgpDecPipe.obj" \
	"$(INTDIR)\pgpDefMod.obj" \
	"$(INTDIR)\pgpDES3.obj" \
	"$(INTDIR)\pgpDevNull.obj" \
	"$(INTDIR)\pgpDSAKey.obj" \
	"$(INTDIR)\pgpElGKey.obj" \
	"$(INTDIR)\pgpEncPipe.obj" \
	"$(INTDIR)\pgpEnv.obj" \
	"$(INTDIR)\pgpErr.obj" \
	"$(INTDIR)\pgpESK.obj" \
	"$(INTDIR)\pgpFIFO.obj" \
	"$(INTDIR)\pgpFile.obj" \
	"$(INTDIR)\pgpFileFIFO.obj" \
	"$(INTDIR)\pgpFileMod.obj" \
	"$(INTDIR)\pgpFileNames.obj" \
	"$(INTDIR)\pgpFileRef.obj" \
	"$(INTDIR)\pgpFileType.obj" \
	"$(INTDIR)\pgpFixedKey.obj" \
	"$(INTDIR)\pgpFlexFIFO.obj" \
	"$(INTDIR)\pgpGlobals.obj" \
	"$(INTDIR)\pgpHash.obj" \
	"$(INTDIR)\pgpHashMod.obj" \
	"$(INTDIR)\pgpHeader.obj" \
	"$(INTDIR)\pgpIDEA.obj" \
	"$(INTDIR)\pgpInfMod.obj" \
	"$(INTDIR)\pgpJoin.obj" \
	"$(INTDIR)\pgpKeyMisc.obj" \
	"$(INTDIR)\pgpKeySpec.obj" \
	"$(INTDIR)\pgpLeaks.obj" \
	"$(INTDIR)\pgpLiteral.obj" \
	"$(INTDIR)\pgpMakePKE.obj" \
	"$(INTDIR)\pgpMakeSig.obj" \
	"$(INTDIR)\pgpMD5.obj" \
	"$(INTDIR)\pgpMem.obj" \
	"$(INTDIR)\pgpMemFile.obj" \
	"$(INTDIR)\pgpMemMod.obj" \
	"$(INTDIR)\pgpMemPool.obj" \
	"$(INTDIR)\pgpMsg.obj" \
	"$(INTDIR)\pgpPassCach.obj" \
	"$(INTDIR)\pgpPipeFile.obj" \
	"$(INTDIR)\pgpPKEMod.obj" \
	"$(INTDIR)\pgpPktList.obj" \
	"$(INTDIR)\pgpPrsAsc.obj" \
	"$(INTDIR)\pgpPrsBin.obj" \
	"$(INTDIR)\pgpPubKey.obj" \
	"$(INTDIR)\pgpRadix64.obj" \
	"$(INTDIR)\pgpReadAnn.obj" \
	"$(INTDIR)\pgpRIPEMD160.obj" \
	"$(INTDIR)\pgpRndEvnt.obj" \
	"$(INTDIR)\pgpRndom.obj" \
	"$(INTDIR)\pgpRndPool.obj" \
	"$(INTDIR)\pgpRndSeed.obj" \
	"$(INTDIR)\pgpRndWin32.obj" \
	"$(INTDIR)\pgpRngMnt.obj" \
	"$(INTDIR)\pgpRngPars.obj" \
	"$(INTDIR)\pgpRngPkt.obj" \
	"$(INTDIR)\pgpRngPriv.obj" \
	"$(INTDIR)\pgpRngPub.obj" \
	"$(INTDIR)\pgpRngRead.obj" \
	"$(INTDIR)\pgpRSAGlue1.obj" \
	"$(INTDIR)\pgpRSAGlue2.obj" \
	"$(INTDIR)\pgpRSAKey.obj" \
	"$(INTDIR)\pgpRSAKeyGen.obj" \
	"$(INTDIR)\pgpSHA.obj" \
	"$(INTDIR)\pgpSig.obj" \
	"$(INTDIR)\pgpSigMod.obj" \
	"$(INTDIR)\pgpSigPipe.obj" \
	"$(INTDIR)\pgpSigSpec.obj" \
	"$(INTDIR)\pgpSplit.obj" \
	"$(INTDIR)\pgpStr2Key.obj" \
	"$(INTDIR)\pgpTextFilt.obj" \
	"$(INTDIR)\pgpTimeDate.obj" \
	"$(INTDIR)\pgpTrstPkt.obj" \
	"$(INTDIR)\pgpTrust.obj" \
	"$(INTDIR)\pgpVerifyRa.obj" \
	"$(INTDIR)\pgpVrfySig.obj" \
	"$(INTDIR)\pgpZBits.obj" \
	"$(INTDIR)\pgpZDeflate.obj" \
	"$(INTDIR)\pgpZInflate.obj" \
	"$(INTDIR)\pgpZTrees.obj"

"$(OUTDIR)\pgp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "pgp___W0"
# PROP BASE Intermediate_Dir "pgp___W0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "NO_RSA_Release"
# PROP Intermediate_Dir "NO_RSA_Release"
# PROP Target_Dir ""
OUTDIR=.\NO_RSA_Release
INTDIR=.\NO_RSA_Release

ALL : "$(OUTDIR)\pgp.dll"

CLEAN : 
	-@erase "$(INTDIR)\pgpAddHdr.obj"
	-@erase "$(INTDIR)\pgpAltRSAGlu.obj"
	-@erase "$(INTDIR)\pgpAnnotate.obj"
	-@erase "$(INTDIR)\pgpArmor.obj"
	-@erase "$(INTDIR)\pgpArmrFil.obj"
	-@erase "$(INTDIR)\pgpBufMod.obj"
	-@erase "$(INTDIR)\pgpByteFIFO.obj"
	-@erase "$(INTDIR)\pgpCAST5.obj"
	-@erase "$(INTDIR)\pgpCFB.obj"
	-@erase "$(INTDIR)\pgpCharMap.obj"
	-@erase "$(INTDIR)\pgpCipher.obj"
	-@erase "$(INTDIR)\pgpCiphrMod.obj"
	-@erase "$(INTDIR)\pgpCompMod.obj"
	-@erase "$(INTDIR)\pgpConf.obj"
	-@erase "$(INTDIR)\pgpConvMod.obj"
	-@erase "$(INTDIR)\pgpCopyMod.obj"
	-@erase "$(INTDIR)\pgpCRC.obj"
	-@erase "$(INTDIR)\pgpDebug.obj"
	-@erase "$(INTDIR)\pgpDecPipe.obj"
	-@erase "$(INTDIR)\pgpDefMod.obj"
	-@erase "$(INTDIR)\pgpDES3.obj"
	-@erase "$(INTDIR)\pgpDevNull.obj"
	-@erase "$(INTDIR)\pgpDSAKey.obj"
	-@erase "$(INTDIR)\pgpElGKey.obj"
	-@erase "$(INTDIR)\pgpEncPipe.obj"
	-@erase "$(INTDIR)\pgpEnv.obj"
	-@erase "$(INTDIR)\pgpErr.obj"
	-@erase "$(INTDIR)\pgpESK.obj"
	-@erase "$(INTDIR)\pgpFIFO.obj"
	-@erase "$(INTDIR)\pgpFile.obj"
	-@erase "$(INTDIR)\pgpFileFIFO.obj"
	-@erase "$(INTDIR)\pgpFileMod.obj"
	-@erase "$(INTDIR)\pgpFileNames.obj"
	-@erase "$(INTDIR)\pgpFileRef.obj"
	-@erase "$(INTDIR)\pgpFileType.obj"
	-@erase "$(INTDIR)\pgpFixedKey.obj"
	-@erase "$(INTDIR)\pgpFlexFIFO.obj"
	-@erase "$(INTDIR)\pgpGlobals.obj"
	-@erase "$(INTDIR)\pgpHash.obj"
	-@erase "$(INTDIR)\pgpHashMod.obj"
	-@erase "$(INTDIR)\pgpHeader.obj"
	-@erase "$(INTDIR)\pgpIDEA.obj"
	-@erase "$(INTDIR)\pgpInfMod.obj"
	-@erase "$(INTDIR)\pgpJoin.obj"
	-@erase "$(INTDIR)\pgpKeyMisc.obj"
	-@erase "$(INTDIR)\pgpKeySpec.obj"
	-@erase "$(INTDIR)\pgpLeaks.obj"
	-@erase "$(INTDIR)\pgpLiteral.obj"
	-@erase "$(INTDIR)\pgpMakePKE.obj"
	-@erase "$(INTDIR)\pgpMakeSig.obj"
	-@erase "$(INTDIR)\pgpMD5.obj"
	-@erase "$(INTDIR)\pgpMem.obj"
	-@erase "$(INTDIR)\pgpMemFile.obj"
	-@erase "$(INTDIR)\pgpMemMod.obj"
	-@erase "$(INTDIR)\pgpMemPool.obj"
	-@erase "$(INTDIR)\pgpMsg.obj"
	-@erase "$(INTDIR)\pgpPassCach.obj"
	-@erase "$(INTDIR)\pgpPipeFile.obj"
	-@erase "$(INTDIR)\pgpPKEMod.obj"
	-@erase "$(INTDIR)\pgpPktList.obj"
	-@erase "$(INTDIR)\pgpPrsAsc.obj"
	-@erase "$(INTDIR)\pgpPrsBin.obj"
	-@erase "$(INTDIR)\pgpPubKey.obj"
	-@erase "$(INTDIR)\pgpRadix64.obj"
	-@erase "$(INTDIR)\pgpReadAnn.obj"
	-@erase "$(INTDIR)\pgpRIPEMD160.obj"
	-@erase "$(INTDIR)\pgpRndEvnt.obj"
	-@erase "$(INTDIR)\pgpRndom.obj"
	-@erase "$(INTDIR)\pgpRndPool.obj"
	-@erase "$(INTDIR)\pgpRndSeed.obj"
	-@erase "$(INTDIR)\pgpRndWin32.obj"
	-@erase "$(INTDIR)\pgpRngMnt.obj"
	-@erase "$(INTDIR)\pgpRngPars.obj"
	-@erase "$(INTDIR)\pgpRngPkt.obj"
	-@erase "$(INTDIR)\pgpRngPriv.obj"
	-@erase "$(INTDIR)\pgpRngPub.obj"
	-@erase "$(INTDIR)\pgpRngRead.obj"
	-@erase "$(INTDIR)\pgpRSAGlue1.obj"
	-@erase "$(INTDIR)\pgpRSAGlue2.obj"
	-@erase "$(INTDIR)\pgpRSAKey.obj"
	-@erase "$(INTDIR)\pgpRSAKeyGen.obj"
	-@erase "$(INTDIR)\pgpSHA.obj"
	-@erase "$(INTDIR)\pgpSig.obj"
	-@erase "$(INTDIR)\pgpSigMod.obj"
	-@erase "$(INTDIR)\pgpSigPipe.obj"
	-@erase "$(INTDIR)\pgpSigSpec.obj"
	-@erase "$(INTDIR)\pgpSplit.obj"
	-@erase "$(INTDIR)\pgpStr2Key.obj"
	-@erase "$(INTDIR)\pgpTextFilt.obj"
	-@erase "$(INTDIR)\pgpTimeDate.obj"
	-@erase "$(INTDIR)\pgpTrstPkt.obj"
	-@erase "$(INTDIR)\pgpTrust.obj"
	-@erase "$(INTDIR)\pgpVerifyRa.obj"
	-@erase "$(INTDIR)\pgpVrfySig.obj"
	-@erase "$(INTDIR)\pgpZBits.obj"
	-@erase "$(INTDIR)\pgpZDeflate.obj"
	-@erase "$(INTDIR)\pgpZInflate.obj"
	-@erase "$(INTDIR)\pgpZTrees.obj"
	-@erase "$(OUTDIR)\pgp.dll"
	-@erase "$(OUTDIR)\pgp.exp"
	-@erase "$(OUTDIR)\pgp.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "../.." /I "../../include" /I "../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D UNFINISHED_CODE_ALLOWED=1 /D HAVE_CONFIG_H=1 /D "PGPLIB" /D PGPTRUSTMODEL=0 /D DEBUG=0 /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../.." /I "../../include" /I "../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D UNFINISHED_CODE_ALLOWED=1 /D HAVE_CONFIG_H=1 /D "PGPLIB" /D PGPTRUSTMODEL=0 /D DEBUG=0 /D NO_RSA=1 /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "../.." /I "../../include" /I "../include"\
 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D UNFINISHED_CODE_ALLOWED=1 /D\
 HAVE_CONFIG_H=1 /D "PGPLIB" /D PGPTRUSTMODEL=0 /D DEBUG=0 /D NO_RSA=1\
 /Fp"$(INTDIR)/pgp.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\NO_RSA_Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/pgp.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\bn\release\bn.lib /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib ..\bn\release\bn.lib /nologo /subsystem:windows /dll\
 /incremental:no /pdb:"$(OUTDIR)/pgp.pdb" /machine:I386 /out:"$(OUTDIR)/pgp.dll"\
 /implib:"$(OUTDIR)/pgp.lib" 
LINK32_OBJS= \
	"$(INTDIR)\pgpAddHdr.obj" \
	"$(INTDIR)\pgpAltRSAGlu.obj" \
	"$(INTDIR)\pgpAnnotate.obj" \
	"$(INTDIR)\pgpArmor.obj" \
	"$(INTDIR)\pgpArmrFil.obj" \
	"$(INTDIR)\pgpBufMod.obj" \
	"$(INTDIR)\pgpByteFIFO.obj" \
	"$(INTDIR)\pgpCAST5.obj" \
	"$(INTDIR)\pgpCFB.obj" \
	"$(INTDIR)\pgpCharMap.obj" \
	"$(INTDIR)\pgpCipher.obj" \
	"$(INTDIR)\pgpCiphrMod.obj" \
	"$(INTDIR)\pgpCompMod.obj" \
	"$(INTDIR)\pgpConf.obj" \
	"$(INTDIR)\pgpConvMod.obj" \
	"$(INTDIR)\pgpCopyMod.obj" \
	"$(INTDIR)\pgpCRC.obj" \
	"$(INTDIR)\pgpDebug.obj" \
	"$(INTDIR)\pgpDecPipe.obj" \
	"$(INTDIR)\pgpDefMod.obj" \
	"$(INTDIR)\pgpDES3.obj" \
	"$(INTDIR)\pgpDevNull.obj" \
	"$(INTDIR)\pgpDSAKey.obj" \
	"$(INTDIR)\pgpElGKey.obj" \
	"$(INTDIR)\pgpEncPipe.obj" \
	"$(INTDIR)\pgpEnv.obj" \
	"$(INTDIR)\pgpErr.obj" \
	"$(INTDIR)\pgpESK.obj" \
	"$(INTDIR)\pgpFIFO.obj" \
	"$(INTDIR)\pgpFile.obj" \
	"$(INTDIR)\pgpFileFIFO.obj" \
	"$(INTDIR)\pgpFileMod.obj" \
	"$(INTDIR)\pgpFileNames.obj" \
	"$(INTDIR)\pgpFileRef.obj" \
	"$(INTDIR)\pgpFileType.obj" \
	"$(INTDIR)\pgpFixedKey.obj" \
	"$(INTDIR)\pgpFlexFIFO.obj" \
	"$(INTDIR)\pgpGlobals.obj" \
	"$(INTDIR)\pgpHash.obj" \
	"$(INTDIR)\pgpHashMod.obj" \
	"$(INTDIR)\pgpHeader.obj" \
	"$(INTDIR)\pgpIDEA.obj" \
	"$(INTDIR)\pgpInfMod.obj" \
	"$(INTDIR)\pgpJoin.obj" \
	"$(INTDIR)\pgpKeyMisc.obj" \
	"$(INTDIR)\pgpKeySpec.obj" \
	"$(INTDIR)\pgpLeaks.obj" \
	"$(INTDIR)\pgpLiteral.obj" \
	"$(INTDIR)\pgpMakePKE.obj" \
	"$(INTDIR)\pgpMakeSig.obj" \
	"$(INTDIR)\pgpMD5.obj" \
	"$(INTDIR)\pgpMem.obj" \
	"$(INTDIR)\pgpMemFile.obj" \
	"$(INTDIR)\pgpMemMod.obj" \
	"$(INTDIR)\pgpMemPool.obj" \
	"$(INTDIR)\pgpMsg.obj" \
	"$(INTDIR)\pgpPassCach.obj" \
	"$(INTDIR)\pgpPipeFile.obj" \
	"$(INTDIR)\pgpPKEMod.obj" \
	"$(INTDIR)\pgpPktList.obj" \
	"$(INTDIR)\pgpPrsAsc.obj" \
	"$(INTDIR)\pgpPrsBin.obj" \
	"$(INTDIR)\pgpPubKey.obj" \
	"$(INTDIR)\pgpRadix64.obj" \
	"$(INTDIR)\pgpReadAnn.obj" \
	"$(INTDIR)\pgpRIPEMD160.obj" \
	"$(INTDIR)\pgpRndEvnt.obj" \
	"$(INTDIR)\pgpRndom.obj" \
	"$(INTDIR)\pgpRndPool.obj" \
	"$(INTDIR)\pgpRndSeed.obj" \
	"$(INTDIR)\pgpRndWin32.obj" \
	"$(INTDIR)\pgpRngMnt.obj" \
	"$(INTDIR)\pgpRngPars.obj" \
	"$(INTDIR)\pgpRngPkt.obj" \
	"$(INTDIR)\pgpRngPriv.obj" \
	"$(INTDIR)\pgpRngPub.obj" \
	"$(INTDIR)\pgpRngRead.obj" \
	"$(INTDIR)\pgpRSAGlue1.obj" \
	"$(INTDIR)\pgpRSAGlue2.obj" \
	"$(INTDIR)\pgpRSAKey.obj" \
	"$(INTDIR)\pgpRSAKeyGen.obj" \
	"$(INTDIR)\pgpSHA.obj" \
	"$(INTDIR)\pgpSig.obj" \
	"$(INTDIR)\pgpSigMod.obj" \
	"$(INTDIR)\pgpSigPipe.obj" \
	"$(INTDIR)\pgpSigSpec.obj" \
	"$(INTDIR)\pgpSplit.obj" \
	"$(INTDIR)\pgpStr2Key.obj" \
	"$(INTDIR)\pgpTextFilt.obj" \
	"$(INTDIR)\pgpTimeDate.obj" \
	"$(INTDIR)\pgpTrstPkt.obj" \
	"$(INTDIR)\pgpTrust.obj" \
	"$(INTDIR)\pgpVerifyRa.obj" \
	"$(INTDIR)\pgpVrfySig.obj" \
	"$(INTDIR)\pgpZBits.obj" \
	"$(INTDIR)\pgpZDeflate.obj" \
	"$(INTDIR)\pgpZInflate.obj" \
	"$(INTDIR)\pgpZTrees.obj"

"$(OUTDIR)\pgp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "pgp - Win32 Release"
# Name "pgp - Win32 Debug"
# Name "pgp - Win32 RSAREF Debug"
# Name "pgp - Win32 RSAREF Release"
# Name "pgp - Win32 NO_RSA Debug"
# Name "pgp - Win32 NO_RSA Release"

!IF  "$(CFG)" == "pgp - Win32 Release"

!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"

!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"

!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"

!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"

!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\cipher\pgpIDEA.c
DEP_CPP_PGPID=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\cipher\pgpCipher.h"\
	".\cipher\pgpIDEA.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpIDEA.obj" : $(SOURCE) $(DEP_CPP_PGPID) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpIDEA.obj" : $(SOURCE) $(DEP_CPP_PGPID) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpIDEA.obj" : $(SOURCE) $(DEP_CPP_PGPID) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpIDEA.obj" : $(SOURCE) $(DEP_CPP_PGPID) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpIDEA.obj" : $(SOURCE) $(DEP_CPP_PGPID) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpIDEA.obj" : $(SOURCE) $(DEP_CPP_PGPID) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cipher\pgpDES3.c
DEP_CPP_PGPDE=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\cipher\pgpCipher.h"\
	".\cipher\pgpDES3.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpDES3.obj" : $(SOURCE) $(DEP_CPP_PGPDE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpDES3.obj" : $(SOURCE) $(DEP_CPP_PGPDE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpDES3.obj" : $(SOURCE) $(DEP_CPP_PGPDE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpDES3.obj" : $(SOURCE) $(DEP_CPP_PGPDE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpDES3.obj" : $(SOURCE) $(DEP_CPP_PGPDE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpDES3.obj" : $(SOURCE) $(DEP_CPP_PGPDE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cipher\pgpCipher.c
DEP_CPP_PGPCI=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\cipher\pgpCAST5.h"\
	".\cipher\pgpCipher.h"\
	".\cipher\pgpDES3.h"\
	".\cipher\pgpIDEA.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpCipher.obj" : $(SOURCE) $(DEP_CPP_PGPCI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpCipher.obj" : $(SOURCE) $(DEP_CPP_PGPCI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpCipher.obj" : $(SOURCE) $(DEP_CPP_PGPCI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpCipher.obj" : $(SOURCE) $(DEP_CPP_PGPCI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpCipher.obj" : $(SOURCE) $(DEP_CPP_PGPCI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpCipher.obj" : $(SOURCE) $(DEP_CPP_PGPCI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cipher\pgpCFB.c
DEP_CPP_PGPCF=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\cipher\pgpCFB.h"\
	".\cipher\pgpCipher.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpCFB.obj" : $(SOURCE) $(DEP_CPP_PGPCF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpCFB.obj" : $(SOURCE) $(DEP_CPP_PGPCF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpCFB.obj" : $(SOURCE) $(DEP_CPP_PGPCF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpCFB.obj" : $(SOURCE) $(DEP_CPP_PGPCF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpCFB.obj" : $(SOURCE) $(DEP_CPP_PGPCF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpCFB.obj" : $(SOURCE) $(DEP_CPP_PGPCF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cipher\pgpCAST5.c
DEP_CPP_PGPCA=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\cipher\pgpCAST5.h"\
	".\cipher\pgpCASTBox5.h"\
	".\cipher\pgpCipher.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpCAST5.obj" : $(SOURCE) $(DEP_CPP_PGPCA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpCAST5.obj" : $(SOURCE) $(DEP_CPP_PGPCA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpCAST5.obj" : $(SOURCE) $(DEP_CPP_PGPCA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpCAST5.obj" : $(SOURCE) $(DEP_CPP_PGPCA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpCAST5.obj" : $(SOURCE) $(DEP_CPP_PGPCA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpCAST5.obj" : $(SOURCE) $(DEP_CPP_PGPCA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\compress\pgpZInflate.c
DEP_CPP_PGPZI=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\compress\pgpZInflate.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpZInflate.obj" : $(SOURCE) $(DEP_CPP_PGPZI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpZInflate.obj" : $(SOURCE) $(DEP_CPP_PGPZI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpZInflate.obj" : $(SOURCE) $(DEP_CPP_PGPZI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpZInflate.obj" : $(SOURCE) $(DEP_CPP_PGPZI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpZInflate.obj" : $(SOURCE) $(DEP_CPP_PGPZI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpZInflate.obj" : $(SOURCE) $(DEP_CPP_PGPZI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\hash\pgpSHA.c
DEP_CPP_PGPSH=\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\hash\pgpHash.h"\
	".\hash\pgpSHA.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpSHA.obj" : $(SOURCE) $(DEP_CPP_PGPSH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpSHA.obj" : $(SOURCE) $(DEP_CPP_PGPSH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpSHA.obj" : $(SOURCE) $(DEP_CPP_PGPSH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpSHA.obj" : $(SOURCE) $(DEP_CPP_PGPSH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpSHA.obj" : $(SOURCE) $(DEP_CPP_PGPSH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpSHA.obj" : $(SOURCE) $(DEP_CPP_PGPSH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\hash\pgpMD5.c
DEP_CPP_PGPMD=\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\hash\pgpHash.h"\
	".\hash\pgpMD5.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpMD5.obj" : $(SOURCE) $(DEP_CPP_PGPMD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpMD5.obj" : $(SOURCE) $(DEP_CPP_PGPMD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpMD5.obj" : $(SOURCE) $(DEP_CPP_PGPMD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpMD5.obj" : $(SOURCE) $(DEP_CPP_PGPMD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpMD5.obj" : $(SOURCE) $(DEP_CPP_PGPMD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpMD5.obj" : $(SOURCE) $(DEP_CPP_PGPMD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\hash\pgpHash.c
DEP_CPP_PGPHA=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\hash\pgpHash.h"\
	".\hash\pgpMD5.h"\
	".\hash\pgpRIPEMD160.h"\
	".\hash\pgpSHA.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpHash.obj" : $(SOURCE) $(DEP_CPP_PGPHA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpHash.obj" : $(SOURCE) $(DEP_CPP_PGPHA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpHash.obj" : $(SOURCE) $(DEP_CPP_PGPHA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpHash.obj" : $(SOURCE) $(DEP_CPP_PGPHA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpHash.obj" : $(SOURCE) $(DEP_CPP_PGPHA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpHash.obj" : $(SOURCE) $(DEP_CPP_PGPHA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\helper\pgpTimeDate.c
DEP_CPP_PGPTI=\
	".\../../include\pgpErr.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\helper\pgpDebug.h"\
	".\helper\pgpLeaks.h"\
	".\helper\pgpMem.h"\
	".\helper\pgpTimeDate.h"\
	".\helper\pgpTypes.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpTimeDate.obj" : $(SOURCE) $(DEP_CPP_PGPTI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpTimeDate.obj" : $(SOURCE) $(DEP_CPP_PGPTI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpTimeDate.obj" : $(SOURCE) $(DEP_CPP_PGPTI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpTimeDate.obj" : $(SOURCE) $(DEP_CPP_PGPTI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpTimeDate.obj" : $(SOURCE) $(DEP_CPP_PGPTI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpTimeDate.obj" : $(SOURCE) $(DEP_CPP_PGPTI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\helper\pgpStr2Key.c
DEP_CPP_PGPST=\
	".\../../include\pgpErr.h"\
	".\../../include\pgpHash.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\helper\pgpDebug.h"\
	".\helper\pgpLeaks.h"\
	".\helper\pgpMem.h"\
	".\helper\pgpStr2Key.h"\
	".\helper\pgpTypes.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpStr2Key.obj" : $(SOURCE) $(DEP_CPP_PGPST) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpStr2Key.obj" : $(SOURCE) $(DEP_CPP_PGPST) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpStr2Key.obj" : $(SOURCE) $(DEP_CPP_PGPST) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpStr2Key.obj" : $(SOURCE) $(DEP_CPP_PGPST) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpStr2Key.obj" : $(SOURCE) $(DEP_CPP_PGPST) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpStr2Key.obj" : $(SOURCE) $(DEP_CPP_PGPST) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\helper\pgpPipeFile.c
DEP_CPP_PGPPI=\
	".\../../include\pgpErr.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\helper\pgpDebug.h"\
	".\helper\pgpFile.h"\
	".\helper\pgpLeaks.h"\
	".\helper\pgpMem.h"\
	".\helper\pgpTypes.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpPipeFile.obj" : $(SOURCE) $(DEP_CPP_PGPPI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpPipeFile.obj" : $(SOURCE) $(DEP_CPP_PGPPI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpPipeFile.obj" : $(SOURCE) $(DEP_CPP_PGPPI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpPipeFile.obj" : $(SOURCE) $(DEP_CPP_PGPPI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpPipeFile.obj" : $(SOURCE) $(DEP_CPP_PGPPI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpPipeFile.obj" : $(SOURCE) $(DEP_CPP_PGPPI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\helper\pgpPassCach.c
DEP_CPP_PGPPA=\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\helper\pgpDebug.h"\
	".\helper\pgpLeaks.h"\
	".\helper\pgpMem.h"\
	".\helper\pgpPassCach.h"\
	".\helper\pgpTypes.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpPassCach.obj" : $(SOURCE) $(DEP_CPP_PGPPA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpPassCach.obj" : $(SOURCE) $(DEP_CPP_PGPPA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpPassCach.obj" : $(SOURCE) $(DEP_CPP_PGPPA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpPassCach.obj" : $(SOURCE) $(DEP_CPP_PGPPA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpPassCach.obj" : $(SOURCE) $(DEP_CPP_PGPPA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpPassCach.obj" : $(SOURCE) $(DEP_CPP_PGPPA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\helper\pgpMsg.c
DEP_CPP_PGPMS=\
	".\../../include\pgpMsg.h"\
	".\../..\config.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpMsg.obj" : $(SOURCE) $(DEP_CPP_PGPMS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpMsg.obj" : $(SOURCE) $(DEP_CPP_PGPMS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpMsg.obj" : $(SOURCE) $(DEP_CPP_PGPMS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpMsg.obj" : $(SOURCE) $(DEP_CPP_PGPMS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpMsg.obj" : $(SOURCE) $(DEP_CPP_PGPMS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpMsg.obj" : $(SOURCE) $(DEP_CPP_PGPMS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\helper\pgpMemFile.c
DEP_CPP_PGPME=\
	".\../../include\pgpCFB.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\helper\pgpDebug.h"\
	".\helper\pgpFile.h"\
	".\helper\pgpLeaks.h"\
	".\helper\pgpMem.h"\
	".\helper\pgpTypes.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpMemFile.obj" : $(SOURCE) $(DEP_CPP_PGPME) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpMemFile.obj" : $(SOURCE) $(DEP_CPP_PGPME) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpMemFile.obj" : $(SOURCE) $(DEP_CPP_PGPME) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpMemFile.obj" : $(SOURCE) $(DEP_CPP_PGPME) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpMemFile.obj" : $(SOURCE) $(DEP_CPP_PGPME) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpMemFile.obj" : $(SOURCE) $(DEP_CPP_PGPME) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\helper\pgpMem.c
DEP_CPP_PGPMEM=\
	".\../../include\pgpErr.h"\
	".\../..\config.h"\
	".\helper\pgpDebug.h"\
	".\helper\pgpLeaks.h"\
	".\helper\pgpMem.h"\
	".\helper\pgpTypes.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpMem.obj" : $(SOURCE) $(DEP_CPP_PGPMEM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpMem.obj" : $(SOURCE) $(DEP_CPP_PGPMEM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpMem.obj" : $(SOURCE) $(DEP_CPP_PGPMEM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpMem.obj" : $(SOURCE) $(DEP_CPP_PGPMEM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpMem.obj" : $(SOURCE) $(DEP_CPP_PGPMEM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpMem.obj" : $(SOURCE) $(DEP_CPP_PGPMEM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\helper\pgpLeaks.c
DEP_CPP_PGPLE=\
	".\../../include\pgpErr.h"\
	".\../..\config.h"\
	".\helper\pgpDebug.h"\
	".\helper\pgpLeaks.h"\
	".\helper\pgpMem.h"\
	".\helper\pgpTypes.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpLeaks.obj" : $(SOURCE) $(DEP_CPP_PGPLE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpLeaks.obj" : $(SOURCE) $(DEP_CPP_PGPLE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpLeaks.obj" : $(SOURCE) $(DEP_CPP_PGPLE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpLeaks.obj" : $(SOURCE) $(DEP_CPP_PGPLE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpLeaks.obj" : $(SOURCE) $(DEP_CPP_PGPLE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpLeaks.obj" : $(SOURCE) $(DEP_CPP_PGPLE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\helper\pgpGlobals.c
DEP_CPP_PGPGL=\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpGlobals.obj" : $(SOURCE) $(DEP_CPP_PGPGL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpGlobals.obj" : $(SOURCE) $(DEP_CPP_PGPGL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpGlobals.obj" : $(SOURCE) $(DEP_CPP_PGPGL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpGlobals.obj" : $(SOURCE) $(DEP_CPP_PGPGL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpGlobals.obj" : $(SOURCE) $(DEP_CPP_PGPGL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpGlobals.obj" : $(SOURCE) $(DEP_CPP_PGPGL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\helper\pgpFlexFIFO.c
DEP_CPP_PGPFL=\
	".\../../include\pgpErr.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\helper\pgpDebug.h"\
	".\helper\pgpFIFO.h"\
	".\helper\pgpLeaks.h"\
	".\helper\pgpMem.h"\
	".\helper\pgpTypes.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpFlexFIFO.obj" : $(SOURCE) $(DEP_CPP_PGPFL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpFlexFIFO.obj" : $(SOURCE) $(DEP_CPP_PGPFL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpFlexFIFO.obj" : $(SOURCE) $(DEP_CPP_PGPFL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpFlexFIFO.obj" : $(SOURCE) $(DEP_CPP_PGPFL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpFlexFIFO.obj" : $(SOURCE) $(DEP_CPP_PGPFL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpFlexFIFO.obj" : $(SOURCE) $(DEP_CPP_PGPFL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\helper\pgpFileFIFO.c
DEP_CPP_PGPFI=\
	".\../../include\pgpCFB.h"\
	".\../../include\pgpCipher.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\helper\pgpDebug.h"\
	".\helper\pgpFIFO.h"\
	".\helper\pgpFile.h"\
	".\helper\pgpFileRef.h"\
	".\helper\pgpLeaks.h"\
	".\helper\pgpMem.h"\
	".\helper\pgpTypes.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpFileFIFO.obj" : $(SOURCE) $(DEP_CPP_PGPFI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpFileFIFO.obj" : $(SOURCE) $(DEP_CPP_PGPFI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpFileFIFO.obj" : $(SOURCE) $(DEP_CPP_PGPFI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpFileFIFO.obj" : $(SOURCE) $(DEP_CPP_PGPFI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpFileFIFO.obj" : $(SOURCE) $(DEP_CPP_PGPFI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpFileFIFO.obj" : $(SOURCE) $(DEP_CPP_PGPFI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\helper\pgpFile.c
DEP_CPP_PGPFIL=\
	".\../../include\pgpCFB.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\helper\pgpDebug.h"\
	".\helper\pgpFile.h"\
	".\helper\pgpLeaks.h"\
	".\helper\pgpMem.h"\
	".\helper\pgpTypes.h"\
	{$(INCLUDE)}"\sys\Stat.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpFile.obj" : $(SOURCE) $(DEP_CPP_PGPFIL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpFile.obj" : $(SOURCE) $(DEP_CPP_PGPFIL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpFile.obj" : $(SOURCE) $(DEP_CPP_PGPFIL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpFile.obj" : $(SOURCE) $(DEP_CPP_PGPFIL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpFile.obj" : $(SOURCE) $(DEP_CPP_PGPFIL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpFile.obj" : $(SOURCE) $(DEP_CPP_PGPFIL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\helper\pgpFIFO.c
DEP_CPP_PGPFIF=\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\helper\pgpByteFIFO.h"\
	".\helper\pgpFIFO.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpFIFO.obj" : $(SOURCE) $(DEP_CPP_PGPFIF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpFIFO.obj" : $(SOURCE) $(DEP_CPP_PGPFIF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpFIFO.obj" : $(SOURCE) $(DEP_CPP_PGPFIF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpFIFO.obj" : $(SOURCE) $(DEP_CPP_PGPFIF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpFIFO.obj" : $(SOURCE) $(DEP_CPP_PGPFIF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpFIFO.obj" : $(SOURCE) $(DEP_CPP_PGPFIF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\helper\pgpErr.c
DEP_CPP_PGPER=\
	".\../../include\pgpErr.h"\
	".\../..\config.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpErr.obj" : $(SOURCE) $(DEP_CPP_PGPER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpErr.obj" : $(SOURCE) $(DEP_CPP_PGPER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpErr.obj" : $(SOURCE) $(DEP_CPP_PGPER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpErr.obj" : $(SOURCE) $(DEP_CPP_PGPER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpErr.obj" : $(SOURCE) $(DEP_CPP_PGPER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpErr.obj" : $(SOURCE) $(DEP_CPP_PGPER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\helper\pgpDebug.c
DEP_CPP_PGPDEB=\
	".\../../include\pgpErr.h"\
	".\../..\config.h"\
	".\helper\pgpDebug.h"\
	".\helper\pgpLeaks.h"\
	".\helper\pgpMem.h"\
	".\helper\pgpTypes.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpDebug.obj" : $(SOURCE) $(DEP_CPP_PGPDEB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpDebug.obj" : $(SOURCE) $(DEP_CPP_PGPDEB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpDebug.obj" : $(SOURCE) $(DEP_CPP_PGPDEB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpDebug.obj" : $(SOURCE) $(DEP_CPP_PGPDEB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpDebug.obj" : $(SOURCE) $(DEP_CPP_PGPDEB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpDebug.obj" : $(SOURCE) $(DEP_CPP_PGPDEB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\helper\pgpCharMap.c
DEP_CPP_PGPCH=\
	".\../../include\pgpErr.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\helper\pgpCharMap.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpCharMap.obj" : $(SOURCE) $(DEP_CPP_PGPCH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpCharMap.obj" : $(SOURCE) $(DEP_CPP_PGPCH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpCharMap.obj" : $(SOURCE) $(DEP_CPP_PGPCH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpCharMap.obj" : $(SOURCE) $(DEP_CPP_PGPCH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpCharMap.obj" : $(SOURCE) $(DEP_CPP_PGPCH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpCharMap.obj" : $(SOURCE) $(DEP_CPP_PGPCH) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\helper\pgpByteFIFO.c
DEP_CPP_PGPBY=\
	".\../../include\pgpErr.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\helper\pgpByteFIFO.h"\
	".\helper\pgpDebug.h"\
	".\helper\pgpLeaks.h"\
	".\helper\pgpMem.h"\
	".\helper\pgpTypes.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpByteFIFO.obj" : $(SOURCE) $(DEP_CPP_PGPBY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpByteFIFO.obj" : $(SOURCE) $(DEP_CPP_PGPBY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpByteFIFO.obj" : $(SOURCE) $(DEP_CPP_PGPBY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpByteFIFO.obj" : $(SOURCE) $(DEP_CPP_PGPBY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpByteFIFO.obj" : $(SOURCE) $(DEP_CPP_PGPBY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpByteFIFO.obj" : $(SOURCE) $(DEP_CPP_PGPBY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\helper\pgpAnnotate.c
DEP_CPP_PGPAN=\
	".\../../include\pgpAnnotate.h"\
	".\../..\config.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpAnnotate.obj" : $(SOURCE) $(DEP_CPP_PGPAN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpAnnotate.obj" : $(SOURCE) $(DEP_CPP_PGPAN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpAnnotate.obj" : $(SOURCE) $(DEP_CPP_PGPAN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpAnnotate.obj" : $(SOURCE) $(DEP_CPP_PGPAN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpAnnotate.obj" : $(SOURCE) $(DEP_CPP_PGPAN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpAnnotate.obj" : $(SOURCE) $(DEP_CPP_PGPAN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\keys\pgpTrust.c
DEP_CPP_PGPTR=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\keys\pgpMemPool.h"\
	".\keys\pgpRngPriv.h"\
	".\keys\pgpRngPub.h"\
	".\keys\pgpTrstPkt.h"\
	".\keys\pgpTrust.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpTrust.obj" : $(SOURCE) $(DEP_CPP_PGPTR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpTrust.obj" : $(SOURCE) $(DEP_CPP_PGPTR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpTrust.obj" : $(SOURCE) $(DEP_CPP_PGPTR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpTrust.obj" : $(SOURCE) $(DEP_CPP_PGPTR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpTrust.obj" : $(SOURCE) $(DEP_CPP_PGPTR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpTrust.obj" : $(SOURCE) $(DEP_CPP_PGPTR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\keys\pgpTrstPkt.c
DEP_CPP_PGPTRS=\
	".\../..\config.h"\
	".\keys\pgpMemPool.h"\
	".\keys\pgpTrstPkt.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpTrstPkt.obj" : $(SOURCE) $(DEP_CPP_PGPTRS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpTrstPkt.obj" : $(SOURCE) $(DEP_CPP_PGPTRS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpTrstPkt.obj" : $(SOURCE) $(DEP_CPP_PGPTRS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpTrstPkt.obj" : $(SOURCE) $(DEP_CPP_PGPTRS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpTrstPkt.obj" : $(SOURCE) $(DEP_CPP_PGPTRS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpTrstPkt.obj" : $(SOURCE) $(DEP_CPP_PGPTRS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\keys\pgpRngRead.c
DEP_CPP_PGPRN=\
	".\../../include\pgpCipher.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpHash.h"\
	".\../../include\pgpKeySpec.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\../include\pgpMakeSig.h"\
	".\../include\pgpPktByte.h"\
	".\..\..\include\pgpLeaks.h"\
	".\keys\pgpMemPool.h"\
	".\keys\pgpRngMnt.h"\
	".\keys\pgpRngPars.h"\
	".\keys\pgpRngPkt.h"\
	".\keys\pgpRngPriv.h"\
	".\keys\pgpRngPub.h"\
	".\keys\pgpRngRead.h"\
	".\keys\pgpTrstPkt.h"\
	".\keys\pgpTrust.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpRngRead.obj" : $(SOURCE) $(DEP_CPP_PGPRN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpRngRead.obj" : $(SOURCE) $(DEP_CPP_PGPRN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpRngRead.obj" : $(SOURCE) $(DEP_CPP_PGPRN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpRngRead.obj" : $(SOURCE) $(DEP_CPP_PGPRN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpRngRead.obj" : $(SOURCE) $(DEP_CPP_PGPRN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpRngRead.obj" : $(SOURCE) $(DEP_CPP_PGPRN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\keys\pgpRngPub.c
DEP_CPP_PGPRNG=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\keys\pgpMemPool.h"\
	".\keys\pgpRngMnt.h"\
	".\keys\pgpRngPars.h"\
	".\keys\pgpRngPriv.h"\
	".\keys\pgpRngPub.h"\
	".\keys\pgpRngRead.h"\
	".\keys\pgpTrstPkt.h"\
	".\keys\pgpTrust.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpRngPub.obj" : $(SOURCE) $(DEP_CPP_PGPRNG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpRngPub.obj" : $(SOURCE) $(DEP_CPP_PGPRNG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpRngPub.obj" : $(SOURCE) $(DEP_CPP_PGPRNG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpRngPub.obj" : $(SOURCE) $(DEP_CPP_PGPRNG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpRngPub.obj" : $(SOURCE) $(DEP_CPP_PGPRNG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpRngPub.obj" : $(SOURCE) $(DEP_CPP_PGPRNG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\keys\pgpRngPriv.c
DEP_CPP_PGPRNGP=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpHash.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\../include\pgpPktByte.h"\
	".\..\..\include\pgpLeaks.h"\
	".\keys\pgpMemPool.h"\
	".\keys\pgpRngPriv.h"\
	".\keys\pgpRngPub.h"\
	".\keys\pgpRngRead.h"\
	".\keys\pgpTrstPkt.h"\
	".\keys\pgpTrust.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpRngPriv.obj" : $(SOURCE) $(DEP_CPP_PGPRNGP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpRngPriv.obj" : $(SOURCE) $(DEP_CPP_PGPRNGP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpRngPriv.obj" : $(SOURCE) $(DEP_CPP_PGPRNGP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpRngPriv.obj" : $(SOURCE) $(DEP_CPP_PGPRNGP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpRngPriv.obj" : $(SOURCE) $(DEP_CPP_PGPRNGP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpRngPriv.obj" : $(SOURCE) $(DEP_CPP_PGPRNGP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\keys\pgpRngPars.c
DEP_CPP_PGPRNGPA=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpHash.h"\
	".\../../include\pgpKeySpec.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\../include\pgpPktByte.h"\
	".\keys\pgpMemPool.h"\
	".\keys\pgpRngPars.h"\
	".\keys\pgpRngPriv.h"\
	".\keys\pgpRngPub.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpRngPars.obj" : $(SOURCE) $(DEP_CPP_PGPRNGPA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpRngPars.obj" : $(SOURCE) $(DEP_CPP_PGPRNGPA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpRngPars.obj" : $(SOURCE) $(DEP_CPP_PGPRNGPA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpRngPars.obj" : $(SOURCE) $(DEP_CPP_PGPRNGPA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpRngPars.obj" : $(SOURCE) $(DEP_CPP_PGPRNGPA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpRngPars.obj" : $(SOURCE) $(DEP_CPP_PGPRNGPA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\keys\pgpRngMnt.c
DEP_CPP_PGPRNGM=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpHash.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTimeDate.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\../include\pgpMakeSig.h"\
	".\../include\pgpPktByte.h"\
	".\keys\pgpMemPool.h"\
	".\keys\pgpRngMnt.h"\
	".\keys\pgpRngPars.h"\
	".\keys\pgpRngPkt.h"\
	".\keys\pgpRngPriv.h"\
	".\keys\pgpRngPub.h"\
	".\keys\pgpRngRead.h"\
	".\keys\pgpTrstPkt.h"\
	".\keys\pgpTrust.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpRngMnt.obj" : $(SOURCE) $(DEP_CPP_PGPRNGM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpRngMnt.obj" : $(SOURCE) $(DEP_CPP_PGPRNGM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpRngMnt.obj" : $(SOURCE) $(DEP_CPP_PGPRNGM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpRngMnt.obj" : $(SOURCE) $(DEP_CPP_PGPRNGM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpRngMnt.obj" : $(SOURCE) $(DEP_CPP_PGPRNGM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpRngMnt.obj" : $(SOURCE) $(DEP_CPP_PGPRNGM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\keys\pgpMemPool.c
DEP_CPP_PGPMEMP=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\keys\pgpMemPool.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpMemPool.obj" : $(SOURCE) $(DEP_CPP_PGPMEMP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpMemPool.obj" : $(SOURCE) $(DEP_CPP_PGPMEMP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpMemPool.obj" : $(SOURCE) $(DEP_CPP_PGPMEMP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpMemPool.obj" : $(SOURCE) $(DEP_CPP_PGPMEMP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpMemPool.obj" : $(SOURCE) $(DEP_CPP_PGPMEMP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpMemPool.obj" : $(SOURCE) $(DEP_CPP_PGPMEMP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pubkey\pgpSig.c
DEP_CPP_PGPSI=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpHash.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\pubkey\pgpPktList.h"\
	".\pubkey\pgpPubKey.h"\
	".\pubkey\pgpSig.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpSig.obj" : $(SOURCE) $(DEP_CPP_PGPSI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpSig.obj" : $(SOURCE) $(DEP_CPP_PGPSI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpSig.obj" : $(SOURCE) $(DEP_CPP_PGPSI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpSig.obj" : $(SOURCE) $(DEP_CPP_PGPSI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpSig.obj" : $(SOURCE) $(DEP_CPP_PGPSI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpSig.obj" : $(SOURCE) $(DEP_CPP_PGPSI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pubkey\pgpRSAKey.c
DEP_CPP_PGPRS=\
	".\../../include\bn.h"\
	".\../../include\bnprime.h"\
	".\../../include\pgpCFB.h"\
	".\../../include\pgpCipher.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpHash.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpStr2Key.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pubkey\pgpKeyMisc.h"\
	".\pubkey\pgpPubKey.h"\
	".\pubkey\pgpRSAGlue.h"\
	".\pubkey\pgpRSAKey.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpRSAKey.obj" : $(SOURCE) $(DEP_CPP_PGPRS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpRSAKey.obj" : $(SOURCE) $(DEP_CPP_PGPRS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpRSAKey.obj" : $(SOURCE) $(DEP_CPP_PGPRS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpRSAKey.obj" : $(SOURCE) $(DEP_CPP_PGPRS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpRSAKey.obj" : $(SOURCE) $(DEP_CPP_PGPRS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpRSAKey.obj" : $(SOURCE) $(DEP_CPP_PGPRS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pubkey\pgpPubKey.c
DEP_CPP_PGPPU=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\pubkey\pgpDSAKey.h"\
	".\pubkey\pgpElGKey.h"\
	".\pubkey\pgpFixedKey.h"\
	".\pubkey\pgpKeyMisc.h"\
	".\pubkey\pgpPubKey.h"\
	".\pubkey\pgpRSAKey.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpPubKey.obj" : $(SOURCE) $(DEP_CPP_PGPPU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpPubKey.obj" : $(SOURCE) $(DEP_CPP_PGPPU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpPubKey.obj" : $(SOURCE) $(DEP_CPP_PGPPU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpPubKey.obj" : $(SOURCE) $(DEP_CPP_PGPPU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpPubKey.obj" : $(SOURCE) $(DEP_CPP_PGPPU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpPubKey.obj" : $(SOURCE) $(DEP_CPP_PGPPU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pubkey\pgpPktList.c
DEP_CPP_PGPPK=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pubkey\pgpPktList.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpPktList.obj" : $(SOURCE) $(DEP_CPP_PGPPK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpPktList.obj" : $(SOURCE) $(DEP_CPP_PGPPK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpPktList.obj" : $(SOURCE) $(DEP_CPP_PGPPK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpPktList.obj" : $(SOURCE) $(DEP_CPP_PGPPK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpPktList.obj" : $(SOURCE) $(DEP_CPP_PGPPK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpPktList.obj" : $(SOURCE) $(DEP_CPP_PGPPK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pubkey\pgpMakeSig.c
DEP_CPP_PGPMA=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpHash.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpRngPub.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pubkey\pgpMakeSig.h"\
	".\pubkey\pgpPubKey.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpMakeSig.obj" : $(SOURCE) $(DEP_CPP_PGPMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpMakeSig.obj" : $(SOURCE) $(DEP_CPP_PGPMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpMakeSig.obj" : $(SOURCE) $(DEP_CPP_PGPMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpMakeSig.obj" : $(SOURCE) $(DEP_CPP_PGPMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpMakeSig.obj" : $(SOURCE) $(DEP_CPP_PGPMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpMakeSig.obj" : $(SOURCE) $(DEP_CPP_PGPMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pubkey\pgpKeySpec.c
DEP_CPP_PGPKE=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpTimeDate.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pubkey\pgpKeySpec.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpKeySpec.obj" : $(SOURCE) $(DEP_CPP_PGPKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpKeySpec.obj" : $(SOURCE) $(DEP_CPP_PGPKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpKeySpec.obj" : $(SOURCE) $(DEP_CPP_PGPKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpKeySpec.obj" : $(SOURCE) $(DEP_CPP_PGPKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpKeySpec.obj" : $(SOURCE) $(DEP_CPP_PGPKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpKeySpec.obj" : $(SOURCE) $(DEP_CPP_PGPKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pubkey\pgpKeyMisc.c
DEP_CPP_PGPKEY=\
	".\../../include\bn.h"\
	".\../../include\pgpCFB.h"\
	".\../../include\pgpCipher.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpHash.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpStr2Key.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pubkey\pgpKeyMisc.h"\
	".\pubkey\pgpPubKey.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpKeyMisc.obj" : $(SOURCE) $(DEP_CPP_PGPKEY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpKeyMisc.obj" : $(SOURCE) $(DEP_CPP_PGPKEY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpKeyMisc.obj" : $(SOURCE) $(DEP_CPP_PGPKEY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpKeyMisc.obj" : $(SOURCE) $(DEP_CPP_PGPKEY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpKeyMisc.obj" : $(SOURCE) $(DEP_CPP_PGPKEY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpKeyMisc.obj" : $(SOURCE) $(DEP_CPP_PGPKEY) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pubkey\pgpFixedKey.c
DEP_CPP_PGPFIX=\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\pubkey\pgpESK.h"\
	".\pubkey\pgpFixedKey.h"\
	".\pubkey\pgpRSAKey.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	
NODEP_CPP_PGPFIX=\
	".\pubkey\keys.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpFixedKey.obj" : $(SOURCE) $(DEP_CPP_PGPFIX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpFixedKey.obj" : $(SOURCE) $(DEP_CPP_PGPFIX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpFixedKey.obj" : $(SOURCE) $(DEP_CPP_PGPFIX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpFixedKey.obj" : $(SOURCE) $(DEP_CPP_PGPFIX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpFixedKey.obj" : $(SOURCE) $(DEP_CPP_PGPFIX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpFixedKey.obj" : $(SOURCE) $(DEP_CPP_PGPFIX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pubkey\pgpESK.c
DEP_CPP_PGPES=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpCFB.h"\
	".\../../include\pgpCipher.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpHash.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpStr2Key.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pubkey\pgpESK.h"\
	".\pubkey\pgpPktList.h"\
	".\pubkey\pgpPubKey.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpESK.obj" : $(SOURCE) $(DEP_CPP_PGPES) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpESK.obj" : $(SOURCE) $(DEP_CPP_PGPES) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpESK.obj" : $(SOURCE) $(DEP_CPP_PGPES) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpESK.obj" : $(SOURCE) $(DEP_CPP_PGPES) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpESK.obj" : $(SOURCE) $(DEP_CPP_PGPES) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpESK.obj" : $(SOURCE) $(DEP_CPP_PGPES) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pubkey\pgpElGKey.c
DEP_CPP_PGPEL=\
	".\../../include\bn.h"\
	".\../../include\bngermain.h"\
	".\../../include\bnprime.h"\
	".\../../include\pgpCFB.h"\
	".\../../include\pgpCipher.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpHash.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpStr2Key.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pubkey\pgpElGKey.h"\
	".\pubkey\pgpFixedKey.h"\
	".\pubkey\pgpKeyMisc.h"\
	".\pubkey\pgpPubKey.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpElGKey.obj" : $(SOURCE) $(DEP_CPP_PGPEL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpElGKey.obj" : $(SOURCE) $(DEP_CPP_PGPEL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpElGKey.obj" : $(SOURCE) $(DEP_CPP_PGPEL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpElGKey.obj" : $(SOURCE) $(DEP_CPP_PGPEL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpElGKey.obj" : $(SOURCE) $(DEP_CPP_PGPEL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpElGKey.obj" : $(SOURCE) $(DEP_CPP_PGPEL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pubkey\pgpDSAKey.c
DEP_CPP_PGPDS=\
	".\../../include\bn.h"\
	".\../../include\bnprime.h"\
	".\../../include\pgpCFB.h"\
	".\../../include\pgpCipher.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpHash.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpStr2Key.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pubkey\pgpDSAKey.h"\
	".\pubkey\pgpFixedKey.h"\
	".\pubkey\pgpKeyMisc.h"\
	".\pubkey\pgpPubKey.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpDSAKey.obj" : $(SOURCE) $(DEP_CPP_PGPDS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpDSAKey.obj" : $(SOURCE) $(DEP_CPP_PGPDS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpDSAKey.obj" : $(SOURCE) $(DEP_CPP_PGPDS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpDSAKey.obj" : $(SOURCE) $(DEP_CPP_PGPDS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpDSAKey.obj" : $(SOURCE) $(DEP_CPP_PGPDS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpDSAKey.obj" : $(SOURCE) $(DEP_CPP_PGPDS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\random\pgpRndWin32.c
DEP_CPP_PGPRND=\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\random\pgpRnd.h"\
	".\random\pgpRndom.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpRndWin32.obj" : $(SOURCE) $(DEP_CPP_PGPRND) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpRndWin32.obj" : $(SOURCE) $(DEP_CPP_PGPRND) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpRndWin32.obj" : $(SOURCE) $(DEP_CPP_PGPRND) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpRndWin32.obj" : $(SOURCE) $(DEP_CPP_PGPRND) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpRndWin32.obj" : $(SOURCE) $(DEP_CPP_PGPRND) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpRndWin32.obj" : $(SOURCE) $(DEP_CPP_PGPRND) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\random\pgpRndSeed.c
DEP_CPP_PGPRNDS=\
	".\../../include\pgpCFB.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\random\pgpRndom.h"\
	".\random\pgpRndPool.h"\
	".\random\pgpRndSeed.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpRndSeed.obj" : $(SOURCE) $(DEP_CPP_PGPRNDS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpRndSeed.obj" : $(SOURCE) $(DEP_CPP_PGPRNDS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpRndSeed.obj" : $(SOURCE) $(DEP_CPP_PGPRNDS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpRndSeed.obj" : $(SOURCE) $(DEP_CPP_PGPRNDS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpRndSeed.obj" : $(SOURCE) $(DEP_CPP_PGPRNDS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpRndSeed.obj" : $(SOURCE) $(DEP_CPP_PGPRNDS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\random\pgpRndPool.c
DEP_CPP_PGPRNDP=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\random\pgpRndom.h"\
	".\random\pgpRndPool.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpRndPool.obj" : $(SOURCE) $(DEP_CPP_PGPRNDP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpRndPool.obj" : $(SOURCE) $(DEP_CPP_PGPRNDP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpRndPool.obj" : $(SOURCE) $(DEP_CPP_PGPRNDP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpRndPool.obj" : $(SOURCE) $(DEP_CPP_PGPRNDP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpRndPool.obj" : $(SOURCE) $(DEP_CPP_PGPRNDP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpRndPool.obj" : $(SOURCE) $(DEP_CPP_PGPRNDP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\random\pgpRndom.c
DEP_CPP_PGPRNDO=\
	".\../../include\pgpCFB.h"\
	".\../../include\pgpCipher.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\random\pgpRndom.h"\
	".\random\pgpRndPool.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpRndom.obj" : $(SOURCE) $(DEP_CPP_PGPRNDO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpRndom.obj" : $(SOURCE) $(DEP_CPP_PGPRNDO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpRndom.obj" : $(SOURCE) $(DEP_CPP_PGPRNDO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpRndom.obj" : $(SOURCE) $(DEP_CPP_PGPRNDO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpRndom.obj" : $(SOURCE) $(DEP_CPP_PGPRNDO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpRndom.obj" : $(SOURCE) $(DEP_CPP_PGPRNDO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\random\pgpRndEvnt.c
DEP_CPP_PGPRNDE=\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\random\pgpRnd.h"\
	".\random\pgpRndom.h"\
	".\random\pgpRndPool.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpRndEvnt.obj" : $(SOURCE) $(DEP_CPP_PGPRNDE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpRndEvnt.obj" : $(SOURCE) $(DEP_CPP_PGPRNDE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpRndEvnt.obj" : $(SOURCE) $(DEP_CPP_PGPRNDE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpRndEvnt.obj" : $(SOURCE) $(DEP_CPP_PGPRNDE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpRndEvnt.obj" : $(SOURCE) $(DEP_CPP_PGPRNDE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpRndEvnt.obj" : $(SOURCE) $(DEP_CPP_PGPRNDE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utils\pgpSigSpec.c
DEP_CPP_PGPSIG=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpHash.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpTimeDate.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\utils\pgpEnv.h"\
	".\utils\pgpSigSpec.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpSigSpec.obj" : $(SOURCE) $(DEP_CPP_PGPSIG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpSigSpec.obj" : $(SOURCE) $(DEP_CPP_PGPSIG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpSigSpec.obj" : $(SOURCE) $(DEP_CPP_PGPSIG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpSigSpec.obj" : $(SOURCE) $(DEP_CPP_PGPSIG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpSigSpec.obj" : $(SOURCE) $(DEP_CPP_PGPSIG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpSigSpec.obj" : $(SOURCE) $(DEP_CPP_PGPSIG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utils\pgpSigPipe.c
DEP_CPP_PGPSIGP=\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpPrsAsc.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../../include\pgpVerifyRa.h"\
	".\../..\config.h"\
	".\utils\pgpSigPipe.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpSigPipe.obj" : $(SOURCE) $(DEP_CPP_PGPSIGP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpSigPipe.obj" : $(SOURCE) $(DEP_CPP_PGPSIGP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpSigPipe.obj" : $(SOURCE) $(DEP_CPP_PGPSIGP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpSigPipe.obj" : $(SOURCE) $(DEP_CPP_PGPSIGP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpSigPipe.obj" : $(SOURCE) $(DEP_CPP_PGPSIGP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpSigPipe.obj" : $(SOURCE) $(DEP_CPP_PGPSIGP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utils\pgpFileType.c
DEP_CPP_PGPFILE=\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\../include\pgpPktByte.h"\
	".\utils\pgpFileType.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpFileType.obj" : $(SOURCE) $(DEP_CPP_PGPFILE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpFileType.obj" : $(SOURCE) $(DEP_CPP_PGPFILE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpFileType.obj" : $(SOURCE) $(DEP_CPP_PGPFILE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpFileType.obj" : $(SOURCE) $(DEP_CPP_PGPFILE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpFileType.obj" : $(SOURCE) $(DEP_CPP_PGPFILE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpFileType.obj" : $(SOURCE) $(DEP_CPP_PGPFILE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utils\pgpEnv.c
DEP_CPP_PGPEN=\
	".\../../include\pgpCipher.h"\
	".\../../include\pgpCompress.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpHash.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\../include\pgpCharMap.h"\
	".\..\..\include\pgpLeaks.h"\
	".\utils\pgpEnv.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpEnv.obj" : $(SOURCE) $(DEP_CPP_PGPEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpEnv.obj" : $(SOURCE) $(DEP_CPP_PGPEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpEnv.obj" : $(SOURCE) $(DEP_CPP_PGPEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpEnv.obj" : $(SOURCE) $(DEP_CPP_PGPEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpEnv.obj" : $(SOURCE) $(DEP_CPP_PGPEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpEnv.obj" : $(SOURCE) $(DEP_CPP_PGPEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utils\pgpEncPipe.c
DEP_CPP_PGPENC=\
	".\../../include\pgpArmor.h"\
	".\../../include\pgpCFB.h"\
	".\../../include\pgpCipher.h"\
	".\../../include\pgpCiphrMod.h"\
	".\../../include\pgpCompMod.h"\
	".\../../include\pgpCompress.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpConvMod.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpHash.h"\
	".\../../include\pgpHeader.h"\
	".\../../include\pgpLiteral.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPKEMod.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpRndPool.h"\
	".\../../include\pgpSigMod.h"\
	".\../../include\pgpSplit.h"\
	".\../../include\pgpStr2Key.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\utils\pgpEncPipe.h"\
	".\utils\pgpEnv.h"\
	".\utils\pgpSigSpec.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpEncPipe.obj" : $(SOURCE) $(DEP_CPP_PGPENC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpEncPipe.obj" : $(SOURCE) $(DEP_CPP_PGPENC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpEncPipe.obj" : $(SOURCE) $(DEP_CPP_PGPENC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpEncPipe.obj" : $(SOURCE) $(DEP_CPP_PGPENC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpEncPipe.obj" : $(SOURCE) $(DEP_CPP_PGPENC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpEncPipe.obj" : $(SOURCE) $(DEP_CPP_PGPENC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utils\pgpDecPipe.c
DEP_CPP_PGPDEC=\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpPrsAsc.h"\
	".\../../include\pgpReadAnn.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\utils\pgpDecPipe.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpDecPipe.obj" : $(SOURCE) $(DEP_CPP_PGPDEC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpDecPipe.obj" : $(SOURCE) $(DEP_CPP_PGPDEC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpDecPipe.obj" : $(SOURCE) $(DEP_CPP_PGPDEC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpDecPipe.obj" : $(SOURCE) $(DEP_CPP_PGPDEC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpDecPipe.obj" : $(SOURCE) $(DEP_CPP_PGPDEC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpDecPipe.obj" : $(SOURCE) $(DEP_CPP_PGPDEC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\utils\pgpConf.c
DEP_CPP_PGPCO=\
	".\../../include\pgpErr.h"\
	".\../../include\pgpMsg.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\utils\pgpConf.h"\
	".\utils\pgpEnv.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpConf.obj" : $(SOURCE) $(DEP_CPP_PGPCO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpConf.obj" : $(SOURCE) $(DEP_CPP_PGPCO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpConf.obj" : $(SOURCE) $(DEP_CPP_PGPCO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpConf.obj" : $(SOURCE) $(DEP_CPP_PGPCO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpConf.obj" : $(SOURCE) $(DEP_CPP_PGPCO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpConf.obj" : $(SOURCE) $(DEP_CPP_PGPCO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\crypt\pgpPKEMod.c
DEP_CPP_PGPPKE=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpJoin.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\../include\pgpPktByte.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\crypt\pgpMakePKE.h"\
	".\pipe\crypt\pgpPKEMod.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpPKEMod.obj" : $(SOURCE) $(DEP_CPP_PGPPKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpPKEMod.obj" : $(SOURCE) $(DEP_CPP_PGPPKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpPKEMod.obj" : $(SOURCE) $(DEP_CPP_PGPPKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpPKEMod.obj" : $(SOURCE) $(DEP_CPP_PGPPKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpPKEMod.obj" : $(SOURCE) $(DEP_CPP_PGPPKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpPKEMod.obj" : $(SOURCE) $(DEP_CPP_PGPPKE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\crypt\pgpMakePKE.c
DEP_CPP_PGPMAK=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpPubKey.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\pipe\crypt\pgpMakePKE.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpMakePKE.obj" : $(SOURCE) $(DEP_CPP_PGPMAK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpMakePKE.obj" : $(SOURCE) $(DEP_CPP_PGPMAK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpMakePKE.obj" : $(SOURCE) $(DEP_CPP_PGPMAK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpMakePKE.obj" : $(SOURCE) $(DEP_CPP_PGPMAK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpMakePKE.obj" : $(SOURCE) $(DEP_CPP_PGPMAK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpMakePKE.obj" : $(SOURCE) $(DEP_CPP_PGPMAK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\crypt\pgpConvMod.c
DEP_CPP_PGPCON=\
	".\../../include\pgpCFB.h"\
	".\../../include\pgpCipher.h"\
	".\../../include\pgpConvKey.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpJoin.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpStr2Key.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\../include\pgpPktByte.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\crypt\pgpConvMod.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpConvMod.obj" : $(SOURCE) $(DEP_CPP_PGPCON) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpConvMod.obj" : $(SOURCE) $(DEP_CPP_PGPCON) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpConvMod.obj" : $(SOURCE) $(DEP_CPP_PGPCON) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpConvMod.obj" : $(SOURCE) $(DEP_CPP_PGPCON) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpConvMod.obj" : $(SOURCE) $(DEP_CPP_PGPCON) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpConvMod.obj" : $(SOURCE) $(DEP_CPP_PGPCON) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\crypt\pgpCiphrMod.c
DEP_CPP_PGPCIP=\
	".\../../include\pgpCFB.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\../include\pgpAddHdr.h"\
	".\../include\pgpPktByte.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\crypt\pgpCiphrMod.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpCiphrMod.obj" : $(SOURCE) $(DEP_CPP_PGPCIP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpCiphrMod.obj" : $(SOURCE) $(DEP_CPP_PGPCIP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpCiphrMod.obj" : $(SOURCE) $(DEP_CPP_PGPCIP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpCiphrMod.obj" : $(SOURCE) $(DEP_CPP_PGPCIP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpCiphrMod.obj" : $(SOURCE) $(DEP_CPP_PGPCIP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpCiphrMod.obj" : $(SOURCE) $(DEP_CPP_PGPCIP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\file\pgpRadix64.c
DEP_CPP_PGPRA=\
	".\../..\config.h"\
	".\pipe\file\pgpRadix64.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpRadix64.obj" : $(SOURCE) $(DEP_CPP_PGPRA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpRadix64.obj" : $(SOURCE) $(DEP_CPP_PGPRA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpRadix64.obj" : $(SOURCE) $(DEP_CPP_PGPRA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpRadix64.obj" : $(SOURCE) $(DEP_CPP_PGPRA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpRadix64.obj" : $(SOURCE) $(DEP_CPP_PGPRA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpRadix64.obj" : $(SOURCE) $(DEP_CPP_PGPRA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\file\pgpPrsAsc.c
DEP_CPP_PGPPR=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpFileType.h"\
	".\../../include\pgpHash.h"\
	".\../../include\pgpJoin.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpPrsBin.h"\
	".\../../include\pgpSplit.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../../include\pgpVerifyRa.h"\
	".\../..\config.h"\
	".\../include\pgpCharMap.h"\
	".\../include\pgpCopyMod.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\file\pgpCRC.h"\
	".\pipe\file\pgpPrsAsc.h"\
	".\pipe\file\pgpRadix64.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	
NODEP_CPP_PGPPR=\
	".\pipe\file\pgpDeMimeMod.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpPrsAsc.obj" : $(SOURCE) $(DEP_CPP_PGPPR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpPrsAsc.obj" : $(SOURCE) $(DEP_CPP_PGPPR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpPrsAsc.obj" : $(SOURCE) $(DEP_CPP_PGPPR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpPrsAsc.obj" : $(SOURCE) $(DEP_CPP_PGPPR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpPrsAsc.obj" : $(SOURCE) $(DEP_CPP_PGPPR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpPrsAsc.obj" : $(SOURCE) $(DEP_CPP_PGPPR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\file\pgpHeader.c
DEP_CPP_PGPHE=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\../include\pgpPktByte.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\file\pgpHeader.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpHeader.obj" : $(SOURCE) $(DEP_CPP_PGPHE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpHeader.obj" : $(SOURCE) $(DEP_CPP_PGPHE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpHeader.obj" : $(SOURCE) $(DEP_CPP_PGPHE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpHeader.obj" : $(SOURCE) $(DEP_CPP_PGPHE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpHeader.obj" : $(SOURCE) $(DEP_CPP_PGPHE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpHeader.obj" : $(SOURCE) $(DEP_CPP_PGPHE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\file\pgpFileMod.c
DEP_CPP_PGPFILEM=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\../include\pgpKludge.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\file\pgpFileMod.h"\
	{$(INCLUDE)}"\sys\Stat.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpFileMod.obj" : $(SOURCE) $(DEP_CPP_PGPFILEM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpFileMod.obj" : $(SOURCE) $(DEP_CPP_PGPFILEM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpFileMod.obj" : $(SOURCE) $(DEP_CPP_PGPFILEM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpFileMod.obj" : $(SOURCE) $(DEP_CPP_PGPFILEM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpFileMod.obj" : $(SOURCE) $(DEP_CPP_PGPFILEM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpFileMod.obj" : $(SOURCE) $(DEP_CPP_PGPFILEM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\file\pgpCRC.c
DEP_CPP_PGPCR=\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\pipe\file\pgpCRC.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpCRC.obj" : $(SOURCE) $(DEP_CPP_PGPCR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpCRC.obj" : $(SOURCE) $(DEP_CPP_PGPCR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpCRC.obj" : $(SOURCE) $(DEP_CPP_PGPCR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpCRC.obj" : $(SOURCE) $(DEP_CPP_PGPCR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpCRC.obj" : $(SOURCE) $(DEP_CPP_PGPCR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpCRC.obj" : $(SOURCE) $(DEP_CPP_PGPCR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\file\pgpArmrFil.c
DEP_CPP_PGPAR=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\file\pgpArmrFil.h"\
	".\pipe\file\pgpFileMod.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpArmrFil.obj" : $(SOURCE) $(DEP_CPP_PGPAR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpArmrFil.obj" : $(SOURCE) $(DEP_CPP_PGPAR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpArmrFil.obj" : $(SOURCE) $(DEP_CPP_PGPAR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpArmrFil.obj" : $(SOURCE) $(DEP_CPP_PGPAR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpArmrFil.obj" : $(SOURCE) $(DEP_CPP_PGPAR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpArmrFil.obj" : $(SOURCE) $(DEP_CPP_PGPAR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\file\pgpArmor.c
DEP_CPP_PGPARM=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpHash.h"\
	".\../../include\pgpJoin.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpSplit.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\../include\pgpPktByte.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\file\pgpArmor.h"\
	".\pipe\file\pgpCRC.h"\
	".\pipe\file\pgpRadix64.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpArmor.obj" : $(SOURCE) $(DEP_CPP_PGPARM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpArmor.obj" : $(SOURCE) $(DEP_CPP_PGPARM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpArmor.obj" : $(SOURCE) $(DEP_CPP_PGPARM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpArmor.obj" : $(SOURCE) $(DEP_CPP_PGPARM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpArmor.obj" : $(SOURCE) $(DEP_CPP_PGPARM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpArmor.obj" : $(SOURCE) $(DEP_CPP_PGPARM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\parser\pgpVrfySig.c
DEP_CPP_PGPVR=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpHash.h"\
	".\../../include\pgpHashMod.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMsg.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\parser\pgpVrfySig.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpVrfySig.obj" : $(SOURCE) $(DEP_CPP_PGPVR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpVrfySig.obj" : $(SOURCE) $(DEP_CPP_PGPVR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpVrfySig.obj" : $(SOURCE) $(DEP_CPP_PGPVR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpVrfySig.obj" : $(SOURCE) $(DEP_CPP_PGPVR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpVrfySig.obj" : $(SOURCE) $(DEP_CPP_PGPVR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpVrfySig.obj" : $(SOURCE) $(DEP_CPP_PGPVR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\parser\pgpVerifyRa.c
DEP_CPP_PGPVE=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpDevNull.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpHash.h"\
	".\../../include\pgpJoin.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMsg.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\parser\pgpVerifyRa.h"\
	".\pipe\parser\pgpVrfySig.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpVerifyRa.obj" : $(SOURCE) $(DEP_CPP_PGPVE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpVerifyRa.obj" : $(SOURCE) $(DEP_CPP_PGPVE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpVerifyRa.obj" : $(SOURCE) $(DEP_CPP_PGPVE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpVerifyRa.obj" : $(SOURCE) $(DEP_CPP_PGPVE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpVerifyRa.obj" : $(SOURCE) $(DEP_CPP_PGPVE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpVerifyRa.obj" : $(SOURCE) $(DEP_CPP_PGPVE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\parser\pgpReadAnn.c
DEP_CPP_PGPRE=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpESK.h"\
	".\../../include\pgpHash.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpMsg.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUI.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\parser\pgpReadAnn.h"\
	".\pipe\parser\pgpVrfySig.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpReadAnn.obj" : $(SOURCE) $(DEP_CPP_PGPRE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpReadAnn.obj" : $(SOURCE) $(DEP_CPP_PGPRE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpReadAnn.obj" : $(SOURCE) $(DEP_CPP_PGPRE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpReadAnn.obj" : $(SOURCE) $(DEP_CPP_PGPRE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpReadAnn.obj" : $(SOURCE) $(DEP_CPP_PGPRE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpReadAnn.obj" : $(SOURCE) $(DEP_CPP_PGPRE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\parser\pgpPrsBin.c
DEP_CPP_PGPPRS=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpCFB.h"\
	".\../../include\pgpCipher.h"\
	".\../../include\pgpCiphrMod.h"\
	".\../../include\pgpCompMod.h"\
	".\../../include\pgpCompress.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpEnv.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpHash.h"\
	".\../../include\pgpHashMod.h"\
	".\../../include\pgpHeader.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpSig.h"\
	".\../../include\pgpTextFilt.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\../include\pgpPktByte.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\parser\pgpPrsBin.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpPrsBin.obj" : $(SOURCE) $(DEP_CPP_PGPPRS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpPrsBin.obj" : $(SOURCE) $(DEP_CPP_PGPPRS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpPrsBin.obj" : $(SOURCE) $(DEP_CPP_PGPPRS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpPrsBin.obj" : $(SOURCE) $(DEP_CPP_PGPPRS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpPrsBin.obj" : $(SOURCE) $(DEP_CPP_PGPPRS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpPrsBin.obj" : $(SOURCE) $(DEP_CPP_PGPPRS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\sig\pgpSigMod.c
DEP_CPP_PGPSIGM=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpHash.h"\
	".\../../include\pgpJoin.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpSigSpec.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\../include\pgpCopyMod.h"\
	".\../include\pgpMakeSig.h"\
	".\../include\pgpPktByte.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\sig\pgpHashMod.h"\
	".\pipe\sig\pgpSigMod.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpSigMod.obj" : $(SOURCE) $(DEP_CPP_PGPSIGM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpSigMod.obj" : $(SOURCE) $(DEP_CPP_PGPSIGM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpSigMod.obj" : $(SOURCE) $(DEP_CPP_PGPSIGM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpSigMod.obj" : $(SOURCE) $(DEP_CPP_PGPSIGM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpSigMod.obj" : $(SOURCE) $(DEP_CPP_PGPSIGM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpSigMod.obj" : $(SOURCE) $(DEP_CPP_PGPSIGM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\sig\pgpHashMod.c
DEP_CPP_PGPHAS=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpHash.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\sig\pgpHashMod.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpHashMod.obj" : $(SOURCE) $(DEP_CPP_PGPHAS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpHashMod.obj" : $(SOURCE) $(DEP_CPP_PGPHAS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpHashMod.obj" : $(SOURCE) $(DEP_CPP_PGPHAS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpHashMod.obj" : $(SOURCE) $(DEP_CPP_PGPHAS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpHashMod.obj" : $(SOURCE) $(DEP_CPP_PGPHAS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpHashMod.obj" : $(SOURCE) $(DEP_CPP_PGPHAS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\text\pgpZTrees.c
DEP_CPP_PGPZT=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\text\pgpZip.h"\
	".\pipe\text\pgpZTailor.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpZTrees.obj" : $(SOURCE) $(DEP_CPP_PGPZT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpZTrees.obj" : $(SOURCE) $(DEP_CPP_PGPZT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpZTrees.obj" : $(SOURCE) $(DEP_CPP_PGPZT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpZTrees.obj" : $(SOURCE) $(DEP_CPP_PGPZT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpZTrees.obj" : $(SOURCE) $(DEP_CPP_PGPZT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpZTrees.obj" : $(SOURCE) $(DEP_CPP_PGPZT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\text\pgpZDeflate.c
DEP_CPP_PGPZD=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\text\pgpZip.h"\
	".\pipe\text\pgpZTailor.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpZDeflate.obj" : $(SOURCE) $(DEP_CPP_PGPZD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpZDeflate.obj" : $(SOURCE) $(DEP_CPP_PGPZD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpZDeflate.obj" : $(SOURCE) $(DEP_CPP_PGPZD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpZDeflate.obj" : $(SOURCE) $(DEP_CPP_PGPZD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpZDeflate.obj" : $(SOURCE) $(DEP_CPP_PGPZD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpZDeflate.obj" : $(SOURCE) $(DEP_CPP_PGPZD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\text\pgpZBits.c
DEP_CPP_PGPZB=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\../include\pgpByteFIFO.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\text\pgpZip.h"\
	".\pipe\text\pgpZTailor.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpZBits.obj" : $(SOURCE) $(DEP_CPP_PGPZB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpZBits.obj" : $(SOURCE) $(DEP_CPP_PGPZB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpZBits.obj" : $(SOURCE) $(DEP_CPP_PGPZB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpZBits.obj" : $(SOURCE) $(DEP_CPP_PGPZB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpZBits.obj" : $(SOURCE) $(DEP_CPP_PGPZB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpZBits.obj" : $(SOURCE) $(DEP_CPP_PGPZB) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\text\pgpTextFilt.c
DEP_CPP_PGPTE=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\text\pgpTextFilt.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpTextFilt.obj" : $(SOURCE) $(DEP_CPP_PGPTE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpTextFilt.obj" : $(SOURCE) $(DEP_CPP_PGPTE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpTextFilt.obj" : $(SOURCE) $(DEP_CPP_PGPTE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpTextFilt.obj" : $(SOURCE) $(DEP_CPP_PGPTE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpTextFilt.obj" : $(SOURCE) $(DEP_CPP_PGPTE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpTextFilt.obj" : $(SOURCE) $(DEP_CPP_PGPTE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\text\pgpLiteral.c
DEP_CPP_PGPLI=\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\../include\pgpAddHdr.h"\
	".\../include\pgpPktByte.h"\
	".\pipe\text\pgpLiteral.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpLiteral.obj" : $(SOURCE) $(DEP_CPP_PGPLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpLiteral.obj" : $(SOURCE) $(DEP_CPP_PGPLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpLiteral.obj" : $(SOURCE) $(DEP_CPP_PGPLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpLiteral.obj" : $(SOURCE) $(DEP_CPP_PGPLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpLiteral.obj" : $(SOURCE) $(DEP_CPP_PGPLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpLiteral.obj" : $(SOURCE) $(DEP_CPP_PGPLI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\text\pgpInfMod.c
DEP_CPP_PGPIN=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\../include\pgpZInflate.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\text\pgpInfMod.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpInfMod.obj" : $(SOURCE) $(DEP_CPP_PGPIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpInfMod.obj" : $(SOURCE) $(DEP_CPP_PGPIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpInfMod.obj" : $(SOURCE) $(DEP_CPP_PGPIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpInfMod.obj" : $(SOURCE) $(DEP_CPP_PGPIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpInfMod.obj" : $(SOURCE) $(DEP_CPP_PGPIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpInfMod.obj" : $(SOURCE) $(DEP_CPP_PGPIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\text\pgpDefMod.c
DEP_CPP_PGPDEF=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\../include\pgpByteFIFO.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\text\pgpDefMod.h"\
	".\pipe\text\pgpZip.h"\
	".\pipe\text\pgpZTailor.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpDefMod.obj" : $(SOURCE) $(DEP_CPP_PGPDEF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpDefMod.obj" : $(SOURCE) $(DEP_CPP_PGPDEF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpDefMod.obj" : $(SOURCE) $(DEP_CPP_PGPDEF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpDefMod.obj" : $(SOURCE) $(DEP_CPP_PGPDEF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpDefMod.obj" : $(SOURCE) $(DEP_CPP_PGPDEF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpDefMod.obj" : $(SOURCE) $(DEP_CPP_PGPDEF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\text\pgpCompMod.c
DEP_CPP_PGPCOM=\
	".\../../include\pgpCompress.h"\
	".\../../include\pgpPipeline.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\../include\pgpAddHdr.h"\
	".\../include\pgpPktByte.h"\
	".\pipe\text\pgpCompMod.h"\
	".\pipe\text\pgpDefMod.h"\
	".\pipe\text\pgpInfMod.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpCompMod.obj" : $(SOURCE) $(DEP_CPP_PGPCOM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpCompMod.obj" : $(SOURCE) $(DEP_CPP_PGPCOM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpCompMod.obj" : $(SOURCE) $(DEP_CPP_PGPCOM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpCompMod.obj" : $(SOURCE) $(DEP_CPP_PGPCOM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpCompMod.obj" : $(SOURCE) $(DEP_CPP_PGPCOM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpCompMod.obj" : $(SOURCE) $(DEP_CPP_PGPCOM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\utils\pgpSplit.c
DEP_CPP_PGPSP=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\utils\pgpPipeline.h"\
	".\pipe\utils\pgpSplit.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpSplit.obj" : $(SOURCE) $(DEP_CPP_PGPSP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpSplit.obj" : $(SOURCE) $(DEP_CPP_PGPSP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpSplit.obj" : $(SOURCE) $(DEP_CPP_PGPSP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpSplit.obj" : $(SOURCE) $(DEP_CPP_PGPSP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpSplit.obj" : $(SOURCE) $(DEP_CPP_PGPSP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpSplit.obj" : $(SOURCE) $(DEP_CPP_PGPSP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\utils\pgpJoin.c
DEP_CPP_PGPJO=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\utils\pgpJoin.h"\
	".\pipe\utils\pgpPipeline.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpJoin.obj" : $(SOURCE) $(DEP_CPP_PGPJO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpJoin.obj" : $(SOURCE) $(DEP_CPP_PGPJO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpJoin.obj" : $(SOURCE) $(DEP_CPP_PGPJO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpJoin.obj" : $(SOURCE) $(DEP_CPP_PGPJO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpJoin.obj" : $(SOURCE) $(DEP_CPP_PGPJO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpJoin.obj" : $(SOURCE) $(DEP_CPP_PGPJO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\utils\pgpDevNull.c
DEP_CPP_PGPDEV=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\utils\pgpDevNull.h"\
	".\pipe\utils\pgpPipeline.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpDevNull.obj" : $(SOURCE) $(DEP_CPP_PGPDEV) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpDevNull.obj" : $(SOURCE) $(DEP_CPP_PGPDEV) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpDevNull.obj" : $(SOURCE) $(DEP_CPP_PGPDEV) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpDevNull.obj" : $(SOURCE) $(DEP_CPP_PGPDEV) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpDevNull.obj" : $(SOURCE) $(DEP_CPP_PGPDEV) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpDevNull.obj" : $(SOURCE) $(DEP_CPP_PGPDEV) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\utils\pgpCopyMod.c
DEP_CPP_PGPCOP=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\utils\pgpCopyMod.h"\
	".\pipe\utils\pgpPipeline.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpCopyMod.obj" : $(SOURCE) $(DEP_CPP_PGPCOP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpCopyMod.obj" : $(SOURCE) $(DEP_CPP_PGPCOP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpCopyMod.obj" : $(SOURCE) $(DEP_CPP_PGPCOP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpCopyMod.obj" : $(SOURCE) $(DEP_CPP_PGPCOP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpCopyMod.obj" : $(SOURCE) $(DEP_CPP_PGPCOP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpCopyMod.obj" : $(SOURCE) $(DEP_CPP_PGPCOP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\utils\pgpBufMod.c
DEP_CPP_PGPBU=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\utils\pgpBufMod.h"\
	".\pipe\utils\pgpPipeline.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpBufMod.obj" : $(SOURCE) $(DEP_CPP_PGPBU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpBufMod.obj" : $(SOURCE) $(DEP_CPP_PGPBU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpBufMod.obj" : $(SOURCE) $(DEP_CPP_PGPBU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpBufMod.obj" : $(SOURCE) $(DEP_CPP_PGPBU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpBufMod.obj" : $(SOURCE) $(DEP_CPP_PGPBU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpBufMod.obj" : $(SOURCE) $(DEP_CPP_PGPBU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\utils\pgpAddHdr.c
DEP_CPP_PGPAD=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpFIFO.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\../include\pgpPktByte.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\utils\pgpAddHdr.h"\
	".\pipe\utils\pgpPipeline.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpAddHdr.obj" : $(SOURCE) $(DEP_CPP_PGPAD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpAddHdr.obj" : $(SOURCE) $(DEP_CPP_PGPAD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpAddHdr.obj" : $(SOURCE) $(DEP_CPP_PGPAD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpAddHdr.obj" : $(SOURCE) $(DEP_CPP_PGPAD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpAddHdr.obj" : $(SOURCE) $(DEP_CPP_PGPAD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpAddHdr.obj" : $(SOURCE) $(DEP_CPP_PGPAD) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\keys\pgpRngPkt.c
DEP_CPP_PGPRNGPK=\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpFile.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\../include\pgpPktByte.h"\
	".\keys\pgpRngPkt.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpRngPkt.obj" : $(SOURCE) $(DEP_CPP_PGPRNGPK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpRngPkt.obj" : $(SOURCE) $(DEP_CPP_PGPRNGPK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpRngPkt.obj" : $(SOURCE) $(DEP_CPP_PGPRNGPK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpRngPkt.obj" : $(SOURCE) $(DEP_CPP_PGPRNGPK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpRngPkt.obj" : $(SOURCE) $(DEP_CPP_PGPRNGPK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpRngPkt.obj" : $(SOURCE) $(DEP_CPP_PGPRNGPK) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pipe\utils\pgpMemMod.c
DEP_CPP_PGPMEMM=\
	".\../../include\pgpAnnotate.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pipe\utils\pgpMemMod.h"\
	".\pipe\utils\pgpPipeline.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpMemMod.obj" : $(SOURCE) $(DEP_CPP_PGPMEMM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpMemMod.obj" : $(SOURCE) $(DEP_CPP_PGPMEMM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpMemMod.obj" : $(SOURCE) $(DEP_CPP_PGPMEMM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpMemMod.obj" : $(SOURCE) $(DEP_CPP_PGPMEMM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpMemMod.obj" : $(SOURCE) $(DEP_CPP_PGPMEMM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpMemMod.obj" : $(SOURCE) $(DEP_CPP_PGPMEMM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\helper\pgpFileNames.c
DEP_CPP_PGPFILEN=\
	".\../../include\pgpErr.h"\
	".\../..\config.h"\
	".\helper\pgpDebug.h"\
	".\helper\pgpFileNames.h"\
	".\helper\pgpLeaks.h"\
	".\helper\pgpMem.h"\
	".\helper\pgpTypes.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpFileNames.obj" : $(SOURCE) $(DEP_CPP_PGPFILEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpFileNames.obj" : $(SOURCE) $(DEP_CPP_PGPFILEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpFileNames.obj" : $(SOURCE) $(DEP_CPP_PGPFILEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpFileNames.obj" : $(SOURCE) $(DEP_CPP_PGPFILEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpFileNames.obj" : $(SOURCE) $(DEP_CPP_PGPFILEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpFileNames.obj" : $(SOURCE) $(DEP_CPP_PGPFILEN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\helper\pgpFileRef.c
DEP_CPP_PGPFILER=\
	".\../../include\pgpErr.h"\
	".\../../include\pgpFileMod.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\helper\pgpDebug.h"\
	".\helper\pgpFile.h"\
	".\helper\pgpFileNames.h"\
	".\helper\pgpFileRef.h"\
	".\helper\pgpLeaks.h"\
	".\helper\pgpMacFile.h"\
	".\helper\pgpMacUtils.h"\
	".\helper\pgpMem.h"\
	".\helper\pgpTypes.h"\
	{$(INCLUDE)}"\sys\Stat.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	
NODEP_CPP_PGPFILER=\
	".\helper\MacFiles.h"\
	".\helper\MacStrings.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpFileRef.obj" : $(SOURCE) $(DEP_CPP_PGPFILER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpFileRef.obj" : $(SOURCE) $(DEP_CPP_PGPFILER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpFileRef.obj" : $(SOURCE) $(DEP_CPP_PGPFILER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpFileRef.obj" : $(SOURCE) $(DEP_CPP_PGPFILER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpFileRef.obj" : $(SOURCE) $(DEP_CPP_PGPFILER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpFileRef.obj" : $(SOURCE) $(DEP_CPP_PGPFILER) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pubkey\pgpAltRSAGlu.c
DEP_CPP_PGPAL=\
	".\../../include\bn.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\pubkey\pgpKeyMisc.h"\
	".\pubkey\pgpRSAGlue.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpAltRSAGlu.obj" : $(SOURCE) $(DEP_CPP_PGPAL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpAltRSAGlu.obj" : $(SOURCE) $(DEP_CPP_PGPAL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpAltRSAGlu.obj" : $(SOURCE) $(DEP_CPP_PGPAL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpAltRSAGlu.obj" : $(SOURCE) $(DEP_CPP_PGPAL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpAltRSAGlu.obj" : $(SOURCE) $(DEP_CPP_PGPAL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpAltRSAGlu.obj" : $(SOURCE) $(DEP_CPP_PGPAL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\hash\pgpRIPEMD160.c
DEP_CPP_PGPRI=\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\hash\pgpHash.h"\
	".\hash\pgpRIPEMD160.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpRIPEMD160.obj" : $(SOURCE) $(DEP_CPP_PGPRI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpRIPEMD160.obj" : $(SOURCE) $(DEP_CPP_PGPRI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpRIPEMD160.obj" : $(SOURCE) $(DEP_CPP_PGPRI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpRIPEMD160.obj" : $(SOURCE) $(DEP_CPP_PGPRI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpRIPEMD160.obj" : $(SOURCE) $(DEP_CPP_PGPRI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpRIPEMD160.obj" : $(SOURCE) $(DEP_CPP_PGPRI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pubkey\pgpRSAKeyGen.c
DEP_CPP_PGPRSA=\
	".\../../include\bn.h"\
	".\../../include\bnprime.h"\
	".\../../include\pgpCFB.h"\
	".\../../include\pgpCipher.h"\
	".\../../include\pgpDebug.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpHash.h"\
	".\../../include\pgpMem.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpStr2Key.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\..\..\include\pgpLeaks.h"\
	".\pubkey\pgpESK.h"\
	".\pubkey\pgpKeyMisc.h"\
	".\pubkey\pgpPubKey.h"\
	".\pubkey\pgpRSAGlue.h"\
	".\pubkey\pgpRSAKey.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpRSAKeyGen.obj" : $(SOURCE) $(DEP_CPP_PGPRSA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpRSAKeyGen.obj" : $(SOURCE) $(DEP_CPP_PGPRSA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpRSAKeyGen.obj" : $(SOURCE) $(DEP_CPP_PGPRSA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpRSAKeyGen.obj" : $(SOURCE) $(DEP_CPP_PGPRSA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpRSAKeyGen.obj" : $(SOURCE) $(DEP_CPP_PGPRSA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpRSAKeyGen.obj" : $(SOURCE) $(DEP_CPP_PGPRSA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pubkey\pgpRSAGlue2.c
DEP_CPP_PGPRSAG=\
	".\../../include\bn.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\pubkey\pgpESK.h"\
	".\pubkey\pgpKeyMisc.h"\
	".\pubkey\pgpRSAGlue.h"\
	".\pubkey\pgpRSAKey.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	
NODEP_CPP_PGPRSAG=\
	".\pubkey\bnprint.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpRSAGlue2.obj" : $(SOURCE) $(DEP_CPP_PGPRSAG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpRSAGlue2.obj" : $(SOURCE) $(DEP_CPP_PGPRSAG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpRSAGlue2.obj" : $(SOURCE) $(DEP_CPP_PGPRSAG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpRSAGlue2.obj" : $(SOURCE) $(DEP_CPP_PGPRSAG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpRSAGlue2.obj" : $(SOURCE) $(DEP_CPP_PGPRSAG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpRSAGlue2.obj" : $(SOURCE) $(DEP_CPP_PGPRSAG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pubkey\pgpRSAGlue1.c
DEP_CPP_PGPRSAGL=\
	".\../../include\bn.h"\
	".\../../include\pgpErr.h"\
	".\../../include\pgpRndom.h"\
	".\../../include\pgpTypes.h"\
	".\../../include\pgpUsuals.h"\
	".\../..\config.h"\
	".\pubkey\pgpESK.h"\
	".\pubkey\pgpKeyMisc.h"\
	".\pubkey\pgpRSAGlue.h"\
	".\pubkey\pgpRSAKey.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	
NODEP_CPP_PGPRSAGL=\
	".\pubkey\bnprint.h"\
	

!IF  "$(CFG)" == "pgp - Win32 Release"


"$(INTDIR)\pgpRSAGlue1.obj" : $(SOURCE) $(DEP_CPP_PGPRSAGL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 Debug"


"$(INTDIR)\pgpRSAGlue1.obj" : $(SOURCE) $(DEP_CPP_PGPRSAGL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Debug"


"$(INTDIR)\pgpRSAGlue1.obj" : $(SOURCE) $(DEP_CPP_PGPRSAGL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 RSAREF Release"


"$(INTDIR)\pgpRSAGlue1.obj" : $(SOURCE) $(DEP_CPP_PGPRSAGL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Debug"


"$(INTDIR)\pgpRSAGlue1.obj" : $(SOURCE) $(DEP_CPP_PGPRSAGL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "pgp - Win32 NO_RSA Release"


"$(INTDIR)\pgpRSAGlue1.obj" : $(SOURCE) $(DEP_CPP_PGPRSAGL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
