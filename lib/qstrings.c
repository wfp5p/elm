

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.2 $   $State: Exp $
 *
 *                      Copyright (c) 1988-1995 USENET Community Trust
 * 			Copyright (c) 1986,1987 Dave Taylor
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 *******************************************************************************
 * $Log: qstrings.c,v $
 * Revision 1.2  1995/09/29  17:41:30  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:33  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** This file contains equivalent routines to the string routines, but
     modifed to handle quoted strings.

**/

#include "elm_defs.h"

char *qstrpbrk(const char *source, const char *keys)
{
	/** Returns a pointer to the first character of source that is any
	    of the specified keys, or NULL if none of the keys are present
	    in the source string.
	**/

	const char *s, *k;
	int len;

	s = source;
	while (*s != '\0') {
	  len = len_next_part(s);
	  if (len == 1) {
	    for (k = keys; *k; k++)
	      if (*k == *s)
		return (char *)s;
	  }
	  s += len;
	}

	return(NULL);
}

int qstrspn(const char *source, const char *keys)
{
	/** This function returns the length of the substring of
	    'source' (starting at zero) that consists ENTIRELY of
	    characters from 'keys'.  This is used to skip over a
	    defined set of characters with parsing, usually.
	**/

	int loc = 0, key_index = 0, len;

	while (source[loc] != '\0') {
	  key_index = 0;
	  len = len_next_part(&source[loc]);
	  if (len > 1)
	    return(loc);

	  while (keys[key_index] != source[loc])
	    if (keys[key_index++] == '\0')
	      return(loc);
	  loc++;
	}

	return(loc);
}

int qstrcspn(const char *source, const char *keys)
{
	/** This function returns the length of the substring of
	    'source' (starting at zero) that consists entirely of
	    characters NOT from 'keys'.  This is used to skip to a
	    defined set of characters with parsing, usually.
	    NOTE that this is the opposite of strspn() above
	**/

	int loc = 0, key_index = 0, len;

	while (source[loc] != '\0') {
	  key_index = 0;
	  len = len_next_part(&source[loc]);
	  if (len > 1) {
	    loc += len;
	    continue;
	  }

	  while (keys[key_index] != '\0')
	    if (keys[key_index++] == source[loc])
	      return(loc);
	  loc++;
	}

	return(loc);
}
