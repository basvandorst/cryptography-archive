!include <ntwin32.MAK>

wslib = wsock32.lib

TARGET = gensock

OBJS = gensock.obj

.SUFFIXES:	.cpp

.cpp.obj:
	$(cc) $(DEFS) $(cflags) $(cvarsdll) $(cdebug) $<

all:	gensock.dll

$(TARGET).lib $(TARGET).exp : gensck32.def
	$(implib) -out:$(TARGET).lib $(OBJS) -machine:$(CPU) -def:gensck32.def

$(TARGET).dll : $(OBJS) $(TARGET).exp $(TARGET).res
	$(link) -out:$(TARGET).dll -dll -entry:dll_entry_point$(DLLENTRY) \
		-debug:full -debugtype:cv -subsystem:windows \
		$(LINK_FLAGS) $(TARGET).rbj $(OBJS) $(TARGET).exp\
		$(guilibsdll) wsock32.lib

# Update the resource if necessary
$(TARGET).res: $(TARGET).rc
	rc -r -fo $(TARGET).res $(cvars) $(TARGET).rc
!IFDEF CPUTYPE
    cvtres -$(CPU) $(TARGET).res -o $(TARGET).rbj
!ENDIF

clean:
	del *.obj *.lib *.exp *.dll *~
