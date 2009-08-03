
static char rcsid[] = "@(#)$Id: getfullnam.c,v 1.3 1996/08/08 19:49:20 wfp5p Exp $";

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
 * $Log: getfullnam.c,v $
 * Revision 1.3  1996/08/08  19:49:20  wfp5p
 * Alpha 11
 *
 * Revision 1.2  1995/09/29  17:41:12  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:32  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** 

**/

#include "elm_defs.h"
#include "elm_globals.h"
#ifdef PWDINSYS
#  include <sys/pwd.h>
#else
#  include <pwd.h>
#endif

char *
get_full_name(logname)
const char *logname;
{
	/* return a pointer to the full user name for the passed logname
	 * or NULL if cannot be found
	 * If PASSNAMES get it from the gcos field, otherwise get it
	 * from ~/.fullname.
	 */

#ifndef PASSNAMES
	FILE *fp;
	char fullnamefile[SLEN];
#endif
	static char fullname[SLEN];
	struct passwd *getpwnam(), *pass;

	if((pass = getpwnam(logname)) == NULL)
	  return(NULL);
#ifdef PASSNAMES	/* get full_username from gcos field */
	strcpy(fullname, gcos_name(pass->pw_gecos, logname));
#else			/* get full_username from ~/.fullname file */
	sprintf(fullnamefile, "%s/.fullname", pass->pw_dir);

	if(can_access(fullnamefile, READ_ACCESS) != 0)
	  return(NULL);		/* fullname file not accessible to user */
	if((fp = fopen(fullnamefile, "r")) == NULL)
	  return(NULL);		/* fullname file cannot be opened! */
	if(fgets(fullname, SLEN, fp) == NULL) {
	  fclose(fp);
	  return(NULL);		/* fullname file empty! */
	}
	fclose(fp);
	no_ret(fullname);	/* remove trailing '\n' */
#endif
	return(fullname);
}
