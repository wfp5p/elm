

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.2 $   $State: Exp $
 *
 *                      Copyright (c) 1988-1995 USENET Community Trust
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 *******************************************************************************
 * $Log: len_next.c,v $
 * Revision 1.2  1995/09/29  17:41:15  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:32  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** get the length of the next part of the address/data field

	This code returns the length of the next part of the
  string field containing address/data.  It takes into account
  quoting via " as well as \ escapes.
  Quoting via ' is not taken into account, as RFC-822 does not
  consider a ' character a valid 'quoting character'

  A 1 is returned for a single character unless:

  A 0 is returned at end of string.

  A 2 is returned for strings that start \

  The length of quoted sections is returned for quoted fields

**/

#include "elm_defs.h"

int len_next_part(register const char *str)
{
	register const char *s;

	switch (*str) {

	case '\0':
		return 0;

	case '\\':
		return (*++str != '\0' ? 2 : 1);

	case '"':
		for (s = str+1 ; *s != '\0' ; ++s) {
			if (*s == '\\') {
				if (*++s == '\0')
					break;
			} else if (*s == '"') {
				++s;
				break;
			}
		}
		return (s - str);

	default:
		return 1;

	}

	/*NOTREACHED*/
}

#ifdef _TEST
main()
{
	char buf[256], *s;
	int len;

	while (gets(buf) != NULL) {
		for (s = buf ; (len = len_next_part(s)) > 0 ; s += len)
			printf("%4d %-.*s\n", len, len, s);
	}
	exit(0);
}
#endif
