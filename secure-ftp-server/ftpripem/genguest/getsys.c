#include <stdio.h>
#include <ctype.h>
#include <strings.h>
#include <malloc.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/termio.h>
#include <sys/resource.h>
#include <sys/vfs.h>
#include <pwd.h>

#include "getsyspr.h"

extern int Debug;
extern FILE *DebugStream;

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
   {
      struct timeval tm;
      struct timezone tz;

   /* Get the time of day.  */

      if((thissize=sizeof(tm)) <= maxbytes) {
         gettimeofday(&tm,&tz);
		CopyRandomBytes(&tm,thissize,bytes,&numbytes,&maxbytes,
		 "time of day (gettimeofday)");	
      }
   }

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
		CopyRandomBytes(&myusage,thissize,bytes,&numbytes,&maxbytes,
		 "process resource utilization");	
      }
   }
#endif

#ifdef SYSV
#define DOSTAT
#endif

#ifdef DOSTAT
   {
      struct statfs buf;
      char *path;

      /* Obtain information about the filesystem on which the user's
       * home directory resides.
       */

      if((thissize=sizeof(struct statfs)) <= maxbytes) {
         GetUserHome(&path);
         statfs(path, &buf, sizeof(struct statfs), 0);
		CopyRandomBytes(&buf,thissize,bytes,&numbytes,&maxbytes,
		 "file system stats on user's home device");	
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
      char *path;

      /* Obtain information about the filesystem on which the user's
       * home directory resides.  This is only slightly different
       * between SYSV and BSD.
		 */

      if((thissize=sizeof(STATFS)) <= maxbytes) {
         GetUserHome(&path);
         statfs(path, &buf);
		CopyRandomBytes(&buf,thissize,bytes,&numbytes,&maxbytes,
		 "file system stats on user's home device");	
      }
   }
#endif


	return (numbytes);
}

/*--- function CopyRandomBytes -------------------------------------
 *
 *  Copy system-derived data into the user's output buffer.
 *  Optionally report on what's going on.
 *
 *  Entry:	thisBuf		contains "random" bytes.
 *				thisSize	is the number of bytes in inBuf to add.
 *				userbuf  is the start of the user buffer.
 *				numbytes	is the current index into userbuf for where we
 *							should add this data.
 *				maxbytes	is the number of bytes left in userbuf.
 *				message 	is a text string to output for debugging.
 *
 *	 Exit:	numbytes has been updated.
 *				maxbytes has been updated.
 */
void
CopyRandomBytes(thisBuf,thisSize,userBuf,numBytes,maxBytes,message)
void *thisBuf;
int  thisSize;
unsigned char *userBuf;
int *numBytes;
int *maxBytes;
char *message;
{
	int j, bytes_to_copy;

	bytes_to_copy = thisSize <= *maxBytes ? thisSize : *maxBytes;
	if(Debug>1) {
		fprintf(DebugStream,"%d bytes of %s obtained: ",bytes_to_copy,
		 message);
		for(j=0; j<bytes_to_copy; j++) {
			if(j%36 == 0) fprintf(DebugStream,"\n ");
			fprintf(DebugStream,"%-2.2x",((unsigned char *)(thisBuf))[j]);
		}
		putc('\n',DebugStream);
	}
   memcpy(userBuf+*numBytes,thisBuf,bytes_to_copy);
   *numBytes += bytes_to_copy;
   *maxBytes -= bytes_to_copy;
}

/*--- function GetUserHome --------------------------------------
 *
 *  Return the pathname of the user's home directory.
 *  Implemented only under Unix; for other systems, just returns
 *  a string of 0 length followed by a zero byte.
 *
 *  Entry:  home      points to a pointer which we desire to be
 *                    be updated to point to the user's home dir.
 *
 *  Exit:   home      contains the home pathname, followed by a
 *                    zero byte.
 */
void
GetUserHome(home)
char **home;
{
#if defined(UNIX) && !defined(MAX_PORTABLE)
   struct passwd *pwptr;

   pwptr = getpwuid(getuid());

   if(pwptr) {
      strcpyalloc(home,pwptr->pw_dir);
   } else {
      strcpyalloc(home,"");
   }
#else
	strcpyalloc(home,"");
#endif
}

