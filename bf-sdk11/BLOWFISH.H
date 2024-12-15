


//		BLOWFISH.H
//		interface for BFENG.OBJ
//		programming language: MS Visual C++ 1.0
//		memory model: large
//		last update: 05/25/96
//      (c)1996 Markus Hahn


#ifndef __BLOWFISH_H
#define __BLOWFISH_H



// the imported functions...

// we have to define a lot of extra stuff to be able
// using the assembly functions even in C++ under Windows

extern "C" {                                                       

void __far __pascal Blowfish_Init(unsigned char pKey[], unsigned int unKeySize);  
void __far __pascal Blowfish_ECBEncrypt(unsigned long *pBuffer, unsigned int unCount);
void __far __pascal Blowfish_ECBDecrypt(unsigned long *pBuffer, unsigned int unCount);
void __far __pascal Blowfish_CBCEncrypt(unsigned long *pBuffer, unsigned int unCount,
                                        unsigned long *ulCBCLeft, unsigned long *ulCBCRight);
void __far __pascal Blowfish_CBCDecrypt(unsigned long *pBuffer, unsigned int unCount,
                                        unsigned long *ulCBCLeft, unsigned long *ulCBCRight);
void __far __pascal Blowfish_Done();
void __far __pascal Blowfish_SetRounds(unsigned int unRounds);
void __far __pascal Blowfish_SetBoxes(unsigned long *pBuffer);
void __far __pascal Blowfish_GetBoxes(unsigned long *pBuffer);
void* __far __pascal Blowfish_GetBoxPointer();
char __far __pascal Blowfish_WeakKey();

};

#endif


