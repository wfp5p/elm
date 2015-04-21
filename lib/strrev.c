
/*******************************************************************************
 *  The Elm Mail System
 *
 *                      Copyright (c) 1988-1995 USENET Community Trust
 *			Copyright (c) 1986,1987 Dave Taylor
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 ******************************************************************************/

/** 

**/

#include "elm_defs.h"

void elm_strrev(char *string)
{
	/** reverse string... pretty trivial routine, actually! **/

	char *head, *tail, c;

	for (head = string, tail = string + strlen(string) - 1; head < tail; head++, tail--)
		{
		c = *head;
		*head = *tail;
		*tail = c;
		}
}
