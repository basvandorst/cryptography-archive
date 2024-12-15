/*____________________________________________________________________________
____________________________________________________________________________*/

/* ======================================================================

   Eudora Extended Message Services API SDK 1.0b3 (July 11 1996)
   This SDK supports EMSAPI version 1
   Copyright 1995, 1996 QUALCOMM Inc.
   Send comments and questions to <emsapi-info@qualcomm.com>

   Function to encode and decode binary data to quoted-printable data.
	 Code extracted from Eudora. Based on Steve Dorner's code.
                 
   Last Edited: July 23, 1996
   
   Filename: ctencode.c

   Myra Callen <t_callen@qualcomm.com>
   Laurence Lundblade <lgl@qualcomm.com>

   
*/

#include "copycat.h"
#include "rfc822.h"
#include "ctencode.h"

#define SKIP -1
#define FAIL -2
#define PAD  -3
#define kNewLine			"\p\015\012"

static UPtr gEncode = (UPtr)
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static short gDecode[] = 
{
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,
	FAIL,SKIP,SKIP,FAIL,FAIL,SKIP,FAIL,FAIL,	/* 0 */
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	/* 1 */
	SKIP,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,
	FAIL,FAIL,FAIL,0x3e,FAIL,FAIL,FAIL,0x3f,	/* 2 */
	0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,
	0x3c,0x3d,FAIL,FAIL,FAIL,PAD ,FAIL,FAIL,	/* 3 */
	FAIL,0x00,0x01,0x02,0x03,0x04,0x05,0x06,
	0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,	/* 4 */
	0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,
	0x17,0x18,0x19,FAIL,FAIL,FAIL,FAIL,FAIL,	/* 5 */
	FAIL,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,
	0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,	/* 6 */
	0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,
	0x31,0x32,0x33,FAIL,FAIL,FAIL,FAIL,FAIL,	/* 7 */
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	/* 8 */
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	/* 9 */
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	/* A */
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	/* B */
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	/* C */
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	/* D */
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	/* E */
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,
	FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,FAIL,	/* F */
/* 0    1    2    3    4    5    6    7    8
	9    A    B    C    D    E    F  */
};

/*
 * the encoding is like this:
 *
 * Binary:<111111><222222><333333>
 *				765432107654321076543210
 * Encode:<1111><2222><3333><4444>
 */
 
/*
 * Bit extracting macros
 */
#define Bot2(b) ((b)&0x3)
#define Bot4(b) ((b)&0xf)
#define Bot6(b) ((b)&0x3f)
#define Top2(b) Bot2((b)>>6)
#define Top4(b) Bot4((b)>>4)
#define Top6(b) Bot6((b)>>2)


/************************************************************************
 * Encode64 - convert binary data to base64
 *  binPtr -> the binary data (or nil to close encoder)
 *	binLen -> the length of the binary data
 *  sixFourPtr -> pointer to buffer for the base64 data
 *	sixFourLen <- the length of the base64 data
 *  e64 <-> state; caller must preserve
 *  returns kCTEDoneOK or kCTEFailed
 ************************************************************************/
