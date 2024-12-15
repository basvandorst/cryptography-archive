#ifndef CRC32FILE_H
#define CRC32FILE_H

typedef struct { unsigned long c; unsigned char len[8]; } crc32file;

#define CRC32FILE_HASHLEN 4

extern void crc32file_clear();
extern void crc32file_add();
extern void crc32file_addn();
extern void crc32file_hash();

#endif
