/*--- getsys.c -- System-dependent routines to return various
 *  information from the system or user.
 *
 *  I predict that this module will be the least portable of
 *  the modules in "rpem", despite efforts on my part to adapt
 *  to different systems.
 *
 *  Mark Riordan  riordanmr@clvax1.cl.msu.edu   10 March 1991
 *  This code is hereby placed in the public domain.
 */

#include <stdio.h>
#include <stdlib.h>
#include "getsyspr.h"

#ifdef MSDOS
#include <time.h>
#include <string.h>
#ifdef __TURBOC__
#include <dos.h>
#else
#include <memory.h>
#include <malloc.h>
#endif
#include <conio.h>
#endif

#ifdef UNIX
#include <sys/time.h>
#endif

#ifdef USEBSD
#include <sys/types.h>
#include <sys/resource.h>

#ifdef ultrix
#include <sys/stat.h>
#else
#include <sys/vfs.h>
#endif

#ifdef __MACH__
#include <libc.h>
#endif
#endif

#ifdef sun
#include <unistd.h>
#endif
#ifdef ultrix
#include <unistd.h>
#endif
#ifdef __convexc__
#include <unistd.h>
#endif
#ifdef sgi
#include <unistd.h>
#endif
#ifdef HP
#include <sys/unistd.h>
#endif

#ifdef UNIX
#include <pwd.h>
#endif

#ifdef SYSV
#include <sys/types.h>
#ifdef sgi
#include <sys/statfs.h>
#else
#include <statfs.h>
#endif
#endif

#ifndef MSDOS
extern FILE *userstream;
#endif

/*--- function GetRandomBytes ----------------------------------------
 *
 *  Return an array of random bytes depending upon certain
 *  transient system-dependent information.
 *  Don't bet your life on the "randomness" of this information.
 *
 *  Entry    maxbytes is the maximum number of bytes to return.
 *
 *  Exit     bytes    contains a number of bytes containing
 *                    such information as the time, process
 *                    resources used, and other information that
 *                    will change from time to time.
 *           Returns the number of bytes placed in "bytes".
 */
int
GetRandomBytes(bytes,maxbytes)
unsigned char *bytes;
int maxbytes;
{
   int numbytes = 0, thissize;
#define LINESIZE 120

#ifdef MSDOS

   time_t myclock;
   time_t mytime;
   size_t biggestfree;

   /* Obtain the elapsed processor time (not really too useful).
    */
   if((thissize=sizeof(myclock)) <= maxbytes) {
      myclock = clock();
      memcpy(bytes+numbytes,&myclock,thissize);
      numbytes += thissize;
      maxbytes -= thissize;
   }

#ifndef __TURBOC__
   /* Get the size of the largest free memory block. */

   if((thissize=sizeof(size_t)) <= maxbytes) {
      biggestfree = _memmax();
      memcpy(bytes+numbytes,&biggestfree,thissize);
      numbytes += thissize;
      maxbytes -= thissize;
   }
#endif

   /* Get the time of day.  */

   if((thissize=sizeof(mytime)) <= maxbytes) {
      time(&mytime);
      memcpy(bytes+numbytes,&mytime,thissize);
      numbytes += thissize;
      maxbytes -= thissize;
   }
#endif

#ifdef UNIX
   {
      struct timeval tm;
      struct timezone tz;

   /* Get the time of day.  */

      if((thissize=sizeof(tm)) <= maxbytes) {
         gettimeofday(&tm,&tz);
         memcpy(bytes+numbytes,&tm,thissize);
         numbytes += thissize;
         maxbytes -= thissize;
      }
   }
#endif

#ifdef USEBSD
#ifndef HP
#define DOGETRUSAGE
#endif
#endif

#ifdef DOGETRUSAGE

#define TYPTIME struct rusage
#define RUSAGE_SELF 0
   {
      struct rusage myusage;
      struct timeval tm;
      struct timezone tz;

      /* Get the process resource utilization. */

      if((thissize=sizeof(myusage)) <= maxbytes) { 
         getrusage(RUSAGE_SELF,&myusage);
         memcpy(bytes+numbytes,&myusage,thissize);
         numbytes += thissize;
         maxbytes -= thissize;
      }
   }
#endif

#ifdef SYSV
#define DOSTAT
#endif
#ifdef HP
#define DOSTAT
#endif

#ifdef DOSTAT
   {
      struct statfs buf;
      char path[LINESIZE];

      /* Obtain information about the filesystem on which the user's
       * home directory resides.
       */

      if((thissize=sizeof(struct statfs)) <= maxbytes) {
         GetUserHome(path,LINESIZE);
         statfs(path, &buf, sizeof(struct statfs), 0);
         memcpy(bytes+numbytes,&buf,thissize);
         numbytes += thissize;
         maxbytes -= thissize;
      }
   }
#endif

#ifdef USEBSD
   {
#ifdef ultrix
#define STATFS struct stat
#else
#define STATFS struct statfs
#endif
      STATFS buf;
      char path[LINESIZE];

      /* Obtain information about the filesystem on which the user's
       * home directory resides.  This is only slightly different
       * between SYSV and BSD.
       */

      if((thissize=sizeof(STATFS)) <= maxbytes) {
         GetUserHome(path,LINESIZE);
         statfs(path, &buf);
         memcpy(bytes+numbytes,&buf,thissize);
         numbytes += thissize;
         maxbytes -= thissize;
      }
   }
#endif

   return (numbytes);
}

