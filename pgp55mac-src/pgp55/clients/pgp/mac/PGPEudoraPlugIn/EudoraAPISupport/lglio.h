/*  ======================================================================
  LGL's little IO package for reading files one line at a time
  This one expects lines to end in \r\n. (Internet canonical)
  
     
   Last Edited: Apr 16, 1996

   Laurence Lundblade <lgl@qualcomm.com> 

   Copyright 1995, 1996 QUALCOMM Inc.
 
  
  Usage:
    Open the file with lglOpen and get the Handle back
    Lock the handle you get back until you're done reading
    Each call to lglRead returns a line. A pointer to the next line
      and its length is returned.  The data is stored in the IO packages
      own buffer so you should copy it out. The lines are never null
      null terminated. If the lines in the files longer than the
      the buffer then chunks of the file are returned a buffer at a
      time.  You can tell if you got a line or not by checking to see
      if the last two characters fetched are \r\n.
    Close the file. This unlocks and Disposes of the Handle
*/

#define kLglBSize (8192)


typedef struct lglIO {
	short refNum, eof;
	char  buffer[kLglBSize], *line;
	long  bytesInBuf;
} **lglIOHandle;
	
OSErr lglOpen(lglIOHandle *, FSSpec);
OSErr lglRead(lglIOHandle, char **, long *);
void  lglClose(lglIOHandle);