#ifndef _ytcrypt_h_
#  define _ytcrypt_h_

extern void offcrypt(int initiate);
extern void startcrypt(char *key, int private);
extern void gotcrypt(int i, int private);
extern void gotiv(int i);

#endif /* _ytcrypt_h_ */
