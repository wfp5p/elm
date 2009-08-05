

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.5 $   $State: Exp $
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
 * $Log: figadrssee.c,v $
 * Revision 1.5  1995/09/29  17:41:10  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.4  1995/09/11  15:18:53  wfp5p
 * Alpha 7
 *
 * Revision 1.3  1995/07/18  18:59:49  wfp5p
 * Alpha 6
 *
 * Revision 1.2  1995/06/22  14:48:35  wfp5p
 * Performance enhancements from Paul Close
 *
 * Revision 1.1.1.1  1995/04/19  20:38:32  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** 

**/

#include "elm_defs.h"

/*
 * figure_out_addressee() - Extract the full name of
 * the primary recipient from a header value contained in "buf" (header
 * name stripped, value only) and save the result to "save_fullname".
 * If, however, this user appears anywhere in the list then save this
 * user's name instead.
 *
 * The "save_fullname" buffer must be initialized to an empty string
 * prior to calling this routine.  The routine may be invoked multiple
 * times, e.g. for a header that spans multiple lines.
 *
 * This routine is NONdestructive to "buf".
 */

void
figure_out_addressee(buf, username, save_fullname)
const char *buf, *username;
char *save_fullname;
{
    const char *curr_field;
    char *next_field;
    char address[SLEN], fullname[SLEN];

    /* if this user was seen as a recipient, preserve that value */
    if (save_fullname[0] == username[0] && strcmp(save_fullname, username) == 0)
	return;

    for (curr_field = buf ; *curr_field != '\0' ; curr_field = next_field) {

	if (parse_arpa_mailbox(curr_field, address, sizeof(address),
		fullname, sizeof(fullname), &next_field) != 0) {
	    /* illegal address - silently ignore it */
	    continue;
	}

	/* see if this is one of the user's addresses */
	if (!okay_address(address, (char *)NULL)) {
	    strncpy(save_fullname, username,SLEN);
	    return;
	}

	/* if we don't have a name yet, then use this one */
	if (save_fullname[0] == '\0') {
	    (void) strncpy(save_fullname, 
		(fullname[0] != '\0' ? fullname : address),SLEN);
	}

    }

    return;
}
