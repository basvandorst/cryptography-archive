/* ======================================================================

   Functions to manage MIME type data structures for use
                 with Eudora translation API on the Mac.
                 
   Last Edited: July 23, 1996
   
   Filename: mimetype.c

   Laurence Lundblade <lgl@qualcomm.com>
   Myra Callen <t_callen@qualcomm.com>

   Copyright 1995, 1996 QUALCOMM Inc.
   
   Some of this code is from the "c-client" and is 
      Copyright University of Washington 
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "emsapi-mac.h"
#include "mimetype.h"
#include "copycat.h"
#include "rfc822.h"



/* ----------------------------------------------------------------------
   Create a data structure to hold a MIME type. Add parameters later on with
   calls to add_mime_parameter.
   
   Args: type -- the main MIME type: e.g., text, application, image
         sub_type -- the sub type: e.g., plain, octet-stream, jpeg
         mime_v -- the MIME verion number
         
   Returns: Handle for the emsMIMEtype data structure 
*/
emsMIMEtype **make_mime_type(
ConstStringPtr mime_type, ConstStringPtr sub_type, ConstStringPtr mime_v)
{
	emsMIMEtype *mType;
	Handle      mTypeHandle;

    mTypeHandle = NewHandle(sizeof(emsMIMEtype));
    mType       = (emsMIMEtype *)*mTypeHandle;
    HLock(mTypeHandle);
	BlockMoveData(mime_type, mType->mime_type, *mime_type + 1);
	BlockMoveData(sub_type,  mType->sub_type,  *sub_type + 1);
	if(mime_v != NULL) 
      BlockMoveData(mime_v,  mType->mime_version,  *mime_v + 1);
    else 
      BlockMoveData("\p1.0", mType->mime_version, 4);

	mType->params = (emsMIMEparam **)NULL; 
	HUnlock(mTypeHandle);   
    return((emsMIMEtype **)mTypeHandle);
}



/* ----------------------------------------------------------------------
    Free a data structure holding a MIME type
    
    Args: Handle for the emsMIMEtype to be freed
*/
void free_mime_type(emsMIMEtype **MIMEtypeHandle)
{
    emsMIMEparam **paramHandle, **nextHandle;

	for(paramHandle = (*MIMEtypeHandle)->params;
	    paramHandle != NULL; paramHandle = nextHandle) {
		 nextHandle = (*paramHandle)->next;
		 if((*paramHandle)->value != NULL)
		     DisposeHandle((*paramHandle)->value);
	     DisposeHandle((Handle)paramHandle);
	}
    DisposeHandle((Handle)MIMEtypeHandle);
    MIMEtypeHandle = NULL;
}



