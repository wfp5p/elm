
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
 * $Log: reverse.c,v $
 * Revision 1.2  1995/09/29  17:41:34  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:33  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** 

**/

#include "elm_defs.h"


void reverse(char *string)
{
	/** reverse string... pretty trivial routine, actually! **/

	register char *head, *tail, c;

	for (head = string, tail = string + strlen(string) - 1; head < tail; head++, tail--)
		{
		c = *head;
		*head = *tail;
		*tail = c;
		}
}
