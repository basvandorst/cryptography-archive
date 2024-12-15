#ifndef MD5FILE_H
#define MD5FILE_H

typedef unsigned long md5file_word;

typedef struct md5file
 {
  md5file_word state[4];
  md5file_word count[2];
  unsigned char buffer[64];
 }
md5file;

#define MD5FILE_HASHLEN 16

extern void md5file_clear();
extern void md5file_add();
extern void md5file_addn();
extern void md5file_hash();

#endif
