
static char rcsid[] = "@(#)$Id: chloc.c,v 1.2 1995/09/29 17:41:06 wfp5p Exp $";

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.2 $   $State: Exp $
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
 * $Log: chloc.c,v $
 * Revision 1.2  1995/09/29  17:41:06  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:31  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** 

**/

#include "elm_defs.h"

int
chloc(string, ch)
const char *string;
register int ch;
{
	/** returns the index of ch in string, or -1 if not in string **/
	register const char *s;

	for (s = string; *s; s++)
		if (*s == ch)
			return(s - string);
	return(-1);
}

int
qchloc(string, ch)
const char *string;
register int ch;
{
	/* returns the index of ch in string, or -1 if not in string
         * skips over quoted portions of the string
	 */
	register const char *s;
	register int l;

	for (s = string; *s; s++) {
		l = len_next_part(s);
		if (l > 1) { /* a quoted char/string can not be a match */
			s += l - 1;
			continue;
		}

		if (*s == ch)
			return(s - string);
	}

	return(-1);
}

