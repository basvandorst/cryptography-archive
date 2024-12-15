/* Secure Drive V1.3d */
/* Header file for cryptdsk.c and login.c */

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <bios.h>
#include <dos.h>
#include <string.h>
#include <ctype.h>
#include "md5.h"
#include "usuals.h"


#define MAXPASS 256 /* Max length of passphrase */
#define ROUNDS  8
#define KEYLEN  (6*ROUNDS+4)    /* length of key schedule */
#define MAXDRV  4
#define PASS_ITER 2048          /* Key hashing iterations */
#define ESC 0x01b               /* Esc code*/

struct diskdata
 {
  unsigned char dddrv;
           char drvltr;
  unsigned firstcyl;
  unsigned char firsthd;
  unsigned char firstsec;
  unsigned lastcyl;
  unsigned char maxsec;
  unsigned char maxhd;
  unsigned secsize;
  unsigned serial[2];
  unsigned char active;
  unsigned char flag2m;  /* diskette is 2M-formatted 1.3d*/
  unsigned fatsiz;       /* FAT size for 2M-formatted diskettes 1.3d */
 };

struct tsrdata
 {
  unsigned char tsrver[4];
  void far *real_dbios_vect;
  unsigned char fkeychk[4];
  unsigned char fkey[104];
  unsigned char fkeyv10;
           char pad1[3];
  unsigned char hkeychk[4];
  unsigned char hkey[104];
  unsigned char hkeyv10;
           char pad2[3];
           int  incfof;
           int  outcfof;
  struct diskdata fda;
  struct diskdata fdb;
  struct diskdata hd[MAXDRV];
           int stkseg;
           int stkofs;
           char rqtype;
  unsigned char drive;
           char numsec;
           char seclft;
           int callcx;
           int key;
           char badkey;
 unsigned  int stcyl;
          char stsec;
          char cursec;
 unsigned char sthd;
          char curhd;
 };

void extern far IdeaCFB(void *iv,void *key,void *plain,
                        void *cipher,unsigned len);
void extern far IdeaCFBx(void *iv,void *key,void *cipher,
                         void *plain,unsigned len);

void calcdiskparams(unsigned char *buf,unsigned *maxtrack,
                    unsigned *maxhead,unsigned *maxsector,
                    unsigned *secsize,unsigned serial[2]);
void invgets(char *input); /* Get without displaying, allow backspace */
void readsec(unsigned drive,unsigned head,unsigned track,
             unsigned sector,unsigned nsects,void *buffer);
void writesec(unsigned drive,unsigned head,unsigned track,
              unsigned sector,unsigned nsects,void *buffer);
int getyn(void);
void getkey(unsigned char *key,unsigned char *check,int confirm);
void getkeydf(unsigned char *key,unsigned char *check,unsigned char *bufck);
void setkeye(unsigned char *key,unsigned char *check);
void setkeydf(unsigned char *key,unsigned char *check,unsigned char *bufck);
void setkey10(unsigned char *key,unsigned char *check);
void setkey13(unsigned char *key,unsigned char *check);
void set_compat_mode(void);
void read_filekey(void);
void apply_filekey(char *);

void readptbl(unsigned char letter,unsigned *ptdrive,       /*1.1*/
              unsigned *pthead,unsigned *pttrack);          /*1.1*/
void rdptx(unsigned drive,unsigned head,unsigned track,     /*1.1*/
          unsigned char *letter, unsigned *ptdrive,         /*1.1*/
          unsigned *pthead,unsigned *pttrack,int recurse,   /*1.1*/
          unsigned char *buf);                              /*1.1*/

int rldbios(int cmd,int drive, int head, int track,int sect,
            int nsect,void *buf);

struct tsrdata far *gettsradr(void);
void en_key_idea(word16 *userkey, word16 *Z);
extern struct tsrdata far *cryptdata;

struct absr32m
 {
  long sector;
  int  nsect;
  unsigned char far  *xferad;
 };