short Encode64(UPtr binPtr, long binLen, UPtr sixFourPtr, 
				long *sixFourBufSize, Enc64Ptr e64)
{
	UPtr 		binSpot;			/* the byte currently being decoded */
	UPtr 		sixFourSpot = sixFourPtr;
				/* the spot to which to copy the encoded chars */
	short 	bpl;
	UPtr		end;						/* end of integral decoding */
	PStr		newLine = kNewLine;

	bpl = e64->bytesOnLine;	/* in inner loop; want local copy */

	if (binLen)
	{
	
		/*
		 * do we have any stuff left from last time?
		 */
		if (e64->partialCount)
		{
			short needMore = 3 - e64->partialCount;
			if (binLen >= needMore)
			{
				/*
				 * we can encode some bytes
				 */
				BlockMoveData(binPtr,e64->partial+e64->partialCount,needMore);
				binLen -= needMore;
				binPtr += needMore;
			
			  do	
				{	
					if ((bpl)==68)							
					{
						int m_nl;
						for (m_nl=0;m_nl<*(newLine);)	
							*(sixFourSpot)++ = (newLine)[++m_nl];
						(bpl) = 0;
					}
					(bpl) += 4;
					*(sixFourSpot)++ = gEncode[Top6((e64->partial)[0])];
					*(sixFourSpot)++ = gEncode[Bot2((e64->partial)[0])<<4 |
						Top4((e64->partial)[1])];	
					*(sixFourSpot)++ = gEncode[Bot4((e64->partial)[1])<<2 |
							Top2((e64->partial)[2])];
					*(sixFourSpot)++ = gEncode[Bot6((e64->partial)[2])];
				}	
				while (0);

				if ( *sixFourBufSize <  sixFourSpot - sixFourPtr )
						return( kCTEFailed );

				e64->partialCount = 0;
			}
			/*
			 * if we don't have enough bytes to complete the leftovers, we
			 * obviously don't have 3 bytes.  So the encoding code will fall
			 * through to the point where we copy the leftovers to the partial
			 * buffer.  As long as we're careful to append and not copy blindly,
			 * we'll be fine.
			 */
		}
	
		/*
		 * we encode the integral multiples of three
		 */
		end = binPtr + 3*(binLen/3);
		for (binSpot = binPtr; binSpot < end; binSpot += 3)
		{ do
			{	
				if ((bpl)==68)							
				{
					int m_nl;
					for (m_nl=0;m_nl<*(newLine);)	
						*(sixFourSpot)++ = (newLine)[++m_nl];
					(bpl) = 0;
				}
				(bpl) += 4;
				*(sixFourSpot)++ = gEncode[Top6((binSpot)[0])];
				*(sixFourSpot)++ = gEncode[Bot2((binSpot)[0])<<4 |
Top4((binSpot)[1])];	
				*(sixFourSpot)++ = gEncode[Bot4((binSpot)[1])<<2 |
Top2((binSpot)[2])];	
				*(sixFourSpot)++ = gEncode[Bot6((binSpot)[2])];
			}	
			while (0);

			if ( *sixFourBufSize <  sixFourSpot - sixFourPtr )
					return( kCTEFailed );
		}
	
		/*
		 * now, copy the leftovers to the partial buffer
		 */
		binLen = binLen % 3;
		if (binLen)
		{
			BlockMoveData(binSpot,e64->partial+e64->partialCount,binLen);
			e64->partialCount += binLen;
		}
  }
	else
	{
		/*
		 * we've been called to cleanup the leftovers
		 */
		if (e64->partialCount)
		{
			if (e64->partialCount<2) e64->partial[1] = 0;
			e64->partial[2] = 0;

			do
			{	
				if ((bpl)==68)							
				{
					int m_nl;
					for (m_nl=0;m_nl<*(newLine);)	
						*(sixFourSpot)++ = (newLine)[++m_nl];
					(bpl) = 0;
				}
				(bpl) += 4;
				*(sixFourSpot)++ = gEncode[Top6((e64->partial)[0])];
				*(sixFourSpot)++ = gEncode[Bot2((e64->partial)[0])<<4 |
									Top4((e64->partial)[1])];	
				*(sixFourSpot)++ = gEncode[Bot4((e64->partial)[1])<<2 |
									Top2((e64->partial)[2])];	
				*(sixFourSpot)++ = gEncode[Bot6((e64->partial)[2])];
			}	
			while (0);

			if ( *sixFourBufSize <  sixFourSpot - sixFourPtr )
					return( kCTEFailed );
		
			/*
			 * now, replace the unneeded bytes with ='s
			 */
			sixFourSpot[-1] = '=';
			if (e64->partialCount==1) sixFourSpot[-2] = '=';
		}
	}
	e64->bytesOnLine = bpl;	/* copy back to state buffer */
	*sixFourBufSize = sixFourSpot - sixFourPtr;  /* return actual buffer size */

	return( kCTEDoneOK );
}



/************************************************************************
 * Decode64 - convert base64 data to binary
 *  sixFourPtr -> the base64 data (or nil to close decoder)
 *  sixFourLen -> the length of the base64 data
 *  binPtr -> pointer to buffer to hold binary data
 *  binLen <- length of binary data
 *  d64 <-> pointer to decoder state
 *	decErrCnt <- the number of decoding errors found
 *  returns kCTEDoneOK or kCTEFailed
 ************************************************************************/
