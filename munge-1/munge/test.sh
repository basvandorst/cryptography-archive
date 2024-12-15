#!/bin/sh
rm -f test foo foo.master
echo 1
./munge -dtcv  <test.M >test
diff test test.master
echo 2
./munge -tdcv  <test2.M >test
diff test test.master
echo 3
./munge -tdcv  <test3.M >test
diff test test.master
echo 4
./munge -3tdcv  <test4.M >test
diff test test.master
echo 5
cp crypto.c foo.master
./munge -tcv <foo.master >foo.M
./munge -tdv foo.M
diff foo.master foo
rm -f foo
echo 6
./munge -tDv <foo.master >foo.M
./munge -tdv foo.M
diff foo.master foo
rm -f foo
echo 7
./munge -tIv <foo.master >foo.M
./munge -tdv foo.M
diff foo.master foo
rm -f foo
echo 8
./munge -2tv <foo.master >foo.M
./munge -2tdv foo.M
diff foo.master foo
rm -f foo
echo 9
./munge -ntcv <foo.master >foo.M
./munge -dtv foo
diff foo.master foo
rm -f foo
echo 9A
./munge -otcv <foo.master >foo.M
./munge -dtv foo
diff foo.master foo
rm -f foo
echo 9B
./munge -ptcv <foo.master >foo.M
./munge -dtv foo
diff foo.master foo
rm -f foo
echo 9C
./munge -o3tcv <foo.master >foo.M
./munge -2dtv foo
diff foo.master foo
rm -f foo
echo 9D
./munge -p2tcv <foo.master >foo.M
./munge -2dtv foo
diff foo.master foo
rm -f foo
echo 10
echo "type ninetynine at the prompts"
# test compatibility between -2 and single key mode
./munge -2cv <foo.master >foo.M
./munge -dt foo
diff foo foo.master
echo 11
./munge -d2tcv  <test11.M >test
diff test test.master
echo 12
./munge -tdcv  <test12.M >test
diff test test.master
echo 13
./munge -tdcv  <test13.M >test
diff test test.master
rm -f foo foo.master test foo.M
