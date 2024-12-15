PROJ = WINVN
PROJFILE = WINVN.MAK
DEBUG = 1

PWBRMAKE  = pwbrmake
NMAKEBSC1  = set
NMAKEBSC2  = nmake
CC  = cl
CFLAGS_G  = /W2 /G2 /Gw /Zp -EM
CFLAGS_D  = /qc /Oa /Zi
CFLAGS_R  = /Oe /Og /Os /Gs /FPa
LLIBS_R  =  /NOD:SLIBCA SLIBCAW
LLIBS_D  =  /NOD:SLIBCE SLIBCEW
LLIBS_G  = LIBW.LIB
ASM  = masm
AFLAGS_G  = /Mx /T
AFLAGS_D  = /Zi
LINKER  = link
ILINK  = ilink
LRF  = echo > NUL
BIND  = bind
RC  = rc
IMPLIB  = implib
LFLAGS_G  =   /BATCH/NOE
LFLAGS_D  =  /CO /M /INC /NOF /NOP
LFLAGS_R  = /EXE /NOF /BATCH
MAPFILE_D  = $(PROJ).map
MAPFILE_R  = NUL
BRFLAGS  =  /o $(PROJ).bsc
BROWSE  = 0

DEF_FILE  = WINVN.DEF
OBJS  = PCEMUL.obj WINVN.obj WVBLOCK.obj WVCNCM.obj WVDOC.obj WVFILE.obj\
        WVGROUP.obj WVHEADER.obj WVPOST.obj WVSCREEN.obj WVSOCK.obj\
        WVUSENET.obj WVUTIL.obj WVART.obj WVAUTH.obj FIND_VEc.obj VEC_SRCh.obj\
        WVLIST.obj SHELLSOR.obj WVMAIL.obj
LIBS_EXT  = ..\ftpdev\NETMSC5.1\sPC.LIB ..\FTPDEV\NETMSC5.1\sSOCKET.LIB\
        ..\ftpdev\NETMSC5.1\sNETLIB.LIB
LIBS  = $(LIBS_EXT)
RESS  = winvn.res
SBRS  = PCEMUL.sbr WINVN.sbr WVBLOCK.sbr WVCNCM.sbr WVDOC.sbr WVFILE.sbr\
        WVGROUP.sbr WVHEADER.sbr WVPOST.sbr WVSCREEN.sbr WVSOCK.sbr\
        WVUSENET.sbr WVUTIL.sbr WVART.sbr WVAUTH.sbr FIND_VEc.sbr VEC_SRCh.sbr\
        WVLIST.sbr SHELLSOR.sbr WVMAIL.sbr

all: $(PROJ).exe

.SUFFIXES:
.SUFFIXES: .sbr .obj .res .c .rc

PCEMUL.obj : PCEMUL.C c:\windev\include\windows.h wvglob.h\
        C:\C600\INCLUDE\stdio.h C:\C600\INCLUDE\string.h
!IF $(DEBUG)
        $(CC) /c $(CFLAGS_G) $(CFLAGS_D) /FoPCEMUL.obj PCEMUL.C
!ELSE
        $(CC) /c $(CFLAGS_G) $(CFLAGS_R) /FoPCEMUL.obj PCEMUL.C
!ENDIF

PCEMUL.sbr : PCEMUL.C c:\windev\include\windows.h wvglob.h\
        C:\C600\INCLUDE\stdio.h C:\C600\INCLUDE\string.h
!IF $(DEBUG)
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_D) /FRPCEMUL.sbr PCEMUL.C
!ELSE
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_R) /FRPCEMUL.sbr PCEMUL.C
!ENDIF

WINVN.obj : WINVN.C c:\windev\include\windows.h wvglob.h WinVn.h winundoc.h\
        C:\C600\INCLUDE\ctype.h C:\C600\INCLUDE\string.h wvdlg.h
!IF $(DEBUG)
        $(CC) /c $(CFLAGS_G) $(CFLAGS_D) /FoWINVN.obj WINVN.C
