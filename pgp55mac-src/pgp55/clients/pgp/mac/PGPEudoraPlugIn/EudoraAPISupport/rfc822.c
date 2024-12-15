

#include "rfc822.h"
#include "lglio.h"


/* ============== yes, it's all the rest of this file... ================== */
/* ============== not needed for most translator ========================== */
static char *strpbrk(const char *string, const char *check_chars);

/* All these special characters will confuse CodeWarrior so you may not get
 * the colorized comments and key words you are use to.
 */				
extern const char *hspecials =
	" ()<>@,;:\\\"";			/* parse-host specials */                
extern const char *wspecials =
	" ()<>@,;:\\\"[]";		/* parse-word specials */				
extern const char *ptspecials =
	" ()<>@,;:\\\"[]/?=";	/* parse-token specials for parsing */


/* Stolen from Pine and the c-client */
/* Parse RFC822 word
 * Accepts: string pointer
 * Returns: pointer to end of word
 */

char *rfc822_parse_word (char *s, const char *delimiters)
{
  char *st,*str;
  if (!s) return NULL;		/* no-op if no string */
  rfc822_skipws (&s);		/* flush leading whitespace */
  if (*s == '\0') return NULL;	/* end of string */
				/* default delimiters to standard */
  if (!delimiters) delimiters = wspecials;
  str = s;			/* hunt pointer for strpbrk */
  while (1) {			/* look for delimiter */
    if (!(st = strpbrk (str,delimiters))) {
      while (*s != '\0') ++s;	/* no delimiter, hunt for end */
      return s;			/* return it */
    }
    switch (*st) {		/* dispatch based on delimiter */
    case '"':			/* quoted string */
				/* look for close quote */
      while (*++st != '"') switch (*st) {
        case '\0':		/* unbalanced quoted string */
	      return NULL;		/* sick sick sick */
        case '\\':		/* quoted character */
	      if (!*++st) return NULL;	/* skip the next character */
        default:			/* ordinary character */
	      break;			/* no special action */
      }
      str = ++st;		/* continue parse */
      break;
    case '\\':			/* quoted character */
      /* This is wrong; a quoted-pair can not be part of a word.  However,
       * domain-literal is parsed as a word and quoted-pairs can be used
       * *there*.  Either way, it's pretty pathological.
       */
      if (st[1]) {		/* not on NUL though... */
	    str = st + 2;		/* skip quoted character and go on */
	    break;
      }
    default:			/* found a word delimiter */
      return (st == s) ? NULL : st;
    }
  }
}


/* Skips RFC822 whitespace
 * Accepts: pointer to string pointer
 */
void rfc822_skipws (char **s)
{
  char *t;
  long  n = 0;
				/* while whitespace or start of comment */
  while ((**s == ' ') || (n = (**s == '(')) != 0 ) {
    t = *s;			/* note comment pointer */
    if (**s == '(') while (n) {	/* while comment in effect */
      switch (*++*s) {		/* get character of comment */
        case '(':			/* nested comment? */
	      n++;			/* increment nest count */
	      break;
        case ')':			/* end of comment? */
	      n--;			/* decrement nest count */
	      break;
        case '"':			/* quoted string? */
	      while (*++*s != '\"') if (!**s || (**s == '\\' && !*++*s)) {
	        /* Unterminated quoted string within comment */
	        *t = '\0';		/* nuke duplicate messages in case reparse */
	        return;
	      }
	      break;
        case '\\':		/* quote next character? */
	      if (*++*s) break;
        case '\0':		/* end of string */
	     /* "Unterminated comment */
	      *t = '\0';		/* nuke duplicate messages in case reparse */
	      return;			/* this is wierd if it happens */
        default:			/* random character */
	      break;
      }
    }
    ++*s;			/* skip past whitespace character */
  }
}


/* Unquote an RFC822 format string
 * Accepts: string
 * Returns: string
 */

