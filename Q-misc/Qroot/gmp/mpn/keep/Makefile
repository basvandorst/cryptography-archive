# Makefile for mpn layer
# This is a sub-makefile of ../Makefile.
# BOTCH: Going via tmp.s makes parallel makes loooose!!!
# BOTCH: Dependecies should be created.  Machine-dependent!!
# BOTCH: We must support extra objs on a per-configuration basis.

mc68020_DIRS = mc68020:mc68000
mc68020_sun_CPPFLAGS = -DMIT_SYNTAX
hppa1.1_DIRS = hppa1_1:hppa1_0
hppa1.0_DIRS = hppa1_0
sparc_DIRS = sparc
sparc8_DIRS = sparc8:sparc
DIRS = ${$(TCPU)_DIRS}
TCFLAGS = ${$(TCPU)_$(TMACH)_CFLAGS}
TCPPFLAGS = ${$(TCPU)_$(TMACH)_CPPFLAGS}

CC = gcc
CPP = /lib/cpp
CFLAGS = -O -g $(OPT) $(TCFLAGS)
CPPFLAGS = $(TCPPFLAGS)
TCPU = sparc
TMACH = sun
VPATH = ${DIRS}:generic

OBJS = add_1.o add_n.o addmul_1.o cmp.o divmod.o divmod_1.o \
 lshift.o mod_1.o mul.o mul_1.o mul_n.o random2.o rshift.o \
 sqrt.o sub_1.o sub_n.o submul_1.o get_str.o set_str.o

INCLUDES = -I.. -I$(TCPU) -Igeneric

.SUFFIXES: .S .s .c

%.o : %.S
	$(CPP) $(CPPFLAGS) $< tmp-$*.s
	$(CC) -c tmp-$*.s -o $@
	rm -f tmp-$*.s
%.o : %.s
	$(CC) -c $(CFLAGS) $<
%.o : %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $(INCLUDES) $<

AR = ar
RANLIB = ranlib

libmpn.a: $(OBJS)
	rm -f $@
	$(AR) cr $@ $(OBJS)
	$(RANLIB) $@

.PHONY: clean
clean:
	rm -f $(OBJS) libmpn.a tmp-*.s

echo:
	@echo DIRS=$(DIRS)
	@echo VPATH=$(VPATH)
	@echo TCPU=$(TCPU)
	@echo ${$(TCPU)_DIRS}
