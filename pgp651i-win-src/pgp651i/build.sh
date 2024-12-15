#!/bin/sh

# If we're on Solaris, we'll have to use gnu make
if [ `uname -s` = "SunOS" ]; then
  MAKE="gmake"
else
  MAKE="make"
fi

# Build pfl
cd libs/pfl
./configure
$MAKE

# Build PGPcdk
cd ../pgpcdk
./configure
$MAKE headers
$MAKE

# Build PGP shared code
cd ../../clients/pgp/shared
./configure
$MAKE

# Build PGP command line
cd ../cmdline
./configure
$MAKE
