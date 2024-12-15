/*
   stubs.h - this should go away
 */

#include "usuals.h"
#include "prototypes.h"

char * get_ext_c_ptr(void) { return NULL; }

char *LANG(char *x) { return x; }

/* == from crypto.c ====================================================*/

PGPBoolean legal_ctb(PGPByte ctb) { return 0; }


/* == from random.c ====================================================*/

struct IdeaCfbContext;
int cryptRandOpen(struct IdeaCfbContext *cfb) { return 0; }

void cryptRandSave(struct IdeaCfbContext *cfb) { }

int cryptRandWriteFile(char const *name, struct IdeaCfbContext *cfb,
unsigned bytes) { return 0; }


/* == from charset.c ===================================================*/

void init_charset(void) { }

int set_CONVERSION() { return 0; }
int get_CONVERSION() { return 0; }

void CONVERT_TO_CANONICAL_CHARSET(char *s) { }

char EXT_C(char c) { return c; }
char INT_C(char c) { return c; }


/* == from keymgmt.c ===================================================*/

struct newkey;
int maintUpdate(PGPFileSpecRef ringfilespec, struct newkey const *nkeys)
{ return 0; }




