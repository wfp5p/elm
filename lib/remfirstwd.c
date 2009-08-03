static char rcsid[] = "@(#)$Id: remfirstwd.c,v 1.2 1995/09/29 17:41:32 wfp5p Exp $";

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
 * $Log: remfirstwd.c,v $
 * Revision 1.2  1995/09/29  17:41:32  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:33  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** 

**/

#include "elm_defs.h"


void
remove_first_word(string)
char *string;
{	/** removes first word of string, ie up to first non-white space
	    following a white space! **/

	register int loc;

	for (loc = 0; string[loc] != ' ' && string[loc] != '\0'; loc++) 
	    ;

	while (string[loc] == ' ' || string[loc] == '\t')
	  loc++;
	
	move_left(string, loc);
}

void
remove_header_keyword(string)
char *string;
{	/** removes a RFC822 header keyword from the string.
	    i.e. removes up to (and including) the first colon,
	    plus any white-space immediately following it.  **/

	register int loc;

	for (loc = 0; string[loc] != ':' && string[loc] != '\0'; loc++) 
	    ;

	if (string[loc] == ':') {
	    loc++; /* move beyond the colon */
	    while (string[loc] == ' ' || string[loc] == '\t')
		loc++;
	}
	
	move_left(string, loc);
}