short Decode64(UPtr sixFourPtr, long sixFourLen, UPtr binPtr, long *binBufSize, 
				Dec64Ptr d64, long *decErrCnt)
{
	short 	decode;								/* the decoded short */
	Byte 		c;								/* the decoded byte */
	/* separate short & the byte so the compiler can worry about byteorder */
	UPtr 		end = sixFourPtr + sixFourLen;	/* stop decoding here */
	UPtr 		binSpot = binPtr;			/* current output character */
	short 	decoderState;		/* which of 4 bytes are we seeing now? */
	long 		invalCount;	/* how many bad chars found this time around? */
	long 		padCount;		/* how many pad chars found so far? */
	Byte 		partial;/* partially decoded byte from/for last/next time */
	Boolean wasCR;
	UPtr		sixFourStartPtr;

	/*
	 * fetch state from caller's buffer
	 */
	decoderState = d64->decoderState;
	invalCount = 0;	/* we'll add the invalCount to the buffer later */
	padCount = d64->padCount;
	partial = d64->partial;
	wasCR = d64->wasCR;

	if (sixFourLen)
	{
		sixFourStartPtr = sixFourPtr;
		for (;sixFourPtr<end;sixFourPtr++)
		{
			switch(decode=gDecode[*sixFourPtr])
			{
				case SKIP: break;				/* skip whitespace */
				case FAIL: invalCount++; break;	/* count invalid characters */
				case PAD: padCount++; break;	/* count pad characters */
				default:
					/*
					 * found a non-pad character,
					 * so if we had previously found a pad,
					 * that pad was an error
				 	*/
					if (padCount) {invalCount+=padCount;padCount=0;}
				
					/*
					 * extract the right bits
					 */
					c = decode;
					switch (decoderState)
					{
						case 0:
							partial = c<<2;
							decoderState++;
							break;
						case 1:
							*binSpot++ = partial|Top4(c);
							partial = Bot4(c)<<4;
							decoderState++;
							break;
						case 2:
							*binSpot++ = partial|Top6(c);
							partial = Bot2(c)<<6;
							decoderState++;
							break;
						case 3:
							*binSpot++ = partial|c;
							decoderState=0;
							break;
					} /* switch decoderState */
			} /* switch decode */
		
			/* make sure we're not writing past the end of the buffer */
			if ( *binBufSize <  binSpot - binPtr )
						return( kCTEFailed );

		
		} /* for sixFourPtr */
	} /* if sixFourLen */
	else
	{
		/*
		 * all done.  Did all end up evenly?
		 */
		switch (decoderState)
		{
			case 0:
				invalCount += padCount;
				/* came out evenly, so should be no pads */
				break;
			case 1:
				invalCount++;				/* data missing */
				invalCount += padCount;
				/* since data missing; should be no pads */
				break;
			case 2:
				invalCount += ABS(padCount-2);	/* need exactly 2 pads */
				break;
			case 3:
				invalCount += ABS(padCount-1);	/* need exactly 1 pad */
				break;
		}
	}

	*binBufSize = binSpot - binPtr;   	/* return actual buffer size */

	/*
	 * save state in caller's buffer
	 */
	d64->decoderState = decoderState;
	d64->invalCount += invalCount;
	d64->padCount = padCount;
	d64->partial = partial;
	d64->wasCR = wasCR;

	*decErrCnt = invalCount;
	return( kCTEDoneOK );
}



/************************************************************************
 * EncodeQP - convert binary data to quoted-printable
 *  binPtr -> the binary data
 *	binLen -> the length of the binary data (or nil to close encoder)
 *  qpPtr -> pointer to buffer for the quoted-printable data
 *	qpBufSize <-> length of the quoted-printable buffer, returns actual length
 *  eQP <-> encoder state for bytes per line; caller must preserve
 *  returns kCTEDoneOK or kCTEFailed
 ************************************************************************/
