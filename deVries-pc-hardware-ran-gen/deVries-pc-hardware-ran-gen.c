// RANDOM.C
// From: Nico E de Vries <nevries@cc.ruu.nl>
// Date: Wed, 27 Jan 93 11:13:32 MET
// (C) 1993 AIP, all rights reserved
// AIP is not responsible for any damages resulting from use of this program.
//
// Use of this source or derivates of this source is allowed only
//   * for non commercial use (no fee asked)
//   * if this copyright is not changed in any way
//   * if the name AIP is not used in the executable or its docs
//     (use of the name Nico de Vries is allowed if he agrees)
//   * and if AIP is notified of its use in a specific program
// No permision is needed. Notification is sufficient.
//
// (Notifications can be sent to Nico de Vries or directly to AIP)
//
// Notes:
//    This routine only works in MS-DOS real mode. Not under Windows,
//    DesqView, OS/2 etc.


#include <dos.h>
#include <stdlib.h>

// onld interrupt handler
static void interrupt (*vivOld)(void);

// needed to make clock run OK
static int iIntCtr;

// storage for temporary information
static volatile int iActive;
static volatile unsigned uCtr;
static int iPtr;
static unsigned puArray[32];

static void interrupt Handler (void){
   // 0.86 ms timeslice
   outportb (0x43,0x36);
   outportb (0x40,0x0);
   outportb (0x40,0x4);

   if (iActive){
      if (iPtr==32){
	 iActive=0;
	 iPtr=0;
      } else {
	 puArray[iPtr] = uCtr;
	 iPtr++;
      }
   }

   if (++iIntCtr==64){
      (*vivOld)(); // 18.2 times per second call
      iIntCtr=0;
   }else
      outportb (0x20, 0x20);
}

// remove handler
void RemHan (void){
   disable();
   setvect (0x08, vivOld);
   enable();

   // set timer to 18.2 times/second
   outportb (0x43,0x36);
   outportb (0x40,0);
   outportb (0x40,0);
}

// install handler
void InstHan (void){
   disable();
   vivOld = getvect (0x08); // timer
   setvect (0x08, Handler);
   enable();
}

// generate true random number
unsigned long Random (void){
   int i,j;
   unsigned long res=0;
   unsigned long tmp;
   InstHan();
   // to be on the safe side we intialize uCtr with the timer
   uCtr = peek(0x40,0x6c) & 0xFF;

   // repeat 8 times to change random "bursts" into random numbers
   for (j=0;j<8;j++){
      iActive=1;
      while (iActive){
	 while (uCtr) uCtr--;
	 uCtr = 0xFF;
      }
      for (i=0;i<32;i++)
	 tmp = (tmp<<1) ^ (puArray[i] & 1);
      res^=tmp;
   }
   RemHan();
   return res;
}

#if 0 // TEST SOFTWARE

void main (void){
   int i,j;
   unsigned long r;
   printf ("RANDOM [1.0] 'generates 16 true random numbers'\n");
   printf ("(C) Copyright 1992, All rights reserved, AIP.\n");
   printf ("'AS IS' use allowed for non commercial use only.\n");
   printf ("AIP is not responsible for any damages resulting from use of this program.\n\n");
   for (i=0;i<16;i++){
      r = Random();
      printf ("%08lX ",r);
      for (j=0;j<32;j++){
	 printf ("%d",(r&(1L<<(31-j)))!=0);
      }
      printf ("\n");
   }
}
#endif

#if 1 // TEST SOFTWARE #2, graphical investigation

// Can be used for testing AIP random generator.
// At least hercules, EGA or VGA needed.
// Source uses Borland BGI.

#include <graphics.h>

static int ctr[256];

// update histogram
void upd (int i){
   ctr[i]++;
   line (i+266,255-ctr[i],i+266,255-ctr[i]);
}

void main (void){
   int i,j;
   unsigned long r;

   // INITIALIZE GRAPHICS
   int gdriver = DETECT, gmode, errorcode;
   initgraph(&gdriver, &gmode, "");
   errorcode = graphresult();
   if (errorcode != grOk){exit(1);}


   for (;;){
      // get 4 values in the range 0..255
      unsigned long r = Random();
      int x1 = r&255;
      int x2 = (r>>8)&255;
      int y1 = (r>>16)&255;
      int y2 = (r>>24)&255;

      // update histogram
      upd (x1); upd (x2); upd (y1); upd (y2);

      // update 2D image
      line (x1,y1,x1,y1);
      line (x2,y2,x2,y2);

      if (kbhit()) break;
   }

   closegraph();
}
#endif

