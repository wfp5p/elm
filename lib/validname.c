

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
 * $Log: validname.c,v $
 * Revision 1.3  1996/03/14  17:27:45  wfp5p
 * Alpha 9
 *
 * Revision 1.2  1995/09/29  17:41:47  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:31  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

#include "elm_defs.h"

#ifndef NOCHECK_VALIDNAME		 /* Force a return of valid */
# ifdef PWDINSYS
#  include <sys/pwd.h>
# else
#  include <pwd.h>
# endif
#endif

int valid_name(const char *name)
{
	/** Determine whether "name" is a valid logname on this system.
	    It is valid if there is a password entry, or if there is
	    a mail file in the mail spool directory for "name".
	 **/

#ifdef NOCHECK_VALIDNAME		 /* Force a return of valid */

	return(TRUE);

#else

	char filebuf[SLEN];
	struct passwd *getpwnam();

	sprintf(filebuf, "%s/%s", mailhome, name);
	return (getpwnam(name) != NULL); /* && access(filebuf, ACCESS_EXISTS) == 0);*/
#endif
}
