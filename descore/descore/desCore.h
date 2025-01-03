/*
 *	des - fast & portable DES encryption & decryption.
 *	Copyright (C) 1992  Dana L. How
 *	Please see the file `README' for the complete copyright notice.
 */

#ifndef	lint
static char desCore_hRcs[] = "$Id: desCore.h,v 1.10 1992/04/16 23:08:44 how E $";
#endif

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
