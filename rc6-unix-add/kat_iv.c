

/* AES known answer tests with intermediate values.
   This code assumes little endian (Intel)

   The results are output to the following files:

    ecb_vkiv.txt - variable key known answer test electronic codebook mode
                   with intermediate values - ENCRYPTION.
    ecb_vtiv.txt - variable text known answer test electronic codebook mode
                   with intermediate values - ENCRYPTION.
    dec_vkiv.txt - variable key known answer test electronic codebook mode
                   with intermediate values - DECRYPTION.
    dec_vtiv.txt - variable text known answer test electronic codebook mode
                   with intermediate values - DECRYPTION.

   IMPORTANT REMARK:  The aes_iv.c and aes_iv.h on this diskette are different
   from the aes.c and aes.h on other diskettes.  This program will not compile
   with the other aes.c and aes.h files.
*/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "aes_iv.h"



#define BLOCKSIZE   128
#define ALG_NAME    "RC6 (TM)"
#define SUBMITTER   "RSA Laboratories"


void KAT3();
void KAT4();
void outputHeaderKAT3();
void outputHeaderKAT4();
void KAT5();
void KAT6();
void outputHeaderKAT5();
void outputHeaderKAT6();

int
main () {
  FILE *fp;
  char filename[100], fullname[100];


  sprintf (filename, "ecb_vkiv.txt");
  sprintf (fullname, "%s",filename);
  fp = fopen (fullname,"w");
  if (fp == NULL) {
    printf ("error opening file %s\n", filename);
    exit(1);
  }
  outputHeaderKAT3 (fp, filename);
  printf ("KAT3\n"); fflush(stdout);
  KAT3(fp, 16);
  KAT3(fp, 24);
  KAT3(fp, 32);
  fprintf (fp, "==========\n");
  fclose(fp);

  sprintf (filename, "ecb_vtiv.txt");
  sprintf (fullname, "%s",filename);
  fp = fopen (fullname,"w");
  if (fp == NULL) {
    printf ("error opening file %s\n", filename);
    exit(1);
  }
  outputHeaderKAT4 (fp, filename);
  printf ("KAT4\n"); fflush(stdout);
  KAT4(fp, 16);
  KAT4(fp, 24);
  KAT4(fp, 32);
  fprintf (fp, "==========\n");
  fclose(fp);


  sprintf (filename, "dec_vkiv.txt");
  sprintf (fullname, "%s",filename);
  fp = fopen (fullname,"w");
  if (fp == NULL) {
    printf ("error opening file %s\n", filename);
    exit(1);
  }
  outputHeaderKAT5 (fp, filename);
  printf ("KAT5\n"); fflush(stdout);
  KAT5(fp, 16);
  KAT5(fp, 24);
  KAT5(fp, 32);
  fprintf (fp, "==========\n");
  fclose(fp);

  sprintf (filename, "dec_vtiv.txt");
  sprintf (fullname, "%s",filename);
  fp = fopen (fullname,"w");
  if (fp == NULL) {
    printf ("error opening file %s\n", filename);
    exit(1);
  }
  outputHeaderKAT6 (fp, filename);
  printf ("KAT6\n"); fflush(stdout);
  KAT6(fp, 16);
  KAT6(fp, 24);
  KAT6(fp, 32);
  fprintf (fp, "==========\n");
  fclose(fp);

  return 0;

}



void
outputBlock (FILE *fp, Block block)
{
  int k;

  for (k=0; k < 16; ++k)
    fprintf (fp,"%.2x", block.charsBlock[k]);
}



void
outputKey (FILE *fp, unsigned char *K, int b)
{
  register int i;

  for (i=0; i < b; ++i)
    fprintf (fp, "%02.2x",K[i]);
}



