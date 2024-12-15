#ifdef __STDC__
# define	P(s) s
#else
# define P(s) ()
#endif


/* ripemsoc.c */
char *GetUserRecordFromServer
  P((char *, TypKeySource *, char *, int, BOOL *, BOOL *, RIPEMInfo *));
char *GetUserRecordFromFinger P((char *, char *, int, int *, RIPEMInfo *));

#undef P
