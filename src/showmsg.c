

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.8 $   $State: Exp $
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
 * $Log: showmsg.c,v $
 * Revision 1.8  1999/03/24  14:04:05  wfp5p
 * elm 2.5PL0
 *
 * Revision 1.7  1996/10/28  16:58:10  wfp5p
 * Beta 1
 *
 * Revision 1.6  1996/08/08  19:49:30  wfp5p
 * Alpha 11
 *
 * Revision 1.5  1996/05/09  15:51:27  wfp5p
 * Alpha 10
 *
 * Revision 1.4  1996/03/14  17:29:51  wfp5p
 * Alpha 9
 *
 * Revision 1.3  1995/09/29  17:42:27  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.2  1995/06/12  20:33:37  wfp5p
 * Alpha 2 clean up
 *
 * Revision 1.1.1.1  1995/04/19  20:38:38  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** This file contains all the routines needed to display the specified
    message.
**/

#include "elm_defs.h"
#include "elm_globals.h"
#include "s_elm.h"
#include "port_wait.h"

int    pipe_abort  = FALSE;	/* did we receive a SIGNAL(SIGPIPE)? */

static FILE *pipe_wr_fp;		/* file pointer to write to external pager */

/*
 * FOO - I believe the SIGWINCH handling is botched.  By ignoring it,
 * it is causing Elm to lose track of screen changes.  I don't understand
 * why this needs to be done, the SIGWINCH handler should be harmless.
 * For the time being, I'm removing SIGWINCH, which will prevent the
 * routine from dinking around with it.
 */
#ifdef SIGWINCH
# undef SIGWINCH
#endif

static int show_line(char *buffer, int buf_len, int builtin);