void
outputHeaderKAT3 (FILE *fp, char *filename)
{
  fprintf (fp,"=========================\n\n");
  fprintf (fp,"FILENAME:  \"%s\"\n\n", filename);
  fprintf (fp,"Electronic Codebook (ECB) Mode\n");
  fprintf (fp,"Variable Key Known Answer Tests With Intermediate Values - ENCRYPTION\n\n");
  
  fprintf (fp,"Algorithm Name: %s\n",ALG_NAME);
  fprintf (fp,"Principal Submitter: %s\n\n", SUBMITTER);

  fprintf (fp, "-- PT contains the plaintext stored in registers A,B,C,D.\n");
  fprintf (fp, "-- IV[0] contains the intermediate values of registers A,B,C,D\n"); 
  fprintf (fp, "   after the pre-whitening step (i.e., B=B+S[0],D=D+S[1]).\n");
  fprintf (fp, "-- IV[i] contains the intermediate values of registers A,B,C,D \n");
  fprintf (fp, "   at the END of round i (1 <= i <= 20).\n");
  fprintf (fp, "-- CT contains the ciphertext stored in registers A,B,C,D.\n");
  
}


void
outputHeaderKAT4 (FILE *fp, char *filename)
{
  fprintf (fp,"=========================\n\n");
  fprintf (fp,"FILENAME:  \"%s\"\n\n", filename);
  fprintf (fp,"Electronic Codebook (ECB) Mode\n");
  fprintf (fp,"Variable Text Known Answer Tests With Intermediate Values - ENCRYPTION\n\n");

  fprintf (fp,"Algorithm Name: %s\n",ALG_NAME);
  fprintf (fp,"Principal Submitter: %s\n\n", SUBMITTER);
  
  fprintf (fp, "-- PT contains the plaintext stored in registers A,B,C,D.\n");
  fprintf (fp, "-- IV[0] contains the intermediate values of registers A,B,C,D\n"); 
  fprintf (fp, "   after the pre-whitening step (i.e., B=B+S[0],D=D+S[1]).\n");
  fprintf (fp, "-- IV[i] contains the intermediate values of registers A,B,C,D \n");
  fprintf (fp, "   at the END of round i (1 <= i <= 20).\n");
  fprintf (fp, "-- CT contains the ciphertext stored in registers A,B,C,D.\n");
}



void
outputHeaderKAT5 (FILE *fp, char *filename)
{
  fprintf (fp,"=========================\n\n");
  fprintf (fp,"FILENAME:  \"%s\"\n\n", filename);
  fprintf (fp,"Electronic Codebook (ECB) Mode\n");
  fprintf (fp,"Variable Key Known Answer Tests With Intermediate Values - DECRYPTION\n\n");

  fprintf (fp,"Algorithm Name: %s\n",ALG_NAME);
  fprintf (fp,"Principal Submitter: %s\n\n", SUBMITTER);

  fprintf (fp,"-- CT contains the ciphertext stored in registers A,B,C,D.\n");
  fprintf (fp,"-- IV[i] contains the intermediate values of registers A,B,C,D\n");
  fprintf (fp,"   at the BEGINNING of round i (1 <= i <= 20).\n");
  fprintf (fp,"-- IV[0] contains the intermediate values of registers A,B,C,D\n");
  fprintf (fp,"   at the END of round 1.\n");
  fprintf (fp,"-- PT contains the plaintext stored in registers A,B,C,D.\n");

}


void
outputHeaderKAT6 (FILE *fp, char *filename)
{
  fprintf (fp,"=========================\n\n");
  fprintf (fp,"FILENAME:  \"%s\"\n\n", filename);
  fprintf (fp,"Electronic Codebook (ECB) Mode\n");
  fprintf (fp,"Variable Text Known Answer Tests With Intermediate Values - DECRYPTION\n\n");

  fprintf (fp,"Algorithm Name: %s\n",ALG_NAME);
  fprintf (fp,"Principal Submitter: %s\n\n", SUBMITTER);

  fprintf (fp,"-- CT contains the ciphertext stored in registers A,B,C,D.\n");
  fprintf (fp,"-- IV[i] contains the intermediate values of registers A,B,C,D\n");
  fprintf (fp,"   at the BEGINNING of round i (1 <= i <= 20).\n");
  fprintf (fp,"-- IV[0] contains the intermediate values of registers A,B,C,D\n");
  fprintf (fp,"   at the END of round 1.\n");
  fprintf (fp,"-- PT contains the plaintext stored in registers A,B,C,D.\n");
}


