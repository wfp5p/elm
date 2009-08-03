
static char rcsid[] = "@(#)$Id: putenv.c,v 1.2 1995/09/29 17:41:28 wfp5p Exp $";

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
 * $Log: putenv.c,v $
 * Revision 1.2  1995/09/29  17:41:28  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:33  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/*
 * This code was stolen from cnews.  Modified to make "newenv" static so
 * that realloc() can be used on subsequent calls to avoid memory leaks.
 *
 * We only need this if Configure said there isn't a putenv() in libc.
 */

#include "elm_defs.h"

#ifndef PUTENV /*{*/

/* peculiar return values */
#define WORKED 0
#define FAILED 1

int
putenv(var)			/* put var in the environment */
const char *var;
{
	char **envp;
	int oldenvcnt;
	extern char **environ;
	static char **newenv = NULL;

	/* count variables, look for var */
	for (envp = environ; *envp != 0; envp++) {
		const char *varp = var;
		char *ep = *envp;
		int namesame;

		namesame = NO;
		for (; *varp == *ep && *varp != '\0'; ++ep, ++varp)
			if (*varp == '=')
				namesame = YES;
		if (*varp == *ep && *ep == '\0')
			return WORKED;	/* old & new var's are the same */
		if (namesame) {
			*envp = (char *)var;	/* replace var with new value */
			return WORKED;
		}
	}
	oldenvcnt = envp - environ;

	/* allocate new environment with room for one more variable */
	if (newenv == NULL)
	    newenv = (char **)malloc((unsigned)((oldenvcnt+1+1)*sizeof(*envp)));
	else
	    newenv = (char **)realloc((char *)newenv, (unsigned)((oldenvcnt+1+1)*sizeof(*envp)));
	if (newenv == NULL)
		return FAILED;

	/* copy old environment pointers, add var, switch environments */
	(void) bcopy((char *)environ, (char *)newenv, oldenvcnt*sizeof(*envp));
	newenv[oldenvcnt] = (char *)var;
	newenv[oldenvcnt+1] = NULL;
	environ = newenv;
	return WORKED;
}

#endif /*}PUTENV*/

