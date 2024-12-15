#ifdef DEBUG
#define EXP_LIMIT       1000
#else
#define EXP_LIMIT       2
#endif
#define MARYMAX         7
#define TWOPOWERMARYMAX 128 /* 2**MARYMAX */

extern int MdExp(), MdExpPlain(), MdExpBlock(), BmExp();
extern int currentmary, TableL[], TableU[];
