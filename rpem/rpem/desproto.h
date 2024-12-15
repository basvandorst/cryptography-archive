#ifdef __STDC__
# define	P(s) s
#else
# define P(s) ()
#endif

int des_init P((DES *desbuf , int mode ));
void des_done P((DES *keybuf ));
void des_setkey P((DES *keybuf , char *key ));
int des_encrypt P((DES *keybuf , char *block ));
void des_decrypt P((DES *keybuf , char *block ));

#undef P
