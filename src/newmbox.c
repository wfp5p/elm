

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.14 $   $State: Exp $
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
 * $Log: newmbox.c,v $
 * Revision 1.14  1999/03/24  14:04:03  wfp5p
 * elm 2.5PL0
 *
 * Revision 1.13  1998/02/11  22:02:15  wfp5p
 * Beta 2
 *
 * Revision 1.11  1996/08/08  19:49:27  wfp5p
 * Alpha 11
 *
 * Revision 1.10  1996/03/14  17:29:42  wfp5p
 * Alpha 9
 *
 * Revision 1.9  1995/09/29  17:42:19  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.8  1995/09/11  15:19:20  wfp5p
 * Alpha 7
 *
 * Revision 1.7  1995/07/18  18:59:58  wfp5p
 * Alpha 6
 *
 * Revision 1.6  1995/06/22  14:48:48  wfp5p
 * Performance enhancements from Paul Close
 *
 * Revision 1.5  1995/06/12  20:33:34  wfp5p
 * Alpha 2 clean up
 *
 * Revision 1.4  1995/06/08  13:41:05  wfp5p
 * A few mostly cosmetic changes
 *
 * Revision 1.3  1995/05/10  13:34:51  wfp5p
 * Added mailing list stuff by Paul Close <pdc@sgi.com>
 *
 * Revision 1.2  1995/04/20  21:01:49  wfp5p
 * Added the showreply feature and emacs key bindings.
 *
 * Revision 1.1.1.1  1995/04/19  20:38:37  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/**  read new folder **/

#include "elm_defs.h"
#include "elm_globals.h"
#include "mime.h"
#include "mailfile.h"
#include "s_elm.h"
#include <assert.h>

#ifdef I_TIME
#  include <time.h>
#endif
#ifdef I_SYSTIME
#  include <sys/time.h>
#endif

static int folder_is_spool P_((const char *));
static void mk_temp_mail_fn P_((char *, const char *));
static void mailFile_write_error P_((void));
static int read_headers P_((int));


long bytes();
#if  !defined(ANSI_C) && !defined(atol) /* avoid problems with systems that declare atol as a macro */
extern void rewind();
extern long atol();
#endif

