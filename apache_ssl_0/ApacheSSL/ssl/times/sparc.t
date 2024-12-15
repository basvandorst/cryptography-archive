SSLeay 0.5.0 - gcc (2.7.0) -O3 -fomit-frame-pointer -mv8
type           8 bytes     64 bytes    256 bytes   1024 bytes   8192 bytes
md2             54.95k      152.30k      204.89k      224.32k      230.92k
md5            642.09k     2680.34k     4305.41k     5081.43k     5278.38k
sha            378.75k     1556.69k     2500.61k     2930.43k     3068.33k
sha-1          334.39k     1371.41k     2203.23k     2591.40k     2703.36k
rc4           3263.76k     3687.04k     3726.55k     3256.82k     3439.46k
des cfb        687.97k      711.15k      724.33k      721.94k      726.36k
des cbc        740.07k      814.28k      821.16k      811.01k      801.95k
des ede3       293.62k      304.76k      306.52k      304.75k      305.80k
idea cfb       617.01k      642.96k      649.64k      642.13k      644.44k
idea cbc       658.88k      703.15k      711.00k      710.98k      710.73k
rsa  512 bits   0.074s
rsa 1024 bits   0.421s
rsa 2048 bits   2.942s
rsa 4096 bits  21.800s

SSLeay 0.5.0 - gcc (2.5.7) -O3 -fomit-frame-pointer -mv8
type           8 bytes     64 bytes    256 bytes   1024 bytes   8192 bytes
md2             57.55k      161.52k      216.94k      239.05k      243.84k
md5            632.58k     2620.07k     4300.36k     5100.54k     5266.69k
sha            376.14k     1560.73k     2522.13k     2976.43k     3091.11k
sha-1          335.66k     1366.53k     2219.43k     2626.44k     2748.99k
rc4           2992.76k     3647.24k     3710.24k     3753.75k     3619.27k
des cfb        671.81k      688.66k      693.43k      681.98k      685.40k
des cbc        716.72k      761.47k      769.62k      765.77k      764.59k
des ede3       274.05k      283.03k      283.49k      285.03k      285.77k
idea cfb       633.34k      656.71k      656.30k      652.58k      653.71k
idea cbc       680.65k      719.91k      724.91k      724.33k      726.36k
rsa  512 bits   0.072s
rsa 1024 bits   0.406s
rsa 2048 bits   2.770s
rsa 4096 bits  20.530s
*************************** link order dependant ********************

SSLeay 0.4.5d - gcc -O3 -fomit-frame-pointer -mv8
type           8 bytes     64 bytes    256 bytes   1024 bytes   8192 bytes
md2             57.31k      160.04k      216.58k      236.76k      244.94k
md5            642.48k     2651.65k     4203.45k     4993.86k     5178.02k
sha            378.53k     1578.38k     2526.37k     2976.09k     3126.61k
rc4           3223.90k     3225.40k     3726.90k     3745.45k     3470.68k
cfb des        575.44k      620.84k      632.15k      631.13k      627.41k
cbc des        618.29k      691.96k      683.39k      695.98k      690.43k
ede3 des       230.47k      249.94k      251.31k      250.20k      250.78k
cfb idea       585.47k      608.41k      616.72k      597.45k      610.98k
cbc idea       648.43k      666.07k      671.15k      679.25k      677.21k
rsa  512 bits   0.078s
rsa 1024 bits   0.459s
rsa 2048 bits   3.243s
rsa 4096 bits  24.870s


SSLeay 0.4.5d - cc -fast -DPROTO -Xa
type           8 bytes     64 bytes    256 bytes   1024 bytes   8192 bytes
md2             45.59k      124.11k      167.20k      182.54k      189.47k
md5            598.55k     2522.14k     4133.27k     4936.93k     5181.72k
sha            335.68k     1375.91k     2259.11k     2657.62k     2757.23k
rc4           2812.33k     3093.72k     3123.20k     2931.93k     3003.73k
cfb des        483.23k      514.50k      517.63k      513.71k      514.38k
cbc des        525.17k      554.27k      555.12k      555.02k      556.28k
ede3 des       188.69k      193.85k      197.66k      196.82k      196.61k
cfb idea       518.97k      552.64k      547.00k      569.69k      554.41k
cbc idea       573.25k      599.06k      605.02k      599.97k      603.48k
rsa  512 bits   0.177s
rsa 1024 bits   1.214s
rsa 2048 bits   8.940s
rsa 4096 bits  69.410s

