# Makefile for KFS
#
# Copyright (c) 1993 by William W. Dorsey
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 1, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

#CPPFLAGS=	-nostdinc -I/sys -I/sys/sun3 -I/usr/include -DKERNEL
CPPFLAGS=	-nostdinc -I/sys -I/sys/sun4c -I/usr/include -DKERNEL
#CPPFLAGS=	-DKERNEL
CC=gcc
CC_OPTIM=	-O
CFLAGS=	-g -Wall ${CC_OPTIM}	#-msoft-fp


KFS_SRC=	kfs_vfsops.c kfs_vnodeops.c kfs_uiorw.c \
		kfs_subr.c kfs_acl.c kfs_sys.c md5c.c \
		crypt.c des.c
MNTK_SRC=	mount_kfs.c
KCTL_SRC=	kfsctl.c kfsutil.c
GA_SRC=		getacl.c
SA_SRC=		setacl.c

SRC=		${KFS_SRC} ${MNTK_SRC} ${KCTL_SRC} ${GA_SRC} ${SA_SRC}

KFS_OBJ=	${KFS_SRC:%.c=%.o}
MNTK_OBJ=	${MNTK_SRC:%.c=%.o}
KCTL_OBJ=	${KCTL_SRC:%.c=%.o}
SRV_OBJ=	${SRV_SRC:%.c=%.o}
GA_OBJ=		${GA_SRC:%.c=%.o}
SA_OBJ=		${SA_SRC:%.c=%.o}

OBJ=		${KFS_OBJ} ${MNTK_OBJ} ${SK_OBJ} ${KCTL_OBJ} ${GA_OBJ} ${SA_OBJ}

ALL=		kfs.rel mount_kfs kfsctl getacl setacl

all: ${ALL}

kfs.rel: ${KFS_OBJ}
	${LD} -r -o $@.tmp ${KFS_OBJ} && chmod 644 $@.tmp
	mv $@.tmp $@

mount_kfs: ${MNTK_OBJ}
	${LINK.o} -o $@.tmp ${MNTK_OBJ}
	mv $@.tmp $@

kfsctl: ${KCTL_OBJ}
	${LINK.o} -static -o $@.tmp ${KCTL_OBJ}
	mv -f $@.tmp $@
#	chmod u+s,g+x $@

getacl: ${GA_OBJ}
	${LINK.o} -o $@.tmp ${GA_OBJ}
	mv $@.tmp $@
	chmod a+rx $@

setacl: ${SA_OBJ}
	${LINK.o} -o $@.tmp ${SA_OBJ}
	mv $@.tmp $@
	chmod a+rx $@

clean:
	rm -f kfs.rel ${OBJ} ${ALL}

install:
	ln -s ../../kfs/kfs.rel /sys/`arch -k`/OBJ/kfs_rel.o

tar:
	rm -f /tmp/kfs.tar
	tar cf /tmp/kfs.tar -C .. kfs-0.1

depend:
	${CC} ${CPPFLAGS} -MM ${SRC} > Deplist.tmp
	mv Deplist.tmp Deplist

include Deplist
Deplist::;
