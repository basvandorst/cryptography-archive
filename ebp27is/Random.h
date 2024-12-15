#include "usuals.h"
#include "safer.h"  /* Declaration of SaferCfbContext */

int cryptRandOpen(struct SaferCfbContext *);
byte cryptRandByte(void);
void cryptRandInit(struct SaferCfbContext *);
int cryptRandWriteFile(char const *, struct SaferCfbContext *, unsigned);
void cryptRandSave(struct SaferCfbContext *);

unsigned trueRandEvent(int event);
void trueRandFlush(void);
void trueRandConsume(unsigned count);
void trueRandAccumLater(unsigned bitcount);
void trueRandAccum(unsigned count);
int trueRandByte(void);

int getstring(char *strbuf, unsigned maxlen, int echo);
