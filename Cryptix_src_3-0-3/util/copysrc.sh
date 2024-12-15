#!/bin/sh
sh util/clean.sh

echo "Copying source release..."
mkdir -p tmp/doc tmp/guide tmp/images tmp/license tmp/native tmp/oldpgp tmp/src tmp/src10 tmp/util

cp README.html tmp
cp FAQ.html tmp

cp doc/placeholder.txt tmp/doc/packages.html
cp doc/placeholder.txt tmp/doc
cp doc/build.* tmp/doc

cp -R guide tmp/guide
cp -R images tmp/images
cp -R license tmp/license
cp -R native tmp/native
cp -R oldpgp tmp/oldpgp
cp -R src tmp/src
cp -R src10 tmp/src10
cp -R util tmp/util

find tmp -name ".#*" |xargs rm -f

echo "Deleting CVS directories..."
find tmp -name "CVS" |xargs rm -rf
