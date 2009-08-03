
static char rcsid[] = "@(#)$Id: expand.c,v 1.3 1996/03/14 17:27:35 wfp5p Exp $";

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
 * $Log: expand.c,v $
 * Revision 1.3  1996/03/14  17:27:35  wfp5p
 * Alpha 9
 *
 * Revision 1.2  1995/09/29  17:41:09  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:32  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** This is a library routine for the various utilities that allows
    users to have the standard 'Elm' folder directory nomenclature
    for all filenames (e.g. '+', '=' or '%').  It should be compiled
    and then linked in as needed.

**/

#include "elm_defs.h"
#include "s_elmrc.h"

extern nl_catd elm_msg_cat;	/* message catalog	    */

char *expand_define();

static char*
expand_maildir(rcfile, buffer)
FILE *rcfile;
char *buffer;
{
	char *home = NULL, *bufptr;
	int  foundit = 0;

	bufptr = (char *) buffer;		/* same address */
	
	while (! foundit && mail_gets(buffer, SLEN, rcfile) != 0) {
	  if (strncmp(buffer, "maildir", 7) == 0 ||
	      strncmp(buffer, "folders", 7) == 0) {
	    while (*bufptr != '=' && *bufptr) 
	      bufptr++;
	    bufptr++;			/* skip the equals sign */
	    while (isspace(*bufptr) && *bufptr)
	      bufptr++; 
	    home = bufptr;		/* remember this address */

	    while (! isspace(*bufptr) && *bufptr != '\n')
	      bufptr++;

	    *bufptr = '\0';		/* remove trailing space */
	    foundit++;
	  }
	}

	return home;
}

int
expand(filename)
char *filename;
{
	/** Expand the filename since the first character is a meta-
	    character that should expand to the "maildir" variable
	    in the users ".elmrc" file or in the global rc file...

	    Note: this is a brute force way of getting the entry out 
	    of the .elmrc file, and isn't recommended for the faint 
	    of heart!
	**/

	FILE *rcfile;
	char  buffer[SLEN], *home, *expanded_dir;

	if ((home = getenv("HOME")) == NULL) {
	  printf(catgets(elm_msg_cat, ElmrcSet, ElmrcExpandHome,
	     "Can't expand environment variable $HOME to find .elmrc file!\n"));
	  return(FALSE);
	}

/*	sprintf(buffer, "%s/%s", home, elmrcfile); */
        getelmrcName(buffer,SLEN);

	home = NULL;
	if ((rcfile = fopen(buffer, "r")) != NULL) {
	  home = expand_maildir(rcfile, buffer);
	  fclose(rcfile);
	}

	if (home == NULL) { /* elmrc didn't exist or maildir wasn't in it */
	  if ((rcfile = fopen(system_rc_file, "r")) != NULL) {
	    home = expand_maildir(rcfile, buffer);
	    fclose(rcfile);
	  }
	}

	if (home == NULL) {
	  /* Didn't find it, use default */
	  sprintf(buffer, "~/%s", default_folders);
	  home = buffer;
	}

	/** Home now points to the string containing your maildir, with
	    no leading or trailing white space...
	**/

	if ((expanded_dir = expand_define(home)) == NULL)
		return(FALSE);

	sprintf(buffer, "%s%s%s", expanded_dir, 
		(expanded_dir[strlen(expanded_dir)-1] == '/' ||
		filename[0] == '/') ? "" : "/", (char *) filename+1);

	strcpy(filename, buffer);
	return(TRUE);
}

char *expand_define(maildir)
const char *maildir;
{
	/** This routine expands any occurances of "~" or "$var" in
	    the users definition of their maildir directory out of
	    their .elmrc file.

	    Again, another routine not for the weak of heart or staunch
	    of will!
	**/

	static char buffer[SLEN];	/* static buffer AIEE!! */
	char   name[SLEN],		/* dynamic buffer!! (?) */
	       *nameptr,	       /*  pointer to name??     */
	       *value;		      /* char pointer for munging */

	if (*maildir == '~') 
	  sprintf(buffer, "%s%s", getenv("HOME"), ++maildir);
	else if (*maildir == '$') { 	/* shell variable */

	  /** break it into a single word - the variable name **/

	  strcpy(name, (char *) maildir + 1);	/* hurl the '$' */
	  nameptr = (char *) name;
	  while (*nameptr != '/' && *nameptr) nameptr++;
	  *nameptr = '\0';	/* null terminate */
	  
	  /** got word "name" for expansion **/

	  if ((value = getenv(name)) == NULL) {
	    printf(catgets(elm_msg_cat, ElmrcSet, ElmrcExpandShell,
		    "Couldn't expand shell variable $%s in .elmrc!\n"),
		    name);
	    return(NULL);
	  }
	  sprintf(buffer, "%s%s", value, maildir + strlen(name) + 1);
	}
	else strcpy(buffer, maildir);

	return( ( char *) buffer);
}