/* ----------------------------------------------------------------------
    Convert a MIME type structure to a string in the format it is usually
    presented in in a Content-Type: header.  This includes quoting the parameter
    values and so on. 
    
    Args: Handle for the emsMIMEtype 
    
    Returns: Handle to a block of ASCII formatted for use
    		in a Content-Type: header
             (note: it is not a Pascal string, nor a C string)
*/
Handle string_mime_type(emsMIMEtype **MIMEtypeHandle) 
{
    emsMIMEparam   **paramHandle;
    StringPtr      *srcStrHandle;
    emsMIMEtype     *MIMEtype;
    short           resultLen, resultPos;
    Handle          theResult;

	if(MIMEtypeHandle == NULL || *MIMEtypeHandle == NULL) 
	    return(NULL);

    HLock((Handle)MIMEtypeHandle);
    MIMEtype = *MIMEtypeHandle;
    
    /* --- Figure exactly how big the thing to return will be --- */
    resultLen = *MIMEtype->mime_type + 1 + *MIMEtype->sub_type;
    for(paramHandle = (*MIMEtypeHandle)->params; paramHandle != NULL;) {
        emsMIMEparam  **tmpHandle = paramHandle;
        HLock((Handle)tmpHandle);
	    resultLen += *(*tmpHandle)->name +
	                   GetHandleSize((*tmpHandle)->value) + 5;
	    paramHandle = (*tmpHandle)->next;
	    HUnlock((Handle)tmpHandle);
	} 

	                   
	/* --- Allocate the Handle with the result --- */                  
	theResult = NewHandle(resultLen);
	HLock(theResult);
	resultPos = 0;
	        
	/* --- Add the type and sub type --- */
    BlockMoveData(MIMEtype->mime_type+1,
    	*theResult + resultPos, *MIMEtype->mime_type);
    resultPos += *MIMEtype->mime_type;
    
    *(*theResult + resultPos++) = '/';
    
    BlockMoveData(MIMEtype->sub_type+1,
    	*theResult + resultPos, *MIMEtype->sub_type);
    resultPos += *MIMEtype->sub_type;
    
     /* ---- The parameters ---- */
	for(paramHandle = (*MIMEtypeHandle)->params; paramHandle != NULL;) {   
	    emsMIMEparam **tmpHandle = paramHandle;
        HLock((Handle)tmpHandle);
        *(*theResult + resultPos++) = ';';
	    *(*theResult + resultPos++) = ' ';

	 	BlockMoveData((*tmpHandle)->name+1, *theResult + resultPos,
	 	           *(*tmpHandle)->name);
        resultPos += *(*tmpHandle)->name;
    	 	
	    *(*theResult + resultPos++) = '=';
	    *(*theResult + resultPos++) = '"';
	  	
	 	srcStrHandle = (StringHandle)(*tmpHandle)->value;
	 	HLock( (Handle)srcStrHandle);
	 	BlockMoveData(*srcStrHandle,
	 		*theResult + resultPos, GetHandleSize( (Handle)srcStrHandle));
	 	resultPos += GetHandleSize( (Handle)srcStrHandle);
        HUnlock((Handle)srcStrHandle);
        
		*(*theResult + resultPos++) = '"';
        paramHandle = (*tmpHandle)->next;
        HUnlock((Handle)tmpHandle);
    }
	
	HUnlock(theResult);
	HUnlock((Handle)MIMEtypeHandle);
		
	return(theResult);
}   
	


/* ----------------------------------------------------------------------
     Add a parameter to a MIME type
     
     Bug: might have to make the value parameter a Handle
     instead of a StringPtr to
     accomodate items longer than 255 characters
     
    Args: MIMEtypeHandle -- Handle for the MIME type to add too
          name  --  Name of the parameter (Pascal string)
          value --  Value of the parameter (currently a Pascal string)
*/
void add_mime_parameter(
	emsMIMEtype **MIMEtypeHandle, ConstStringPtr name, ConstStringPtr value)
{
    emsMIMEparam **paramHandle, **paramsEndHandle;

    HLock((Handle)MIMEtypeHandle);
	// Find pointer to last pointer in the param list
	paramsEndHandle = NULL;
	for(paramHandle = (*MIMEtypeHandle)->params; paramHandle != NULL;){
	    paramsEndHandle = paramHandle;
	    HLock((Handle)paramsEndHandle);
	    paramHandle = (*paramsEndHandle)->next;
	    HUnlock((Handle)paramsEndHandle);
	}
	
	// The new parameter block (includes Str63 for name) 
    paramHandle = (emsMIMEparam **)NewHandle(sizeof(struct emsMIMEparamS));
	HLock((Handle)paramHandle);
	BlockMoveData(name, (*paramHandle)->name, *name + 1);
	
	// Allocate Handle for value and copy data in 
	(*paramHandle)->value = NewHandle(*value);
	BlockMoveData(value + 1, *((*paramHandle)->value), *value );
	(*paramHandle)->next = NULL;
	HUnlock((Handle)paramHandle);
	
	// Put it at the end of the list
	if(paramsEndHandle == NULL) {
		// It's an emtpy list
		(*MIMEtypeHandle)->params = paramHandle;
	} else {
		// Put at end of list
		HLock((Handle)paramsEndHandle);
		(*paramsEndHandle)->next = paramHandle;
		HUnlock((Handle)paramsEndHandle);
    }
    HUnlock((Handle)MIMEtypeHandle);
}



