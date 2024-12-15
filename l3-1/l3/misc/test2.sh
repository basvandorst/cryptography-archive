#!/bin/sh -v
cat >foo.o <<SS
a
SS
encrypt test  <foo.o >foo.02
cat >foo.o <<SS
aa
SS
encrypt test  <foo.o >foo.03
cat >foo.o <<SS
aaa
SS
encrypt test  <foo.o >foo.04
cat >foo.o <<SS
aaaa
SS
encrypt test  <foo.o >foo.05
cat >foo.o <<SS
aaaaa
SS
encrypt test  <foo.o >foo.06
cat >foo.o <<SS
aaaaaa
SS
encrypt test  <foo.o >foo.07
cat >foo.o <<SS
aaaaaaa
SS
encrypt test  <foo.o >foo.08
cat >foo.o <<SS
aaaaaaaa
SS
encrypt test  <foo.o >foo.09
cat >foo.o <<SS
aaaaaaaaa
SS
encrypt test  <foo.o >foo.10
cat >foo.o <<SS
aaaaaaaaaa
SS
encrypt test  <foo.o >foo.11
cat >foo.o <<SS
aaaaaaaaaaa
SS
encrypt test  <foo.o >foo.12
cat >foo.o <<SS
aaaaaaaaaaaa
SS
encrypt test  <foo.o >foo.13
cat >foo.o <<SS
aaaaaaaaaaaaa
SS
encrypt test  <foo.o >foo.14
cat >foo.o <<SS
aaaaaaaaaaaaaa
SS
encrypt test  <foo.o >foo.15
