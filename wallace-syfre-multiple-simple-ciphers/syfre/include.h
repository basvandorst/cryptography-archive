
//
// written by Marc Wallace, 30 Apr 93.
// last change: version 1.0.


#include "config.h"
#include "data.h"
#include "cypher.h"


// from random.c
//
extern void randinit();
extern char randc();
extern char randc(char, char);

// from syf.c
//
extern Cypher** syf;
extern int syfnum;

// from text.c
//
extern void bin2txt(Data&);
extern void txt2bin(Data&);

// from version.c
//
extern char *version[];
extern int verlimit[];
extern Cypher* cypherlist[];