long EncodeQP(UPtr binPtr, long binLen, UPtr qpPtr, long *qpBufSize, long *eQP)
{
	UPtr 					binSpot = binPtr;
		/* the byte currently being decoded */
	UPtr 					qpSpot = qpPtr;
		/* the spot to which to copy the encoded chars */
	UPtr					binEndPtr;
		/* stop decoding line here */
	short 				bpl;
	short 				c;
	static char 	*hex="0123456789ABCDEF";
	Boolean 			encode;
	UPtr 					nextSpace;
	PStr					newLine = kNewLine;

#define ROOMFOR(x)			\
	do {\
		if (bpl + x > 76)		\
		{\
			*qpSpot++ = '=';	\
			do {							\
				BlockMoveData(newLine+1,qpSpot,*newLine);		\
				qpSpot += *newLine;\
				bpl = 0;				\
			} while (0);			\
		}\
	} while(0)

	bpl = *eQP;	/* in inner loop; want local copy */

	if ( binLen && binPtr ) {
		binEndPtr = binPtr + binLen;

		for (binSpot = binPtr; binSpot < binEndPtr; binSpot++) {
			/* fail if we try to write beyond the output buffer size */
			if ( *qpBufSize + 4 <  qpSpot - qpPtr )
					return( kCTEFailed );

			/*
			 * make copy of char
			 */
			c = *binSpot;

			/*
			 * handle newlines
			 */
			if (c == newLine[1])
				do {
					BlockMoveData(newLine+1,qpSpot,*newLine);
					qpSpot += *newLine;
					bpl = 0;
				} while (0);
	
			else if ( (c == '\012') || (c == '\015') ) ;
				/* skip other newline characters */
			else {
				if ( (c == ' ') || (c == '\t') ) {
					encode = ( (binSpot == binEndPtr - 1) ||
						(binSpot[1] == '\015') );	/* trailing white space */
					if (!encode) {
						for(nextSpace = binSpot + 1;
						nextSpace < binEndPtr; nextSpace++)
							if ( (*nextSpace == ' ') ||
								(*nextSpace == '\015') ) {
								if ( (nextSpace - binSpot) < 20)
									ROOMFOR(nextSpace - binSpot);
								break;
							}
					}
				}
				else
					encode =  (c == '=') || (c < 33) || (c > 126);
					/* weird characters */
				
				if (encode) 
						ROOMFOR(3); 
				else 
						ROOMFOR(1);
				
				encode = encode || !bpl && ( (c == 'F') || (c == '.') );
		
				if (encode) {
					*qpSpot++ = '=';
					*qpSpot++ = hex[(c>>4)&0xf];
					*qpSpot++ = hex[c&0xf];
					bpl += 3;
				}
				else {
					*qpSpot++ = c;
					bpl++;
				}
			} /* else */
			
		}	/* for */
	}	/* if binLen */

	*eQP = bpl;								/* copy back to state buffer */
	*qpBufSize = qpSpot - qpPtr;  			/* return actual buffer size */

	return( kCTEDoneOK );
}


#define HEX(c)	\
	((c)>='0' && (c)<='9' ? (c)-'0' : \
	((c)>='A' && (c)<='F' ? (c)-'A'+10 : \
	((c)>='a' && (c)<='f'  ? (c)-'a'+10 : -1)))

/************************************************************************
 * DecodeQP - convert quoted printable data to binary
 *  qpStartPtr -> the quoted printable data
 *  qpLen -> the length of the quoted printable data (or nil to close decoder)
 *  binPtr -> pointer to buffer to hold binary data
 *  binBufSize  <-> the length of the binary buffer, returns actual length
 *  dQP <-> pointer to decoder state; caller must preserve
 *	decErrCnt <- the number of decoding errors found
 *  returns kCTEDoneOK or kCTEFailed
 * Note: this does not do conversion to local newline convention as the 
 *       Eudora internal version does (Maybe a flag to select would be good?)
 ************************************************************************/
long DecodeQP(UPtr qpPtr, long qpLen, UPtr binPtr, long *binBufSize,
			DecQPPtr dQP, long *decErrCnt)
{
	Byte				c;						/* the decoded byte */
	UPtr				binSpot = binPtr;		/* current output character */
	UPtr				qpSpot = qpPtr;			/* current input character */
	UPtr				qpEndPtr;				/* stop decoding line here */
	QPStates		state;
	Byte				lastChar;
	short				errs = 0;
	short				upperNib, lowerNib;
	long				spaceCnt, cnt;

	/*
	 * fetch state from caller's buffer
	 */
	state = dQP->state;
	lastChar = dQP->lastChar;
	spaceCnt = dQP->spaceCount;

	if ( qpLen && qpPtr ) {
		qpEndPtr = qpPtr + qpLen;
				
		for (qpSpot = qpPtr; qpSpot < qpEndPtr; qpSpot++) {
			c = *qpSpot;
			switch (state) {
				case qpNormal:
					if (c == ' ') {
						spaceCnt++;
					}
					else if (c == '=') {
						for ( cnt = 0; cnt < spaceCnt; cnt++ )
								*binSpot++ = ' ';
						state = qpEqual;
						spaceCnt = 0;
					}
					else if (c == '\015' || c == '\012') {
						*binSpot++ = c;
						spaceCnt = 0;		// Ignore spaces before CR
					}
					else { 	
						for ( cnt = 0; cnt < spaceCnt; cnt++ )
								*binSpot++ = ' ';
						*binSpot++ = c;
						spaceCnt = 0;
					}
					break;
				
				case qpEqual:
					if (c == ' ') {
						spaceCnt++;
					}
					else if (c == '\015' || c == '\012') {
						state = qpNormal;
						spaceCnt = 0;	// Ignore spaces between Equal and CR
					}
					else
						state = qpByte1;
					break;
			
				case qpByte1:
					upperNib = HEX(lastChar);
					lowerNib = HEX(c);
					if ( (upperNib < 0) || (lowerNib < 0) ) 
							errs++;
					else
							*binSpot++ = (upperNib<<4) | lowerNib;
					state = qpNormal;
					break;
			}	/* switch */
			lastChar = c;
			
		}	/* for */
	}	/* if qpLen */
	else if (state != qpNormal) 
			errs++; /* when closing decoder */

	/*
	 * save state in caller's buffer
	 */
	dQP->state = state;
	dQP->lastChar = lastChar;
	dQP->spaceCount = spaceCnt;
	*binBufSize = binSpot - binPtr;

	*decErrCnt = errs;
	return( kCTEDoneOK );
}



