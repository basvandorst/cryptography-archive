#include <gmp.h>
#include "lucas.h"
#include "d3des.h"
#include "idea.h"
#include "prandom.h"
#include <stdio.h>
#include "misc.h"
#include "md5.h"
#include "haval.h"
#include "bf.h"
#include <string.h>
#define LKEYLEN 96
#define HASHLEN 32
#define KEYBYTES (LKEYLEN - 24) /* real number of bytes in key - rest is IVs */
extern char *getenv(char *);
void tran1(unsigned char key[4], char *buf, int buffl);
void tran2(unsigned char key[4], char *buf, int buffl);
void clear_tran();

#if defined(__i386__) || defined(__i486__) || defined(vax) || defined(i386)
#define LITTLE_ENDIAN
#endif

#define MAXBITS ((MAXC * 8) - 128)
