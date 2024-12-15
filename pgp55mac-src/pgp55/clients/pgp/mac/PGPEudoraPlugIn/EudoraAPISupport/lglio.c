/* ======================================================================

  LGL's little I/O functions for reading a file a line at a time
   
   Last Edited: Apr 16, 1996

   Laurence Lundblade <lgl@qualcomm.com> 

   Copyright 1995, 1996 QUALCOMM Inc.
*/

#include "lglio.h"

/* This mostly for reading text files with \r\n EOL,
Hmm, what about binary MIME? */
/* To do: distinguish read error from EOF */

static long findEol(char *, long);

/* ---------------------------------------------------
  Args: io  -- lglIO structure to fill in
        Fsp -- Name of file to open
 */
OSErr lglOpen(lglIOHandle *io, FSSpec Fsp)
{
	OSErr err;
	short refNum;
	
	*io = (lglIOHandle)NewHandle(sizeof(struct lglIO));
	err = FSpOpenDF(&Fsp, fsRdPerm, &refNum);
	if(err) {
	    DisposeHandle((Handle)*io);
	    *io = NULL;
		return(err);
	}
    (**io)->refNum     = refNum;
	(**io)->line       = (**io)->buffer;
	(**io)->eof        = 0;  
	(**io)->bytesInBuf = 0;
	return(noErr);
}


/* ---------------------------------------------------
  Args: io -- where to read from
        line -- place where pointer to read data is returned
        leng -- place where pointer to length is returned
        
  Warning - io MUST BE LOCKED before calling
 */
OSErr lglRead(lglIOHandle ioHandle, char **line, long *leng)
{
	long  numRead, eol, numLeft;
	short err;
	struct lglIO *io;
	
	io = *ioHandle; // This is locked before we got called
	
	if(io->bytesInBuf != 0)
		// Check to see if there's a line in the buffer
		eol = findEol(io->line, io->bytesInBuf - (io->line - io->buffer));
	else
		// No data in buffer, force a read
		eol = -1;
		
	while(eol < 0 &&
		io->bytesInBuf - (io->line - io->buffer) < kLglBSize && !io->eof)
	{
		// Try to fill up the buffer
		numLeft = io->bytesInBuf == 0 ?
				0  : io->bytesInBuf - (io->line - io->buffer);
		BlockMove(io->line, io->buffer, numLeft);
		numRead = kLglBSize - numLeft;
		err = FSRead(io->refNum, &numRead, io->buffer + numLeft);
	    io->bytesInBuf = numRead + numLeft;
		if(err)
			io->eof =1;
		io->line = io->buffer;
		// see if we can find a line in the buffer again
  		eol = findEol(io->line, io->bytesInBuf - (io->line - io->buffer));
	}
	
	// Check for end of file
	if(io->eof && io->buffer + io->bytesInBuf <= io->line) {
		return(-1);
    }
		
	*line = io->line;
	if(eol > 0) {
		// Return the full line of the buffer
		*leng    = eol;
		io->line = io->line + eol; // 2 skips \r\n
	} else {
		// No full line could be found, return what we've got
		*leng          = io->bytesInBuf;
		io->line       = io->buffer;
		io->bytesInBuf = 0;
	}
	return(noErr);
}


void lglClose(lglIOHandle ioHandle)
{
	if ( ioHandle != NULL ) {
		FSClose((*ioHandle)->refNum);
		HUnlock((Handle)ioHandle); // Be sure it's unlocked 
		DisposeHandle((Handle)ioHandle);
		ioHandle = NULL;
	}
}

	

static long findEol(char *buffer, long length)
{
	char *p, *p_end; 
	for(p = buffer, p_end = buffer + length; p < p_end; p++)  
		if(*p == '\015' && p < p_end -1 && *(p+1) == '\012')
			return(p - buffer + 2);
	return(-1);
}
		