
static char rcsid[] = "@(#)$Id: strstr.c,v 1.2 1995/09/29 17:41:44 wfp5p Exp $";

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
 * $Log: strstr.c,v $
 * Revision 1.2  1995/09/29  17:41:44  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:33  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** look for substring in string
**/

#include "elm_defs.h"

/*
 * strstr() -- Locates a substring.
 *
 * This is a replacement for the POSIX function which does not
 * appear on all systems.
 *
 * Synopsis:
 *	#include <string.h>
 *	char *strstr(const char *s1, const char *s2);
 *
 * Arguments:
 *	s1	Pointer to the subject string.
 *	s2	Pointer to the substring to locate.
 *
 * Returns:
 *	A pointer to the located string or NULL
 *
 * Description:
 *	The strstr() function locates the first occurence in s1 of
 *	the string s2.  The terminating null characters are not
 *	compared.
 */

#ifndef STRSTR

char *strstr(s1, s2)
const char *s1, *s2;
{
	int len;
	const char *ptr;
	const char *tmpptr;

	ptr = NULL;
	len = strlen(s2);

	if ( len <= strlen(s1)) {
	    tmpptr = s1;
	    while ((ptr = index(tmpptr, (int)*s2)) != NULL) {
	        if (strncmp(ptr, s2, len) == 0) {
	            break;
	        }
	        tmpptr = ptr+1;
	    }
	}
	return (char *)ptr;
}

#endif /* !STRSTR */
