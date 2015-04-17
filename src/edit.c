

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.5 $   $State: Exp $
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
 * $Log: edit.c,v $
 * Revision 1.5  1999/03/24  14:03:59  wfp5p
 * elm 2.5PL0
 *
 * Revision 1.4  1996/03/14  17:27:57  wfp5p
 * Alpha 9
 *
 * Revision 1.3  1995/09/29  17:42:03  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/09/11  15:19:04  wfp5p
 * Alpha 7
 *
 * Revision 1.1.1.1  1995/04/19  20:38:35  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** This routine is for allowing the user to edit their current folder
    as they wish.

**/

#include "elm_defs.h"
#include "elm_globals.h"
#include "s_elm.h"

long   bytes();

#ifdef ALLOW_MAILBOX_EDITING

static void copy_failed_emergency_exit(char *cur_folder, char *edited_file)
{
	ShutdownTerm();
	error3(catgets(elm_msg_cat, ElmSet, ElmCouldntCopyMailfile,
		"Could not copy from \"%s\" to \"%s\"! [%s]"),
		cur_folder, edited_file, strerror(errno));
	leave(LEAVE_ERROR|LEAVE_KEEP_TEMPFOLDER);
}


void edit_mailbox(void)
{
	/** Allow the user to edit their folder, always resynchronizing
	    afterwards.   Due to intense laziness on the part of the
	    programmer, this routine will invoke $EDITOR on the entire
	    file.  The mailer will ALWAYS resync on the folder
	    even if nothing has changed since, not unreasonably, it's
	    hard to figure out what occurred in the edit session...

	    Also note that if the user wants to edit their incoming
	    mailbox they'll actually be editing the tempfile that is
	    an exact copy.  More on how we resync in that case later
	    in this code.
	**/

	FILE	*real_folder, *temp_folder;
	char	edited_file[SLEN], buffer[SLEN];
	int	len;

	if (curr_folder.flags & FOLDER_IS_SPOOL) {
	  if(save_file_stats(curr_folder.filename) != 0) {
	    error1(catgets(elm_msg_cat, ElmSet, ElmPermFolder,
	      "Problems saving permissions of folder %s!"), curr_folder.filename);
	    Raw(ON);
	    if (sleepmsg > 0)
		sleep(sleepmsg);
	    return(0);
	  }
	}

	strcpy(edited_file, ((curr_folder.flags & FOLDER_IS_SPOOL)
		    ? curr_folder.tempname : curr_folder.filename));
	if (edit_a_file(edited_file) < 0)
	    return (0);

	/* uh oh... now the toughie...  */
	if (curr_folder.flags & FOLDER_IS_SPOOL) {

	  fflush (curr_folder.fp);

	  if (bytes(curr_folder.filename) != curr_folder.size) {

	     /* SIGH.  We've received mail since we invoked the editor
		on the folder.  We'll have to do some strange stuff to
	        remedy the problem... */

	     PutLine0(LINES, 0, catgets(elm_msg_cat, ElmSet, ElmWarnNewMailRecv,
	       "Warning: new mail received..."));
	     CleartoEOLN();

	     if ((temp_folder = fopen(edited_file, "a")) == NULL) {
	       dprint(1, (debugfile,
		    "Attempt to open \"%s\" to append failed in %s\n",
		    edited_file, "edit_mailbox"));
	       set_error(catgets(elm_msg_cat, ElmSet, ElmCouldntReopenTemp,
		 "Couldn't reopen temp file. Edit LOST!"));
	       return(1);
	     }
	     /** Now let's lock the folder up and stream the new stuff
		 into the temp file... **/

	     elm_lock(LOCK_OUTGOING);
	     if ((real_folder = fopen(curr_folder.filename, "r")) == NULL) {
	       dprint(1, (debugfile,
	           "Attempt to open \"%s\" for reading new mail failed in %s\n",
 		   curr_folder.filename, "edit_mailbox"));
	       sprintf(buffer, catgets(elm_msg_cat, ElmSet, ElmCouldntOpenFolder,
		 "Couldn't open %s for reading!  Edit LOST!"), curr_folder.filename);
	       set_error(buffer);

		fflush (curr_folder.fp);
	       elm_unlock();
	       return(1);
	     }
	     if (fseek(real_folder, curr_folder.size, 0) == -1) {
	       dprint(1, (debugfile,
			"Couldn't seek to end of cur_folder (offset %ld) (%s)\n",
			curr_folder.size, "edit_mailbox"));
	       set_error(catgets(elm_msg_cat, ElmSet, ElmCouldntSeekEnd,
		 "Couldn't seek to end of folder.  Edit LOST!"));

		fflush (curr_folder.fp);
	       elm_unlock();
	       return(1);
	     }

	     /** Now we can finally stream the new mail into the tempfile **/

	     while ((len = mail_gets(buffer, SLEN, real_folder)) != 0)
	       if (fwrite(buffer, 1, len, temp_folder) != len) {
	         copy_failed_emergency_exit(curr_folder.filename, edited_file);
	       }

	     fclose(real_folder);
	     if (fclose(temp_folder)) {
	       copy_failed_emergency_exit(curr_folder.filename, edited_file);
	     }

 	   } else elm_lock(LOCK_OUTGOING);

	   if (mailgroupid != groupid)
		   setegid(mailgroupid);

	   /* remove real mail_file and then
	    * link or copy the edited curr_folder.fp to real mail_file */

	   if (copy(edited_file, curr_folder.filename, TRUE) < 0)
	      copy_failed_emergency_exit(curr_folder.filename, edited_file);

	   /* restore file permissions before removing lock */

	   if(restore_file_stats(curr_folder.filename) != 1) {
	     error1(catgets(elm_msg_cat, ElmSet, ElmProblemsRestoringPerms,
	       "Problems restoring permissions of folder %s!"), curr_folder.filename);
	     Raw(ON);
	     if (sleepmsg > 0)
		sleep(sleepmsg);
	   }

	   if (mailgroupid != groupid)
		   setegid(groupid);

	   fflush (curr_folder.fp);
	   elm_unlock();
	   unlink(edited_file);	/* remove the edited curr_folder.fp */
	   error(catgets(elm_msg_cat, ElmSet, ElmChangesIncorporated,
	     "Changes incorporated into new mail..."));

	} else
	  error(catgets(elm_msg_cat, ElmSet, ElmResyncingNewVersion,
	    "Resynchronizing with new version of folder..."));

	if (sleepmsg > 0)
		sleep(sleepmsg);
	ClearScreen();
	newmbox(curr_folder.filename, FALSE);
	showscreen();
	return(1);
}

#endif

int edit_a_file(char *editfile)
{
	/** Edit a file.  This routine is used by edit_mailbox()
	    and edit_aliases_text().  It gets all the editor info
	    from the elmrc file.
	**/

	char     buffer[SLEN];

	PutLine0(LINES-1,0, catgets(elm_msg_cat, ElmSet, ElmInvokeEditor,
	  "Invoking editor..."));

	if (strcmp(editor, "builtin") == 0 || strcmp(editor, "none") == 0) {
	  if (strstr(alternative_editor, "%s") != NULL)
	    sprintf(buffer, alternative_editor, editfile);
	  else
	    sprintf(buffer, "%s %s", alternative_editor, editfile);
	} else {
	  if (strstr(editor, "%s") != NULL)
	    sprintf(buffer, editor, editfile);
	  else
	    sprintf(buffer, "%s %s", editor, editfile);
	}

	if (system_call(buffer, SY_COOKED|SY_ENAB_SIGHUP) == -1) {
	  error1(catgets(elm_msg_cat, ElmSet, ElmProblemsInvokingEditor,
	    "Problems invoking editor %s!"), alternative_editor);
	  if (sleepmsg > 0)
		sleep(sleepmsg);
	  return -1;
	}

	return 0;
}
