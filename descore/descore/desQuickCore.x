# 1 "desQuickCore.c"

# 1 "./desCode.h"







static char desCode_hRcs[] = "$Id: desCode.h,v 1.16 1992/04/17 00:44:43 how E $";



# 1 "./desCore.h"







static char desCore_hRcs[] = "$Id: desCore.h,v 1.10 1992/04/16 23:08:44 how E $";


typedef unsigned char byte, DesData[ 8];
typedef unsigned long word, DesKeys[32];

extern int DesMethod();
extern void DesQuickInit(), DesQuickDone();
extern void DesQuickCoreEncrypt(), DesQuickFipsEncrypt();
extern void DesQuickCoreDecrypt(), DesQuickFipsDecrypt();
extern void DesSmallCoreEncrypt(), DesSmallFipsEncrypt();
extern void DesSmallCoreDecrypt(), DesSmallFipsDecrypt();
extern void (*DesCryptFuncs[])();
extern int des_key_sched(), des_ecb_encrypt();
# 12 "./desCode.h"
extern word des_keymap[], des_bigmap[];









# 24





# 34













# 52



















































# 150










# 205














































































































































































































# 2 "desQuickCore.c"
void DesQuickCoreEncrypt(d, r, s) register byte * d; register word * r; register byte * s; { register word x, y, z;     y = s[ 7];  y<<= 8;  y |= s[ 6];  y<<= 8;  y |= s[ 5];  y<<= 8;  y |= s[ 4]; x = s[ 3]; x<<= 8; x |= s[ 2]; x<<= 8; x |= s[ 1]; x<<= 8; x |= s[ 0] ; x = x >>  1 | x <<  31; y = y >>  1 | y <<  31 ;  z = r[  0]; z ^= y;  x ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[  1]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[  2]; z ^= x;  y ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[  3]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[  4]; z ^= y;  x ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[  5]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[  6]; z ^= x;  y ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[  7]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[  8]; z ^= y;  x ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[  9]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[ 10]; z ^= x;  y ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[ 11]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[ 12]; z ^= y;  x ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[ 13]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[ 14]; z ^= x;  y ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[ 15]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[ 16]; z ^= y;  x ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[ 17]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[ 18]; z ^= x;  y ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[ 19]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[ 20]; z ^= y;  x ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[ 21]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[ 22]; z ^= x;  y ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[ 23]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[ 24]; z ^= y;  x ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[ 25]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[ 26]; z ^= x;  y ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[ 27]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[ 28]; z ^= y;  x ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[ 29]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[ 30]; z ^= x;  y ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[ 31]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z));  x = x <<  1 | x >>  31; y = y <<  1 | y >>  31; d[ 0] = y; y>>= 8; d[ 1] = y; y>>= 8; d[ 2] = y; y>>= 8; d[ 3] = y; d[ 4] =  x;  x>>= 8; d[ 5] =  x;  x>>= 8; d[ 6] =  x;  x>>= 8; d[ 7] =  x; return; }
void DesQuickCoreDecrypt(d, r, s) register byte * d; register word * r; register byte * s; { register word x, y, z;     y = s[ 7];  y<<= 8;  y |= s[ 6];  y<<= 8;  y |= s[ 5];  y<<= 8;  y |= s[ 4]; x = s[ 3]; x<<= 8; x |= s[ 2]; x<<= 8; x |= s[ 1]; x<<= 8; x |= s[ 0] ; x = x >>  1 | x <<  31; y = y >>  1 | y <<  31 ;   z = r[ 31]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[ 30]; z ^= y;  x ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[ 29]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[ 28]; z ^= x;  y ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[ 27]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[ 26]; z ^= y;  x ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[ 25]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[ 24]; z ^= x;  y ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[ 23]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[ 22]; z ^= y;  x ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[ 21]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[ 20]; z ^= x;  y ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[ 19]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[ 18]; z ^= y;  x ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[ 17]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[ 16]; z ^= x;  y ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[ 15]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[ 14]; z ^= y;  x ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[ 13]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[ 12]; z ^= x;  y ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[ 11]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[ 10]; z ^= y;  x ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[  9]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[  8]; z ^= x;  y ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[  7]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[  6]; z ^= y;  x ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[  5]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[  4]; z ^= x;  y ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[  3]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[  2]; z ^= y;  x ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; x ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z)); z = r[  1]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_bigmap + 128) + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 192) + ( 0XFCFC & z)); z = r[  0]; z ^= x;  y ^= *(word *) ((byte *)des_bigmap + ( 0XFCFC & z)); z>>= 16; y ^= *(word *) ((byte *)(des_bigmap + 64) + ( 0XFCFC & z));  x = x <<  1 | x >>  31; y = y <<  1 | y >>  31; d[ 0] = y; y>>= 8; d[ 1] = y; y>>= 8; d[ 2] = y; y>>= 8; d[ 3] = y; d[ 4] =  x;  x>>= 8; d[ 5] =  x;  x>>= 8; d[ 6] =  x;  x>>= 8; d[ 7] =  x; return; }
