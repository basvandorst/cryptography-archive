/****************************************************************
 *                                                              *
 *  This program is part of an implementation of the block      *
 *  cipher RC6(TM) that is being submitted to NIST as a         *
 *  candidate for the AES.                                      *
 *                                                              *
 *  Ronald L. Rivest and RSA Laboratories, the submitters of    *
 *  RC6, retain all rights to RC6 and to this code, except for  *
 *  those which NIST requires to be waived for submissions.     *
 *                                                              *
 *  Copyright (C) 1998 RSA Data Security, Inc.                  *
 *                                                              *
 ****************************************************************/

/*
 * This file is the header for a version of RC6 in C which is
 * specifically written to make available the internal values
 * of RC6 computations.
 */




#define ROUNDS 20


typedef struct {
  unsigned long A, B, C, D;
} DwordsBlock;

typedef union {
  DwordsBlock dwordsBlock;
  unsigned char charsBlock[16];
} Block;

typedef struct {
  Block roundData[ROUNDS+2];
} CryptData;


void* ComputeKeySchedule(unsigned char* key, unsigned long keyLength);
void Encrypt(void* keySchedule, Block* input, CryptData* cryptData);
void Decrypt(void* keySchedule, Block* input, CryptData* cryptData);
void DeleteKeySchedule(void* keySchedule);
