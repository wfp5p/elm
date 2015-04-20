

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
 * $Log: delete.c,v $
 * Revision 1.3  1996/03/14  17:27:57  wfp5p
 * Alpha 9
 *
 * Revision 1.2  1995/09/29  17:42:03  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:35  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/**  Delete or undelete files: just set flag in header record! 
     Also tags specified message(s)...

**/

#include "elm_defs.h"
#include "elm_globals.h"
#include "s_elm.h"

#define msg_line(msg) (((compute_visible((msg)+1)-1) % headers_per_page) + 4)

int delete_msg(int real_del, int update_screen)
{
	/** Delete current message.  If real-del is false, then we're
	    actually requested to toggle the state of the current
	    message... **/
    struct header_rec *hdr;

    if (!inalias)
       hdr = curr_folder.headers[curr_folder.curr_mssg-1];

    if (real_del) {
	if (inalias) {
	    if (aliases[curr_alias-1]->type & SYSTEM)
		error(catgets(elm_msg_cat, ElmSet, ElmNoDelSysAlias,
			"Can't delete a system alias!"));
	    else
		setit(aliases[curr_alias-1]->status, DELETED);
	} else
	    setit(hdr->status, DELETED);
    } else {
	if (inalias) {
	    if (aliases[curr_alias-1]->type & SYSTEM)
		error(catgets(elm_msg_cat, ElmSet, ElmNoDelSysAlias,
			"Can't delete a system alias!"));
	    else if (ison(aliases[curr_alias-1]->status, DELETED))
		clearit(aliases[curr_alias-1]->status, DELETED);
	    else
		setit(aliases[curr_alias-1]->status, DELETED);
	} else if (ison(hdr->status, DELETED))
	    clearit(hdr->status, DELETED);
	else
	    setit(hdr->status, DELETED);
    }

    if (update_screen)
	if (inalias)
	    show_msg_status(curr_alias-1);
	else
	    show_msg_status(curr_folder.curr_mssg-1);
}

int undelete_msg(int update_screen)
{
    /** clear the deleted message flag **/

    if (inalias) {
	clearit(aliases[curr_alias-1]->status, DELETED);
	if (update_screen)
	    show_msg_status(curr_alias-1);
    } else {
	clearit(curr_folder.headers[curr_folder.curr_mssg-1]->status, DELETED);
	if (update_screen)
	    show_msg_status(curr_folder.curr_mssg-1);
    }

}

int show_msg_status(int msg)
{
    /** show the status of the current message only.  **/

    int status, do_standout;
    char *s;

    if (on_page(msg)) {
	if (inalias) {
	    status = aliases[msg]->status;
	    do_standout = (msg+1 == curr_alias && !arrow_cursor);
	} else {
	    status = curr_folder.headers[msg]->status;
	    do_standout = (msg+1 == curr_folder.curr_mssg && !arrow_cursor);
	}
	s = show_status(status);
	MoveCursor(msg_line(msg), 2);
	if (do_standout)
	    StartStandout();
	WriteChar(*s);
	if (do_standout)
	    EndStandout();
    }
}

int tag_message(int update_screen)
{
    /** Tag current message and return TRUE.
    If already tagged, untag it and return FALSE. **/

    int istagged;

    if (inalias) {
	if (aliases[curr_alias-1]->status & TAGGED) {
	    aliases[curr_alias-1]->status &= ~TAGGED;
	    istagged = FALSE;
	} else {
	    aliases[curr_alias-1]->status |= TAGGED;
	    istagged = TRUE;
	}
	if (update_screen)
	    show_msg_tag(curr_alias-1);
    } else {
	if (curr_folder.headers[curr_folder.curr_mssg-1]->status & TAGGED) {
	    curr_folder.headers[curr_folder.curr_mssg-1]->status &= ~TAGGED;
	    istagged = FALSE;
	} else {
	    curr_folder.headers[curr_folder.curr_mssg-1]->status |= TAGGED;
	    istagged = TRUE;
	}
	if (update_screen)
	    show_msg_tag(curr_folder.curr_mssg-1);
    }

    return istagged;
}

int show_msg_tag(int msg)
{
    /** show the tag status of the current message only.  **/

    int status, do_standout;

    if (on_page(msg)) {
	if (inalias) {
	    status = aliases[curr_alias-1]->status;
	    do_standout = (msg+1 == curr_alias && !arrow_cursor);
	} else {
	    status = curr_folder.headers[curr_folder.curr_mssg-1]->status;
	    do_standout = (msg+1 == curr_folder.curr_mssg && !arrow_cursor);
	}
	MoveCursor(msg_line(msg), 4);
	if (do_standout)
	    StartStandout();
	WriteChar(ison(status, TAGGED) ? '+' : ' ');
	if (do_standout)
	    EndStandout();
    }
}

int show_new_status(int msg)
{
    /** If the specified message is on this screen, show
	the new status (could be marked for deletion now,
	and could have tag removed...)
    **/

    int status, do_standout;

    if (on_page(msg)) {
	if (inalias) {
	    status = aliases[msg]->status;
	    do_standout = (msg+1 == curr_alias && !arrow_cursor);
	} else {
	    status = curr_folder.headers[msg]->status;
	    do_standout = (msg+1 == curr_folder.curr_mssg && !arrow_cursor);
	}
	MoveCursor(msg_line(msg), 2);
	if (do_standout)
	    StartStandout();
	PutLine(-1, -1, show_status(status));
	WriteChar(ison(status, TAGGED) ? '+' : ' ');
	if (do_standout)
	    EndStandout();
    }

}
