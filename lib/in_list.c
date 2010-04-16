
/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.4 $   $State: Exp $
 *
 *                      Copyright (c) 1988-1995 USENET Community Trust
 *			Copyright (c) 1986,1987 Dave Taylor
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 *******************************************************************************
 * $Log: in_list.c,v $
 * Revision 1.4  1995/09/29  17:41:14  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.3  1995/06/12  20:33:06  wfp5p
 * Changed improper use of NULL to 0
 * Added a missing declaration
 *
 * Revision 1.2  1995/05/04  15:18:05  wfp5p
 * The wildcard '*' is now allowed in the alternatives list.
 *
 * Revision 1.1.1.1  1995/04/19  20:38:32  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/**

**/

#include "elm_defs.h"


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
	 *
	 * The wildcard * can now also be used.  If there is a wildcard, the
	 * behaviour is changed slightly.
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

           if (strrchr(lower_target, '*') != NULL) /* do something with wildcards */
	   {
	      char tmpStr[SLEN];
	      char *tptr;

	      strcpy(tmpStr,lower_target);
	      tptr = strrchr(tmpStr, '*');
	      *tptr = '\0';

	      if (strncmp(tmpStr,next_item,strlen(tmpStr)) != 0) /* user is different! */
	        return(FALSE);

	      tptr++;
	      strcpy(tmpStr,tptr);

	      tptr = next_item;

	      tptr += strlen(next_item)-strlen(tmpStr);

             if (strncmp(tmpStr,tptr,strlen(tmpStr)) != 0)
	         return(FALSE);
	      else
	        return(TRUE);
	   }

	   else

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
