#ifndef SNEFRUFILE_H
#define SNEFRUFILE_H

typedef struct snefrufile
 {
  unsigned char c[64];
  int n;
  unsigned char len[8];
 }
snefrufile;

#define SNEFRUFILE_HASHLEN 32

extern void snefrufile_clear();
extern void snefrufile_add();
extern void snefrufile_addn();
extern void snefrufile_hash();

#endif
