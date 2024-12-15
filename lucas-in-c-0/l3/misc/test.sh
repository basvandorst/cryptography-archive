#!/bin/sh -v
./genkey test
./encrypt test <d3des.c >foo
./decrypt test <foo >foo.1
diff d3des.c foo.1
echo "return to continue"
read
./sign test <d3des.c >foo
./vsign test foo <d3des.c
echo "return to continue"
read
rm -f foo.1 foo
