/*____________________________________________________________________________
____________________________________________________________________________*/

/* ======================================================================

   Basic RFC-1847 Security-Multipart implementation
                 
   Last Edited: Apr 18, 1996
   
   Filename: rfc1847.c

   Laurence Lundblade <lgl@qualcomm.com>

   Copyright 1995, 1996 QUALCOMM Inc.
   
*/

#include <ctype.h>
#include "copycat.h"
#include "rfc1847.h"
#include "lglio.h"

#include "Types.h"

static OSErr FSWriteBuffer(short refNum, long size, Ptr buffer);
static void  canonLineDelim(char *buffer, long *buf_size);
static short match_MIME_boundary(char *line,
				long len, Str255 boundary, short *final);
static short transfer_encodings(char *string, long len);


#define kCTEnone 0
#define kCTEQP   1
#define kCTEB64  2


/* ---------------------------------------------------------------------
    Create a multipart/signed message

args: signatureType    - Mime type of the signature format being used
      micalg           - Name of message integrity check algorithm
      signatureData    - File with signed data -
      					must have full canonical MIME with headers
      signature        - File containing signature -
      						must be 7bit text with local EOL (\r)
      mimeOutput       - Existing file where result is written 
      MimeBoundary     - Returns the mime boundary generated  
      
To do: Do B64 encoding for the signature (OK for PGP/MIME)
       Maybe should be more concerned about boundary uniqueness

returns: OSErr
*/
short createMultipartSigned(
     Str255  signatureType,
     Str255  micalg, 
     FSSpec *signedData,
     FSSpec *signature,
     FSSpec *mimeOutput,
     Str255  MimeBoundary)
{
    short           OutRefNum, InRefNum;
    short           err;
    char 	       *Buf, **BufHandle;
    short           BufLen;
	unsigned long	secs;
	Str63           NumStringBuf;
	long            numRead;
	      
	BufHandle = NewHandle(sizeof(char) * 4096);
	HLock(BufHandle);
	Buf = *BufHandle;
	
	/* Get the output file open */
  	err = FSpOpenDF(mimeOutput, fsWrPerm, &OutRefNum);
  	if(err) 
  	    goto writeErr;
    SetEOF(OutRefNum, 0L);
 
 	/* Generate the boundary parameter */
    GetDateTime(&secs);
    copyPP("\phe-loved-the-", MimeBoundary);
    NumToString((long)secs < 0 ? -(long)secs : (long)secs, NumStringBuf);
    catPP(NumStringBuf, MimeBoundary);
    catPP("\p-flying-e's", MimeBoundary); 
	
    /* ==== Begin writing the output ==== */	
    /* The m/e MIME headers */
    BufLen = 0;
    BufLen = catPB("\pContent-type: multipart/signed; boundary=", BufLen, Buf);
    BufLen = catPB(MimeBoundary, BufLen, Buf);
    BufLen = catPB("\p;\015\012   micalg=\"", BufLen, Buf);
    BufLen = catPB(micalg, BufLen, Buf);
    BufLen = catPB("\p\";\015\012   protocol=\"", BufLen, Buf);
    BufLen = catPB(signatureType, BufLen, Buf);
    BufLen = catPB("\p\"\015\012\015\012", BufLen, Buf);
    /* The boundary to start first part */
    BufLen = catPB("\p--", BufLen, Buf);
    BufLen = catPB(MimeBoundary, BufLen, Buf);
    BufLen = catPB("\p\015\012", BufLen, Buf);
    err = FSWriteBuffer(OutRefNum, BufLen, (Ptr)Buf);
    if(err)
	  goto writeErr;
	  	
  	/* The PGP signed data (already has canon encoding) */
    err = FSpOpenDF(signedData, fsRdPerm, &InRefNum);
    if(err)
	  goto writeErr;
    numRead = sizeof(Buf);
  	err = FSRead(InRefNum, &numRead, (Ptr)Buf);
  	while((err == noErr || err == -39) && numRead > 0)
    {
        err = FSWriteBuffer(OutRefNum, numRead, (Ptr)Buf);
 	    if(err)
	  	  goto writeErr;
	      /* Read the next chunk in */
		numRead = sizeof(Buf);
		err = FSRead(InRefNum, &numRead, (Ptr)Buf);
	}
	FSClose(InRefNum);

	/* Tie off first part and add headers for the second part */
	BufLen = 0;
	BufLen = catPB("\p\015\012", BufLen, Buf);
	BufLen = catPB("\p--",  BufLen, Buf);
    BufLen = catPB(MimeBoundary, BufLen, Buf);
    BufLen = catPB("\p\015\012", BufLen, Buf);
	BufLen = catPB("\pContent-Type: ", BufLen, Buf);
	BufLen = catPB(signatureType, BufLen, Buf);
	BufLen = catPB("\p\015\012\015\012", BufLen, Buf);
	err = FSWriteBuffer(OutRefNum, BufLen, (Ptr)Buf);
	if(err)
	  goto writeErr;
	
  	/* The signature (assume 7bit local text, e.g., ASCII armour) */
    err = FSpOpenDF(signature, fsRdPerm, &InRefNum);
    if(err)
	  goto writeErr;
    numRead = sizeof(Buf)/2; /* half-fill makes room for canonLineDelim() */
	err = FSRead(InRefNum, &numRead, (Ptr)Buf);
	while((err == noErr || err == -39) && numRead > 0)
	{
        canonLineDelim(Buf, &numRead);
		err = FSWrite(OutRefNum, &numRead, (Ptr)Buf);
		if(err)
		  goto writeErr;
		/* Read the next chunk in */
	    numRead = sizeof(Buf)/2;
	    err = FSRead(InRefNum, &numRead, (Ptr)Buf);
    }
  	FSClose(InRefNum);

	/* Tie off the multipart message */
    BufLen = 0;
	BufLen = catPB("\p\015\012--", BufLen, Buf);
	BufLen = catPB(MimeBoundary, BufLen, Buf);
	BufLen = catPB("\p--\015\012", BufLen, Buf);
	err = FSWriteBuffer(OutRefNum, BufLen, (Ptr)Buf);
	if(err)
	  goto writeErr;
 	FSClose(OutRefNum);
    HUnlock(BufHandle);
    DisposeHandle(BufHandle);
    
	return(noErr);

 writeErr:
    HUnlock(BufHandle);
    DisposeHandle(BufHandle);

    FSClose(OutRefNum);
    FSClose(InRefNum);
    return(err);
}



