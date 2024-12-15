#
# Turbo C makefile for DES486.
#

mapname=des.map

# The following line defines whether debug info is compiled into exe:
#testing=1

!if $d(testing)
debug= -v
ldebug= /v
#asmdebug= /zi
!endif

.c.obj:
    tcc -ms $(debug) -c $<

.asm.obj:
    tasm /ml /m2 /l $(asmdebug) $<
    tasm /dTRIPLE_DES /ml /m2 /l $(asmdebug) $<,3$*

all: des.exe password.exe destime.exe validate.exe 3des.exe

des.exe: des.obj table.obj des486.obj cpu.obj
 tlink /3 /s $(ldebug) c0s table des486 des cpu,des,$(mapname),cs

password.exe: password.obj des486.obj md5.obj
 tlink $(ldebug) c0s password md5 des486 cpu table,password,,cs

destime.exe: destime.obj des486.obj table.obj
 tlink /3 $(ldebug) c0s destime des486 table cpu,destime,,cs emu maths
#note that we emulate the fp in this file, because 486SXs don't have fp.

validate.exe: validate.obj table.obj des486.obj
 tlink /3 /s $(ldebug) c0s table des486 validate cpu,validate,,cs

3des.exe: 3des.obj getpass.obj 3table.obj 3des486.obj cpu.obj md5.obj
 tlink /3 /s $(ldebug) c0s 3table 3des486 3des getpass cpu md5, 3des, 3$(mapname),cs

des486.obj: des486.asm table.asm des.h

des.obj: des.h des.c