!ELSE
        $(CC) /c $(CFLAGS_G) $(CFLAGS_R) /FoWINVN.obj WINVN.C
!ENDIF

WINVN.sbr : WINVN.C c:\windev\include\windows.h wvglob.h WinVn.h winundoc.h\
        C:\C600\INCLUDE\ctype.h C:\C600\INCLUDE\string.h wvdlg.h
!IF $(DEBUG)
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_D) /FRWINVN.sbr WINVN.C
!ELSE
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_R) /FRWINVN.sbr WINVN.C
!ENDIF

WVBLOCK.obj : WVBLOCK.C c:\windev\include\windows.h wvglob.h WinVn.h\
        C:\C600\INCLUDE\ctype.h C:\C600\INCLUDE\string.h wvdlg.h
!IF $(DEBUG)
        $(CC) /c $(CFLAGS_G) $(CFLAGS_D) /FoWVBLOCK.obj WVBLOCK.C
!ELSE
        $(CC) /c $(CFLAGS_G) $(CFLAGS_R) /FoWVBLOCK.obj WVBLOCK.C
!ENDIF

WVBLOCK.sbr : WVBLOCK.C c:\windev\include\windows.h wvglob.h WinVn.h\
        C:\C600\INCLUDE\ctype.h C:\C600\INCLUDE\string.h wvdlg.h
!IF $(DEBUG)
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_D) /FRWVBLOCK.sbr WVBLOCK.C
!ELSE
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_R) /FRWVBLOCK.sbr WVBLOCK.C
!ENDIF

WVCNCM.obj : WVCNCM.C C:\C600\INCLUDE\stdlib.h c:\windev\include\windows.h\
        wvglob.h WinVn.h C:\C600\INCLUDE\string.h wvdlg.h
!IF $(DEBUG)
        $(CC) /c $(CFLAGS_G) $(CFLAGS_D) /FoWVCNCM.obj WVCNCM.C
!ELSE
        $(CC) /c $(CFLAGS_G) $(CFLAGS_R) /FoWVCNCM.obj WVCNCM.C
!ENDIF

WVCNCM.sbr : WVCNCM.C C:\C600\INCLUDE\stdlib.h c:\windev\include\windows.h\
        wvglob.h WinVn.h C:\C600\INCLUDE\string.h wvdlg.h
!IF $(DEBUG)
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_D) /FRWVCNCM.sbr WVCNCM.C
!ELSE
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_R) /FRWVCNCM.sbr WVCNCM.C
!ENDIF

WVDOC.obj : WVDOC.C
!IF $(DEBUG)
        $(CC) /c $(CFLAGS_G) $(CFLAGS_D) /FoWVDOC.obj WVDOC.C
!ELSE
        $(CC) /c $(CFLAGS_G) $(CFLAGS_R) /FoWVDOC.obj WVDOC.C
!ENDIF

WVDOC.sbr : WVDOC.C
!IF $(DEBUG)
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_D) /FRWVDOC.sbr WVDOC.C
!ELSE
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_R) /FRWVDOC.sbr WVDOC.C
!ENDIF

WVFILE.obj : WVFILE.C c:\windev\include\windows.h wvglob.h WinVn.h winundoc.h\
        C:\C600\INCLUDE\io.h C:\C600\INCLUDE\fcntl.h C:\C600\INCLUDE\string.h\
        wvdlg.h
!IF $(DEBUG)
        $(CC) /c $(CFLAGS_G) $(CFLAGS_D) /FoWVFILE.obj WVFILE.C
!ELSE
        $(CC) /c $(CFLAGS_G) $(CFLAGS_R) /FoWVFILE.obj WVFILE.C
!ENDIF

WVFILE.sbr : WVFILE.C c:\windev\include\windows.h wvglob.h WinVn.h winundoc.h\
        C:\C600\INCLUDE\io.h C:\C600\INCLUDE\fcntl.h C:\C600\INCLUDE\string.h\
        wvdlg.h
!IF $(DEBUG)
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_D) /FRWVFILE.sbr WVFILE.C
!ELSE
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_R) /FRWVFILE.sbr WVFILE.C
!ENDIF