/* ---------------------------------------------------------------------------
    Parse a MIME Content-Transfer-Encoding: Header
    
args: Handle with 822 header. Must end in \015\012.

returns: enum of CTE type, or kCTEUnknown for parse errors

 */
CTEType ParseCTEHeader(Handle headerH)
{
	char      *h, *hEnd, *tokenEnd;
  CTEType    CType = kCTEUnknown;

	/* Check for valid handle */
	if ( headerH == NULL )
		return (CType);	

  /* Find the CTE header line */
  HLock(headerH);
	hEnd = *headerH + GetHandleSize(headerH);
  for(h = *headerH; h < hEnd; ) {
		if(cmpCB(h, kCTEncodeHdrCStr, kCTEncodeHdrLen, 1) ==0) {
			break; /* Matched */
	  }
	  while(h < hEnd && *h++ != '\012') {} /* Next Line */
	}
	if(h >= hEnd)
	  goto done; /* Couldn't find CTE line */
	h += kCTEncodeHdrLen; /* Go Past field identifier */
	  
	/* Parse out the token */
	rfc822_skipws(&h);
	if (!(tokenEnd = rfc822_parse_word (h," ()<>@,;:\\\"[]/?=\012\015")))
	  goto done; /* Bogus 822 Syntax */
	if(cmpPB(kCTEbase64TypeStr, h, tokenEnd - h, 1) ==0)   
			CType = kCTEbase64;
	else if(cmpPB(kCTEQPTypeStr, h, tokenEnd - h, 1) ==0) 
	    CType = kCTEQP;
	else if(cmpPB(kCTE7bitTypeStr, h, tokenEnd - h, 1) ==0) 
	    CType = kCTE7bit;
	else if(cmpPB(kCTE8bitTypeStr, h, tokenEnd - h, 1) ==0) 
	    CType = kCTE8bit;
	else if(cmpPB(kCTEbinaryTypeStr, h, tokenEnd - h, 1) ==0) 
	    CType = kCTEbinary;
 done:
	HUnlock(headerH);
	return(CType);  
}


/* ---------------------------------------------------------------------------
    Generate a CTE header line
    
args: Handle with 822 header. Must end in \015\012.

returns: Handle to Pascal string with CTE header line

 */
 Handle GenCTEHeader(CTEType encType)
{
	Handle CTEHeader;

	CTEHeader = NewHandle(kCTEncodeHdrLen+100); /* CTE token > 100 unlikely */
	HLock(CTEHeader);
	copyPP(kCTEncodeHdrPStr, (StringPtr)*CTEHeader);
	catPP("\p ", (StringPtr)*CTEHeader);

	switch(encType) {
		case kCTE7bit: 
			catPP(kCTE7bitTypeStr, (StringPtr)*CTEHeader);
			break;
		case kCTE8bit: 
			catPP(kCTE8bitTypeStr, (StringPtr)*CTEHeader);
			break;		
		case kCTEbase64: 
			catPP(kCTEbase64TypeStr, (StringPtr)*CTEHeader);
			break;		
		case kCTEQP: 
			catPP(kCTEQPTypeStr, (StringPtr)*CTEHeader);
			break;		
		case kCTEbinary: 
			catPP(kCTEbinaryTypeStr, (StringPtr)*CTEHeader);
			break;
	}
	catPP("\p\r\n", (StringPtr)*CTEHeader);
	HUnlock(CTEHeader);
	return(CTEHeader);
}

