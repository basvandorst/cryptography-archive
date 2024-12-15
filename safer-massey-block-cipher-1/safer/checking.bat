check
echo producing output file check.out...
echo Examples of Encrypted Data using SAFER> check.out
echo ======================================>> check.out
echo .>> check.out
echo input data>> check.out
check check.in>> check.out
echo safer -e -ecb -kx 0000000000000000>> check.out
safer -e -ecb -kx 0000000000000000 check.in check.tmp
check check.tmp>> check.out
echo safer -e -ecb -kx 0102030405060708>> check.out
safer -e -ecb -kx 0102030405060708 check.in check.tmp
check check.tmp>> check.out
echo safer -e -ecb -kx 0807060504030201>> check.out
safer -e -ecb -kx 0807060504030201 check.in check.tmp
check check.tmp>> check.out
echo safer -e -ecb -kx 08070605040302010807060504030201 -r 12>> check.out
safer -e -ecb -kx 08070605040302010807060504030201 -r 12 check.in check.tmp
check check.tmp>> check.out
echo safer -e -ecb -kx 01020304050607080807060504030201 -r 12>> check.out
safer -e -ecb -kx 01020304050607080807060504030201 -r 12 check.in check.tmp
check check.tmp>> check.out
echo safer -e -ecb -k AaBcDeFgHiJkLmNoPqRsTuVwXyZz0123456789>> check.out
safer -e -ecb -k AaBcDeFgHiJkLmNoPqRsTuVwXyZz0123456789 check.in check.tmp
check check.tmp>> check.out
echo safer -ecb -kx 42431BA40D291F81D66083C605D3A4D6>> check.out
safer -ecb -kx 42431BA40D291F81D66083C605D3A4D6 check.in check.tmp
check check.tmp>> check.out
echo safer -cbc -kx 42431BA40D291F81D66083C605D3A4D6:74536EBDC211484A>> check.out
safer -cbc -kx 42431BA40D291F81D66083C605D3A4D6:74536EBDC211484A check.in check.tmp
check check.tmp>> check.out
echo safer -cfb -kx 42431BA40D291F81D66083C605D3A4D6:74536EBDC211484A>> check.out
safer -cfb -kx 42431BA40D291F81D66083C605D3A4D6:74536EBDC211484A check.in check.tmp
check check.tmp>> check.out
echo safer -ofb -kx 42431BA40D291F81D66083C605D3A4D6:74536EBDC211484A>> check.out
safer -ofb -kx 42431BA40D291F81D66083C605D3A4D6:74536EBDC211484A check.in check.tmp
check check.tmp>> check.out
echo safer -h -tan -kx 42431BA40D291F81D66083C605D3A4D6>> check.out
echo .>> check.out
safer -h -tan -kx 42431BA40D291F81D66083C605D3A4D6 check.in>> check.out
echo .>> check.out
echo safer -h -abr -kx 42431BA40D291F81D66083C605D3A4D6>> check.out
echo .>> check.out
safer -h -abr -kx 42431BA40D291F81D66083C605D3A4D6 check.in>> check.out
echo check now if the files check.out and check.ref are identical!
