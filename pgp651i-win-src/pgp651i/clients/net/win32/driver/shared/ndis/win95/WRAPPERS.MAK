DDKROOT=$(95DDKROOT)
AFLAGS  = -coff -W2 -c -Cx -DBLD_COFF -DIS_32 -DMASM6 -Sg -I$(DDKROOT)\base\vxdwraps

OBJS    = wrapper1.obj wrapper2.obj wrapper3.obj wrapper4.obj wrapper5.obj wrapper6.obj
LIBING  = $(OBJS: =&^
)
LIBING  = $(LIBING:&=)

target: wrappers.clb

wrappers.clb: always $(OBJS)
        if exist wrappers.clb lib @<<wrappers.lnk
/out:wrappers.clb
wrappers.clb
$(LIBING)
<<
        if not exist wrappers.clb lib @<<wrappers.lnk
/out:wrappers.clb
$(LIBING)
<<

wrapper1.obj: wrappers.asm
        ml $(AFLAGS) -DSEGNUM=1 -Fo$*.obj wrappers.asm

wrapper2.obj: wrappers.asm
        ml $(AFLAGS) -DSEGNUM=2 -Fo$*.obj wrappers.asm

wrapper3.obj: wrappers.asm
        ml $(AFLAGS) -DSEGNUM=3 -Fo$*.obj wrappers.asm

wrapper4.obj: wrappers.asm
        ml $(AFLAGS) -DSEGNUM=4 -Fo$*.obj wrappers.asm

wrapper5.obj: wrappers.asm
        ml $(AFLAGS) -DSEGNUM=5 -Fo$*.obj wrappers.asm

wrapper6.obj: wrappers.asm
        ml $(AFLAGS) -DSEGNUM=6 -Fo$*.obj wrappers.asm

always:
	@rem echo pseudotarget
