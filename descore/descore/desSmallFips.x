# 1 "desSmallFips.c"

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














































































































































































































# 2 "desSmallFips.c"
void DesSmallFipsEncrypt(d, r, s) register byte * d; register word * r; register byte * s; { register word x, y, z;     x = s[ 7];  x<<= 8;  x |= s[ 6];  x<<= 8;  x |= s[ 5];  x<<= 8;  x |= s[ 4]; y = s[ 3]; y<<= 8; y |= s[ 2]; y<<= 8; y |= s[ 1]; y<<= 8; y |= s[ 0] ; z = ((x >>  004) ^  y) &  0X0F0F0F0FL; x ^= z <<  004;  y ^= z; z = ((y >>  020) ^  x) &  0X0000FFFFL; y ^= z <<  020;  x ^= z; z = ((x >>  002) ^  y) &  0X33333333L; x ^= z <<  002;  y ^= z; z = ((y >>  010) ^  x) &  0X00FF00FFL; y ^= z <<  010;  x ^= z; x = x >>  1 | x <<  31; z = (x ^ y) & 0X55555555L; y ^= z; x ^= z; y = y >>  1 | y <<  31 ;  z = r[  0]; z ^= y;  x ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[  1]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[  2]; z ^= x;  y ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[  3]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[  4]; z ^= y;  x ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[  5]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[  6]; z ^= x;  y ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[  7]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[  8]; z ^= y;  x ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[  9]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[ 10]; z ^= x;  y ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[ 11]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[ 12]; z ^= y;  x ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[ 13]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[ 14]; z ^= x;  y ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[ 15]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[ 16]; z ^= y;  x ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[ 17]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[ 18]; z ^= x;  y ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[ 19]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[ 20]; z ^= y;  x ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[ 21]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[ 22]; z ^= x;  y ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[ 23]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[ 24]; z ^= y;  x ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[ 25]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[ 26]; z ^= x;  y ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[ 27]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[ 28]; z ^= y;  x ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[ 29]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[ 30]; z ^= x;  y ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[ 31]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z));  x = x <<  1 | x >>  31; z = (x ^ y) & 0X55555555L; y ^= z; x ^= z; y = y <<  1 | y >>  31; z = ((x >>  010) ^  y) &  0X00FF00FFL; x ^= z <<  010;  y ^= z; z = ((y >>  002) ^  x) &  0X33333333L; y ^= z <<  002;  x ^= z; z = ((x >>  020) ^  y) &  0X0000FFFFL; x ^= z <<  020;  y ^= z; z = ((y >>  004) ^  x) &  0X0F0F0F0FL; y ^= z <<  004;  x ^= z; d[ 0] = x; x>>= 8; d[ 1] = x; x>>= 8; d[ 2] = x; x>>= 8; d[ 3] = x; d[ 4] =  y;  y>>= 8; d[ 5] =  y;  y>>= 8; d[ 6] =  y;  y>>= 8; d[ 7] =  y; return; }
void DesSmallFipsDecrypt(d, r, s) register byte * d; register word * r; register byte * s; { register word x, y, z;     x = s[ 7];  x<<= 8;  x |= s[ 6];  x<<= 8;  x |= s[ 5];  x<<= 8;  x |= s[ 4]; y = s[ 3]; y<<= 8; y |= s[ 2]; y<<= 8; y |= s[ 1]; y<<= 8; y |= s[ 0] ; z = ((x >>  004) ^  y) &  0X0F0F0F0FL; x ^= z <<  004;  y ^= z; z = ((y >>  020) ^  x) &  0X0000FFFFL; y ^= z <<  020;  x ^= z; z = ((x >>  002) ^  y) &  0X33333333L; x ^= z <<  002;  y ^= z; z = ((y >>  010) ^  x) &  0X00FF00FFL; y ^= z <<  010;  x ^= z; x = x >>  1 | x <<  31; z = (x ^ y) & 0X55555555L; y ^= z; x ^= z; y = y >>  1 | y <<  31 ;   z = r[ 31]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[ 30]; z ^= y;  x ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[ 29]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[ 28]; z ^= x;  y ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[ 27]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[ 26]; z ^= y;  x ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[ 25]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[ 24]; z ^= x;  y ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[ 23]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[ 22]; z ^= y;  x ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[ 21]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[ 20]; z ^= x;  y ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[ 19]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[ 18]; z ^= y;  x ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[ 17]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[ 16]; z ^= x;  y ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[ 15]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[ 14]; z ^= y;  x ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[ 13]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[ 12]; z ^= x;  y ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[ 11]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[ 10]; z ^= y;  x ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[  9]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[  8]; z ^= x;  y ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[  7]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[  6]; z ^= y;  x ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[  5]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[  4]; z ^= x;  y ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[  3]; z ^= y; z = z <<  4 | z >>  28;  x ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[  2]; z ^= y;  x ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; x ^= *(word *) ((byte *)des_keymap + ( 0XFC & z)); z = r[  1]; z ^= x; z = z <<  4 | z >>  28;  y ^= *(word *) ((byte *)(des_keymap + 448) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 384) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 320) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 256) + ( 0XFC & z)); z = r[  0]; z ^= x;  y ^= *(word *) ((byte *)(des_keymap + 192) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 128) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)(des_keymap + 64) + ( 0XFC & z)); z>>= 8; y ^= *(word *) ((byte *)des_keymap + ( 0XFC & z));  x = x <<  1 | x >>  31; z = (x ^ y) & 0X55555555L; y ^= z; x ^= z; y = y <<  1 | y >>  31; z = ((x >>  010) ^  y) &  0X00FF00FFL; x ^= z <<  010;  y ^= z; z = ((y >>  002) ^  x) &  0X33333333L; y ^= z <<  002;  x ^= z; z = ((x >>  020) ^  y) &  0X0000FFFFL; x ^= z <<  020;  y ^= z; z = ((y >>  004) ^  x) &  0X0F0F0F0FL; y ^= z <<  004;  x ^= z; d[ 0] = x; x>>= 8; d[ 1] = x; x>>= 8; d[ 2] = x; x>>= 8; d[ 3] = x; d[ 4] =  y;  y>>= 8; d[ 5] =  y;  y>>= 8; d[ 6] =  y;  y>>= 8; d[ 7] =  y; return; }
