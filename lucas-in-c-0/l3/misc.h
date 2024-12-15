void write_number(FILE *,MP_INT *x);
int read_number(FILE *,MP_INT *x);
int desfile(FILE *, FILE *, unsigned char *, unsigned char *, int);
void write_numberx(FILE *,MP_INT *x,unsigned char *,unsigned char *);
int read_numberx(FILE *,MP_INT *x, unsigned char *, unsigned char *);
void mdfile(FILE *, unsigned char *);
void mdstring(char *, unsigned int, unsigned char *);
FILE *mfopen(unsigned char *, unsigned char *);
void zap(void*, unsigned int);
