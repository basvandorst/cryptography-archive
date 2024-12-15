#!/bin/sh

rm -rf build

sh build.sh

mkdir build/snap
mkdir build/snap/cryptix-lib
mkdir build/snap/ijce-lib

cd build/classes

cp cryptix-lib/* ../snap/cryptix-lib
cp ijce-lib/* ../snap/ijce-lib

jar cvf cryptix3.jar java netscape cryptix

cp cryptix3.jar ../snap

cd ../snap

jar cvfM cryptix3-snap-$1-bin.zip cryptix3.jar cryptix-lib ijce-lib

cd ../..

mkdir tmp
mkdir tmp/src
cp -R src/. tmp/src
cp *.bat tmp
cp *.sh tmp
cp *.txt tmp

cd tmp/src
rm -rf CVS
rm -rf cryptix/CVS
rm -rf cryptix/examples/CVS
rm -rf cryptix/examples/pgp/CVS
rm -rf cryptix/math/CVS
rm -rf cryptix/mime/CVS
rm -rf cryptix/pgp/CVS
rm -rf cryptix/pgp/charset/CVS
rm -rf cryptix/pgp/engine/CVS
rm -rf cryptix/pgp/engine/toolkit/CVS
rm -rf cryptix/provider/CVS
rm -rf cryptix/provider/cipher/CVS
rm -rf cryptix/provider/elgamal/CVS
rm -rf cryptix/provider/key/CVS
rm -rf cryptix/provider/mac/CVS
rm -rf cryptix/provider/md/CVS
rm -rf cryptix/provider/mode/CVS
rm -rf cryptix/provider/padding/CVS
rm -rf cryptix/provider/rsa/CVS
rm -rf cryptix/security/CVS
rm -rf cryptix/security/rsa/CVS
rm -rf cryptix/test/CVS
rm -rf cryptix/tools/CVS
rm -rf cryptix/util/CVS
rm -rf cryptix/util/checksum/CVS
rm -rf cryptix/util/core/CVS
rm -rf cryptix/util/gui/CVS
rm -rf cryptix/util/io/CVS
rm -rf cryptix/util/math/CVS
rm -rf cryptix/util/mime/CVS
rm -rf cryptix/util/test/CVS
rm -rf cryptix/util/test/data/CVS
rm -rf cryptix-lib/CVS
rm -rf ijce-lib/CVS
rm -rf java/CVS
rm -rf java/lang/CVS
rm -rf java/security/CVS
rm -rf java/security/interfaces/CVS
rm -rf netscape/CVS
rm -rf netscape/security/CVS
cd ..

jar cvfM ../build/snap/cryptix3-snap-$1-src.zip .

cd ..

rm -rf tmp