WVGROUP.obj : WVGROUP.C c:\windev\include\windows.h winundoc.h wvglob.h\
        WinVn.h C:\C600\INCLUDE\string.h wvdlg.h
!IF $(DEBUG)
        $(CC) /c $(CFLAGS_G) $(CFLAGS_D) /FoWVGROUP.obj WVGROUP.C
!ELSE
        $(CC) /c $(CFLAGS_G) $(CFLAGS_R) /FoWVGROUP.obj WVGROUP.C
!ENDIF

WVGROUP.sbr : WVGROUP.C c:\windev\include\windows.h winundoc.h wvglob.h\
        WinVn.h C:\C600\INCLUDE\string.h wvdlg.h
!IF $(DEBUG)
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_D) /FRWVGROUP.sbr WVGROUP.C
!ELSE
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_R) /FRWVGROUP.sbr WVGROUP.C
!ENDIF

WVHEADER.obj : WVHEADER.C c:\windev\include\windows.h wvglob.h WinVn.h\
        winundoc.h C:\C600\INCLUDE\ctype.h C:\C600\INCLUDE\time.h\
        C:\C600\INCLUDE\string.h wvdlg.h
!IF $(DEBUG)
        $(CC) /c $(CFLAGS_G) $(CFLAGS_D) /FoWVHEADER.obj WVHEADER.C
!ELSE
        $(CC) /c $(CFLAGS_G) $(CFLAGS_R) /FoWVHEADER.obj WVHEADER.C
!ENDIF

WVHEADER.sbr : WVHEADER.C c:\windev\include\windows.h wvglob.h WinVn.h\
        winundoc.h C:\C600\INCLUDE\ctype.h C:\C600\INCLUDE\time.h\
        C:\C600\INCLUDE\string.h wvdlg.h
!IF $(DEBUG)
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_D) /FRWVHEADER.sbr WVHEADER.C
!ELSE
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_R) /FRWVHEADER.sbr WVHEADER.C
!ENDIF

WVPOST.obj : WVPOST.C c:\windev\include\windows.h wvglob.h WinVn.h winundoc.h\
        C:\C600\INCLUDE\string.h wvdlg.h
!IF $(DEBUG)
        $(CC) /c $(CFLAGS_G) $(CFLAGS_D) /FoWVPOST.obj WVPOST.C
!ELSE
        $(CC) /c $(CFLAGS_G) $(CFLAGS_R) /FoWVPOST.obj WVPOST.C
!ENDIF

WVPOST.sbr : WVPOST.C c:\windev\include\windows.h wvglob.h WinVn.h winundoc.h\
        C:\C600\INCLUDE\string.h wvdlg.h
!IF $(DEBUG)
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_D) /FRWVPOST.sbr WVPOST.C
!ELSE
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_R) /FRWVPOST.sbr WVPOST.C
!ENDIF

WVSCREEN.obj : WVSCREEN.C c:\windev\include\windows.h wvglob.h WinVn.h\
        C:\C600\INCLUDE\string.h wvdlg.h
!IF $(DEBUG)
        $(CC) /c $(CFLAGS_G) $(CFLAGS_D) /FoWVSCREEN.obj WVSCREEN.C
!ELSE
        $(CC) /c $(CFLAGS_G) $(CFLAGS_R) /FoWVSCREEN.obj WVSCREEN.C
!ENDIF

WVSCREEN.sbr : WVSCREEN.C c:\windev\include\windows.h wvglob.h WinVn.h\
        C:\C600\INCLUDE\string.h wvdlg.h
!IF $(DEBUG)
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_D) /FRWVSCREEN.sbr WVSCREEN.C
!ELSE
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_R) /FRWVSCREEN.sbr WVSCREEN.C
!ENDIF

