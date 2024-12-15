#ifndef SNEFRU512_H
#define SNEFRU512_H

typedef unsigned long snefru512_word;

extern snefru512_word snefru512_sboxes[16][256];
extern snefru512_word snefru512_rotated[4][16][256];

extern void snefru512_setup();
extern void snefru512();

#endif