int newmbox(const char *new_filename, int adds_only)
{
	/** Read a folder.

	    new_filename - name of folder  to read. It is up to the calling
			  function to make sure that the file can be
			  read by the user. This is not checked in this
			  function. The reason why it is not checked here
			  is due to the situation where the user wants to
			  change folders: the new folder must be checked
			  for access *before* leaving the old one, which
			  is before this function gets called.
	    adds_only	- set if we only want to read newly added messages to
				same old folder.

	**/

	int  same_file;
	char new_tempname[SLEN];

	/* determine whether we are changing files */
	same_file = streq(new_filename, curr_folder.filename);

	if (folder_is_spool(new_filename)) {

	  mk_temp_mail_fn(new_tempname, new_filename);

	  /* If we are changing files and we are changing to a spool file,
	   * make sure there isn't a temp file for it, because if
	   * there is, someone else is using ELM to read the new file,
	   * and we don't want to be reading it at the same time.
	   */
	  if (!same_file) {
	    if (elm_access(new_tempname, ACCESS_EXISTS) == 0) {
	      ShutdownTerm();
	      show_error(catgets(elm_msg_cat, ElmSet, ElmAlreadyRunning1,
		  "You seem to have ELM already reading this mail!"));
	      show_error(catgets(elm_msg_cat, ElmSet, ElmAlreadyRunning2,
		  "You may not have two copies of ELM running simultaneously."));
	      show_error(catgets(elm_msg_cat, ElmSet, ElmAlreadyRunning3,
		  "If this is in error, then you'll need to remove the following file:"));
	      show_error("%\t%s", new_tempname);
	      leave(LEAVE_ERROR|LEAVE_KEEP_LOCK|LEAVE_KEEP_TEMPFOLDER);
	    }
	  }

	  /* If we were reading a spool file and we are not just reading
	   * in the additional new messages to the same file, we need to
	   * remove the corresponding tempfile.
	   */
	  if (!adds_only) {
	    if (curr_folder.tempname[0] && elm_access(curr_folder.tempname, ACCESS_EXISTS) == 0) {
	      if (unlink(curr_folder.tempname) != 0) {
		ShutdownTerm();
		show_error(catgets(elm_msg_cat, ElmSet, ElmSorryCantUnlinkTemp,
		  "Sorry, can't unlink the temp file %s [%s]!"),
		  curr_folder.tempname, strerror(errno));
		leave(LEAVE_ERROR|LEAVE_KEEP_TEMPFOLDER);
	      }
	    }
	  }

	  strcpy(curr_folder.filename, new_filename);
	  strcpy(curr_folder.tempname, new_tempname);
	  curr_folder.flags = FOLDER_IS_SPOOL;

	} else {

	  strcpy(curr_folder.filename, new_filename);
	  curr_folder.tempname[0] = '\0';
	  curr_folder.flags = 0;

	}

	clear_error();
	clear_central_message();

	if (curr_folder.fp != NULL)
	  (void) fclose(curr_folder.fp);

	if ((curr_folder.fp = fopen(curr_folder.filename,"r")) == NULL)  {
	  if (errno != ENOENT) {
	    ShutdownTerm();
	    show_error(catgets(elm_msg_cat, ElmSet, ElmFailOnOpenNewmbox,
		    "Cannot open \"%s\"! [%s]"),
		    curr_folder.filename, strerror(errno));
	    leave(LEAVE_ERROR);
	  }
	  else {
	    curr_folder.size = 0;         /* must non-existant folder */
	    curr_folder.num_mssgs = 0;
	    selected = 0;
	  }
	} else {                          /* folder exists, read headers */
	  read_headers(adds_only);
	}

	if(!same_file)		/* limit mode off if this is a new file */
	  selected = 0;
	if (!adds_only)		/* limit mode off if recreating headers */
	  selected = 0;		/* because we loose the 'Visible' flag */

	dprint(1, (debugfile,
	  "New folder %s type %s temp file %s (%s)\n", curr_folder.filename,
	  ((curr_folder.flags & FOLDER_IS_SPOOL) ? "spool" : "non-spool"),
	  (curr_folder.tempname[0] != '\0' ? curr_folder.tempname : "none"), "newmbox"));

	return(0);
}

static int folder_is_spool(const char *filename)
{
	int i;

	assert(filename != NULL && filename[0] != '\0');


	/* if file name == default mailbox, its a spool file also
	 * even if its not in the spool directory. (SVR4)
	 */
	if (streq(filename, incoming_folder))
	    return TRUE;

	/* if filename begins with mailhome,
	 * and there is a slash in filename,
	 * and there is a filename after it (i.e. last slash is not last char),
	 * and the last character of mailhome is last slash in filename,
	 * it's a spool file .
	 */
	i = strlen(mailhome);
	if (strncmp(filename, mailhome, i) == 0
	      && filename[i] == '/'
	      && strchr(filename+i+1, '/') == NULL)
	  return TRUE;

         if ( (TreatAsSpooled) || (matchInList(magiclist,magiccount,filename,FALSE)) )
         {
            return TRUE;
   	 }
	return FALSE;
}

static void mk_temp_mail_fn(char *tempfn, const char *mbox)
{
	/** create in tempfn the name of the temp file corresponding to
	    mailfile mbox. Mbox is presumed to be a file in mailhome;
	    Strangeness may result if it is not!
	 **/

	char *cp;

	sprintf(tempfn, "%s%s", default_temp, temp_mbox);
        cp = basename(mbox);
/*	if (strcmp(cp, "mbox") == 0 || strcmp(cp, "mailbox") == 0 ||
		strcmp(cp, "inbox") == 0 || *cp == '.')
	    strcat(tempfn, user_name);
	  else
 */
       strcat(tempfn, cp);

        strcat(tempfn, ".");
        strcat(tempfn, user_name);


}

static void mailFile_write_error(void)
{
	ShutdownTerm();
	show_error(catgets(elm_msg_cat, ElmSet, ElmWriteToTempFailed,
			"Write to \"%s\" failed! [%s]"),
			curr_folder.tempname, strerror(errno));
	leave(LEAVE_ERROR);
}

