# this makefile is for maintenance

# DOS command-line length stupidity.

RCS1 = winvn.c wvblock.c wvcncm.c wvfile.c wvgroup.c wvheader.c headarry.c
RCS2 = wvpost.c wvscreen.c wvusenet.c wvutil.c ini_news.c wvsckgen.c
RCS3 = wvart.c wvlist.c wvmail.c wvauth.c shellsor.c wvprint.c wvmapi.c wvcrypt.c
RCS4 = winvn.h wvglob.h wvdlg.h wvmapi.h defaults.h resource.h winvn.rc
RCS5 = wvattach.c wvcoding.c wvtxtblk.c wvcodewn.c primes.h

RCSFILES = $(RCS1) $(RCS2) $(RCS3) $(RCS4) $(RCS5)

all:
	nmake -f winvngen.mak

release:
	nmake -f winvngen.mak DEBUG=0

winvn32:
	nmake -f makefile.nt

# how many junk files can there be ???

clean:
	-del *.obj
	-del *.lib
	-del *.exp
	-del *.res
	-del *.dll
	-del *.bsc
	-del *.map
	-del *.sbr
	-del *.bsc
	-del *.pdb
	-del *.exe
	rm *~

zip:
	-del \winvnsrc.zip
	zip -r \winvnsrc.zip .

.SUFFIXES: .obj .sbr .res .c .rc

# Rules to create files from RCS database

$(RCSFILES):
	co $@

version:
	echo static char * version_string[] = {>version.c
	ident $(RCS1) | mawk -f version.awk >>version.c
	ident $(RCS2) | mawk -f version.awk >>version.c
	ident $(RCS3) | mawk -f version.awk >>version.c
	ident $(RCS4) | mawk -f version.awk >>version.c
	ident $(RCS5) | mawk -f version.awk >>version.c
	echo }; >>version.c

checkout: $(RCSFILES)

rcsclean:
   rm -f $(RCS1)
   rm -f $(RCS2)
   rm -f $(RCS3)
   rm -f $(RCS4)
   rm -f $(RCS5)
