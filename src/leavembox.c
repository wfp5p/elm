

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.9 $   $State: Exp $
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
 * $Log: leavembox.c,v $
 * Revision 1.9  1999/03/24  14:04:01  wfp5p
 * elm 2.5PL0
 *
 * Revision 1.8  1996/10/28  16:58:06  wfp5p
 * Beta 1
 *
 * Revision 1.6  1996/05/09  15:51:22  wfp5p
 * Alpha 10
 *
 * Revision 1.5  1996/03/14  17:29:39  wfp5p
 * Alpha 9
 *
 * Revision 1.4  1995/09/29  17:42:15  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.3  1995/07/18  18:59:55  wfp5p
 * Alpha 6
 *
 * Revision 1.2  1995/06/30  14:56:26  wfp5p
 * Alpha 5
 *
 * Revision 1.1.1.1  1995/04/19  20:38:36  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** leave current folder, updating etc. as needed...
  
**/


#include "elm_defs.h"
#include "elm_globals.h"
#include "port_stat.h"
#include "s_elm.h"
#ifdef USE_FLOCK_LOCKING
#define SYSCALL_LOCKING
#endif
#ifdef USE_FCNTL_LOCKING
#define SYSCALL_LOCKING
#endif
#ifdef SYSCALL_LOCKING
#  if (defined(BSD) || !defined(apollo))
#    include <sys/file.h>
#  endif
#endif
#ifdef I_TIME
#  include <time.h>
#endif
#ifdef I_SYSTIME
#  include <sys/time.h>
#endif
#ifdef I_UTIME
#  include <utime.h>
#endif
#ifdef I_SYSUTIME
#  include <sys/utime.h>
#endif


/**********
   Since a number of machines don't seem to bother to define the utimbuf
   structure for some *very* obscure reason.... 

   Suprise, though, BSD has a different utime() entirely...*sigh*
**********/

#ifndef BSD
# ifndef UTIMBUF

struct utimbuf {
	time_t	actime;		/** access time       **/ 
	time_t	modtime;	/** modification time **/
       };

# endif /* UTIMBUF */
#endif /* BSD */