WVSOCK.obj : WVSOCK.C c:\windev\include\windows.h wvglob.h WinVn.h\
        c:\ftpdev\include\pctcp\types.h c:\ftpdev\include\netinet\types.h\
        c:\ftpdev\include\netinet\in.h c:\ftpdev\include\arpa\inet.h\
        c:\ftpdev\include\pctcp\pctcp.h c:\ftpdev\include\pctcp\options.h\
        c:\ftpdev\include\pctcp\error.h C:\C600\INCLUDE\memory.h\
        C:\C600\INCLUDE\dos.h C:\C600\INCLUDE\string.h wvdlg.h\
        c:\ftpdev\include\4bsddefs.h C:\C600\INCLUDE\sys\types.h\
        c:\ftpdev\include\sys\time.h
!IF $(DEBUG)
        $(CC) /c $(CFLAGS_G) $(CFLAGS_D) /FoWVSOCK.obj WVSOCK.C
!ELSE
        $(CC) /c $(CFLAGS_G) $(CFLAGS_R) /FoWVSOCK.obj WVSOCK.C
!ENDIF

WVSOCK.sbr : WVSOCK.C c:\windev\include\windows.h wvglob.h WinVn.h\
        c:\ftpdev\include\pctcp\types.h c:\ftpdev\include\netinet\types.h\
        c:\ftpdev\include\netinet\in.h c:\ftpdev\include\arpa\inet.h\
        c:\ftpdev\include\pctcp\pctcp.h c:\ftpdev\include\pctcp\options.h\
        c:\ftpdev\include\pctcp\error.h C:\C600\INCLUDE\memory.h\
        C:\C600\INCLUDE\dos.h C:\C600\INCLUDE\string.h wvdlg.h\
        c:\ftpdev\include\4bsddefs.h C:\C600\INCLUDE\sys\types.h\
        c:\ftpdev\include\sys\time.h
!IF $(DEBUG)
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_D) /FRWVSOCK.sbr WVSOCK.C
!ELSE
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_R) /FRWVSOCK.sbr WVSOCK.C
!ENDIF

WVUSENET.obj : WVUSENET.C c:\windev\include\windows.h winundoc.h\
        C:\C600\INCLUDE\stdlib.h wvglob.h WinVn.h C:\C600\INCLUDE\string.h\
        wvdlg.h
!IF $(DEBUG)
        $(CC) /c $(CFLAGS_G) $(CFLAGS_D) /FoWVUSENET.obj WVUSENET.C
!ELSE
        $(CC) /c $(CFLAGS_G) $(CFLAGS_R) /FoWVUSENET.obj WVUSENET.C
!ENDIF

WVUSENET.sbr : WVUSENET.C c:\windev\include\windows.h winundoc.h\
        C:\C600\INCLUDE\stdlib.h wvglob.h WinVn.h C:\C600\INCLUDE\string.h\
        wvdlg.h
!IF $(DEBUG)
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_D) /FRWVUSENET.sbr WVUSENET.C
!ELSE
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_R) /FRWVUSENET.sbr WVUSENET.C
!ENDIF

WVUTIL.obj : WVUTIL.C c:\windev\include\windows.h wvglob.h WinVn.h winundoc.h\
        C:\C600\INCLUDE\ctype.h C:\C600\INCLUDE\string.h wvdlg.h
!IF $(DEBUG)
        $(CC) /c $(CFLAGS_G) $(CFLAGS_D) /FoWVUTIL.obj WVUTIL.C
!ELSE
        $(CC) /c $(CFLAGS_G) $(CFLAGS_R) /FoWVUTIL.obj WVUTIL.C
!ENDIF

WVUTIL.sbr : WVUTIL.C c:\windev\include\windows.h wvglob.h WinVn.h winundoc.h\
        C:\C600\INCLUDE\ctype.h C:\C600\INCLUDE\string.h wvdlg.h
!IF $(DEBUG)
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_D) /FRWVUTIL.sbr WVUTIL.C
!ELSE
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_R) /FRWVUTIL.sbr WVUTIL.C
!ENDIF

WVART.obj : WVART.C c:\windev\include\windows.h winundoc.h wvglob.h WinVn.h\
        C:\C600\INCLUDE\string.h wvdlg.h

WVART.sbr : WVART.C c:\windev\include\windows.h winundoc.h wvglob.h WinVn.h\
        C:\C600\INCLUDE\string.h wvdlg.h

