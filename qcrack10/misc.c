#include "misc.h"

int NumAccts = 0;	/* Total number of accts 	*/
int NumValid = 0;	/* Total number of valid accts  */
int NumSCrypts = 0; 	/* Total compares		*/
int NumFCrypts = 0;	/* Total compare hits		*/
int NumHits = 0;	/* Number of accurate guesses   */
int NumPWs = 0;		/* Number of words in data file */

char *Ver = "v1.0, Copyright (c) 1995, The Crypt Keeper: tck@bend.ucsd.edu\n";

/**************************************************************************
  void DeleteNode (struct PWEntryT **node, struct PWEntryT *first)
  
  Input:
    *node		-> Node to be deleted
    *first		-> Head of list

  Bug: Not freeing the individual members of the structure.
       Note: This code is not currently being called.
**************************************************************************/

void DeleteNode (struct PWEntryT **node, struct PWEntryT *first)
{
  struct PWEntryT *ptr;

  if (*node == (struct PWEntryT *) first) {
    FreeNode(node);
    first = NULL;
    *node = NULL;
    return;
  } 
  ptr = first;

  while (ptr != NULL) {
    if ((struct PWEntryT **) ptr -> next == node) {
      ptr -> next = (*node) -> next;
      FreeNode(node);  
      *node = ptr;
      return;
    } else ptr = ptr -> next;
  }
}

/**************************************************************************
  void FreeNode(struct PWEntryT **ptr)
  
  Input:
    PWEntry *ptr	-> pointer to node to be totally free'd
**************************************************************************/
void FreeNode(struct PWEntryT **PW)
{
  free((*PW) -> UID);
  free((*PW) -> GID);
  free((*PW) -> gecos);
  free((*PW) -> dir);
  free((*PW) -> shell);
  free((*PW) -> next);
  free(*PW);

}

/**************************************************************************
  void PrintCracked (struct PWEntryT **PW, struct PWEntryT *first, 
      char *word, int flags)

  Input:
    PWEntryT *PW	-> Password entry to print
    PWEntryT *first     -> Password entry list start
    *word		-> The guessed word
    flags		-> Control output
**************************************************************************/

void PrintCracked (struct PWEntryT **PW, struct PWEntryT *first, 
    char *word, int flags)
{
  if (flags & FLAG_BEEP) fprintf(stderr, "%c", 7);
  if (NumHits==0) printf("\n");
  NumHits++;
  printf("%s:%s:%s:%s:%s:%s:%s\n",
      (*PW) -> name,
      word,
      (*PW) -> UID,
      (*PW) -> GID,
      (*PW) -> gecos,
      (*PW) -> dir,
      (*PW) -> shell);
  fflush(stdout);
  DeleteNode(PW, first);
}

/**************************************************************************
  void PrintStats(int starttime)

  Input:
    starttime		-> Used for calculating total time.  
  
**************************************************************************/

void PrintStats(int starttime)
{
  int difftime, endtime;
  endtime = time(NULL);

  difftime = endtime - starttime;
  if(difftime == 0) difftime = 1;

  if(NumAccts == 0) 
    fprintf(stderr, "\n\nInit Complete!\n\n");
  else
    fprintf(stderr,"\n\nCracking Complete!\n\n");

  fprintf(stderr,"Total time:      %d s\n", difftime);

  if(NumAccts == 0) {
    fprintf(stderr, "Throughput:      %g fcrypts/s\n\n", 
        (double) NumFCrypts / (double) difftime);
    fprintf(stderr, "Words:           %d words\n", NumPWs);
    fprintf(stderr, "WThroughput:     %g words/s\n", 
        (double) NumPWs / (double) difftime);
  } else {
    fprintf(stderr,"No. scrypts:     %d\n", NumSCrypts);
    fprintf(stderr,"Throughput:      %d c/s\n\n", NumSCrypts/difftime);

    fprintf(stderr,"No. Hits:        %d\n", NumHits);
    fprintf(stderr,"No. Accts:       %d\n", NumAccts);
    fprintf(stderr,"No. Valid:       %d\n", NumValid);  
    if (NumAccts) fprintf(stderr,"Valid/Accts:     %g\n",
        (double) NumValid / (double) (NumAccts));
    if (NumAccts) fprintf(stderr,"Crack Rate:      %g%%\n\n", 
        (double) NumHits / (double) NumValid*100);
    fprintf(stderr,"No. fcrypts:     %d\n", NumFCrypts);
    fprintf(stderr,"No. PWs:         %d\n", NumPWs);
    if (NumPWs!=0)
        fprintf(stderr, "Wordlist rating: %g\n", 
            (double) NumHits / (double) NumPWs * 100);
  }
  fprintf(stderr,"\n\n");
  fflush(stderr);
}
/**************************************************************************
  int GetSalt(int ch)
 
  Returns: 
    0 -> 63 if valid character
    -1      if invalid character

  Input:
    ch		-> Character to look up.
**************************************************************************/ 