int
leave_mbox(resyncing, quitting, prompt)
int resyncing, quitting, prompt;
{
	/** Close folder, deleting some messages, storing others in mbox,
	    and keeping others, as directed by user input and elmrc options.

	    Return	1	Folder altered
			0	Folder not altered
			-1	New mail arrived during the process and
					closing was aborted.
	    If "resyncing" we are just writing out folder to reopen it. We
		therefore only consider deletes and keeps, not stores to mbox.
		Also we don't remove NEW status so that it can be preserved
		across the resync.

	    If "quitting" and "prompt" is false, then no prompting is done.
		Otherwise prompting is dependent upon the variables
		ask_delete, ask_keep, and ask_store, as set by elmrc
		elmrc options.  This behavior makes the 'q' command prompt
		just like 'c' and '$', while retaining the 'Q' command for a
		quick exit that never prompts.
	**/

	FILE *temp;
	char temp_keep_file[SLEN], buffer[SLEN], *msg;
	struct stat    buf;		/* stat command  */
	struct stat    lbuf;		/* lstat command  */
#if defined(BSD) && !defined(UTIMBUF)
	time_t utime_buffer[2];		/* utime command */
#else
	struct utimbuf utime_buffer;	/* utime command */
#endif
	register int to_delete = 0, to_store = 0, to_keep = 0, i,
		     marked_deleted, marked_read, marked_unread,
		     last_sortby, l_ask_delete, l_ask_keep, l_ask_store,
		     asked_storage_q,
		     num_chgd_status, need_to_copy, is_symlink = FALSE,
		     already_unlinked = FALSE;
	int answer;
	int  err;
	long bytes();

	dprint(1, (debugfile, "\n\n-- leaving folder --\n\n"));

	if (curr_folder.num_mssgs == 0)
        {
   	  if (!resyncing && (curr_folder.flags & FOLDER_IS_SPOOL))
		(void) unlink(curr_folder.tempname);
	  return(0);	/* nothing changed */
	}

	l_ask_delete = ((quitting && !prompt) ? FALSE : ask_delete);
	l_ask_keep = ((quitting && !prompt) ? FALSE : ask_keep);
	l_ask_store = ((quitting && !prompt) ? FALSE : ask_store);

/*	if (l_ask_delete || l_ask_keep || l_ask_store) */

	/* Clear the exit dispositions of all messages, just in case
	 * they were left set by a previous call to this function
	 * that was interrupted by the receipt of new mail.
	 */
	for(i = 0; i < curr_folder.num_mssgs; i++)
	  curr_folder.headers[i]->exit_disposition = UNSET;
	  
	/* Determine if deleted messages are really to be deleted */

	/* we need to know if there are none, or one, or more to delete */
	for (marked_deleted=0, i=0; i<curr_folder.num_mssgs && marked_deleted<2; i++)
	  if (ison(curr_folder.headers[i]->status, DELETED))
	    marked_deleted++;

        if(marked_deleted) {
	  if(l_ask_delete) {
	    if (marked_deleted == 1)
	      msg = catgets(elm_msg_cat, ElmSet, ElmLeaveDeleteMessage,
		"Delete message?");
	    else
	      msg = catgets(elm_msg_cat, ElmSet, ElmLeaveDeleteMessages,
		"Delete messages?");
	    answer = enter_yn(msg, always_delete, LINES-3, FALSE);
	  } else {
	    answer = always_delete;
	  }

	  if(answer) {
	    for (i = 0; i < curr_folder.num_mssgs; i++) {
	      if (ison(curr_folder.headers[i]->status, DELETED)) {
		curr_folder.headers[i]->exit_disposition = DELETE;
		to_delete++;
	      }
	    }
	  }
	}
	dprint(3, (debugfile, "Messages to delete: %d\n", to_delete));

	/* If this is a non spool file, or if we are merely resyncing,
	 * all messages with an unset disposition (i.e. not slated for
	 * deletion) are to be kept.
	 * Otherwise, we need to determine if read and unread messages
	 * are to be stored or kept.
	 */
	if (!(curr_folder.flags & FOLDER_IS_SPOOL) || resyncing) {
	  to_store = 0;
	  for (i = 0; i < curr_folder.num_mssgs; i++) {
	    if(curr_folder.headers[i]->exit_disposition == UNSET) {
	      curr_folder.headers[i]->exit_disposition = KEEP;
	      to_keep++;
	    }
	  }
	} else {

	  /* Let's first see if user wants to store read messages 
	   * that aren't slated for deletion */

	  asked_storage_q = FALSE;

	  /* we need to know if there are none, or one, or more marked read */
	  for (marked_read=0, i=0; i < curr_folder.num_mssgs && marked_read < 2; i++) {
	    if((isoff(curr_folder.headers[i]->status, UNREAD))
	      && (curr_folder.headers[i]->exit_disposition == UNSET))
		marked_read++;
	  }
	  if(marked_read) {
	    if(l_ask_store) {
	      if (marked_read == 1)
		msg = catgets(elm_msg_cat, ElmSet, ElmLeaveMoveMessage,
			"Move read message to \"received\" folder?");
	      else
		msg = catgets(elm_msg_cat, ElmSet, ElmLeaveMoveMessages,
			"Move read messages to \"received\" folder?");
	      answer = enter_yn(msg, always_store, LINES-3, FALSE);
	      asked_storage_q = TRUE;
	    } else {
	      answer = always_store;
	    }

	    for (i = 0; i < curr_folder.num_mssgs; i++) {
	      if((isoff(curr_folder.headers[i]->status, UNREAD)) 
		&& (curr_folder.headers[i]->exit_disposition == UNSET)) {

		  if(answer) {
		    curr_folder.headers[i]->exit_disposition = STORE;
		    to_store++;
		  } else {
		    curr_folder.headers[i]->exit_disposition = KEEP;
		    to_keep++;
		  }
	      }
	    }
	  }

	  /* If we asked the user if read messages should be stored,
	   * and if the user wanted them kept instead, then certainly the
	   * user would want the unread messages kept as well.
	   */
	  if(asked_storage_q && !answer) {

	    for (i = 0; i < curr_folder.num_mssgs; i++) {
	      if((ison(curr_folder.headers[i]->status, UNREAD))
		&& (curr_folder.headers[i]->exit_disposition == UNSET)) {
		  curr_folder.headers[i]->exit_disposition = KEEP;
		  to_keep++;
	      }
	    }

	  } else {

	    /* Determine if unread messages are to be kept */

	    /* we need to know if there are none, or one, or more unread */
	    for (marked_unread=0, i=0; i<curr_folder.num_mssgs && marked_unread<2; i++)
	      if((ison(curr_folder.headers[i]->status, UNREAD))
		&& (curr_folder.headers[i]->exit_disposition == UNSET))
		  marked_unread++;

	    if(marked_unread) {
	      if(l_ask_keep) {
		if (marked_unread == 1)
		  msg = catgets(elm_msg_cat, ElmSet, ElmLeaveKeepMessage,
		    "Keep unread message in incoming mailbox?");
		else
		  msg = catgets(elm_msg_cat, ElmSet, ElmLeaveKeepMessages,
		    "Keep unread messages in incoming mailbox?");
		answer = enter_yn(msg, always_keep, LINES-3, FALSE);
	      } else {
		answer = always_keep;
	      }

	      for (i = 0; i < curr_folder.num_mssgs; i++) {
		if((ison(curr_folder.headers[i]->status, UNREAD))
		  && (curr_folder.headers[i]->exit_disposition == UNSET)) {

		    if(!answer) {
		      curr_folder.headers[i]->exit_disposition = STORE;
		      to_store++;
		    } else {
		      curr_folder.headers[i]->exit_disposition = KEEP;
		      to_keep++;
		    }
	      
		}
	      }
	    }
	  }
	}

	dprint(3, (debugfile, "Messages to store: %d\n", to_store));
	dprint(3, (debugfile, "Messages to keep: %d\n", to_keep));

	if(to_delete + to_store + to_keep != curr_folder.num_mssgs) {
	  ShutdownTerm();
	  dprint(1, (debugfile,
	  "Error: %d to delete + %d to store + %d to keep != %d message cnt\n",
	    to_delete, to_store, to_keep, curr_folder.num_mssgs));
	  error(catgets(elm_msg_cat, ElmSet,
	        ElmSomethingWrongInCounts,
		"Something wrong in message counts! Folder unchanged.\n"));
	  leave(LEAVE_EMERGENCY);
	}
	  

	/* If we are not resyncing, we are leaving the mailfile and
	 * the new messages are new no longer. Note that this changes
	 * their status.
	 */
	if(!resyncing) {
	  for (i = 0; i < curr_folder.num_mssgs; i++) {
	    if (ison(curr_folder.headers[i]->status, NEW)) {
	      clearit(curr_folder.headers[i]->status, NEW);
	      curr_folder.headers[i]->status_chgd = TRUE;
	    }
	  }
	}

	/* If all messages are to be kept and none have changed status
	 * we don't need to do anything because the current folder won't
	 * be changed by our writing it out - unless we are resyncing, in
	 * which case we force the writing out of the mailfile.
	 */

	for (num_chgd_status = 0, i = 0; i < curr_folder.num_mssgs; i++)
	  if(curr_folder.headers[i]->status_chgd == TRUE)
	    num_chgd_status++;
	
	if(!to_delete && !to_store && !num_chgd_status && !resyncing) {
	  dprint(3, (debugfile, "Folder keep as is!\n"));
	  error(catgets(elm_msg_cat, ElmSet, ElmFolderUnchanged,
		"Folder unchanged."));
	  if (!resyncing && (curr_folder.flags & FOLDER_IS_SPOOL))
		(void) unlink(curr_folder.tempname);
	  return(0);
	}

	/** we have to check to see what the sorting order was...so that
	    the order in which we write messages is the same as the order
	    of the messages originally.
	    We only need to do this if there are any messages to be
	    written out (either to keep or to store). **/

	if ((to_keep || to_store ) && sortby != MAILBOX_ORDER) {
	  last_sortby = sortby;
	  sortby = MAILBOX_ORDER;
	  sort_mailbox(curr_folder.num_mssgs, FALSE);
	  sortby = last_sortby;
	}

	/* Formulate message as to number of keeps, stores, and deletes.
	 * This is only complex so that the message is good English.
	 */
	if (to_keep > 0) {
	  if (to_store > 0) {
	    if (to_delete > 0) {
	      if (to_keep == 1)
	        MCsprintf(buffer, catgets(elm_msg_cat, ElmSet, ElmLeaveKeepStoreDelete,
		      "[Keeping 1 message, storing %d, and deleting %d.]"), 
		    to_store, to_delete);
	      else
	        MCsprintf(buffer, catgets(elm_msg_cat, ElmSet, ElmLeaveKeepStoreDeletePlural,
		      "[Keeping %d messages, storing %d, and deleting %d.]"), 
		    to_keep, to_store, to_delete);
	    } else {
	      if (to_keep == 1)
		sprintf(buffer, catgets(elm_msg_cat, ElmSet, ElmLeaveKeepStore,
			"[Keeping 1 message and storing %d.]"), 
		      to_store);
	      else
		MCsprintf(buffer, catgets(elm_msg_cat, ElmSet, ElmLeaveKeepStorePlural,
			"[Keeping %d messages and storing %d.]"), 
		      to_keep, to_store);
	    }
	  } else {
	    if (to_delete > 0) {
	      if (to_keep == 1)
		sprintf(buffer, catgets(elm_msg_cat, ElmSet, ElmLeaveKeepDelete,
			"[Keeping 1 message and deleting %d.]"), 
		      to_delete);
	      else
		MCsprintf(buffer, catgets(elm_msg_cat, ElmSet, ElmLeaveKeepDeletePlural,
			"[Keeping %d messages and deleting %d.]"), 
		      to_keep, to_delete);
	    } else {
	      if (to_keep == 1)
		strcpy(buffer, catgets(elm_msg_cat, ElmSet, ElmLeaveKeep,
			"[Keeping message.]"));
	      else
		strcpy(buffer, catgets(elm_msg_cat, ElmSet, ElmLeaveKeepPlural,
			"[Keeping all messages.]"));
	    }
	  }
	} else if (to_store > 0) {
	  if (to_delete > 0) {
	    if (to_store == 1)
	      sprintf(buffer, catgets(elm_msg_cat, ElmSet, ElmLeaveStoreDelete,
		      "[Storing 1 message and deleting %d.]"), 
		    to_delete);
	    else
	      MCsprintf(buffer, catgets(elm_msg_cat, ElmSet, ElmLeaveStoreDeletePlural,
		      "[Storing %d messages and deleting %d.]"), 
		    to_store, to_delete);
	  } else {
	    if (to_store == 1)
	      strcpy(buffer, catgets(elm_msg_cat, ElmSet, ElmLeaveStore,
		      "[Storing message.]"));
	    else
	      strcpy(buffer, catgets(elm_msg_cat, ElmSet, ElmLeaveStorePlural,
		      "[Storing all messages.]"));
	  }
	} else {
	  if (to_delete > 0)
	    strcpy(buffer, catgets(elm_msg_cat, ElmSet, ElmLeaveDelete,
		"[Deleting all messages.]"));
	  else
	    buffer[0] = '\0';
	}
	/* NOTE: don't use variable "buffer" till message is output later */

	/** next, let's lock the file up and make one last size check **/

	if (curr_folder.flags & FOLDER_IS_SPOOL)
	  elm_lock(LOCK_OUTGOING);
	
	fflush (curr_folder.fp);

	if (curr_folder.size != bytes(curr_folder.filename)) {
	    elm_unlock();
	    error(catgets(elm_msg_cat, ElmSet, ElmLeaveNewMailArrived,
			  "New mail has just arrived. Resynchronizing..."));
	    return(-1);
	}
	
	/* Everything's GO - so ouput that user message and go to it. */

	block_signals();
	
	dprint(2, (debugfile, "Action: %s\n", buffer));
	error(buffer);

	/* Store messages slated for storage in received mail folder */
	if (to_store > 0) {
	  if ((err = can_open(recvd_mail, "a"))) {
	    error1(catgets(elm_msg_cat, ElmSet, ElmLeaveAppendDenied,
	      "Permission to append to %s denied!  Leaving folder intact."),
	      recvd_mail);
	    dprint(1, (debugfile,
	      "Error: Permission to append to folder %s denied!! (%s)\n",
	      recvd_mail, "leavembox"));
	    dprint(1, (debugfile, "** %s **\n", strerror(err)));

	    fflush (curr_folder.fp);
	    elm_unlock();

	    unblock_signals();
	    return(0);
	  }
	  if ((temp = fopen(recvd_mail,"a")) == NULL) {
	    dprint(1, (debugfile, "Error: could not append to file %s [%s]\n", 
	      recvd_mail, strerror(errno)));
	    ShutdownTerm();
	    error1(stderr, catgets(elm_msg_cat, ElmSet,
		ElmLeaveCouldNotAppend,
		"Could not append to folder %s!"), recvd_mail);
	    leave(LEAVE_ERROR|LEAVE_KEEP_TEMPFOLDER);
	  }
	  dprint(2, (debugfile, "Storing messages "));
	  for (i = 0; i < curr_folder.num_mssgs; i++) {
	    if(curr_folder.headers[i]->exit_disposition == STORE) {
	      dprint(2, (debugfile, "#%d, ", i+1));
	      copy_message(temp, i+1, CM_UPDATE_STATUS);
	    }
	  }
	  fclose(temp);
	  dprint(2, (debugfile, "\n\n"));
	  chown(recvd_mail, userid, groupid);
	}

	/* Check symbolic link status now (ahead of normal stat() call)
	 * so we know already whether this folder is a symlink.  If it
	 * is, we'll have to keep it around even if it's empty.
	 */

        if (lstat(curr_folder.filename, &lbuf) != 0) {
	  err = errno;
	  dprint(1, (debugfile, "Error: errno %s attempting to stat file %s\n", 
		     strerror(err), curr_folder.filename));
          error2(catgets(elm_msg_cat, ElmSet, ElmLeaveErrorOnStat,
		"Error %s on stat(%s)."), strerror(err), curr_folder.filename);
	}

	is_symlink = S_ISLNK(lbuf.st_mode);


	/* If there are any messages to keep, first copy them to a
	 * temp file, then remove original and copy whole temp file over.
	 */
	if (to_keep > 0) {
	  sprintf(temp_keep_file, "%s%s%d", temp_dir, temp_file, getpid());
	  if ((err = can_open(temp_keep_file, "w"))) {
	    error1(catgets(elm_msg_cat, ElmSet, ElmLeaveTempFileDenied,
"Permission to create temp file %s for writing denied! Leaving folder intact."),
	      temp_keep_file);
	    dprint(1, (debugfile,
	      "Error: Permission to create temp file %s denied!! (%s)\n",
	      temp_keep_file, "leavembox"));
	    dprint(1, (debugfile, "** %s **\n", strerror(err)));

	    fflush (curr_folder.fp);
	    elm_unlock();

	    unblock_signals();
	    return(0);
	  }
	  if ((temp = file_open(temp_keep_file,"w")) == NULL) {
	    dprint(1, (debugfile, "Error: could not create file %s [%s]\n", 
	      temp_keep_file, strerror(errno)));
	    ShutdownTerm();
	    error1(catgets(elm_msg_cat, ElmSet, ElmLeaveCouldNotCreate,
		"Could not create temp file %s!\n"), temp_keep_file);
	    leave(LEAVE_ERROR|LEAVE_KEEP_TEMPFOLDER);
	  }
	  dprint(2, (debugfile, "Copying to temp file messages to be kept "));
	  for (i = 0; i < curr_folder.num_mssgs; i++) {
	    if(curr_folder.headers[i]->exit_disposition == KEEP) {
	      dprint(2, (debugfile, "#%d, ", i+1));
	      copy_message(temp, i+1, CM_UPDATE_STATUS);
	    }
	  }
	  if ( fclose(temp) == EOF ) {
	    ShutdownTerm();
	    error1(catgets(elm_msg_cat, ElmSet, ElmLeaveCloseFailedTemp,
		"Close failed on temp file in leavembox()! [%s]"),
		strerror(errno));
	    leave(LEAVE_ERROR);
	  }
	  dprint(2, (debugfile, "\n\n"));

	} else if (!(curr_folder.flags & FOLDER_IS_SPOOL) && !keep_empty_files
		&& !is_symlink && !resyncing) {

	  /* i.e. if no messages were to be kept and this is not a spool
	   * folder and we aren't keeping empty non-spool folders and this
	   * is not a symlink, simply remove the old original folder and
	   * that's it!
	   */
	  (void)unlink(curr_folder.filename);
	  unblock_signals();
	  return(1);
	}

	/* Otherwise we have some work left to do! */

	/* Get original permissions and access time of the original
	 * mail folder before we remove it.
	 */
	if(save_file_stats(curr_folder.filename) != 0) {
	  error1(catgets(elm_msg_cat, ElmSet, ElmLeaveProblemsSavingPerms,
		"Problems saving permissions of folder %s!"), curr_folder.filename);
	  if (sleepmsg > 0)
		sleep(sleepmsg);
	}
	  
        if (stat(curr_folder.filename, &buf) != 0) {
	  err = errno;
	  dprint(1, (debugfile, "Error: errno %s attempting to stat file %s\n", 
		     strerror(err), curr_folder.filename));
          error2(catgets(elm_msg_cat, ElmSet, ElmLeaveErrorOnStat,
		"Error %s on stat(%s)."), strerror(err), curr_folder.filename);
	}

	/* Close and remove the original folder.
	 * However, if we are going to copy a temp file of kept messages
	 * to it, and this is a locked (spool) mailbox, we need to keep
	 * it locked during this process. Unfortunately,
	 * if we did our USE_FLOCK_LOCKING or USE_FCNTL_LOCKING unlinking
	 * the original will kill the lock, so we have to resort to copying
	 * the temp file to the original file while keeping the original open.
	 * Also, if the file has a link count > 1, then it has links, so to
	 * prevent destroying the links, we do a copy back, even though its
	 * slower.  If the file is a symlink, then we also need to do a copy
	 * back to prevent destroying the linkage.
	 */

	/*
	 * fclose(curr_folder.fp);
	 *
	 * While this fclose is OK for BSD flock file locking, it is
	 * incorrect for SYSV fcntl file locking.  For some reason AT&T
	 * decided to release all file locks when *any* fd to a file
	 * is closed, even if it is not the fd that acquired the lock.
	 * Thus this fclose would release the mailbox file locks.
	 * Instead I am going to just fflush the file here, and do the
	 * individual closes in the subcases to ensure that the
	 * mailbox is locked until we are finished with it.
	 */
	fflush(curr_folder.fp);

#ifdef SYSCALL_LOCKING
	need_to_copy = (curr_folder.flags & FOLDER_IS_SPOOL);
#else
	need_to_copy = FALSE;
#endif /* SYSCALL_LOCKING */
	if (buf.st_nlink > 1)
		need_to_copy = TRUE;

	if (buf.st_mode & 0x7000) { /* copy if special modes set */
		need_to_copy = TRUE;    /* such as enforcement lock */
	}

	/* If we don't have symlinks is_symlink will still be FALSE from
	 * initialization. */
	if (is_symlink)
	  need_to_copy = TRUE;

#ifdef _PC_CHOWN_RESTRICTED
	if (!need_to_copy) {
/*
 * Chown may or may not be restricted to root in SVR4, if it is,
 *	then need to copy must be true, and no restore of permissions
 *	should be performed.
 */
		if (pathconf(curr_folder.filename, _PC_CHOWN_RESTRICTED)) {
			need_to_copy = TRUE;
		}
	}
#endif  /* _PC_CHOWN_RESTRICTED */


	if(to_keep) {

          if (mailgroupid != groupid && (curr_folder.flags & FOLDER_IS_SPOOL))
      	    SETGID(mailgroupid);

	  if(!need_to_copy) {
	    /* If the file is on the same file system, it is simplest
	     * to do a rename and avoid the copy.  We prefer rename to
	     * link, since this avoids the need to unlink or rename
	     * the old file and since this works in AFS filespace
	     * as well as in UNIX.  This action is relatively safe
	     * since the allocation for the new file already exists.
	     */

	    if (rename(temp_keep_file, curr_folder.filename) != 0)
	    {

	      if(errno == EXDEV)
	      {

		/* cannot rename across file systems - use copy instead */
		need_to_copy = already_unlinked = TRUE;
	      } else {
		err = errno;
		ShutdownTerm();
		dprint(1, (debugfile,
			"rename(%s, %s) failed (leavembox) [%s]\n", 
		       temp_keep_file, curr_folder.filename, strerror(err)));
		error1(catgets(elm_msg_cat, ElmSet, ElmLeaveRenameFailed,
			"Rename failed! [%s]"), strerror(err));
		if (mailgroupid != groupid && (curr_folder.flags & FOLDER_IS_SPOOL))
		  SETGID(groupid);
		leave(LEAVE_ERROR|LEAVE_KEEP_TEMPFOLDER);
	      }
	    }
	  }

	  if(need_to_copy) {

	    if (copy(temp_keep_file, curr_folder.filename, TRUE) < 0) {

		/*
		 * WARNING - on systems that drop locks when any
		 * file descriptor to a file is closed, we've lost
		 * the lock to "curr_folder.filename".  Since we are about
		 * to bail out of that file, it should not be a problem.
		 */

	      /* copy to curr_folder.filename failed - try to copy to special file */
	      sprintf(curr_folder.filename,"%s/%s", user_home, unedited_mail);
	      if (copy(temp_keep_file, curr_folder.filename, FALSE) < 0) {

		/* couldn't copy to special file either */
		err = errno;
		ShutdownTerm();
		dprint(1, (debugfile, 
			"leavembox: couldn't copy to %s either!!  Help! [%s]", 
			curr_folder.filename, strerror(err)));
		error1(catgets(elm_msg_cat, ElmSet,
			ElmLeaveCantCopyMailbox,
			"Can't copy folder!  Contents preserved in %s."),
			temp_keep_file);
		if (mailgroupid != groupid && (curr_folder.flags & FOLDER_IS_SPOOL))
		  SETGID(groupid);
		leave(LEAVE_ERROR|LEAVE_KEEP_TEMPFOLDER);
	      } else {
		dprint(1, (debugfile,
			"\nWoah! Confused - Saved mail in %s (leavembox)\n", 
			curr_folder.filename));
		error1(catgets(elm_msg_cat, ElmSet, ElmLeaveSavedMailIn,
			"Saved mail in %s."), curr_folder.filename);
		if (sleepmsg > 0)
		    sleep((sleepmsg + 1) / 2);
	      }
	    }
	  }

	  /* link or copy complete - remove temp keep file */
	  unlink(temp_keep_file);

	} else if ((curr_folder.flags & FOLDER_IS_SPOOL)
		    || keep_empty_files || is_symlink || resyncing) {

	  /* If this is an empty spool file, or if this is an empty non-spool 
	   * file and we keep empty non-spool files (we always keep empty
	   * spool files), or if this is an empty non-spool file and it's a
	   * symbolic link, or if we're resynching and not keeping any
	   * messages, create an empty file. */

	  if (!(curr_folder.flags & FOLDER_IS_SPOOL))
	    error1(catgets(elm_msg_cat, ElmSet, ElmLeaveKeepingEmpty,
		"Keeping empty folder '%s'."), curr_folder.filename);
	  temp = fopen(curr_folder.filename, "w");
	  fclose(temp);
	}

	/*
	 * File permissions haven't changed if need_to_copy is true and we
	 * were forced to overwrite the original file.  Restore permissions
	 * of original folder only if we were able to link our temp mailbox
	 * directly to the folder, causing it to have temp mbox permissions.
	 * Except that we find we need to copy across filesystems after we've
	 * already_unlinked the original file.
	 */

	if (!need_to_copy || already_unlinked) {
	  if(restore_file_stats(curr_folder.filename) == -1) {
	    error1(catgets(elm_msg_cat, ElmSet, ElmLeaveProblemsRestoringPerms,
		  "Problems restoring permissions of folder %s!"),
		  curr_folder.filename);
	    if (sleepmsg > 0)
		sleep(sleepmsg);
	  }
	}

#if defined(BSD) && !defined(UTIMBUF)
	utime_buffer[0]     = buf.st_atime;
	utime_buffer[1]     = buf.st_mtime;
#else
	utime_buffer.actime = buf.st_atime;
	utime_buffer.modtime= buf.st_mtime;
#endif

#if defined(BSD) && !defined(UTIMBUF)
	if (utime(curr_folder.filename, utime_buffer) != 0) 
#else
	if (utime(curr_folder.filename, &utime_buffer) != 0) 
#endif
	{
	  err = errno;
	  dprint(1, (debugfile, 
		 "Error: encountered error doing utime (leavmbox)\n"));
	  dprint(1, (debugfile, "** %s **\n", strerror(err)));
	  error2(catgets(elm_msg_cat, ElmSet, ElmLeaveChangingAccessTime,
		"Error %s trying to change file %s access time."), 
		   strerror(err), curr_folder.filename);
	}
	if (mailgroupid != groupid && (curr_folder.flags & FOLDER_IS_SPOOL))
	    SETGID(groupid);

	fflush (curr_folder.fp);
	curr_folder.size = bytes(curr_folder.filename);
	elm_unlock();	/* remove the lock on the file ASAP! */
	fclose(curr_folder.fp);
	curr_folder.fp = NULL;
	if (!resyncing && (curr_folder.flags & FOLDER_IS_SPOOL))
	    (void) unlink(curr_folder.tempname);
	unblock_signals();
	return(1);	
}

