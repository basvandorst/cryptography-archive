#!/bin/sh
if [ "${jar}" = "" ]; then jar=jar; fi

if [ ! -f doc/tree.html ]; then
    sh util/makedocs.sh
fi

echo "Creating Cryptix-doc${CRYPTIX_VERSION}.zip file..."
rm -f ../Cryptix-doc${CRYPTIX_VERSION}.zip
${jar} cfM ../Cryptix-doc${CRYPTIX_VERSION}.zip guide doc *.html COPYRIGHT.txt