WVAUTH.obj : WVAUTH.C c:\windev\include\windows.h

WVAUTH.sbr : WVAUTH.C c:\windev\include\windows.h

FIND_VEc.obj : FIND_VEc.C c:\ftpdev\include\pctcp\types.h\
        c:\ftpdev\include\pctcp\pctcp.h

FIND_VEc.sbr : FIND_VEc.C c:\ftpdev\include\pctcp\types.h\
        c:\ftpdev\include\pctcp\pctcp.h

VEC_SRCh.obj : VEC_SRCh.C C:\C600\INCLUDE\dos.h

VEC_SRCh.sbr : VEC_SRCh.C C:\C600\INCLUDE\dos.h

WVLIST.obj : WVLIST.C c:\windev\include\windows.h wvglob.h WinVn.h winundoc.h\
        C:\C600\INCLUDE\ctype.h C:\C600\INCLUDE\string.h wvdlg.h

WVLIST.sbr : WVLIST.C c:\windev\include\windows.h wvglob.h WinVn.h winundoc.h\
        C:\C600\INCLUDE\ctype.h C:\C600\INCLUDE\string.h wvdlg.h

SHELLSOR.obj : SHELLSOR.C C:\C600\INCLUDE\stdio.h C:\C600\INCLUDE\string.h

SHELLSOR.sbr : SHELLSOR.C C:\C600\INCLUDE\stdio.h C:\C600\INCLUDE\string.h

winvn.res : winvn.rc c:\windev\include\windows.h WinVn.h winvn.dlg wvcncm.dlg\
        saveart.dlg wvfind.dlg list.dlg grplist.dlg wvdlg.h

WVMAIL.obj : WVMAIL.C c:\windev\include\windows.h wvglob.h WinVn.h winundoc.h\
        C:\C600\INCLUDE\string.h wvdlg.h

WVMAIL.sbr : WVMAIL.C c:\windev\include\windows.h wvglob.h WinVn.h winundoc.h\
        C:\C600\INCLUDE\string.h wvdlg.h


$(PROJ).bsc : $(SBRS)
        $(PWBRMAKE) @<<
$(BRFLAGS) $(SBRS)
<<

$(PROJ).exe : $(DEF_FILE) $(OBJS) $(LIBS) $(RESS)
!IF $(DEBUG)
        $(LRF) @<<$(PROJ).lrf
$(RT_OBJS: = +^
) $(OBJS: = +^
)
$@
$(MAPFILE_D)
$(LLIBS_G: = +^
) +
$(LLIBS_D: = +^
) +
$(LIBS: = +^
)
$(DEF_FILE) $(LFLAGS_G) $(LFLAGS_D);
<<
!ELSE
        $(LRF) @<<$(PROJ).lrf
$(RT_OBJS: = +^
) $(OBJS: = +^
)
$@
$(MAPFILE_R)
$(LLIBS_G: = +^
) +
$(LLIBS_R: = +^
) +
$(LIBS: = +^
)
$(DEF_FILE) $(LFLAGS_G) $(LFLAGS_R);
<<
!ENDIF
!IF $(DEBUG)
        $(ILINK) -a -e "$(LINKER) @$(PROJ).lrf" $@
!ELSE
        $(LINKER) @$(PROJ).lrf
!ENDIF
        $(RC) $(RESS) $@


.c.sbr :
!IF $(DEBUG)
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_D) /FR$@ $<
!ELSE
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_R) /FR$@ $<
!ENDIF

.c.obj :
!IF $(DEBUG)
        $(CC) /c $(CFLAGS_G) $(CFLAGS_D) /Fo$@ $<
!ELSE
        $(CC) /c $(CFLAGS_G) $(CFLAGS_R) /Fo$@ $<
!ENDIF

.rc.res :
        $(RC) /r $<


run: $(PROJ).exe
        WIN $(PROJ).exe $(RUNFLAGS)

debug: $(PROJ).exe
        WIN CVW $(CVFLAGS) $(PROJ).exe $(RUNFLAGS)