/* -------------------------------------------------------------------------
     Minimal MIME security/multiparts parser + transfer decoder  

The file on the inFsp must be a full MIME body in canonical form. It should
be of type multipart/signed with the boundary the same as the
one passed in (from
a previous partial MIME parse).   
  
args: inFsp:     The file with the security multipart in it
      p1Fsp:     The first part of the security multipart (encoding stays on,
       must exist)
      p2Fsp:     The second part (encoding always removed, must exist)
      boundaryHandle: The boundary string (not parsed here,
      must be given to us)
           
returns: standard Mac error code or -2000 for some MIME parsing sort of error
   
to do: test with werid input, transfer decoding
*/ 
OSErr ParseMultipart(
     FSSpec    *inFsp,
     FSSpec    *p1Fsp,
     FSSpec    *p2Fsp,
     StringPtr *boundaryHandle)
{
	char 		*line;
	long		 len;
	short 		 theOutRefNum, transferEncoding;
	lglIOHandle  inIOHandle;
	const		 kCTElen = 26; // Length of "Content-Transfer-Encoding:"
	OSErr		 err;
	Str255       boundary;
	Boolean      thisLineHasEOL, lastLineHasEOL;
	
	/* Convert boundary to a Pascal string */
	HLock( (Handle) boundaryHandle);
	copyBP( (const char *)*boundaryHandle,
		GetHandleSize( (Handle) boundaryHandle), boundary);
	HUnlock( (Handle)  boundaryHandle);
	
	/* Open the input file using lgl's mini r/o stream i/o functions */
	err = lglOpen(&inIOHandle, *inFsp);
	if(err) {
		return(err);
	}
	HLock((Handle)inIOHandle);
	
	/* ---- Get first out of the multipart - leave transfer encoding on ---- */	
	if(p1Fsp) {
	    err = FSpOpenDF(p1Fsp, fsWrPerm, &theOutRefNum);
	    if(err) goto err1;
	}
    
	/* Get first instance of boundary */
	while((err = lglRead(inIOHandle, &line, &len)) == noErr)
	  if(match_MIME_boundary(line, len, boundary, NULL))
			break;
    if(err) {
        err = -2000; /* Hit end of file: some MIME parsing error */
        goto err1;
    }
    
    /* Copy out the first part - this should handle binary MIME too! */
    /* The flags deal with MIME's \r\n convention before a boundary */
    lastLineHasEOL = 0;
    while(lglRead(inIOHandle, &line, &len) == noErr) {
        if(lastLineHasEOL && match_MIME_boundary(line, len, boundary, NULL))
		  break;
	    thisLineHasEOL = len > 1 &&
	    	line[len-1] == '\012' && line[len-2] == '\015';
		if(lastLineHasEOL) 
		  if(p1Fsp && (err = FSWriteBuffer(theOutRefNum, 2,
		  		"\015\012")) != noErr )
		    goto err1;
		   
		if(p1Fsp && 
		   (err = FSWriteBuffer(theOutRefNum,
		   	thisLineHasEOL ? len-2 : len, (Ptr)line)) != noErr)
		  goto err1;
		lastLineHasEOL = thisLineHasEOL;
    }

	FSClose(theOutRefNum);
    if(err) {
        err = -2000;/* Hit end of file: some MIME parsing error */
        goto err1;
    }    

	/* ---- do the second part, this time remove transfer encoding ---- */
	err = FSpOpenDF(p2Fsp, fsWrPerm, &theOutRefNum);
	if(err)
		 goto err1;
	/* Read until a blank line checking for transfer encoding */
	transferEncoding = kCTEnone;
	while(lglRead(inIOHandle, &line, &len) == noErr && len > 2)
		if(cmpCB(line, "Content-Transfer-Encoding:",
			len>kCTElen ? kCTElen : len, 1) == 0)
			transferEncoding = transfer_encodings(line + kCTElen,
								len - kCTElen);	
	
	if(len != 2) {
		err = -2000; /* Some MIME parse error */
		goto err1;
	}		

    /* Copy out the second part - this should handle binary MIME too! */
    lastLineHasEOL = 0;
    while(lglRead(inIOHandle, &line, &len) == noErr) {
        if(lastLineHasEOL && match_MIME_boundary(line, len, boundary, NULL))
		  break;
		switch(transferEncoding) {
		  case kCTEnone:
		    thisLineHasEOL = len > 1 && line[len-1] == '\012' &&
		    line[len-2] == '\015';
		    if(lastLineHasEOL) 
		      if( (err = FSWriteBuffer(theOutRefNum, 2, "\015\012")) != noErr)
		        goto err1;
		    
		    if( (err = FSWriteBuffer(theOutRefNum,
		    	thisLineHasEOL ? len-2 : len, (Ptr)line)) != noErr)
		      goto err1;
		    lastLineHasEOL = thisLineHasEOL;
		    break;
				
		   case kCTEQP:
		   case kCTEB64:
		     break;
		     /* Not handling any other transfer-encoding's yet */
		}
    }

    /* All worked OK! */
	err = noErr;
	
err1:
	FSClose(theOutRefNum);
	lglClose(inIOHandle); /* This unlocks and disposes */	
    return(err);
}



