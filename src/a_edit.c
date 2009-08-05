

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.3 $   $State: Exp $
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
 * $Log: a_edit.c,v $
 * Revision 1.3  1996/03/14  17:27:47  wfp5p
 * Alpha 9
 *
 * Revision 1.2  1995/09/29  17:41:53  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:34  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** This routine is for allowing the user to edit their aliases.text
    as they wish. 

**/

#include "elm_defs.h"
#include "elm_globals.h"

edit_aliases_text()
{
	/** Allow the user to edit their aliases text, always resynchronizing
	    afterwards.   This routine calls the function edit_a_file()
	    to do the actual editing.  Thus editing is done the same
	    way here as in mailbox editing.

	    We will ALWAYS resync on the aliases text file
	    even if nothing has changed since, not unreasonably, it's
	    hard to figure out what occurred in the edit session...
	**/

	char     at_file[SLEN];

	sprintf(at_file,"%s/%s", user_home, ALIAS_TEXT);
	if (edit_a_file(at_file) < 0) {
	    return (0);
	}
/*
 *	Redo the hash table...always!
 */
	install_aliases();

/*
 *	clear any limit in effect.
 */
	selected = 0;

	return(1);
}