/* Variable key known answer tests with intermediate values
   (b = key bytes)
 */
void
KAT3 (FILE *fp, int b)
{
  unsigned char *K;
  unsigned long *S;
  int i, j;
  Block pt;
  CryptData ct;

  K = (unsigned char *) malloc (b);

  /* set the plaintext to all 0's */
  pt.dwordsBlock.A = pt.dwordsBlock.B = pt.dwordsBlock.C = pt.dwordsBlock.D = 0;

  /* set the key to all 0's */
  memset (K, 0, b);

  fprintf (fp, "==========\n\n");
  fprintf (fp, "KEYSIZE=%d\n\n", 8*b);
  fprintf (fp, "PT=");
  outputBlock (fp, pt);
  fprintf (fp, "\n\n");


  for (i=0; i < 8*b; ++i) {

    K[i/8] = (unsigned char) (1 << (7 - (i&7)));
    fprintf (fp, "I=%d\n",i+1);
    fprintf (fp, "KEY=");
    outputKey (fp, K, b);
    fprintf (fp, "\n");

    S = (unsigned long *) ComputeKeySchedule (K, b);

    Encrypt ((void *) S, &pt, &ct);

    fprintf (fp, "IV[0]=");
    outputBlock (fp, ct.roundData[0]);
    fprintf (fp, "\n");
    for (j=1; j <= ROUNDS; ++j) {
      fprintf (fp, "IV[%d]=", j);
      outputBlock (fp, ct.roundData[j]);
      fprintf (fp, "\n");
    }
    fprintf (fp, "CT=");
    outputBlock (fp, ct.roundData[j]);
    fprintf (fp, "\n\n");

    K[i/8] = 0;
    DeleteKeySchedule ((void *) S);
  }

  free (K);
}


/* Variable text known answer tests with intermediate values
   (b = key bytes)
 */
void
KAT4 (FILE *fp, int b)
{
  unsigned char *K;
  unsigned long *S;
  int i, j;
  Block pt;
  CryptData ct;

  K = (unsigned char *) malloc (b);

  /* set the plaintext to all 0's */
  pt.dwordsBlock.A = pt.dwordsBlock.B = pt.dwordsBlock.C = pt.dwordsBlock.D = 0;

  /* set the key to all 0's */
  memset (K, 0, b);

  fprintf (fp, "==========\n\n");
  fprintf (fp, "KEYSIZE=%d\n\n", 8*b);
  fprintf (fp, "KEY=");
  outputKey (fp, K, b);
  fprintf (fp, "\n\n");

  S = (unsigned long *) ComputeKeySchedule (K, b);

  for (i=0; i < BLOCKSIZE; ++i) {

    fprintf (fp, "I=%d\n",i+1);
    pt.charsBlock[i/8] = (unsigned char) (1 << (7 - (i&7)));
    fprintf (fp, "PT=");
    outputBlock (fp, pt);
    fprintf (fp, "\n");

    Encrypt ((void *) S, &pt, &ct);

    fprintf (fp, "IV[0]=");
    outputBlock (fp, ct.roundData[0]);
    fprintf (fp, "\n");
    for (j=1; j <= ROUNDS; ++j) {
      fprintf (fp, "IV[%d]=", j);
      outputBlock (fp, ct.roundData[j]);
      fprintf (fp, "\n");
    }
    fprintf (fp, "CT=");
    outputBlock (fp, ct.roundData[j]);
    fprintf (fp, "\n\n");
    pt.charsBlock[i/8] = 0;
  }

  free (K);
  DeleteKeySchedule ((void *) S);

}



/* Variable key known answer tests with intermediate values - DECRYPTION
   (b = key bytes)
 */
