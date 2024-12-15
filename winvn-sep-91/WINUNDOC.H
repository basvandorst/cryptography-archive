/*  winundoc.h */
/*  Declarations for KERNEL File I/O and String Functions */
/*  Standard but undocumented functions revealed by Charles Petzold. */

int  FAR PASCAL _lopen	(LPSTR, int);
int  FAR PASCAL _lclose (int);
int  FAR PASCAL _lcreat (LPSTR, int);
LONG FAR PASCAL _llseek (int, long, int);
WORD FAR PASCAL _lread	(int, LPSTR, int);
WORD FAR PASCAL _lwrite (int, LPSTR, int);

#define READ	    0
#define WRITE	    1
#define READ_WRITE  2

int   FAR PASCAL lstrlen(LPSTR);
LPSTR FAR PASCAL lstrcpy(LPSTR, LPSTR);
LPSTR FAR PASCAL lstrcat(LPSTR, LPSTR);
int   FAR PASCAL lstrcmp(LPSTR, LPSTR);
