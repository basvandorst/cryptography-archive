
/* This routine takes in a floating point number and rounds it to */
/* the nearest integer. 					  */

int
round(afloat)
  double afloat;
{
int rounded_int;

  /* this will truncate afloat */
  rounded_int = afloat;
  /* positive and negative numbers are handled differently */
  if (afloat < 0) 
  {
    /* if the fractional part is -.5 or less round down */
    if (afloat - rounded_int <= -.5) rounded_int--;
  }
  else
  {
    /* if the fractional part is .5 or greater round up */
    if (afloat - rounded_int >= .5) rounded_int++;
  }    
    
  return(rounded_int);
}
