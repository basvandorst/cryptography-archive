#ifndef FINGERPRINTFILE_H
#define FINGERPRINTFILE_H

#include "snefrufile.h"
#include "md5file.h"
#include "crc32file.h"

typedef struct
 {
  snefrufile s;
  md5file m;
  crc32file c;
  unsigned char len[5];
 }
fingerprintfile;

#define FINGERPRINTFILE_HASHLEN 57

extern void fingerprintfile_clear();
extern void fingerprintfile_add();
extern void fingerprintfile_addn();
extern void fingerprintfile_hash();
extern unsigned int fingerprintfile_fmt();
extern unsigned int fingerprintfile_scan();

#endif
