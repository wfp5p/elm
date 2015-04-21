
/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.3 $   $State: Exp $
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
 * $Log: shiftlower.c,v $
 * Revision 1.3  1995/09/29  17:41:38  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/06/22  14:48:37  wfp5p
 * Performance enhancements from Paul Close
 *
 * Revision 1.1.1.1  1995/04/19  20:38:33  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** 

**/

#include "elm_defs.h"

char *shift_lower(char *string)
{
	/** return 'string' shifted to lower case.  Do NOT touch the
	    actual string handed to us! **/

	static int first_time = TRUE;
	static char lower[1<<(8*sizeof(char))];
	int d;

	static char buffer[VERY_LONG_STRING];
	char *bufptr = buffer;

	if (first_time) {
	  for (d=1<<(8*sizeof(char)); d >= 0; d--)
	    lower[d] = tolower(d);
	  first_time = FALSE;
	}

	if (string == NULL) {
		buffer[0] = 0;
		return( (char *) buffer);
	}
	for (; *string; string++, bufptr++)
	  *bufptr = lower[*string];
	
	*bufptr = 0;
	
	return( (char *) buffer);
}