int show_msg(int number)
{
	/*** Display number'th message.  Get starting and ending lines
	     of message from headers data structure, then fly through
	     the file, displaying only those lines that are between the
	     two!

	     Return 0 to return to the index screen or a character entered
	     by the user to initiate a command without returning to
	     the index screen (to be processed via process_showmsg_cmd()).
	***/

	char title1[SLEN], title2[SLEN], title3[SLEN], titlebuf[SLEN];
	char who[LONG_STRING], buffer[VERY_LONG_STRING];
	waitstatus_t wait_stat;

	int weed_header, weeding_out = 0;	/* weeding    */
	int using_to,				/* misc use   */
	    pipe_fd[2],				/* pipe file descriptors */
	    new_pipe_fd,			/* dup'ed pipe fil des */
	    lines,				/* num lines in msg */
	    fork_ret = 0,			/* fork return value */
	    wait_ret,				/* wait return value */
	    form_letter = FALSE,		/* Form ltr?  */
	    form_letter_section = 0,		/* section    */
	    padding = 0,			/*   counter  */
	    builtin = FALSE,			/* our pager? */
	    val = 0,				/* return val */
	    buf_len,				/* line length */
	    err;				/* place holder for errno */
	struct header_rec *current_header = curr_folder.headers[number-1];
#ifdef	SIGTSTP
	SIGHAND_TYPE	(*oldstop)(), (*oldcont)();
#endif
#ifdef	SIGWINCH
	SIGHAND_TYPE	(*oldwinch)();
#endif

	lines = current_header->lines;

	dprint(4, (debugfile,"displaying %d lines from message %d using %s\n",
		lines, number, pager));

	if (number > curr_folder.num_mssgs || number < 1)
	  return(val);

	if(ison(current_header->status, NEW)) {
	  clearit(current_header->status, NEW);   /* it's been read now! */
	  current_header->status_chgd = TRUE;
	}
	if(ison(current_header->status, UNREAD)) {
	  clearit(current_header->status, UNREAD);   /* it's been read now! */
	  current_header->status_chgd = TRUE;
	}

#ifdef MIME_RECV
	if ((current_header->status & MIME_MESSAGE) &&
	    ((current_header->status & MIME_NEEDDECOD) ||
	     (current_header->status & MIME_NOTPLAIN)) &&
	    !getenv("NOMETAMAIL") ) {
	    char fname[STRING], Cmd[SLEN];
	    int code;
	    FILE *fpout;

	    if (fseek(curr_folder.fp, current_header->offset, 0) != -1) {
		sprintf(fname, "%semm.%d.%d", temp_dir, getpid(), getuid());
		if ((fpout = fopen(fname, "w")) != NULL) {
		    copy_message(fpout, curr_folder.curr_mssg, CM_DECODE);
		    (void) fclose (fpout);
 	            sprintf(Cmd, "metamail -p -z -m Elm %s", fname);
		    Raw(OFF);
		    softkeys_off();
		    EnableFkeys(OFF);
		    ClearScreen();
		    code = system_call(Cmd, SY_ENAB_SIGINT);
		    Raw(ON | NO_TITE);	/* Raw on but don't switch screen */
		    (void) unlink (fname);
		    if (code == 0) {
		      PutLine(LINES,0, catgets(elm_msg_cat, ElmSet, ElmPressAnyKeyIndex,
			     "Press any key to return to index."));
		      (void) GetKey(0);
		      NewLine();
		      Raw(OFF | NO_TITE); /* Raw off so raw on takes effect */
		      Raw(ON); /* Finally raw on and switch screen */
 	              softkeys_on();
		      EnableFkeys(ON);

		      return(0);
		    }
		}
	    }
	}
#endif

	if (fseek(curr_folder.fp, current_header->offset, 0) == -1) {
	  err = errno;
	  dprint(1, (debugfile,
		  "Error: seek %ld bytes into file, errno %s (show_message)\n",
		  current_header->offset, strerror(err)));
	  show_error(catgets(elm_msg_cat, ElmSet, ElmSeekFailedFile,
		  "ELM [seek] couldn't read %d bytes into file (%s)."),
		  current_header->offset, strerror(err));
	  return(val);
	}

	builtin = (
	  (strbegConst(pager,"builtin")|| strbegConst(pager,"internal"))
	  || ( builtin_lines < 0
	      ? lines < LINES + builtin_lines : lines < builtin_lines)
	);

	if (builtin) {

	  start_builtin(lines);

	} else {

	  /* put terminal out of raw mode so external pager has normal env */
	  Raw(OFF);

	  /* create pipe for external pager and fork */

	  if(pipe(pipe_fd) == -1) {
	    err = errno;
	    dprint(1, (debugfile, "Error: pipe failed, errno %s (show_msg)\n",
	      strerror(err)));
	    show_error(catgets(elm_msg_cat, ElmSet, ElmPreparePagerPipe,
	      "Could not prepare for external pager(pipe()-%s)."),
	      strerror(err));
	    Raw(ON);
	    return(val);
	  }

	  fork_ret = fork();
  	  if (fork_ret == -1) {

	    err = errno;
	    dprint(1, (debugfile, "Error: fork failed, errno %s (show_msg)\n",
	      strerror(err)));
	    show_error(catgets(elm_msg_cat, ElmSet, ElmPreparePagerFork,
	      "Could not prepare for external pager(fork()-%s)."),
	      strerror(err));
	    Raw(ON);
	    return(val);

	  } else if (fork_ret == 0) {

	    /* child fork */

	    /* close write-only pipe fd and fit read-only pipe fd to stdin */

	    close(pipe_fd[1]);
	    close(fileno(stdin));
	    if((new_pipe_fd = dup(pipe_fd[0])) == -1) {
	      err = errno;
	      dprint(1, (debugfile, "Error: dup failed, errno %s (show_msg)\n",
		strerror(err)));
	      show_error(catgets(elm_msg_cat, ElmSet, ElmPreparePagerDup,
	        "Could not prepare for external pager(dup()-%s)."),
		strerror(err));
	      _exit(err);
	    }
	    close(pipe_fd[0]);	/* original pipe fd no longer needed */

	    /* use stdio on new pipe fd */
	    if(fdopen(new_pipe_fd, "r") == NULL) {
	      err = errno;
	      dprint(1,
		(debugfile, "Error: child fdopen failed, errno %s (show_msg)\n",
		strerror(err)));
	      show_error(catgets(elm_msg_cat, ElmSet, ElmPreparePagerChildFdopen,
		"Could not prepare for external pager(child fdopen()-%s)."),
		strerror(err));
	      _exit(err);
	    }

	    /* now execute pager and exit */

	    /* system_call() will return user to user's normal permissions.
	     * This is what makes this pipe secure - user won't have elm's
	     * special SETGID permissions (if so configured) and will only
	     * be able to execute a pager that user normally has permission
	     * to execute */

	    _exit(system_call(pager, SY_ENAB_SIGINT));

	  } /* else this is the parent fork */

	  /* close read-only pipe fd and do write-only with stdio */
	  close(pipe_fd[0]);

	  if((pipe_wr_fp = fdopen(pipe_fd[1], "w")) == NULL) {
	    err = errno;
	    dprint(1,
	      (debugfile, "Error: parent fdopen failed, errno %s (show_msg)\n",
	      strerror(err)));
	    show_error(catgets(elm_msg_cat, ElmSet, ElmPreparePagerParentFdopen,
	      "Could not prepare for external pager(parent fdopen()-%s)."),
	      strerror(err));

	    /* Failure - must close pipe and wait for child */
	    close(pipe_fd[1]);
	    while ((wait_ret = wait(&wait_stat)) != fork_ret && wait_ret!= -1)
	      ;

	    Raw(OFF);
	    return(val);	/* pager may have already touched the screen */
	  }

	  /* and that's it! */
	  set_lines_displayed(0);
#ifdef	SIGTSTP
	  oldstop = signal(SIGTSTP,SIG_DFL);
	  oldcont = signal(SIGCONT,SIG_DFL);
#endif
#ifdef	SIGWINCH
	  oldwinch = signal(SIGWINCH,SIG_DFL);
#endif
	  EnableFkeys(OFF);
	}

	ClearScreen();

	pipe_abort = FALSE;

	form_letter = (current_header->status & FORM_LETTER);

	if (form_letter) {
	  if (filter)
	    form_letter_section = 1;	/* initialize to section 1 */
	}

	if (title_messages && filter) {

	  using_to =
	    tail_of(current_header->from, who, current_header->to);

	  sprintf(title1, "%s %d/%d  ",
		    curr_folder.headers[curr_folder.curr_mssg-1]->status & DELETED
			  ? nls_deleted : form_letter ? nls_form : nls_message,
		    number, curr_folder.num_mssgs);
	  sprintf(title2, "%s %s", using_to? nls_to : nls_from, who);
	  elm_date_str(title3, current_header);
	  strcat(title3, " ");
	  strcat(title3, current_header->time_zone);

	  /* truncate or pad title2 portion on the right
	   * so that line fits exactly */
	  padding =
	    COLS - (strlen(title1) + (buf_len=strlen(title2)) + strlen(title3));

	  sprintf(titlebuf, "%s%-*.*s%s\n", title1, buf_len+padding,
	      buf_len+padding, title2, title3);

	  if (builtin)
	    display_line(titlebuf, strlen(titlebuf));
	  else
	    fprintf(pipe_wr_fp, "%s", titlebuf);

	  /** if there's a subject, let's output it next,
	      centered if it fits on a single line.  **/

	  if ((buf_len = strlen(current_header->subject)) > 0 &&
		matchInList(weedlist,weedcount,"Subject:",TRUE)) {
	    padding = (buf_len < COLS ? COLS - buf_len : 0);
	    sprintf(buffer, "%*s%s\n", padding/2, "", current_header->subject);
	  } else
	    strcpy(buffer, "\n");

	  if (builtin)
	    display_line(buffer, strlen(buffer));
	  else
	    fprintf(pipe_wr_fp, "%s", buffer);

	  /** was this message address to us?  if not, then to whom? **/

	  if (! using_to && matchInList(weedlist,weedcount,"To:",TRUE) && filter &&
	      strcmp(current_header->to, user_name) != 0 &&
	      strlen(current_header->to) > 0) {
	    sprintf(buffer, catgets(elm_msg_cat, ElmSet, ElmMessageAddressedTo,
		  "%s(message addressed to %.60s)\n"),
	          strlen(current_header->subject) > 0 ? "\n" : "",
		  current_header->to);
	    if (builtin)
	      display_line(buffer, strlen(buffer));
	    else
	      fprintf(pipe_wr_fp, "%s", buffer);
	  }

	  /** The test above is: if we didn't originally send the mail
	      (e.g. we're not reading "mail.sent") AND the user is currently
	      weeding out the "To:" line (otherwise they'll get it twice!)
	      AND the user is actually weeding out headers AND the message
	      wasn't addressed to the user AND the 'to' address is non-zero
	      (consider what happens when the message doesn't HAVE a "To:"
	      line...the value is NULL but it doesn't match the username
	      either.  We don't want to display something ugly like
	      "(message addressed to )" which will just clutter up the
	      screen!).

	      And you thought programming was EASY!!!!
	  **/

	  /** one more friendly thing - output a line indicating what sort
	      of status the message has (e.g. Urgent etc).  Mostly added
	      for X.400 support, this is nonetheless generally useful to
	      include...
	  **/

	  buffer[0] = '\0';

	  /* we want to flag Urgent, Confidential, Private and Expired tags */

	  if (current_header->status & PRIVATE)
	    strcpy(buffer, catgets(elm_msg_cat, ElmSet, ElmTaggedPrivate,
		"\n(** This message is tagged Private"));
	  else if (current_header->status & CONFIDENTIAL)
	    strcpy(buffer, catgets(elm_msg_cat, ElmSet, ElmTaggedCompanyConfidential,
		"\n(** This message is tagged Company Confidential"));

	  if (current_header->status & URGENT) {
	    if (buffer[0] == '\0')
	      strcpy(buffer, catgets(elm_msg_cat, ElmSet, ElmTaggedUrgent,
		"\n(** This message is tagged Urgent"));
	    else if (current_header->status & EXPIRED)
	      strcat(buffer, catgets(elm_msg_cat, ElmSet, ElmCommaUrgent, ", Urgent"));
	    else
	      strcat(buffer, catgets(elm_msg_cat, ElmSet, ElmAndUrgent, " and Urgent"));
	  }

	  if (current_header->status & EXPIRED) {
	    if (buffer[0] == '\0')
	      strcpy(buffer, catgets(elm_msg_cat, ElmSet, ElmMessageHasExpired,
		"\n(** This message has Expired"));
	    else
	      strcat(buffer, catgets(elm_msg_cat, ElmSet, ElmAndHasExpired,
		", and has Expired"));
	  }

	  if (buffer[0] != '\0') {
	    strcat(buffer, " **)\n");
	    if (builtin)
	      display_line(buffer, strlen(buffer));
	    else
	      fprintf(pipe_wr_fp, buffer);
	  }

	  if (builtin)			/* this is for a one-line blank    */
	    display_line("\n",1);	/*   separator between the title   */
	  else				/*   stuff and the actual message  */
	    fprintf(pipe_wr_fp, "\n");	/*   we're trying to display       */

	}

	weed_header = filter;	/* allow us to change it after header */

	while (lines > 0 && pipe_abort == FALSE) {

	    if ((buf_len = mail_gets(buffer, VERY_LONG_STRING, curr_folder.fp)) == 0) {

	      dprint(1, (debugfile,
		"Premature end of file! Lines left = %d msg = %d (show_msg)\n",
		lines, number));

	      show_error(catgets(elm_msg_cat, ElmSet, ElmPrematureEndOfFile,
		"Premature end of file!"));
	      if (sleepmsg > 0)
		    sleep(sleepmsg);
	      break;
	    }
	    if (buf_len > 0)  {
	      if(buffer[buf_len - 1] == '\n') {
	        lines--;
	        set_lines_displayed(get_lines_displayed() + 1);
	      }
	      while(buf_len > 0 && (buffer[buf_len - 1] == '\n'
				    ||buffer[buf_len - 1] == '\r'))
		--buf_len;
	    }

  	    if (buf_len == 0) {
	      weed_header = 0;		/* past header! */
	      weeding_out = 0;
	    }

	    if (form_letter && weed_header)
		/* skip it.  NEVER display random headers in forms! */;
	    else if (weed_header && matchInList(weedlist,weedcount,buffer,TRUE))
	      weeding_out = 1;	 /* aha!  We don't want to see this! */
	    else if (weeding_out) {
	      weeding_out = (whitespace(buffer[0]));	/* 'n' line weed */
	      if (! weeding_out) 	/* just turned on! */
	        val = show_line(buffer, buf_len, builtin);
	    }
	    else if (form_letter && strbegConst(buffer,"***") && filter) {
	      strcpy(buffer,
"\n------------------------------------------------------------------------------\n");
	      val = show_line(buffer, buf_len, builtin);	/* hide '***' */
	      form_letter_section++;
	    }
	    else if (form_letter_section == 1 || form_letter_section == 3)
	      /** skip this stuff - we can't deal with it... **/;
	    else
	      val = show_line(buffer, buf_len, builtin);

	    if (val != 0)	/* discontinue the display */
	      break;
	}

	if (!builtin) {
/*	  EnableFkeys(ON);*/
	  fclose(pipe_wr_fp);
	  while ((wait_ret = wait(&wait_stat)) != fork_ret
		  && wait_ret!= -1)
	    ;
	  /* turn raw on **after** child terminates in case child
	   * doesn't put us back to cooked mode after we return ourselves
	   * to raw.
	   */
	  Raw(ON);
          EnableFkeys(ON);
#ifdef	SIGTSTP
	  (void)signal(SIGTSTP,oldstop);
	  (void)signal(SIGCONT,oldcont);
#endif
#ifdef	SIGWINCH
	  (void)signal(SIGWINCH,oldwinch);
#endif
	}

	/* If we are to prompt for a user input command and we don't
	 * already have one */
	if ((prompt_after_pager || builtin) && val == 0) {
	  MoveCursor(LINES,0);
	  if (Term.status & TERM_CAN_SO)
	      StartStandout();
	  PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmCommandIToReturn,
		" Command ('i' to return to index): "));
	  if (Term.status & TERM_CAN_SO)
	      EndStandout();
	  val = GetKey(0);
	}

	/* 'q' means quit current operation and pop back up to previous level -
	 * in this case it therefore means return to index screen.
	 */
	return(val == 'i' || val == 'q' ? 0 : val);
}

static int show_line(char *buffer, int buf_len, int builtin)
{
	/** Hands the given line to the output pipe.  'builtin' is true if
	    we're using the builtin pager.
	    Return the character entered by the user to indicate
	    a command other than continuing with the display (only possible
	    with the builtin pager), otherwise 0. **/

	strcpy(buffer + buf_len, "\n");
	++buf_len;
	if (builtin) {
	  return(display_line(buffer, buf_len));
	}
	errno = 0;
	fprintf(pipe_wr_fp, "%s", buffer);
	if (errno != 0)
	  dprint(1, (debugfile, "\terror %s hit!\n", strerror(errno)));
	return(0);
}

