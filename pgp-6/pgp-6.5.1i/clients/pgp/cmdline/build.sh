#!/bin/ksh

if [ -z $1 ] ; then
        echo '\n\tPlease define the platform you ar building for (SOLARIS, LINUX, AIX, HPUX)'
        echo '\tsyntax:   build.sh [platform] \n'
        exit 1
fi

MYDIR=`pwd`
LIBSDIR=../../../libs
DOCSDIR=../../../docs

DEBUG=DEBUG=0
OPTMIZE=OPT=-O
DIRNAME=pgp-6.5.1

if [[ $1 = LINUX ]] ; then
	LINUX=LINUX=1
	PACKAGENAME=pgp-6.5.1-rsa.i386
fi

if [[ $1 = HPUX ]] ; then
	OPTMIZE=
	PACKAGENAME=pgp-6.5.1-hpux-rsa.tar
fi

if [[ $1 = SOLARIS ]] ; then
	DIRNAME=pgpcmdln
	PACKAGENAME=pgp-6.5.1-sol-rsa
fi

if [[ $1 = AIX ]] ; then
	PACKAGENAME=pgp-6.5.1-aix-rsa.tar
fi

# Create directories

mkdir $LIBSDIR/network/ldaplib/unix

# Build pfl

cd $LIBSDIR/pfl
autoconf ./configure.in > ./configure
chmod +x ./configure
chmod +x ./unix/config/config.sub
chmod +x ./unix/config/config.guess
./configure
make clean
make CC=gcc $OPTMIZE $DEBUG $LINUX world

# Build PGPsdk

cd ../pgpcdk
autoconf ./configure.in > ./configure
chmod +x ./configure
chmod +x ./unix/config/config.sub
chmod +x ./unix/config/config.guess
./configure
make clean
make CC=gcc $OPTMIZE $DEBUG $LINUX world

# Build PGPsdk UI library

cd ./unix/ui
make clean
make CC=gcc $OPTMIZE $DEBUG $LINUX all

# Build PGP shared code

cd ../../../../clients/pgp/shared
chmod +x ./configure
./configure
make clean
make CC=gcc $OPTMIZE $DEBUG $LINUX all

# Build PGPcmdline

cd ../cmdline
autoconf ./configure.in > ./configure
chmod +x ./configure
chmod +x ./unix/config/config.sub
chmod +x ./unix/config/config.guess
./configure
make clean
make CC=gcc $OPTMIZE $DEBUG $LINUX all

# Build distribution tree

cd $MYDIR

rm -rf ./packaging/$DIRNAME
mkdir ./packaging/$DIRNAME
mkdir ./packaging/$DIRNAME/docs
mkdir ./packaging/$DIRNAME/keyrings
mkdir ./packaging/$DIRNAME/man
mkdir ./packaging/$DIRNAME/man/man1

cp $DOCSDIR/builds/cmdline/PGPCmdLineGuide.pdf ./packaging/$DIRNAME/docs
cp $DOCSDIR/builds/common/IntrotoCrypto.pdf ./packaging/$DIRNAME/docs
cp ./CREDITS ./packaging/$DIRNAME/docs
cp ./packaging/pgp-6.0/docs/WhatsNew.html ./packaging/$DIRNAME/docs
cp ./packaging/pgp-6.0/docs/LICENSE ./packaging/$DIRNAME/docs

if [[ $2 = FREE ]] ; then
	cp $DOCSDIR/builds/freeware/Freeware.txt ./packaging/$DIRNAME/docs
fi

cp ./packaging/pgp-6.0/keyrings/SampleKeys.asc ./packaging/$DIRNAME/keyrings
cp ./packaging/pgp-6.0/man/man1/pgp.1 ./packaging/$DIRNAME/man/man1

cp ./pgp ./packaging/$DIRNAME
cp ./packaging/pgp-6.0/docs/WHATSNEW ./packaging/$DIRNAME

# Package product

if [[ $1 = SOLARIS ]] ; then
	cd ./packaging
	make
	mv pgpcmdln-sol-6.5.1 $PACKAGENAME
fi

if [[ $1 = AIX ]] ; then
	cd ./packaging
	tar -cf $PACKAGENAME $DIRNAME
fi

if [[ $1 = HPUX ]] ; then
	cd ./packaging
	tar -cf $PACKAGENAME $DIRNAME
fi

if [[ $1 = LINUX ]] ; then
	cd ./packaging
	tar -cf $PACKAGENAME $DIRNAME
fi

