static char rcsid[] = "@(#)$Id: move_left.c,v 1.2 1995/09/29 17:41:21 wfp5p Exp $";

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
 * $Log: move_left.c,v $
 * Revision 1.2  1995/09/29  17:41:21  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:32  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** 

**/

#include "elm_defs.h"

void
move_left(string, chars)
char *string;
int  chars;
{
	/** moves string chars characters to the left DESTRUCTIVELY **/

	register char *source, *destination;

	source = string + chars;
	destination = string;
	while (*source != '\0' && *source != '\n')
		*destination++ = *source++;

	*destination = '\0';
}
