static char rcsid[] = "@(#)$Id: in_list.c,v 5.1 1992/10/03 22:41:36 syd Exp $";

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 5.1 $   $State: Exp $
 *
 *			Copyright (c) 1988-1992 USENET Community Trust
 *			Copyright (c) 1986,1987 Dave Taylor
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *	Syd Weinstein, Elm Coordinator
 *	elm@DSI.COM			dsinc!elm
 *
 *******************************************************************************
 * $Log: in_list.c,v $
 * Revision 5.1  1992/10/03  22:41:36  syd
 * Initial checkin as of 2.4 Release at PL0
 *
 *
 ******************************************************************************/

/** 

**/

#include "headers.h"


int
in_list(list, target)
char *list, *target;

{
	/* Returns TRUE iff target is an item in the list - case ignored.
	 * If target is simple (an atom of an address) match must be exact.
	 * If target is complex (contains a special character that separates
	 * address atoms), the target need only match a whole number of atoms
	 * at the right end of an item in the list. E.g.
	 * target:	item:			match:
	 * joe		joe			yes
	 * joe		jojoe			no (wrong logname)
	 * joe		machine!joe		no (similar logname on a perhaps
	 *					   different machine - to
	 *					   test this sort of item the 
	 *					   passed target must include
	 *					   proper machine name, is
	 *					   in next two examples)
	 * machine!joe	diffmachine!joe		no  "
	 * machine!joe	diffmachine!machine!joe	yes
	 * joe@machine	jojoe@machine		no  (wrong logname)
	 * joe@machine	diffmachine!joe@machine	yes
	 */

	register char	*rest_of_list,
			*next_item,
			ch;
	int		offset;
	char		*shift_lower(),
				lower_list[VERY_LONG_STRING],
				lower_target[SLEN];

	rest_of_list = strcpy(lower_list, shift_lower(list));
	strcpy(lower_target, shift_lower(target));
	while((next_item = strtok(rest_of_list, ", \t\n")) != NULL) {
	    /* see if target matches the whole item */
	    if(strcmp(next_item, lower_target) == 0)
		return(TRUE);

            if(index(lower_target,'*')!=0){
	      if (globmatch(next_item,lower_target))
                return(TRUE);
            } else
	    if(strpbrk(lower_target,"!@%:") != NULL) {

	      /* Target is complex */

	      if((offset = strlen(next_item) - strlen(lower_target)) > 0) {

		/* compare target against right end of next item */
		if(strcmp(&next_item[offset], lower_target) == 0) {

		  /* make sure we are comparing whole atoms */
		  ch=next_item[offset-1];
		  if(ch == '!' || ch == '@' || ch == '%' || ch == ':')
		    return(TRUE);
		}
	      }
	    }
	    rest_of_list = NULL;
	}
	return(FALSE);
}

int
globmatch (string, pat)
char *string, *pat;
{
  char buf[1024];
  int i=0,j=0,k,offset=0;
  
  for (;;) {
    k=0;
    while (pat[j]!='*' && pat[j]!='\0') {
      buf[k++]=pat[j++];
    }
    buf[k]='\0';

    /* check everything up to the first '*' */
    if (strncmp(string+offset,buf,i=strlen(buf))!=0)
      return(FALSE);

    /* if we have run out of pattern, then the string matches... */
    if (pat[j]=='\0' || pat[j+1]=='\0')
      return(TRUE);

    ++j; /* advance to the next 'real' character in the pattern */

    /* ffwd to the next real char that matches */
    while (string[i] != pat[j]) {
      /* if we still have pattern, but no string, then it doesn't match */
      if (string[i]=='\0')
	return(FALSE);
      i++;
    }
    offset+=i;
  }
  /* not reached */
}
