#ifdef __STDC__
# define	P(s) s
#else
# define P(s) ()
#endif

#define ENCODE_CHUNKSIZE 48

char *CodeAndWriteBytes
  P((unsigned char *, unsigned int, char *, FILE *));
char *BufferCodeAndWriteBytes
  P((unsigned char *, unsigned int, char *, BufferStream *));
void WriteCoded P((unsigned char *, unsigned int, char *, FILE *));
char *CrackRecipients P((char *, TypList *));
char *WriteEOL P((BufferStream *));
char *WriteMessage P((unsigned char *, unsigned int, BOOL, BufferStream *));

#undef P