#ifdef MSDOS
#include <conio.h>

#define TIMER_PORT                     0x40
#define TIMER_MODE_OFFSET              3
#define TIMER_SHIFT_SELECT_COUNTER     6
#define TIMER_SHIFT_READ_LOAD          4
#define TIMER_SHIFT_MODE               1
#endif


/*--- function GetUserInput -----------------------------------------
 *
 *  Get a string of bytes from the user, intended for use as
 *  a seed to a pseudo-random number generator or something similar.
 *
 *  Return not only those bytes but also an array of timing
 *  information based on the inter-keystroke times.
 *  This maximizes the amount of "random" information we obtain
 *  from the user.
 *
 *    Entry *num_userbytes   is the maximum number of bytes we can
 *                              put in userbytes.
 *          *num_timebytes   is the maximum number of bytes we can
 *                              put in timebytes.
 *          echo             is TRUE iff we want to echo characters
 *                           typed by the user.  (Non-echoing is
 *                           implemented only for MS-DOS.)
 *
 *    Exit  userbytes        is an array of bytes entered by the
 *                           user, not including the newline.
 *          *num_userbytes   is the number of data bytes in this array.
 *          timebytes        is an array of bytes reflecting inter-
 *                           keystroke timings.  (Only for MS-DOS.)
 *          *num_timebytes   is the number of data bytes in this array.
 */
void
GetUserInput(userbytes,num_userbytes,timebytes,num_timebytes,echo)
unsigned char userbytes[];
int *num_userbytes;
unsigned char timebytes[];
int *num_timebytes;
int echo;
{
#ifdef MSDOS
#ifndef OS2
#define TIMER_OK
#endif
#endif

   int max_user = *num_userbytes, max_time = *num_timebytes;
   int done = 0;
   unsigned char *userby = userbytes;
   unsigned char *timeby = timebytes;
   int ch;
   unsigned int counter = 1;
#ifdef TIMER_OK
   unsigned char byte1, byte2;
   int databyte;
#endif


#ifdef UNIX
   FILE *userstream;
#endif

#ifdef TIMER_OK
   /* Set the timer to its highest resolution.
    * This gives 65536*18.2 ticks/second--pretty high resolution.
    * There *are* some things
    * that a PC can do that a multisystem system can't!
    */

   databyte = 2<<TIMER_SHIFT_SELECT_COUNTER | 3<<TIMER_SHIFT_READ_LOAD |
      3<<TIMER_SHIFT_MODE;
   outp(TIMER_PORT+TIMER_MODE_OFFSET,databyte);
   outp(TIMER_PORT+2,0xff&counter);
   outp(TIMER_PORT+2,counter>>8);
   byte1 = (unsigned char) inp(TIMER_PORT);
   byte2 = (unsigned char) inp(TIMER_PORT);
   if(max_time > 0) {
      *(timeby++) = byte2;
      max_time--;
   }
#endif

#ifdef UNIX
   userstream = fopen("/dev/tty","r");
   if(!userstream) {
      fputs("Unable to read from terminal\n",stderr);
   }
#endif

   while(!done) {
#ifdef MSDOS
      if(echo) {
         ch = getche();
      } else {
         ch = getch();
      }
#ifdef TIMER_OK
      byte1 = (unsigned char)inp(TIMER_PORT);
      byte2 = (unsigned char)inp(TIMER_PORT);
#endif
#else
      ch = fgetc(userstream);
#endif
      done = (ch=='\r') || (ch=='\n');
      if(!done) {
         if(max_user > 0) {
            *(userby++) = (unsigned char)ch;
            max_user--;
         }
#ifdef TIMER_OK
         if(max_time > 0) {
            *(timeby++) = byte2;
            max_time--;
         }
#ifdef DEBUG
         printf("ch=%c byte=%d\n",ch,byte2);
#endif
#endif
      }
   }
#ifdef MSDOS
   fputc('\n',stderr);
#else
   if(!echo) fputc('\n',stderr);
#endif

   *num_userbytes = userby - userbytes;
   *num_timebytes = timeby - timebytes;

#ifdef UNIX
   fclose(userstream);
#endif
}


