
/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.4 $   $State: Exp $
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
 * $Log: header_cmp.c,v $
 * Revision 1.4  1996/03/14  17:27:38  wfp5p
 * Alpha 9
 *
 * Revision 1.3  1995/09/29  17:41:13  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/06/22  14:48:36  wfp5p
 * Performance enhancements from Paul Close
 *
 * Revision 1.1.1.1  1995/04/19  20:38:32  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** 
	compare a header, ignoring case and allowing linear white space
	around the :.  Header must be anchored to the start of the line.

	returns NULL if no match, or first character after trailing linear
	white space of the :.

**/

#include "elm_defs.h"

char *header_cmp(const char *header, const char *prefix,
		 const char *suffix)
{
	int len;

	len = strlen(prefix);
	if (strncasecmp(header, prefix, len))
		return(NULL);

	/* skip over while space if any */
	header += len;

	if (*header != ':')	/* headers must end in a : */
		return(NULL);

	/* skip over white space if any */
	header++;

	while (*header) {
		if (!isspace(*header))
			break;
		header++;
	}

	if (suffix != NULL) {
		len = strlen(suffix);
		if (len > 0)
			if (strncasecmp(header, suffix, len))
				return(NULL);
	}

	return (char *)header;
}

/*
 * read_headers() in newmbox.c does a lot of header_cmp()s on string
 * constants.  Since the compiler already knows how long a string
 * constant is, we save a ton of strlen() calls by specifying the
 * lengths here.
 */
int header_ncmp(const char *header, const char *prefix, int preflen, const char *suffix, int sufflen)
{
	if (strncasecmp(header, prefix, preflen))
		return 0;

	header += preflen;

	if (*header != ':')	/* headers must end in a : */
		return 0;

	if (suffix != NULL && sufflen > 0) {
		/* skip over white space if any */
		header++;

		while (*header) {
			if (!isspace(*header))
				break;
			header++;
		}

		if (strncasecmp(header, suffix, sufflen))
			return 0;
	}

	return 1;
}
