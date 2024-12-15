#include "misc.h"

void GetList(void);

void main(int argc, char **argv)
{
  FILE *wfile;
  time_t starttime;
  struct PWEntryT *first = NULL, *pwptr;
  struct HT HTEntry;
  int i, x=0, y=0, flg=0;
  int baseptr;
  char *hash; 

  printf("\nQCrack %s", Ver);

  if(argc < 3) {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  QCrack [-bns] <passwdfile> <wordlist_from_qinit>\n");
    fprintf(stderr, "          -b: Beep at each crack\n");
    fprintf(stderr, "          -n: Don't test for login id at load time\n");
    fprintf(stderr, "          -s: Supress passwd parser messages\n");
    return;
  }

  if(argv[1][0] == '-') {
    flg |= 0x01;
    for(i=1; i<strlen(argv[1]); i++) {
      switch(argv[1][i]) {
        case 'n':
        case 'N':
          flg |= 0x02;
          fprintf(stderr, "** Not login id testing\n");
          break;
        case 'b':
        case 'B':
          flg |= 0x04;
          fprintf(stderr, "** Beeping enabled\n");
          break;
        case 's':
        case 'S': 
          flg |= 0x08;
          fprintf(stderr, "** Passwd parsing messages supressed\n");
          break;
      }
    }
    fprintf(stderr,"\n");
  }

  starttime = time(NULL);

  if(flg & 0x01) baseptr = 2;
  else baseptr = 1;
  first = GetPWInfo(argv[baseptr], flg);

  wfile = fopen(argv[baseptr+1], "rb");

  if (wfile == NULL) {
    fprintf(stderr,"wordlist file open error.\n");
    return;
  } 

  pwptr = first;
  if(pwptr != NULL) {
    while (1) {
  
      fread(&HTEntry, sizeof(struct HT), 1, wfile); 
      if(feof(wfile)) {
        PrintStats(starttime);
        exit(0);
      }
      NumPWs++;
      pwptr = first;
      while (pwptr != NULL) {
        NumSCrypts++;
        x = GetSalt(pwptr -> salt[0]);
        y = GetSalt(pwptr -> salt[1]);
        x=(x*64)+y;
        if (x>0) {  
          if(pwptr -> hit == HTEntry.hasharray[x]) { 
            NumFCrypts++;
            hash = mycrypt(HTEntry.currentword, pwptr -> salt);
            hash+=2;
            if(strcmp(hash, pwptr -> passwd)==0)
                PrintCracked(&pwptr, first, HTEntry.currentword, flg); 
          }
        } 
        pwptr = pwptr -> next;
      }
    }
  }
}