/* -------------------------------------------------------------------------
   Pick out a specific parameter from a MIME type
   
   Args: mtype_h  Handle to a struct ems_MIME_type
         paramName  string with name of parameter to look for
      
   Returns handle to string which is the value or NULL
   	(It is not a pascal string)
 */
Handle get_mime_parameter(
	emsMIMEtype **MIMEtypeHandle, ConstStringPtr paramName)
{
	long result;
	
	emsMIMEparam **paramHandle;
	for(paramHandle = (*MIMEtypeHandle)->params;
	    paramHandle != NULL;
	    paramHandle = (*paramHandle)->next) {
	    HLock((Handle)paramHandle);
	    result = EqualString((*paramHandle)->name, paramName, FALSE, TRUE);
	    HUnlock((Handle)paramHandle);
	    if(result) {
	        return((*paramHandle)->value);
	    }
	}
    return(NULL);
}				



/* -------------------------------------------------------------------------
  Convenient MIME type matcher - saves locking some handles
  
  If either type or subtype is NULL, then it won't be checked.
  
  Args: MIMEtypeHandle -- The mime type to check
        mType  -- The major MIME type to check
        subtype -- The MIME subtype to check
  
  Returns 1 if the MIME type matches, 0 if not
 */
 short match_mime_type(
 const emsMIMEtype **MIMEtypeHandle,
 ConstStringPtr mType, ConstStringPtr subtype)
 {
    short result;
       
    HLock((Handle)MIMEtypeHandle);
    result = 1;
    
 	if(mType != NULL) 
 	    result = EqualString((*MIMEtypeHandle)->mime_type,
 	    			mType, FALSE, TRUE);
 
    if(result && subtype != NULL) 
        result = EqualString((*MIMEtypeHandle)->sub_type,
        			subtype, FALSE, TRUE);
        
    HUnlock((Handle)MIMEtypeHandle);
    return(result);
}



/* ---------------------------------------------------------------------------
    Parse a MIME "Content-Type:" header
 
 args: headerH - Handle containing header to be parsed;
 			length is length of Handle
                 String should have CRLF line endings
       MIMEtypeHandlePtr - place where MIME type is returned. Set to NULL
                               if no valid Content-type is found.
           
 */