char *rfc822_quote (char *src)
{
  char *ret = src;
  
  if (strpbrk (src,"\\\"")) {	/* any quoting in string? */
    char *dst = ret;
    while (*src) {		/* copy string */
      if (*src == '\"') src++;	/* skip double quote entirely */
      else {
	    /* skip over single quote, copy next always */
	    if (*src == '\\') src++;
	    *dst++ = *src++;	/* copy character */
      }
    }
    *dst = '\0';		/* tie off string */
  }
  return ret;			/* return our string */
}


static char *strpbrk(const char *string, const char *check_chars)
{
    char *s, *c;
    
    for(s = (char *)string; *s; s++)
    {
    	for(c = (char *)check_chars; *c ; c++) {
	        if(*s == *c)
	            return(s);
	    }
	}
	return(NULL);
}

/* 
  Read file and extract RFC-822 header - does line folding too
 
 args: Fsp    - File name with a canonical 822 object in it
       Header - pointer to place to put handle with the headers which will be
                NULL terminated have a Handle size one greater than headers
                
 returns: OSErr
 
  The call allocates memory for *HeaderH, caller must free this memory.
 */

OSErr ReadHeader(FSSpec *Fsp, Handle *HeaderH)
{
  OSErr        	 err = noErr;
  lglIOHandle  	 inIOH;
  char        	*line;
  long         	 len, headerLen, oldSize;
  Boolean		 		 gotLine = true, notBlank = true;
    
  *HeaderH = NewHandle(4096); /* Start out with this size for header,
  								disposed by caller */
  if ( *HeaderH == NULL )
   		return(nilHandleErr);
	oldSize = GetHandleSize(*HeaderH);

	/* Open the file to parse */
  /* allocates memory, disposed in lglClose() */
  if( (err = lglOpen(&inIOH, *Fsp)) != noErr) 
  	return(err);
  if ( !inIOH )
  	return(nilHandleErr);
  HLock((Handle)inIOH);  /* unlocked in lglClose() */

	headerLen = 0;
	gotLine = (lglRead(inIOH, &line, &len) == noErr);

	while ( gotLine && notBlank ) {
		if ( headerLen + len + 1 > oldSize ) {
  			/* Need to make handle bigger */
  			SetHandleSize(*HeaderH, 2 * oldSize);
			  if( MemError() != noErr ) {
			  	/* try again */
			  	Handle tempH;
			  	tempH = NewHandle(2 * oldSize);
			  	if ( tempH == NULL ) {
			  		lglClose(inIOH); /* Closes and HUnlocks() */
			  		return(err);
			  	}
			  	else {
			  		HLock(*HeaderH);
			  		HLock(tempH);
			  		BlockMoveData(**HeaderH, *tempH, oldSize);		
			  		HUnlock(tempH);
			  		HUnlock(*HeaderH);
			  		DisposeHandle(*HeaderH);		// free old handle
			  		*HeaderH = tempH;		// point our handle to new data
			  		oldSize = GetHandleSize(*HeaderH);
			  	}
			  }
		}
		if ( (*line == ' ') && (headerLen != 0) )   
	      headerLen -=2; /* Had a continuation line,
	      					overwrite CRLF at end of prev. line */
	  
  	HLock(*HeaderH);
	  BlockMove(line, **HeaderH + headerLen, len);
  	HUnlock(*HeaderH);
		headerLen += len;
		gotLine = (lglRead(inIOH, &line, &len) == noErr);
		notBlank = (len > 2);
  }
  SetHandleSize(*HeaderH, headerLen+1);
  err = MemError();
  if( err != noErr ) {
  	lglClose(inIOH); /* Closes and HUnlocks() */
  	return(err);
  }
  HLock(*HeaderH);
  (**HeaderH)[headerLen] = '\0';
  HUnlock(*HeaderH);
	lglClose(inIOH); /* Closes and HUnlocks() */
	
	return(noErr);
}    
