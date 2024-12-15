!include <ntwin32.MAK>

MODOBJS= arraymodule.obj mathmodule.obj parsermodule.obj regexmodule.obj \
	regexpr.obj stropmodule.obj structmodule.obj \
	md5module.obj md5c.obj rotormodule.obj

MODLIBS=	

srcdir=		..

DEFS=		-DHAVE_CONFIG_H

INCLDIR=	$(srcdir)\Include
OPT=		$(cvars) $(cdebug) $(cflags)
CFLAGS=		$(OPT) -I$(INCLDIR) -I.. $(DEFS)

MAKESETUP=	$(srcdir)/makesetup

OBJS=		$(MODOBJS)

XLIB=		Modules.lib

MYLIBS=		$(XLIB) \
		..\Python\Python.lib \
		..\Objects\Objects.lib \
		..\Parser\Parser.lib

all:		$(XLIB) python.exe

$(XLIB):	$(OBJS)
		$(implib) -out:$(XLIB) $(OBJS)

python.exe:	config.obj $(MYLIBS)
		$(link) config.obj $(linkdebug) $(conflags) $(conlibs) \
			$(MYLIBS) $(MODLIBS) $(LIBS) $(SYSLIBS) -out:python.exe
		copy python.exe ..

config.obj:	config.c Makefile
		$(CC) $(CFLAGS) -c config.c

clean:
		del *.obj *.lib *~ python.exe

almodule.obj: almodule.c
arraymodule.obj: arraymodule.c
audioopmodule.obj: audioopmodule.c
cdmodule.obj: cdmodule.c
clmodule.obj: clmodule.c
dbmmodule.obj: dbmmodule.c
fcntlmodule.obj: fcntlmodule.c
flmodule.obj: flmodule.c
fmmodule.obj: fmmodule.c
glmodule.obj: glmodule.c
imageopmodule.obj: imageopmodule.c
imgfilemodule.obj: imgfilemodule.c
mathmodule.obj: mathmodule.c
md5c.obj: md5c.c
md5module.obj: md5module.c
mpzmodule.obj: mpzmodule.c
nismodule.obj: nismodule.c
parsermodule.obj: parsermodule.c
posixmodule.obj: posixmodule.c
pwdmodule.obj: pwdmodule.c
regexmodule.obj: regexmodule.c
regexpr.obj: regexpr.c
rgbimgmodule.obj: rgbimgmodule.c
rotormodule.obj: rotormodule.c
selectmodule.obj: selectmodule.c
sgimodule.obj: sgimodule.c
socketmodule.obj: socketmodule.c
stdwinmodule.obj: stdwinmodule.c
stropmodule.obj: stropmodule.c
structmodule.obj: structmodule.c
sunaudiodevmodule.obj: sunaudiodevmodule.c
svmodule.obj: svmodule.c
threadmodule.obj: threadmodule.c
timemodule.obj: timemodule.c
timingmodule.obj: timingmodule.c
xxmodule.obj: xxmodule.c
yuvconvert.obj: yuvconvert.c

