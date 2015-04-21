

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.3 $   $State: Exp $
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
 * $Log: opt_utils.c,v $
 * Revision 1.3  1996/03/14  17:27:43  wfp5p
 * Alpha 9
 *
 * Revision 1.2  1995/09/29  17:41:23  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:33  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** This file contains routines that might be needed for the various
     machines that the mailer can run on.  Please check the Makefile
     for more help and/or information. 

**/

#include "elm_defs.h"
#include "s_error.h"

#ifdef PWDINSYS
#  include <sys/pwd.h>
#else
#  include <pwd.h>
#endif


#ifndef STRTOK

char *strtok(source, keys)
char *source, *keys;
{
	/** This function returns a pointer to the next word in source
	    with the string considered broken up at the characters 
	    contained in 'keys'.  Source should be a character pointer
	    when this routine is first called, then NULL subsequently.
	    When strtok has exhausted the source string, it will 
	    return NULL as the next word. 

	    WARNING: This routine will DESTROY the string pointed to
	    by 'source' when first invoked.  If you want to keep the
	    string, make a copy before using this routine!!
	 **/

	int  last_ch;
	static   char *sourceptr;
		 char *return_value;

	if (source != NULL)
	  sourceptr = source;
	
	if (*sourceptr == '\0') 
	  return(NULL);		/* we hit end-of-string last time!? */

	sourceptr += strspn(sourceptr, keys);	/* skip leading crap */
	
	if (*sourceptr == '\0') 
	  return(NULL);		/* we've hit end-of-string */

	last_ch = strcspn(sourceptr, keys);	/* end of good stuff */

	return_value = sourceptr;		/* and get the ret   */

	sourceptr += last_ch;			/* ...value 	     */

	if (*sourceptr != '\0')		/* don't forget if we're at END! */
	  sourceptr++;			   /* and skipping for next time */

	return_value[last_ch] = '\0';		/* ..ending right    */
	
	return((char *) return_value);		/* and we're outta here! */
}

#endif


#ifndef STRPBRK

char *strpbrk(source, keys)
char *source, *keys;
{
	/** Returns a pointer to the first character of source that is any
	    of the specified keys, or NULL if none of the keys are present
	    in the source string. 
	**/

	char *s, *k;

	for (s = source; *s != '\0'; s++) {
	  for (k = keys; *k; k++)
	    if (*k == *s)
	      return(s);
	}
	
	return(NULL);
}

#endif

#ifndef STRSPN

strspn(source, keys)
char *source, *keys;
{
	/** This function returns the length of the substring of
	    'source' (starting at zero) that consists ENTIRELY of
	    characters from 'keys'.  This is used to skip over a
	    defined set of characters with parsing, usually. 
	**/

	int loc = 0, key_index = 0;

	while (source[loc] != '\0') {
	  key_index = 0;
	  while (keys[key_index] != source[loc])
	    if (keys[key_index++] == '\0')
	      return(loc);
	  loc++;
	}

	return(loc);
}

#endif

#ifndef STRCSPN

strcspn(source, keys)
char *source, *keys;
{
	/** This function returns the length of the substring of
	    'source' (starting at zero) that consists entirely of
	    characters NOT from 'keys'.  This is used to skip to a
	    defined set of characters with parsing, usually. 
	    NOTE that this is the opposite of strspn() above
	**/

	int loc = 0, key_index = 0;

	while (source[loc] != '\0') {
	  key_index = 0;
	  while (keys[key_index] != '\0')
	    if (keys[key_index++] == source[loc])
	      return(loc);
	  loc++;
	}

	return(loc);
}

#endif

#ifndef TEMPNAM
/* and a tempnam for temporary files */
static int cnt = 0;

char *tempnam( dir, pfx)
 char *dir, *pfx;
{
	char space[SLEN];
	char *newspace;

	if (dir == NULL) {
		dir = "/usr/tmp";
	} else if (*dir == '\0') {
		dir = "/usr/tmp";
	}
	
	if (pfx == NULL) {
		pfx = "";
	}

	sprintf(space, "%s%s%d.%d", dir, pfx, getpid(), cnt);
	cnt++;
	
	newspace = malloc(strlen(space) + 1);
	if (newspace != NULL) {
		strcpy(newspace, space);
	}
	return newspace;
}

#endif