void
KAT5 (FILE *fp, int b)
{
  unsigned char *K;
  unsigned long *S;
  int i, j;
  Block init_pt;
  CryptData end_pt, ct;

  K = (unsigned char *) malloc (b);

  /* set the plaintext to all 0's */
  init_pt.dwordsBlock.A = init_pt.dwordsBlock.B = 0;
  init_pt.dwordsBlock.C = init_pt.dwordsBlock.D = 0;

  /* set the key to all 0's */
  memset (K, 0, b);

  fprintf (fp, "==========\n\n");
  fprintf (fp, "KEYSIZE=%d\n\n", 8*b);


  for (i=0; i < 8*b; ++i) {

    K[i/8] = (unsigned char) (1 << (7 - (i&7)));
    fprintf (fp, "I=%d\n",i+1);
    fprintf (fp, "KEY=");
    outputKey (fp, K, b);
    fprintf (fp, "\n");

    S = (unsigned long *) ComputeKeySchedule (K, b);

    /* Encrypt the plaintext to get the ciphertext that we want to decrypt */
    Encrypt ((void *) S, &init_pt, &ct);
    fprintf (fp, "CT=");
    outputBlock (fp, ct.roundData[ROUNDS+1]);
    fprintf (fp, "\n");

    Decrypt ((void *) S, &ct.roundData[ROUNDS+1], &end_pt);

    fprintf (fp, "IV[20]=");
    outputBlock (fp, end_pt.roundData[0]);
    fprintf (fp, "\n");
    for (j=1; j <= ROUNDS; ++j) {
      fprintf (fp, "IV[%d]=", 20-j);
      outputBlock (fp, end_pt.roundData[j]);
      fprintf (fp, "\n");
    }
    fprintf (fp, "PT=");
    outputBlock (fp, end_pt.roundData[j]);
    fprintf (fp, "\n\n");

    K[i/8] = 0;
    DeleteKeySchedule ((void *) S);
  }

  free (K);
}


/* Variable text known answer tests with intermediate values - DECRYPTION
   (b = key bytes)
 */
void
KAT6 (FILE *fp, int b)
{
  unsigned char *K;
  unsigned long *S;
  int i, j;
  Block init_pt;
  CryptData end_pt, ct;

  K = (unsigned char *) malloc (b);

  /* set the plaintext to all 0's */
  init_pt.dwordsBlock.A = init_pt.dwordsBlock.B = 0;
  init_pt.dwordsBlock.C = init_pt.dwordsBlock.D = 0;

  /* set the key to all 0's */
  memset (K, 0, b);

  fprintf (fp, "==========\n\n");
  fprintf (fp, "KEYSIZE=%d\n\n", 8*b);
  fprintf (fp, "KEY=");
  outputKey (fp, K, b);
  fprintf (fp, "\n\n");

  S = (unsigned long *) ComputeKeySchedule (K, b);

  for (i=0; i < BLOCKSIZE; ++i) {

    fprintf (fp, "I=%d\n",i+1);
    init_pt.charsBlock[i/8] = (unsigned char) (1 << (7 - (i&7)));

    /* Encrypt the plaintext to get the ciphertext that we want to decrypt */
    Encrypt ((void *) S, &init_pt, &ct);

    fprintf (fp, "CT=");
    outputBlock (fp, ct.roundData[ROUNDS+1]);
    fprintf (fp, "\n");

    Decrypt ((void *) S, &ct.roundData[ROUNDS+1], &end_pt);

    fprintf (fp, "IV[20]=");
    outputBlock (fp, end_pt.roundData[0]);
    fprintf (fp, "\n");
    for (j=1; j <= ROUNDS; ++j) {
      fprintf (fp, "IV[%d]=", 20-j);
      outputBlock (fp, end_pt.roundData[j]);
      fprintf (fp, "\n");
    }
    fprintf (fp, "PT=");
    outputBlock (fp, end_pt.roundData[j]);
    fprintf (fp, "\n\n");
    init_pt.charsBlock[i/8] = 0;
  }

  free (K);
  DeleteKeySchedule ((void *) S);

}