/*--- function GetUserName ------------------------------------------
 *
 *  Return the name of the user.
 *  Under Unix, get the user's name using time-honored techniques.
 *  Under MS-DOS, grab the value of an environment variable, or
 *  just use "me" if there's no such variable.
 *
 *  Entry:  namelen  is the size of buffer "name".
 *
 *  Exit:   name     is the name of the user, zero-terminated.
 */
void
GetUserName(name,namelen)
char *name;
int namelen;
{
   char *cptr;
#ifdef UNIX
   struct passwd *pwptr;
   char line[LINESIZE];
#define DEFUSERNAME  "me"
#endif

#define USER_NAME_ENV        "USER_NAME"
#define USER_NAME_DEFAULT    "me"


#ifdef UNIX

   cptr = getlogin();
   if(!cptr) {
      pwptr = getpwuid(getuid());
      if(pwptr) {
         cptr = pwptr->pw_name;
      } else {
         cptr = NULL;
      }
   }
#else
   cptr = NULL;
#endif
   if(!cptr) {
      cptr = getenv(USER_NAME_ENV);
      if(!cptr) cptr = USER_NAME_DEFAULT;
   }
   strncpy(name,cptr,namelen);
}

/*--- function GetUserAddress -------------------------------------------
 *
 *  Return the zero-terminated user's email address.
 *  For non-Unix hosts, it's just the user's name.
 *  For Unix, it's the name followed by @<hostname>.
 *
 *  Entry:  addrlen  is the size of the buffer "address".
 *
 *  Exit:   address  contains the user's email address (as close
 *                   as we can figure it).
 */

void
GetUserAddress(address,addrlen)
char *address;
int addrlen;
{
#ifdef UNIX
#define LINESIZE 120
   char line[LINESIZE];
#endif

   GetUserName(address,addrlen);
#ifdef UNIX
   /* Add "@hostname" to the username.  */
   if(!gethostname(line,LINESIZE)) {
      strncat(address,"@",LINESIZE-strlen(address));
      strncat(address,line,LINESIZE-strlen(address));
   }
#endif
}

/*--- function GetUserHome --------------------------------------
 *
 *  Return the pathname of the user's home directory.
 *  Implemented only under Unix; for other systems, just returns
 *  a string of 0 length followed by a zero byte.
 *
 *  Entry:  homelen   is the size of the "home" buffer.
 *
 *  Exit:   home      contains the home pathname, followed by a
 *                    zero byte.
 */
void
GetUserHome(home,homelen)
char *home;
int homelen;
{
#ifdef UNIX
   struct passwd *pwptr;

   pwptr = getpwuid(getuid());

   if(pwptr) {
      strncpy(home,pwptr->pw_dir,homelen);
   } else {
      home[0] = '\0';
   }
#else
   home[0] = '\0';
#endif
}
