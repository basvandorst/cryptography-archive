#!/bin/sh

# Build pfl
cd libs/pfl
./configure
make

# Build PGPcdk
cd ../pgpcdk
./configure
make headers
make

# Build PGP shared code
cd ../../clients/pgp/shared
./configure
make

# Build PGP command line
cd ../cmdline
./configure
make
