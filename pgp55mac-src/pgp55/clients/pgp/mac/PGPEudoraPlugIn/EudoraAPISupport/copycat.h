/* ======================================================================

   Yet another attempt to make sense of Mac string and buffer manipulation
   
   Filename: copycat.h

   Last Edited: July 17, 1996

   Laurence Lundblade <lgl@qualcomm.com> 
   Myra Callen <t_callen@qualcomm.com> 

   Copyright 1996 QUALCOMM Inc.
*/
long  copyPB(ConstStringPtr source, char *dest);
long 	copyBP(const char *source, long sourceLen, StringPtr dest);

long 	catPB(ConstStringPtr addition, long destLen, char *dest);
long  catBP(const char *source, long sourceLen, StringPtr dest);

long  copyPP(ConstStringPtr source, StringPtr dest);
long  catPP(ConstStringPtr source, StringPtr addition);

long  copyPC(ConstStringPtr source, char *dest);
long  copyCP(const char *source, StringPtr dest);

short cmpCB(const char *o, const char *r, long n, short caseignore);
short cmpPB(ConstStringPtr o, const char *r, long n, short caseignore);