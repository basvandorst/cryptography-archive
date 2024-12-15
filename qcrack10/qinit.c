#include "misc.h"

void main(void)
{
  char s[STRLEN];

  time_t starttime;

  fprintf(stderr,"\nQInit %s, ",Ver);

  starttime = time(NULL);

  do {
    gets(s);
    if(!feof(stdin)) {
      s[8] = '\0';
      myqinitcrypt(s);
      NumFCrypts+=4096;
      NumPWs++;
    }
  } while (!feof(stdin));  

  PrintStats(starttime);

}
