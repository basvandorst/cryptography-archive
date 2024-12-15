#ifndef MISC_INCLUDED

#define MISC_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

extern unsigned char cov_2char[64];

struct HT {
  char currentword[9];
  unsigned char hasharray[4096];
};

struct PWEntryT {
  char name[9];		/* Login name			*/
  char password[14];	/* Encr. passwd			*/
  char salt[3];		/* Salt for encr. pw.		*/
  char passwd[12];	/* Encr. passwd			*/
  unsigned char hit;    /* Encoded hash			*/
  char *UID;		/* User ID			*/
  char *GID;		/* Group ID			*/
  char *gecos;		/* Gecos field (real name, etc. */
  char *dir;		/* Home directory		*/
  char *shell;	 	/* Shell			*/
  struct PWEntryT *next;
};

extern int NumAccts;	/* Total number of accts 	*/
extern int NumValid;	/* Total number of valid accts  */
extern int NumSCrypts; 	/* Total compares		*/
extern int NumFCrypts;	/* Total compare hits		*/
extern int NumHits;	/* Number of accurate guesses   */
extern int NumPWs;	/* Number of words in data file */

extern char *Ver;

#define	FLAG_TESTID	0x02
#define FLAG_BEEP	0x04
#define FLAG_NOMSG	0x08

#define STRLEN		1024

void myqinitcrypt (char *buf);
char *mycrypt (char *buf, char *salt); 
int GetSalt (int ch); 
void PrintStats (int starttime);
void PrintCracked (struct PWEntryT **PW, 
    struct PWEntryT *first, char *word, int flags);
struct PWEntryT *GetPWInfo(char *passwdfile, int flags);
void DeleteNode (struct PWEntryT **node, struct PWEntryT *first);
void FreeNode(struct PWEntryT **ptr);

#endif

