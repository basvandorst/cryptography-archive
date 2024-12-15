#!/bin/sh
if [ "${jar}" = "" ]; then jar=jar; fi

if [ ! -d build ]; then
    sh util/finalbuild.sh
fi

echo "Creating Cryptix-classes${CRYPTIX_VERSION}.zip file..."
cd build
rm -f ../../Cryptix-classes${CRYPTIX_VERSION}.zip
${jar} cfM ../../Cryptix-classes${CRYPTIX_VERSION}.zip *
cd ..