void parse_mime_ct( Handle headerH, emsMIMEtype ***MIMEtypeHandlePtr)
{
	char         *s, *hEnd, *sEnd;
	Handle        tmp; 
	emsMIMEparam **paramHandle, *param;
	emsMIMEtype  **mTypeHandle, *mType;
	char         *name, c;
	int           txxxx;

    *MIMEtypeHandlePtr = NULL;

	/* Check for valid handle */
	if ( !headerH )
		return;		
    
	/* Find the Content-Type: header */
	HLock( (Handle)headerH);
    hEnd = *headerH + GetHandleSize( (Handle) headerH);
  	for(s = *headerH; s < hEnd; ) {
		if(cmpCB(s, kContentTypeHdrCStr, kContentTypeHdrLen, 1) ==0) {
			break; /* Matched */
	  }
	  while(s < hEnd && *s++ != '\012') {} /* Next Line */
	}
	if(s >= hEnd)
		return; /* Couldn't find CTE line */
	s += kContentTypeHdrLen; /* Go Past field identifier */  
	
	/* Make value of CT header into a C string with no CRLF*/
    for(sEnd = s; sEnd < hEnd && *sEnd!='\012' && *sEnd!='\015';sEnd++)
    	{
    	}
    	
    tmp = NewHandle(sEnd - s + 1);
    HLock(tmp);
    BlockMoveData(s, *tmp, sEnd - s); /* Don't copy EOL character */
    (*tmp)[sEnd - s] = '\0';
    s = *tmp;
    HUnlock( (Handle) headerH);   

	/* Begin the old Pine/c-client parser (some what hacked up now) */
	/* Allocate the MIME type structure we're gonna return */
    mTypeHandle = (emsMIMEtype **)NewHandle(sizeof(emsMIMEtype));
    *MIMEtypeHandlePtr = mTypeHandle;
    HLock((Handle)mTypeHandle);
    mType = *mTypeHandle;
    mType->params = NULL;
    mType->mime_type[0]  = 0;
    mType->sub_type[0] = 0;

    
	/* First get the major type */
	rfc822_skipws(&s);
	if (!(name = rfc822_parse_word (s,ptspecials))) goto done;
    c = *name;		/* remember delimiter */
    *name = '\0';	/* tie off type */
    BlockMoveData(s, mType->mime_type, strlen(s)+1);
    CtoPstr( (char *)mType->mime_type);
    *name = c;		        /* restore delimiter */
    /* Now get the sub-type */
    rfc822_skipws (&name);	/* skip whitespace */
    if ((*name == '/') &&	/* subtype? */
	    ( name = rfc822_parse_word ((s = ++name),ptspecials) ) != 0 )
	 {
	  c = *name;		/* save delimiter */
	  *name = '\0';		/* tie off subtype */
	  rfc822_skipws (&s);	/* copy subtype */
	  BlockMoveData(s, mType->sub_type, strlen(s)+1);
	  CtoPstr( (char *)mType->sub_type);
	  *name = c;		/* restore delimiter */
	  rfc822_skipws (&name);	/* skip whitespace */
    } else {
      /* There was no subtype */
      BlockMoveData("\p", mType->sub_type,  2);
	  if (!name) {		/* did the fool have a subtype delimiter? */
	    name = s;		/* barf, restore pointer */
	    rfc822_skipws (&name);/* skip leading whitespace */
      }
    }
	
	/* parameter list? */
	paramHandle = NULL;
    while (name && (*name == ';') &&
	     (name = rfc822_parse_word ((s = ++name),ptspecials)) != 0 ) {
	  c = *name;		/* remember delimiter */
	  *name = '\0';		/* tie off attribute name */
	  rfc822_skipws (&s);	/* skip leading attribute whitespace */
	  if (!*s) *name = c;	/* must have an attribute name */
	  else {			/* instantiate a new parameter */
	    if (mType->params) {
	        HUnlock((Handle)paramHandle);
	        paramHandle = (*paramHandle)->next =
	        	(emsMIMEparam **)NewHandle(sizeof(emsMIMEparam));
	    } else {
	        paramHandle = mType->params =
	        	(emsMIMEparam **)NewHandle(sizeof(emsMIMEparam));
	    }
	    HLock((Handle)paramHandle);
	    param = *paramHandle;
	    param->next = NULL;
	    BlockMoveData(s, param->name, strlen(s) + 1);
	    CtoPstr( (char *)param->name);
	    *name = c;		/* restore delimiter */
	    rfc822_skipws (&name);/* skip whitespace before equal sign */
	    if ((*name != '=') ||	/* missing value is a no-no too */
	      !(name = rfc822_parse_word ((s = ++name),ptspecials)))
	      param->value = NULL;
	    else {		/* good, have equals sign */
	      c = *name;		/* remember delimiter */
	      *name = '\0';	/* tie off value */
	      rfc822_skipws (&s);	/* skip leading value whitespace */
	      param->value = NewHandle(strlen(s)+1); // trailing NULL too
	      HLock(param->value);
	      BlockMoveData(s, *(param->value),  strlen(s)+1);
	      rfc822_quote(*(param->value));
	      HUnlock(param->value);
	      
	      SetHandleSize(param->value,
	      	strlen(*(param->value))); // whack trailng null
          txxxx =  strlen(*(param->value));
          txxxx = GetHandleSize(param->value);
	      *name = c;		/* restore delimiter */
	      rfc822_skipws (&name);
	    }
	  }
    }
    
    if(paramHandle)
      HUnlock((Handle)paramHandle);
      
    /* At end of it all */
    if (!name) {	
      /* Missing parameter or parameter value */
	} else if (*name) {		
	  /* Junk at end of parameters - oh well */
    }
    
  done: 
    HUnlock((Handle)mTypeHandle);
    HUnlock(tmp);
    DisposeHandle(tmp);  
}