/* =======================  Private functions ========================== */

/* -------------------------------------------------------------------------
  See if an input string matches the given MIME boundary
  
args: line     - the input line, not necessarilly NULL terminated
      len      - the length of the input line
      boundary - Pascal string with MIME boundary string
      final    - The flag final will be set if it is the final MIME boundary
returns: 1 if it matches, 0 if not
*/
static short match_MIME_boundary(
char *line, long len, Str255 boundary, short *final)
{
  short bLen = *boundary;
	/* Get the length correct: the leading "--",
	trailing "\015\012" and sometimes
	    trailing "--" */
	if((len != bLen + 4 && len != bLen + 6) ||
	    line[0] != '-' || line[1] != '-' ||
	    line[len-1] != '\n' || line[len-2] != '\r')
		return(0);
	/* Match the actual boundary string	*/ 
	if(cmpCB(line+2, (const char *)boundary+1, bLen, 0))
		return(0);
	/* If it's final match the "--" before the "\015\012" */
	if(len != bLen + 4 && (line[len-3] != '-' || line[len-4] != '-'))
		return(0);
	/* Have got a match -- Set flag if requested */
	if(final != NULL)
		*final = len == bLen ? 0 :1;
	return(1); 
}
	


/* -------------------------------------------------------------------------
    Write out the buffer -- however many FSWrite calls it takes
 */
static OSErr FSWriteBuffer(short refNum, long size, Ptr buffer)
{
	short err;
	long  numWrite, leftToWrite;
	
	err = noErr;
	leftToWrite = size;
	while(!err && leftToWrite)
	{
		numWrite = leftToWrite;
	    err = FSWrite(refNum, &numWrite, buffer + (size - leftToWrite));
		leftToWrite -= numWrite;
	}
	return(err);
}	



/* -------------------------------------------------------------------------
   Turn the local newline convention (\r) into Internet canonical (\r\n)
   Buffer must be twice the size of the data actually in it to handle 
   worst case of a string of newlines as input
*/
static void
canonLineDelim(char *buffer, long *buf_size)
{
	char *b, *b_end, *n_b;
	long  newline_count;
	
	/* First count the newlines */
	newline_count = 0;
	for(b = buffer, b_end = buffer + *buf_size; b < b_end; b++)
		if(*b == '\r')
			newline_count++;
	for(b = b_end - 1, n_b = b_end -1 + newline_count;
		b >= buffer; *n_b-- = *b--)
		if(*b == '\r')
			*n_b-- = '\n';
	*buf_size += newline_count;
}	


/* -------------------------------------------------------------------------
   Match the transfer encoding value string
   
 */
static short
transfer_encodings(char *string, long len)
{
	char *s_end;
	short token_len;
	
	for(s_end = string + len; isspace(*string) && len;	string++, len--)\
		{
		}
		
	for(token_len = 0; !isspace(string[token_len]) && len; len--, token_len++)
		{
		}
	
	if((int)cmpCB(string, "base64", token_len, 1) == 0)
		return(kCTEB64);
	if(cmpCB(string, "quoted-printable", token_len, 1) == 0)
        return(kCTEQP);
   return(kCTEnone);
}
