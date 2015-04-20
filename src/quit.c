

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
 * $Log: quit.c,v $
 * Revision 1.3  1996/03/14  17:29:45  wfp5p
 * Alpha 9
 *
 * Revision 1.2  1995/09/29  17:42:22  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.1.1.1  1995/04/19  20:38:38  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** quit: leave the current folder and quit the program.

**/

#include "elm_defs.h"
#include "elm_globals.h"
#include "s_elm.h"

long bytes();

void quit(int prompt)
{
    if (leave_mbox(FALSE, TRUE, prompt) == -1) {
	/* new mail has arrived */
	return;
    }
    leave(LEAVE_NORMAL);
}

void quit_abandon(int do_prompt)
{
	/** Exit, abandoning all changes to the mailbox (if there were
	    any, and if the user say's it's ok)
	**/

	char *msg;
	register int i, changes;

	dprint(1, (debugfile, "\n\n-- exiting --\n\n"));

	if (do_prompt) {

	    /* Determine if any messages are scheduled for deletion, or if
	     * any message has changed status
	     */
	    for (changes = 0, i = 0; i < curr_folder.num_mssgs; i++)
	      if (ison(curr_folder.headers[i]->status, DELETED) || curr_folder.headers[i]->status_chgd)
		changes++;

	    if (changes) {
	      if (changes == 1)
		msg = catgets(elm_msg_cat, ElmSet, ElmAbandonChange,
		    "Abandon change to mailbox?");
	      else
		msg = catgets(elm_msg_cat, ElmSet, ElmAbandonChangePlural,
		    "Abandon changes to mailbox?");
	      if (!enter_yn(msg, FALSE, LINES-3, FALSE))
		return;
	    }

	}

	if (curr_folder.fp) {
	    fflush(curr_folder.fp);
	    elm_unlock();
	    fclose(curr_folder.fp);
	    curr_folder.fp = NULL;
	}
	if (curr_folder.flags & FOLDER_IS_SPOOL)
	    (void) unlink(curr_folder.tempname);

	leave(LEAVE_NORMAL);
}

int resync(void)
{
	/** Resync on the current folder. Leave current and read it back in.
	    Return indicates whether a redraw of the screen is needed.
	 **/
	int  err;

	  if(leave_mbox(TRUE, FALSE, TRUE) ==-1)
	    /* new mail - leave not done - can't change to another file yet
	     * check for change in curr_folder.size in main() will do the work
	     * of calling newmbox to add in the new messages to the current
	     * file and fix the sorting sequence that leave_mbox may have
	     * changed for its own purposes */
	    return(FALSE);

	  if (can_access(curr_folder.filename, READ_ACCESS) != 0) {
	    err = errno;
	    if (!streq(curr_folder.filename, incoming_folder) || err != ENOENT) {
	      ShutdownTerm();
	      show_error(catgets(elm_msg_cat, ElmSet, ElmCantOpenFolderRead,
			"Can't open '%s' for reading! [%s]"),
			curr_folder.filename, strerror(err));
	      leave(LEAVE_ERROR);
	    }
	  }

	  newmbox(curr_folder.filename, FALSE);
	  return(TRUE);
}

int change_file(char *p1)
{
    int screen_changed;
    char newfile[SLEN], prompt1[SLEN];

    screen_changed = FALSE;
    newfile[0] = '\0';
    sprintf(prompt1, "%s%s", nls_Prompt, p1);

    if (select_folder(newfile, sizeof(newfile), READ_ACCESS, FALSE,
		prompt1, "Select Folder to Open", &screen_changed) < 0)
	return screen_changed;

    if (leave_mbox(FALSE, FALSE, TRUE) == -1) {
	/* new mail - leave not done - can't change to another file yet
	 * check for change in curr_folder.size in main() will do the work
	 * of calling newmbox to add in the new messages to the current
	 * file and fix the sorting sequence that leave_mbox may have
	 * changed for its own purposes */
	return screen_changed;
    }

    newmbox(newfile, FALSE);
    return TRUE;
}
