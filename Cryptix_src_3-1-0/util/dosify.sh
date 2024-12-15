#!/bin/sh
if [ "${java}" = "" ]; then java=java; fi

find . -name "*.java" |xargs ${java} util.FixLineEndings -dos --
find . -name "*.html" |xargs ${java} util.FixLineEndings -dos --
find . -name "*.txt" |xargs ${java} util.FixLineEndings -dos --
find . -name "*.bat" |xargs ${java} util.FixLineEndings -dos --
find . -name "*.properties" |xargs ${java} util.FixLineEndings -dos --
find native -name "*.c" |xargs ${java} util.FixLineEndings -dos --
find native -name "*.h" |xargs ${java} util.FixLineEndings -dos --
