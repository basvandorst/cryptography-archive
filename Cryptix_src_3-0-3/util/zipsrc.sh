#!/bin/sh
if [ "${jar}" = "" ]; then jar=jar; fi

sh util/copysrc.sh

echo "Creating Cryptix_src${CRYPTIX_VERSION}.zip file..."
rm -f Cryptix_src${CRYPTIX_VERSION}.zip
cd tmp
${jar} cfM ../Cryptix_src${CRYPTIX_VERSION}.zip *
cd ..

echo "Deleting temporary copy of source..."
rm -rf tmp