static int read_headers(int add_new_only)
{
	/** Reads the headers into the curr_folder.headers[] array and leaves the
	    file rewound for further I/O requests.   If the file being
	    read is a mail spool file (ie incoming) then it is copied to
	    a temp file and closed, to allow more mail to arrive during
	    the elm session.  If 'add_new_only' is set, the program will copy
	    the status flags from the previous data structure to the new
	    one if possible and only read in newly added messages.
	**/

	FILE *temp;
	struct header_rec *current_header = NULL;
	char *buffer, *c;
	char tbuffer[VERY_LONG_STRING];
	long fbytes = 0L, line_bytes = 0L, last_line_bytes = 1L, content_start = 0L,
	  content_remaining = -1L, lines_start = 0L;
	register long line = 0;
	register int count = 0, another_count,
	  subj = 0, copyit = 0, in_header = FALSE;
	int count_x, count_y = 17;
	int in_to_list = FALSE, in_cc_list = FALSE, forwarding_mail = FALSE,
	  first_line = TRUE, content_length_found = FALSE, was_empty_line;
	char tmp_cc_list[LONG_STRING];
	struct mailFile mailFile;
	FAST_COMP_DECLARE;

	static int first_read = 0;
	if (curr_folder.flags & FOLDER_IS_SPOOL) {
	  elm_lock(LOCK_INCOMING);	/* ensure no mail arrives while we do this! */
	  if (! add_new_only) {
	    if (elm_access(curr_folder.tempname, ACCESS_EXISTS) == 0) {
	      /* Hey!  What the hell is this?  The temp file already exists? */
	      /* Looks like a potential clash of processes on the same file! */
	      ShutdownTerm();
	      show_error(catgets(elm_msg_cat, ElmSet, ElmWhatsThisTempExists,
		"What's this?  The temp folder \"%s\" already exists??"),
		curr_folder.tempname);
	      leave(LEAVE_ERROR|LEAVE_KEEP_TEMPFOLDER);
	    }
	    if ((temp = file_open(curr_folder.tempname,"w")) == NULL) {
	      ShutdownTerm();
	      show_error(catgets(elm_msg_cat, ElmSet, ElmCouldntOpenForTemp,
		     "Cannot open \"%s\"! [%s]"),
		     curr_folder.tempname, strerror(errno));
	      leave(LEAVE_ERROR);
	    }
	   copyit++;
	   chown(curr_folder.tempname, userid, groupid);
	   chmod(curr_folder.tempname, 0600);	/* shut off file for other people! */
	 }
	 else {
	   /*
	    * Used to open this file for append, however with the new
	    * content length stuff, we might want to fseek backwards
	    * in the file.  This will fail if append mode is set, so
	    * we can not do that anymore.  Since there is no way to
	    * fopen a file for write-only without either truncating it
	    * or putting it into append mode (neither of which is right
	    * for us), we do a "r+" open (also opens the file for reading),
	    * then fseek to the end of the file.
	    */
	   if ((temp = fopen(curr_folder.tempname,"r+")) == NULL) {
	     ShutdownTerm();
	     show_error(catgets(elm_msg_cat, ElmSet,
		     ElmCouldntReopenForTemp,
		     "Cannot reopen \"%s\"! [%s]"),
		     curr_folder.tempname, strerror(errno));
	     leave(LEAVE_ERROR);
	   }
	   if (fseek(temp, 0, 2) == -1) {
	     ShutdownTerm();
	     show_error(catgets(elm_msg_cat, ElmSet, ElmCouldntSeekTempEnd,
		     "Couldn't fseek to end of reopened temp mbox! [%s]"),
		     strerror(errno));
	     leave(LEAVE_ERROR);
	   }
	   copyit++;
	  }
	}

	if (! first_read++) {
	  ClearLine(LINES-1);
	  ClearLine(LINES);
	  if (add_new_only)
	    PutLine(LINES, 0, catgets(elm_msg_cat, ElmSet, ElmReadingInMessage,
		     "Reading in %s, message: %d"),
		     curr_folder.filename, curr_folder.num_mssgs);
	  else
	    PutLine(LINES, 0, catgets(elm_msg_cat, ElmSet, ElmReadingInMessage0,
		     "Reading in %s, message: 0"), curr_folder.filename);
	  count_x = LINES;
          count_y = 22 + strlen(curr_folder.filename);
	}
	else {
	  count_x = LINES-2;
	  PutLine(LINES-2, 0, catgets(elm_msg_cat, ElmSet, ElmReadingMessage0,
		"Reading message: 0"));
	}

	if (add_new_only) {
	   if (fseek(curr_folder.fp, curr_folder.size, 0) == -1) {
	     ShutdownTerm();
	     show_error(catgets(elm_msg_cat, ElmSet, ElmCouldntSeekEndFolder,
		    "Couldn't seek to %ld (end of folder) in %s! [%s]"),
		    curr_folder.size, curr_folder.filename, strerror(errno));
	     leave(LEAVE_EMERGENCY);
	   }
	   count = curr_folder.num_mssgs;		/* next available  */
	   fbytes = curr_folder.size;		/* start correctly */
	}

	/** find the size of the folder then unlock the file **/

	fflush(curr_folder.fp);
	curr_folder.size = bytes(curr_folder.filename);
        if (!(curr_folder.flags & FOLDER_IS_SPOOL))
   	   elm_unlock();

	/** now let's copy it all across accordingly... **/

	mailFile_attach(&mailFile, curr_folder.fp);
	if (copyit)
	  mailFile_copy(&mailFile, temp, mailFile_write_error);

	/* for large mailboxes, the default readmsginc is intolerable */
	if (curr_folder.size > 1000000 && readmsginc == 1)
	  readmsginc = 13;

	while (fbytes < curr_folder.size) {

	  if ((line_bytes = mailFile_gets(&buffer, &mailFile)) == 0)
	    break;

          /** remember if last line was a blank line **/
          was_empty_line = last_line_bytes == 1L;
          last_line_bytes = line_bytes;


	  /* Fix below to increment line count ONLY if we got a full line.
	   * Input lines longer than the mail_gets buffer size would
	   * get counted each time a subsequent part of them was
	   * read in. This meant that when the faulty line count was used
	   * to display the message, part of the next message
	   * was displayed at the end of the message.
	   */
	  if(buffer[line_bytes - 1] == '\n') line++;

	  /* preload first char of line for fast string comparisons */
	  fast_comp_load(buffer[0]);

	  if (fbytes == 0L || first_line) { 	/* first line of file... */
	    if (curr_folder.flags & FOLDER_IS_SPOOL) {
	      if (fast_stribegConst(buffer, "Forward to ")) {
	        set_central_message(catgets(elm_msg_cat, ElmSet, ElmMailBeingForwardTo,
			"Mail being forwarded to %s"),
                   	(char *) (buffer + 11));
	        forwarding_mail = TRUE;
	      }
	    }

	    /** flush leading blank lines before next test... **/
	    if (line_bytes == 1) {
	      fbytes++;
	      continue;
	    }
	    else
	      first_line = FALSE;

	    if (!fast_strbegConst(buffer, "From ") && !forwarding_mail) { /*}*/
	      ShutdownTerm();
	      show_error(catgets(elm_msg_cat, ElmSet, ElmFolderCorrupt,
		  "Folder is corrupt!!  I can't read it!!"));
              leave(LEAVE_ERROR);
	    }
	  }

	  if (content_remaining <= 0) {
	    if (was_empty_line && fast_strbegConst(buffer,"From ")) { /*}*/
	      /** allocate new header pointers, if needed... **/

	      if (count >= curr_folder.max_headers) {
		int new_max;
		new_max = curr_folder.max_headers + KLICK;
		if (curr_folder.max_headers == 0) {
		  curr_folder.headers = (struct header_rec **)
		    safe_malloc(new_max * sizeof(struct header_rec *));
		}
		else {
		  curr_folder.headers = (struct header_rec **)
		    safe_realloc((char *) curr_folder.headers,
			new_max * sizeof(struct header_rec *));
		}
		while (curr_folder.max_headers < new_max)
		  curr_folder.headers[curr_folder.max_headers++] = NULL;
	      }

	      /** allocate new header structure, if needed... **/

	      if (curr_folder.headers[count] == NULL) {
		curr_folder.headers[count] = (struct header_rec *)
			safe_malloc(sizeof(struct header_rec));
	      }

	      dprint(1, (debugfile,
		   "\n**** Calling real_from for \"From_\" ****\n"));
	      if (real_from(buffer, curr_folder.headers[count])) {

	        dprint(1, (debugfile, "\ncontent_remaining = %ld, content_start = %ld, lines_start = %ld, fbytes = %ld\n",
			content_remaining, content_start, lines_start, fbytes));

		current_header = curr_folder.headers[count];

		current_header->offset = (long) fbytes;
		current_header->content_length = -1; /* not found yet */
		current_header->index_number = count+1;
		content_length_found = FALSE;
		/* set default status - always 'visible'  - and
		 * if a spool file, presume 'new', otherwise
		 * 'read', for the time being until overridden
		 * by a Status: header.
		 * We presume 'read' for nonspool mailfile messages
		 * to be compatible messages stored with older versions of elm,
		 * which didn't support a Status: header.
		 */
		if (curr_folder.flags & FOLDER_IS_SPOOL)
		  current_header->org_status = current_header->status
		      = VISIBLE | NEW | UNREAD;
		else
		  current_header->org_status = current_header->status
		      = VISIBLE;

		strcpy(current_header->subject, "");	/* clear subj    */
		strcpy(current_header->to, "");		/* clear to    */
		strcpy(current_header->allfrom, "");	/* clear allto */
		strcpy(current_header->allto, "");	/* clear allto */
		strcpy(tmp_cc_list, "");		/* clear tmp_cc_list */
		strcpy(current_header->mailx_status, "");	/* clear status flags */
		strcpy(current_header->time_menu, "");	/* clear menu date */
		strcpy(current_header->messageid, "<no.id>"); /* set no id into message id */
		current_header->encrypted = 0;		/* clear encrypted */
		current_header->exit_disposition = UNSET;
		current_header->status_chgd = FALSE;
		current_header->ml_to.len = current_header->ml_to.max = 0;
		current_header->ml_to.str = NULL;

		/* Set the number of lines for the _preceding_ message,
		 * but only if there was a preceding message and
		 * only if it wasn't calculated already. It would
		 * have been calculated already if we are only
		 * reading headers of new messages that have just arrived,
		 * and the preceding message was one of the old ones.
		 */
		if ((count) && (!add_new_only || count > curr_folder.num_mssgs)) {
		  curr_folder.headers[count-1]->lines = line;

		  if (curr_folder.headers[count-1]->content_length < 0)
		    curr_folder.headers[count-1]->content_length = fbytes - content_start;
		}

		count++;
		subj = 0;
		line = 0;
		in_header = TRUE;
		if (count % readmsginc == 0) {
		  PutLine(count_x, count_y, "%d", count);
		  FlushOutput();
		}
	      }
	    } else if (!forwarding_mail && count == 0) {
	      /* if this is the first "From" in file but the "From" line is
	       * not of the proper format, we've got a corrupt folder.
	       */
	      ShutdownTerm();
	      show_error(catgets(elm_msg_cat, ElmSet, ElmFolderCorrupt,
		  "Folder is corrupt!!  I can't read it!!"));
	      leave(LEAVE_ERROR);
	    } else if (in_header == FALSE && content_length_found == TRUE && line_bytes > 1) {
		/* invalid content length, skip back to beginning of
		 * this messages text and ignore the content length
		 * field.  This requires restoring the current position
		 * in the spool file and the number of lines in the
		 * message.
		 */
	      if (mailFile_seek(&mailFile, content_start) == -1) {
		ShutdownTerm();
		show_error(catgets(elm_msg_cat, ElmSet,
		    ElmCouldntSeekBytesIntoFolder,
		   "Couldn't seek %ld bytes into folder. [%s]"),
		   curr_folder.size, strerror(errno));
		leave(LEAVE_EMERGENCY);
	      }
	      fbytes = content_start;
	      line = lines_start;
	      content_length_found = FALSE;
	      current_header->content_length = -1; /* mark as if not found yet */
	      line_bytes = 0;
	    }
	  }

	  if (in_header == TRUE) {
	    if (fast_header_cmp(buffer,">From", (char *)NULL)) {
	      buffer[line_bytes - 1] = '\0';
	      strfcpy(current_header->allfrom, buffer+7, STRING);
	      parse_arpa_who(buffer+6, current_header->from);
	    }
	    else if (fast_stribegConst(buffer,">From"))
	      forwarded(buffer, current_header); /* return address */
	    else if (fast_header_cmp(buffer,"Subject", (char *)NULL) ||
		     fast_header_cmp(buffer,"Subj", (char *)NULL) ||
		     fast_header_cmp(buffer,"Re", (char *)NULL)) {
	      if (! subj++) {
		strfcpy(tbuffer, buffer, sizeof(tbuffer));
	        remove_header_keyword(tbuffer);
	        copy_sans_escape(current_header->subject, tbuffer, STRING);
		remove_possible_trailing_spaces(current_header->subject);
	      }
	    }
	    else if (fast_header_cmp(buffer,"From", (char *)NULL)) {
	      buffer[line_bytes - 1] = '\0';
	      strfcpy(current_header->allfrom, buffer+6, STRING);
	      dprint(1, (debugfile,
		   "\n**** Calling parse_arpa_who for \"From\" ****\n"));
	      parse_arpa_who(buffer+5, current_header->from);
	    }
	    else if (fast_header_cmp(buffer, "Message-Id", (char *)NULL)) {
	      buffer[line_bytes - 1] = '\0';
	      strfcpy(current_header->messageid,
		     (char *) buffer + 12, STRING);
	    }

	    else if (fast_header_cmp(buffer, "Content-Length", (char *)NULL)) {
	      buffer[line_bytes - 1] = '\0';
	      current_header->content_length = atol((char *) buffer + 15);
	      /* Check if content_length is > remaining size of file */
	      if (current_header->content_length > curr_folder.size-fbytes)
		current_header->content_length = -1;
	      else
	        content_length_found = TRUE;

	    }


	    /** when it was sent... **/

	    else if (fast_header_cmp(buffer, "Date", (char *)NULL)) {
	      dprint(1, (debugfile,
		      "\n**** Calling parse_arpa_date for \"Date\" ****\n"));
	      strfcpy(tbuffer, buffer, sizeof(tbuffer));
	      remove_header_keyword(tbuffer);
	      parse_arpa_date(tbuffer, current_header);
	    }

	    /** some status things about the message... **/

	    else if (fast_header_cmp(buffer, "Priority", "urgent") ||
		     fast_header_cmp(buffer, "Importance", "2") ||
		     fast_header_cmp(buffer, "Importance", "high"))
	      current_header->org_status = current_header->status |= URGENT;
	    else if (fast_header_cmp(buffer, "Sensitivity", (char *)NULL)) {
	      if (fast_header_cmp(buffer, "Sensitivity", "2") ||
		  fast_header_cmp(buffer, "Sensitivity", "Personal") ||
		  fast_header_cmp(buffer, "Sensitivity", "Private"))
	        current_header->org_status = current_header->status |= PRIVATE;
	      else if (fast_header_cmp(buffer, "Sensitivity", "3") ||
		       fast_header_cmp(buffer, "Sensitivity", "Company-Confidential"))
		current_header->org_status = current_header->status |= CONFIDENTIAL;
	    }
	    else if (fast_header_cmp(buffer, "Content-Type", "mailform"))
	      current_header->org_status = current_header->status |= FORM_LETTER;
	    else if (fast_header_cmp(buffer, "Action", (char *)NULL))
	      current_header->org_status = current_header->status |= ACTION;
#ifdef	MIME_RECV
	    else if (fast_header_cmp(buffer, MIME_HDR_MIMEVERSION, MIME_VERSION_10))
	      current_header->org_status = current_header->status |= MIME_MESSAGE;
	    /* Next two lines for backward compatability to old drafts */
	    else if (fast_header_cmp(buffer, MIME_HDR_MIMEVERSION, MIME_VERSION_OLD))
	      current_header->org_status = current_header->status |= MIME_MESSAGE;
	    /* Next two lines for minimal support for mail generated by
	     * Sun's Openwindows mailtool */
	    else if (fast_header_cmp(buffer, MIME_HDR_CONTENTTYPE, "X-")) {
	      current_header->org_status = current_header->status |=
		  MIME_MESSAGE|MIME_NOTPLAIN;
	    }
	    else if (fast_header_cmp(buffer, MIME_HDR_CONTENTTYPE, (char *)NULL)) {
	      if (! (current_header->status & MIME_NOTPLAIN) &&
		  notplain(buffer+strlen(MIME_HDR_CONTENTTYPE)+1))
		current_header->org_status = current_header->status |=
		    MIME_NOTPLAIN;
	    }
	    else if (fast_header_cmp(buffer, MIME_HDR_CONTENTENCOD, (char *)NULL)) {
	      if (needs_mmdecode(buffer+strlen(MIME_HDR_CONTENTENCOD)+1))
		current_header->org_status = current_header->status |=
		    MIME_NEEDDECOD;
	    }
#endif /* MIME_RECV */

	    /** next let's see if it's to us or not... **/

	    /* copy to:, apparently-to: and cc: fields */
	    else if (fast_header_cmp(buffer, "To", NULL)) {
	      strfcat(current_header->allto, buffer+3, VERY_LONG_STRING);
	      in_to_list = TRUE;
	      current_header->to[0] = '\0';	/* nothing yet */
	      figure_out_addressee((char *) buffer +3, user_name,
				   current_header->to);
	    }
	    else if (fast_header_cmp(buffer, "Apparently-To", NULL)) {
	      strfcat(current_header->allto, buffer+14, VERY_LONG_STRING);
	    }
	    else if (fast_header_cmp(buffer, "Cc", NULL)) {
	      strfcat(tmp_cc_list, buffer+3, LONG_STRING);
	      in_cc_list = TRUE;
	    }
	    else if (fast_header_cmp(buffer, "Status", NULL)) {
	      strfcpy(tbuffer, buffer, sizeof(tbuffer));
	      remove_header_keyword(tbuffer);
	      strfcpy(current_header->mailx_status, tbuffer, WLEN);

	      c = strchr(current_header->mailx_status, '\n');
	      if (c != NULL)
		*c = '\0';
	      remove_possible_trailing_spaces(current_header->mailx_status);

	      /* Okay readjust the status. If there's an 'R', message
	       * is read; if there is no 'R' but there is an 'O', message
	       * is unread. In any case it isn't new because a new message
	       * wouldn't have a Status: header.
	       */
	      if (strchr(current_header->mailx_status, 'R') != NULL) {
		clearit(current_header->status, (NEW | UNREAD));
		clearit(current_header->org_status, (NEW | UNREAD));
	      }
	      else if (strchr(current_header->mailx_status, 'O') != NULL) {
		clearit(current_header->status, NEW);
		setit(current_header->status, UNREAD);
		clearit(current_header->org_status, NEW);
		setit(current_header->org_status, UNREAD);
	      }

	      if (strchr(current_header->mailx_status, 'r') != NULL) {
		setit(current_header->status, REPLIED_TO);
		setit(current_header->org_status, REPLIED_TO);
	      }
	    }

	    else if (buffer[0] == LINE_FEED || buffer[0] == '\0') {
	      in_header = FALSE;	/* in body of message! */
	      in_to_list = FALSE;
	      in_cc_list = FALSE;
	      if (*tmp_cc_list != '\0') {
		current_header->cc_index = strlen(current_header->allto);
		strfcat(current_header->allto, tmp_cc_list, VERY_LONG_STRING);
	      } else
	        current_header->cc_index = -1;
	      content_remaining = current_header->content_length;
	      content_start = fbytes + 1;
	      lines_start = line;
	    }
	    if (in_header == TRUE) {
	       if ((!whitespace(buffer[0])) && strchr(buffer, ':') == NULL) {
	        in_header = FALSE;	/* in body of message! */
	        in_to_list = FALSE;
	        in_cc_list = FALSE;
		if (*tmp_cc_list != '\0') {
		  current_header->cc_index = strlen(current_header->allto);
		  strfcat(current_header->allto, tmp_cc_list, VERY_LONG_STRING);
		} else
		  current_header->cc_index = -1;
	        content_remaining = current_header->content_length;
	        content_start = fbytes;
	        lines_start = line;
	      }
	    }
	    /*
	     * in_to_list gets set by To:, but we get here before the next
	     * line is read, so we skip this if we're still on 'To:'
	     */
	    if (in_to_list == TRUE && !fast_header_cmp(buffer, "To", NULL)) {
	      if (whitespace(buffer[0])) {
		strfcat(current_header->allto, buffer, VERY_LONG_STRING);
	        figure_out_addressee(buffer, user_name, current_header->to);
	      }
	      else in_to_list = FALSE;
	    }
	    if (in_cc_list == TRUE && !fast_header_cmp(buffer, "Cc", NULL)) {
	      if (whitespace(buffer[0])) {
		strfcat(tmp_cc_list, buffer, LONG_STRING);
	      }
	      else in_cc_list = FALSE;
	    }
	  }
	  if (in_header == FALSE && fast_strbegConst(buffer, MSSG_START_ENCODE))
	    current_header->encrypted = 1;
	  fbytes += (long) line_bytes;
	  content_remaining -= (long) line_bytes;
	}

	if (count) {
	  curr_folder.headers[count-1]->lines = line + 1;
	  if (curr_folder.headers[count-1]->content_length < 0)
	    curr_folder.headers[count-1]->content_length = fbytes - content_start;
	}

	mailFile_detach(&mailFile);	/* detach mailfile, flush temp if */
					/* copying*/

	if (curr_folder.flags & FOLDER_IS_SPOOL) {
	  fflush (curr_folder.fp);
	  elm_unlock();	/* remove lock file! */
	  if ((ferror(curr_folder.fp)) || (fclose(curr_folder.fp) == EOF)) {
	    ShutdownTerm();
	    show_error(catgets(elm_msg_cat, ElmSet, ElmCloseOnFolderFailed,
			   "Close failed on folder \"%s\"! [%s]"),
			    curr_folder.filename, strerror(errno));
	    leave(LEAVE_ERROR);
	  }

	  if ((ferror(temp)) || (fclose(temp) == EOF)) {
	      ShutdownTerm();
	      show_error(catgets(elm_msg_cat, ElmSet, ElmCloseOnTempFailed,
			      "Close failed on temp file \"%s\"! [%s]"),
			      curr_folder.tempname, strerror(errno));
	      leave(LEAVE_ERROR);
	  }

	  /* sanity check on append - is resulting temp file longer??? */
	  if (bytes(curr_folder.tempname) != curr_folder.size) {
	    ShutdownTerm();
	    show_error(catgets(elm_msg_cat, ElmSet, ElmLengthNESpool,
	       "Huh!?  newmbox - length of mbox != spool mailbox length!!"));
	    leave(LEAVE_ERROR);
	  }

	  if ((curr_folder.fp = fopen(curr_folder.tempname,"r")) == NULL) {
	    ShutdownTerm();
	    show_error(catgets(elm_msg_cat, ElmSet, ElmCouldntReopenForTemp,
		   "Cannot reopen \"%s\"! [%s]"),
	           curr_folder.tempname, strerror(errno));
	    leave(LEAVE_ERROR);
	  }
	}
	else
          rewind(curr_folder.fp);

	/* Sort folder *before* we establish the current message, so that
	 * the current message is based on the post-sort order.
	 * Note that we have to set the message count
	 * before the sort for the sort to correctly keep the correct
	 * current message if we are only adding new messages here. */

	curr_folder.num_mssgs = count;
	sort_mailbox(count, 1);

	/* Now lets figure what the current message should be.
	 * If we are only reading in newly added messages from a mailfile
	 * that already had some messages, current should remain the same.
	 * If we have a folder of no messages, current should be zero.
	 * Otherwise, if we have point_to_new on then the current message
	 * is the first message of status NEW if there is one.
	 * If we don't have point_to_new on or if there are no messages of
	 * of status NEW, then the current message is the first message.
	 */
	if(!(add_new_only && curr_folder.curr_mssg != 0)) {
	  if(count == 0)
	    curr_folder.curr_mssg = 0;
	  else {
	    curr_folder.curr_mssg = 1;
	    if (point_to_new) {
	      for(another_count = 0; another_count < count; another_count++) {
		if(ison(curr_folder.headers[another_count]->status, NEW)) {
		  curr_folder.curr_mssg = another_count+1;
		  goto found_new;
		}
	      }
	      for(another_count = 0; another_count < count; another_count++) {
		if(ison(curr_folder.headers[another_count]->status, UNREAD)) {
		  curr_folder.curr_mssg = another_count+1;
		  goto found_new;
		}
	      }
	      switch (sortby) {
		case SENT_DATE:
		case RECEIVED_DATE:
		case MAILBOX_ORDER:
		  curr_folder.curr_mssg = count;
	      }
	      found_new: ;
	    }
	  }
	}
        get_page(curr_folder.curr_mssg);
	return(count);
}
