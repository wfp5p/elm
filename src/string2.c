

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.3 $   $State: Exp $
 *
 *                      Copyright (c) 1988-1995 USENET Community Trust
 * 			Copyright (c) 1986,1987 Dave Taylor
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 *******************************************************************************
 * $Log: string2.c,v $
 * Revision 1.3  1995/09/29  17:42:34  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/09/11  15:19:31  wfp5p
 * Alpha 7
 *
 * Revision 1.1.1.1  1995/04/19  20:38:39  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** This file contains string functions that are shared throughout the
    various ELM utilities...

**/

#include "elm_defs.h"
#include "elm_globals.h"

int occurances_of(int ch, char *string)
{
	/** returns the number of occurances of 'ch' in string 'string' **/

	register int count = 0;

	for (; *string; string++)
	  if (*string == ch) count++;

	return(count);
}

int remove_possible_trailing_spaces(char *string)
{
	/** an incredibly simple routine that will read backwards through
	    a string and remove all trailing whitespace.
	**/

	register int i, j;

	for ( j = i = strlen(string); --i >= 0 && whitespace(string[i]); )
		/** spin backwards, semicolon intented **/ ;

	if (i > 0 && string[i-1] == '\\')
		i++;		/* allow for line to end with \blank  */

	if (i < j)
	  string[i+1] = '\0';	/* note that even in the worst case when there
				   are no trailing spaces at all, we'll simply
				   end up replacing the existing '\0' with
				   another one!  No worries, as M.G. would say
				*/
}