#ifdef HASSIGPROCMASK
	sigset_t	toblock, oldset;
#else  /* HASSIGPROCMASK */
#  ifdef HASSIGBLOCK
	int		toblock, oldset;
#  else /* HASSIGBLOCK */
#    ifdef HASSIGHOLD
	/* Nothing required */
#    else /* HASSIGHOLD */
	SIGHAND_TYPE	(*oldhup)();
	SIGHAND_TYPE	(*oldint)();
	SIGHAND_TYPE	(*oldquit)();
	SIGHAND_TYPE	(*oldstop)();
#    endif /* HASSIGHOLD */
#  endif /* HASSIGBLOCK */
#endif /* HASSIGPROCMASK */

/*
 * Block all keyboard generated signals.  Need to do this while
 * rewriting mailboxes to avoid inadvertant corruption.  In
 * particular, a SIGHUP (from logging out under /bin/sh), can
 * corrupt a spool mailbox during an elm autosync.
 */

block_signals()
{
	dprint(1,(debugfile, "block_signals\n"));
#ifdef HASSIGPROCMASK
	sigemptyset(&oldset);
	sigemptyset(&toblock);
	sigaddset(&toblock, SIGHUP);
	sigaddset(&toblock, SIGINT);
	sigaddset(&toblock, SIGQUIT);
#ifdef SIGTSTP
	sigaddset(&toblock, SIGTSTP);
#endif /* SIGTSTP */

	sigprocmask(SIG_BLOCK, &toblock, &oldset);

#else /* HASSIGPROCMASK */
#  ifdef HASSIGBLOCK
	toblock = sigmask(SIGHUP) | sigmask(SIGINT) | sigmask(SIGQUIT);
#ifdef SIGTSTP
	toblock |= sigmask(SIGTSTP);
#endif /* SIGTSTP */

	oldset = sigblock(toblock);

#  else /* HASSIGBLOCK */
#    ifdef HASSIGHOLD
	sighold(SIGHUP);
	sighold(SIGINT);
	sighold(SIGQUIT);
#ifdef SIGTSTP
	sighold(SIGTSTP);
#endif /* SIGTSTP */

#    else /* HASSIGHOLD */
	oldhup  = signal(SIGHUP, SIG_IGN);
	oldint  = signal(SIGINT, SIG_IGN);
	oldquit = signal(SIGQUIT, SIG_IGN);
#ifdef SIGTSTP
	oldstop = signal(SIGTSTP, SIG_IGN);
#endif /* SIGTSTP */
#    endif /* HASSIGHOLD */
#  endif /* HASSIGBLOCK */
#endif /* HASSIGPROCMASK */
}

/*
 * Inverse of the previous function.  Restore keyboard generated
 * signals.
 */
unblock_signals()
{
	dprint(1,(debugfile, "unblock_signals\n"));
#ifdef HASSIGPROCMASK
	sigprocmask(SIG_SETMASK, &oldset, (sigset_t *)0);

#else  /* HASSIGPROCMASK */
#  ifdef HASSIGBLOCK
	sigsetmask(oldset);

#  else /* HASSIGBLOCK */
#    ifdef HASSIGHOLD
	sigrelse(SIGHUP);
	sigrelse(SIGINT);
	sigrelse(SIGQUIT);
#ifdef SIGTSTP
	sigrelse(SIGTSTP);
#endif /* SIGTSTP */

#    else /* HASSIGHOLD */
	signal(SIGHUP, oldhup);
	signal(SIGINT, oldint);
	signal(SIGQUIT, oldquit);
#ifdef SIGTSTP
	signal(SIGTSTP, oldstop);
#endif /* SIGTSTP */

#    endif /* HASSIGHOLD */
#  endif /* HASSIGBLOCK */
#endif /* HASSIGPROCMASK */
}

