#!/bin/sh
if [ "${java}" = "" ]; then java=java; fi

find . -name "*.java" |xargs ${java} util.FixLineEndings -unix --
find . -name "*.html" |xargs ${java} util.FixLineEndings -unix --
find . -name "*.txt" |xargs ${java} util.FixLineEndings -unix --
find . -name "*.sh" -and "!" -name "unixify.sh" |xargs ${java} util.FixLineEndings -unix --
find . -name "*.properties" |xargs ${java} util.FixLineEndings -unix --
find native -name "*.c" |xargs ${java} util.FixLineEndings -unix --
find native -name "*.h" |xargs ${java} util.FixLineEndings -unix --
