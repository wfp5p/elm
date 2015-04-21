

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.4 $   $State: Exp $
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
 * $Log: newalias.c,v $
 * Revision 1.4  1996/03/14  17:30:09  wfp5p
 * Alpha 9
 *
 * Revision 1.3  1995/09/29  17:42:45  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/04/20  21:02:08  wfp5p
 * Added the showreply feature and emacs key bindings.
 *
 * Revision 1.1.1.1  1995/04/19  20:38:40  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** Install a new set of aliases for the 'Elm' mailer. 

	If invoked without the -g argument, it assumes that
  it is working with an individual users alias tables, and
  generates the .alias.pag and .alias.dir files in their
  .elm directory.
	If, however, it is invoked with the -g flag,
  it assumes that the user is updating the system alias
  file and uses the defaults for everything.

  The format for the input file is;
    alias1, alias2, ... = username = address
or  alias1, alias2, ... = groupname= member, member, member, ...
                                     member, member, member, ...

**/

#define INTERN
#include "elm_defs.h"
#include "s_newalias.h"

#include <stdarg.h>

static void na_error(char *err_message);
void show_error(const char *s, ...);


static int  is_system=0;		/* system file updating?     */
static int  sleepmsg=0;		/* not in elm, dont wait for messages */

/* used by mk_aliases.c */
int get_is_system(void)
{
	return is_system;
}

int main(int argc, char *argv[])
{
	char inputname[SLEN], dataname[SLEN];
	int  a;

	initialize_common();

	for (a = 1; a < argc; ++a) {
	  if (strcmp(argv[a], "-g") == 0)
	    is_system = 1;
#ifdef DEBUG
	  else if (strcmp(argv[a], "-d") == 0)
	    debug = 10;
#endif
	  else {
	      fprintf(stderr, catgets(elm_msg_cat,
	            NewaliasSet, NewaliasUsage, "Usage: %s [-g]\n"), argv[0]);
	      exit(1);
	  }
	}

	if (is_system) {   /* update system aliases */
	    printf(catgets(elm_msg_cat, NewaliasSet, NewaliasUpdateSystem,
	            "Updating the system alias file..."));

	    strcpy(inputname, system_text_file);
	    strcpy(dataname,  system_data_file);
	}
	else {
	    printf(catgets(elm_msg_cat, NewaliasSet, NewaliasUpdatePersonal,
		"Updating your personal alias file..."));
	    MCsprintf(inputname, "%s/%s", user_home, ALIAS_TEXT);
	    MCsprintf(dataname,  "%s/%s", user_home, ALIAS_DATA); 
	}

	if ((a = do_newalias(inputname, dataname, FALSE, TRUE)) < 0) {
	    exit(1);
	}
	else {
	    printf(catgets(elm_msg_cat, NewaliasSet, NewaliasProcessed,
	            "processed %d aliases.\n"), a);
	    exit(0);
	}

	/*NOTREACHED*/
}

static void na_error(char *err_message)
{
	fflush(stdout);
	fprintf(stderr, "\n%s\n", err_message);
	return;
}

/*
 * non curses version of show_error()
 * show_error() is also defined in src/out_utils.c and is used by
 * lib/mk_aliases.c  Since newmail is not a curses program, it can't use it
 */
void show_error(const char *s, ...)
{
	va_list args;

	va_start(args, s);
	fprintf(stderr, s, args);
	va_end(args);

}
