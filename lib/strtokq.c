

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
 * $Log: strtokq.c,v $
 * Revision 1.2  1995/09/29  17:41:45  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:33  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/


/* Like strtok, but returns quoted strings as one token (quotes removed)
 * if flag is non-null. Quotes and backslahes can be escaped with backslash.
 */

#include "elm_defs.h"

char *strtokq(char *source, const char *keys, int flag)
{
	register int  last_ch;
	static   char *sourceptr = NULL;
		 char *return_value;

	if (source != NULL)
	  sourceptr = source;

	if (sourceptr == NULL || *sourceptr == '\0')
	  return(NULL);		/* we hit end-of-string last time!? */

	sourceptr += strspn(sourceptr, keys);	/* skip leading crap */

	if (*sourceptr == '\0')
	  return(NULL);		/* we've hit end-of-string */

	if (flag)
	  if (*sourceptr == '"' || *sourceptr == '\'') {   /* quoted string */
	    register char *sp;
	    char quote = *sourceptr++;

	    for (sp = sourceptr; *sp != '\0' && *sp != quote; sp++)
	      if (*sp == '\\') sp++;	/* skip escaped characters */
					/* expand_macros will fix them later */

	    return_value = sourceptr;
	    sourceptr = sp;
	    if (*sourceptr != '\0') sourceptr++;
	    *sp = '\0';				/* zero at end */

	    return return_value;
	  }

	last_ch = strcspn(sourceptr, keys);	/* end of good stuff */

	return_value = sourceptr;		/* and get the ret   */

	sourceptr += last_ch;			/* ...value 	     */

	if (*sourceptr != '\0')	/* don't forget if we're at END! */
	  sourceptr++;		   /* and skipping for next time */

	return_value[last_ch] = '\0';		/* ..ending right    */

	return((char *) return_value);		/* and we're outta here! */
}
