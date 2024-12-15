/* ======================================================================

   Functions for very basic RFC-822 header manipulation
                 
   Last Edited: July 18, 1996

   Laurence Lundblade <lgl@qualcomm.com>

   Copyright 1995, 1996 QUALCOMM Inc.
   
   Some of this code is from the c-client and is 
      Copyright University of Washington 
*/


OSErr ReadHeader(FSSpec *Fsp, Handle *HeaderH);
void  rfc822_skipws (char **s);
char *rfc822_parse_word (char *s, const char *delimiters);
char *rfc822_quote (char *src);

extern const char *hspecials, *wspecials, *ptspecials;