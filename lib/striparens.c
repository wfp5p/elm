static char rcsid[] = "@(#)$Id: striparens.c,v 1.2 1995/09/29 17:41:42 wfp5p Exp $";

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
 * $Log: striparens.c,v $
 * Revision 1.2  1995/09/29  17:41:42  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:33  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/* 
 * strip_parens() - Delete all (parenthesized) information from a string.
 * get_parens() - Extract all (parenthesized) information from a string.
 *
 * These routines handle RFC-822 comments.  Nested parens are understood.
 * get_parens() does not include the parens in the return result.  Both
 * routines are non-destructive.  They return a pointer to static data
 * that will be overwritten on the next call to either routine.
 */

#include "elm_defs.h"

static char paren_buffer[VERY_LONG_STRING];

char *strip_parens(src)
register const char *src;
{
	register int len;
	register char *dest = paren_buffer;

	while (*src != '\0') {
		len = rfc822_toklen(src);
		if (*src != '(') {	/*)*/
			strncpy(dest, src, len);
			dest += len;
		}
		src += len;
	}
	*dest = '\0';
	return paren_buffer;
}

char *get_parens(src)
register const char *src;
{
	register int len;
	register char *dest = paren_buffer;

	while (*src != '\0') {
		len = rfc822_toklen(src);
		if (len > 2 && *src == '(') {	/*)*/
			strncpy(dest, src+1, len-2);
			dest += (len-2);
		}
		src += len;
	}
	*dest = '\0';
	return paren_buffer;
}

#ifdef _TEST
main()
{
	char buf[1024];
	while (fputs("\nstr> ", stdout), gets(buf) != NULL) {
		printf("strip_parens() |%s|\n", strip_parens(buf));
		printf("get_parens()   |%s|\n", get_parens(buf));
	}
	putchar('\n');
	exit(0);
}
#endif