int GetSalt(int ch)
{
  switch(ch) {
    case '/': return 0;
    case '.': return 1;
    case '0': case '1': case '2': case '3': case '4': case '5': case '6':
    case '7': case '8': case '9': return (ch-'0'+2);
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
    case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
    case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
    case 'V': case 'W': case 'X': case 'Y': case 'Z': return (ch-'A'+2+10);
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
    case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
    case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
    case 'v': case 'w': case 'x': case 'y': case 'z': return (ch-'a'+2+10+26);
  }
  return -1;
}

/**************************************************************************
  int GetPWInfo(struct PWEntryT *PW, char *passwdfile, int flags)

  Returns:
    ptr to list
 
  Input:
    char *passwdfile 	-> Filename of file to read.
    int flags		-> Reading flags 
**************************************************************************/ 

struct PWEntryT *GetPWInfo(char *passwdfile, int flags)
{
  FILE *fp;
  char s[STRLEN];
  char *tmp, *hash;
  char sep[] = ":\n\r";
  struct PWEntryT *CurLine, *first;
  int plen;
  int i;
  
  fp = fopen (passwdfile, "r");

  if (fp == NULL) {
    fprintf (stderr, "\n\nCan't open passwdfile: [%s]\n", passwdfile);
    return NULL;
  }

  first = (struct PWEntryT *) malloc (sizeof(struct PWEntryT));
  if ((first) == NULL) {
    fprintf(stderr, "\nMemory allocation error allocating first node.\n");
    return NULL;
  }

  CurLine = first;

  do {
    CurLine -> next = (struct PWEntryT *) malloc (sizeof(struct PWEntryT));
    if ((first) == NULL) {
      fprintf(stderr, "\nMemory allocation error allocating first node.\n");
      return NULL;
    }
  
    fgets (s, STRLEN, fp);
    if (feof(fp)) break; 
    NumAccts++; 
   
    tmp = strtok(s, sep);
    strcpy(CurLine -> name, tmp);

    tmp = strtok(NULL, sep);
    plen = strlen(tmp);
    strcpy(CurLine -> password, tmp);

    tmp = strtok(NULL, sep);
    CurLine -> UID = (char *) malloc (strlen(tmp)+1);
    strcpy(CurLine -> UID, tmp);

    tmp = strtok(NULL, sep);
    CurLine -> GID = (char *) malloc (strlen(tmp)+1);
    strcpy(CurLine -> GID, tmp);

    tmp = strtok(NULL, sep);
    CurLine -> gecos = (char *) malloc (strlen(tmp)+1);
    strcpy(CurLine -> gecos, tmp);

    tmp = strtok(NULL, sep);
    CurLine -> dir = (char *) malloc (strlen(tmp)+1);
    strcpy(CurLine -> dir, tmp);

    tmp = strtok(NULL, sep);
    CurLine -> shell = (char *) malloc (strlen(tmp)+1);
    strcpy(CurLine -> shell, tmp);

    if ( ((plen == 13) || (plen == 0)) && (strlen(CurLine -> name) > 0) ) {
      NumValid++;
			/* There is apparently some kinda bug in strtok;
			   this code will never be executed since plen will
               	           never be = 0. */
      
      if(plen == 0) PrintCracked(&CurLine, first, "<NONE>", flags);
      else {
        CurLine -> salt[0] = CurLine -> password[0];
        CurLine -> salt[1] = CurLine -> password[1];
        CurLine -> salt[2] = '\0';
        for(i=0; i<11; i++) CurLine -> passwd[i] = CurLine -> password[i+2];
        CurLine -> passwd[11] = '\0';
        if (!(flags & FLAG_TESTID)) {
          NumFCrypts++;
          hash = mycrypt (CurLine -> name, CurLine -> salt);
          hash+=2;
          if (strcmp(hash, CurLine -> passwd) == 0) 
              PrintCracked(&CurLine, first, CurLine -> name, flags);
          else {
            CurLine -> hit = 
                (GetSalt(CurLine -> passwd[0]) << 2) |
                (GetSalt(CurLine -> passwd[1]) & 3);
          } 
        }
      }
      if(CurLine == NULL) {
        first = (struct PWEntryT *) malloc (sizeof(struct PWEntryT));
        if(first == NULL) {
          fprintf(stderr,"\nMemory allocation error allocating first node.\n");
          return NULL;
        }
        CurLine = first;
        CurLine -> next = NULL;
      } else {
        CurLine -> next = (struct PWEntryT *) malloc (sizeof(struct PWEntryT));
        if(CurLine -> next == NULL) { 
          fprintf(stderr,"\nMemory allocation error allocating node.\n");
          return NULL;
        }
        CurLine = CurLine -> next;
        CurLine -> next = NULL;
      }
    } else {
      if(!(flags & FLAG_NOMSG))  
          fprintf(stderr, "Malformed entry, line %d.\n", NumAccts);
    }
  } while (!feof(fp));

  if(!(flags & FLAG_NOMSG)) {
    fprintf(stderr, "\n--------------------------------------------------\n");
    fprintf(stderr, "\nTotal Entries: %d\n", NumAccts);
    fprintf(stderr, "Number Valid:  %d\n", NumValid);
    fprintf(stderr, "%% Valid        %g%%\n", (double) NumValid/NumAccts*100);
  }

  return first;
}
