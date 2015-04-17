

/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.7 $   $State: Exp $
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
 * $Log: syscall.c,v $
 * Revision 1.7  1999/03/24  14:04:07  wfp5p
 * elm 2.5PL0
 *
 * Revision 1.6  1996/03/14  17:29:59  wfp5p
 * Alpha 9
 *
 * Revision 1.5  1995/09/29  17:42:35  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.4  1995/09/11  15:19:32  wfp5p
 * Alpha 7
 *
 * Revision 1.3  1995/06/12  20:33:37  wfp5p
 * Alpha 2 clean up
 *
 * Revision 1.2  1995/06/01  13:13:32  wfp5p
 * Readmsg was fixed to work correctly if called from within elm.  From Chip
 * Rosenthal <chip@unicom.com>
 *
 * Revision 1.1.1.1  1995/04/19  20:38:39  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** These routines are used for user-level system calls, including the
    '!' command and the '|' commands...

**/

#include "elm_defs.h"
#include "elm_globals.h"
#include "s_elm.h"
#include "port_wait.h"

#ifndef I_UNISTD
void _exit();
#endif


#ifdef ALLOW_SUBSHELL

int subshell(void)
{
	/** spawn a subshell with either the specified command
	    returns non-zero if screen rewrite needed
	**/

	char command[SLEN];
	int  old_raw, helpful, ret;

	helpful = (user_level == 0);

	if (helpful)
	  PutLine0(LINES-3, COLS-40, catgets(elm_msg_cat, ElmSet, ElmUseShellName,
		"(Use the shell name for a shell.)"));
	PutLine0(LINES-2, 0, catgets(elm_msg_cat, ElmSet, ElmShellCommand,
		"Shell command: "));
	if (enter_string(command, sizeof(command), -1, -1, ESTR_ENTER) < 0
		    || command[0] == '\0') {
	  if (helpful)
	    MoveCursor(LINES-3,COLS-40);
	  else
	    MoveCursor(LINES-2,0);
	  CleartoEOS();
	  return 0;
	}

	MoveCursor(LINES,0);
	CleartoEOLN();

	old_raw = !!(Term.status & TERM_IS_RAW);
	if (old_raw)
	  Raw(OFF);
	softkeys_off();
	EnableFkeys(OFF);

	umask(original_umask);	/* restore original umask so users new files are ok */
	ret = system_call(command, SY_USER_SHELL|SY_ENAB_SIGINT|SY_DUMPSTATE);
	umask(077);		/* now put it back to private for mail files */

	PutLine0(LINES, 0, catgets(elm_msg_cat, ElmSet, ElmPressAnyKeyToReturn,
		"\n\nPress any key to return to ELM: "));
	fflush(stdout);
	Raw(ON | NO_TITE);
	(void) getchar();
	NewLine();
	Raw(OFF | NO_TITE); /* Done even if old_raw == ON, to get ti/te right */
	if (old_raw == ON)
	  Raw(ON);

	softkeys_on();
	EnableFkeys(ON);

	if (ret)
	  error1(catgets(elm_msg_cat, ElmSet, ElmReturnCodeWas,
		"Return code was %d."), ret);

	return 1;
}

#endif /* ALLOW_SUBSHELL */

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

int system_call(char *string, int options)
{
	/** The following might be encoded into the "options" parameter:

	    SY_USER_SHELL	When set, we will use the user-defined
				"shell" instead of "/bin/sh" for the
				shell escape.

	    SY_ENV_SHELL	When set, put "SHELL=[name-of-shell]" in
				the child's environment.  This hack makes
				mail transport programs work right even
				for users with restricted shells.

	    SY_ENAB_SIGHUP	When set, we will set SIGHUP, SIGTSTP, and
				SIGCONT to their default behaviour during
				the shell escape rather than ignoring them.
				This is particularly important with stuff
				like `vi' so it can preserve the session on
				a SIGHUP and do its thing with job control.

	    SY_ENAB_SIGINT	This option implies SY_ENAB_SIGHUP.  In
				addition to the signals listed above, this
				option will also set SIGINT and SIGQUIT
				to their default behaviour rather than
				ignoring them.

	    SY_DUMPSTATE	Create a state file for use by the "readmsg"
				program.  This is so that if "readmsg" is
				invoked it can figure out what folder we are
				in and what message(s) are selected.

	    SY_COOKED		If tty currently is in "raw" mode then
				kick it over to "cooked" and restore when
				done.  Also disables function keys for
				the duration of the escape.  Upon return,
				the cursor postion is marked invalid.

	    The SY_COOKED option generally is used when escaping to
	    some interactive command.  That's why it invalidates the
	    cursor.  When this option is not specified, the calling
	    routine must decide whether or not the cursor postion
	    is valid upon return, and act accordingly.

	**/

	int pfd[2], retstat, err, pid, w, old_raw, iteration;
	char *sh;
	waitstatus_t status;
	SIGHAND_TYPE (*istat)(), (*qstat)();
#ifdef SIGWINCH
	SIGHAND_TYPE (*wstat)();
#endif
#ifdef SIGTSTP
	SIGHAND_TYPE (*oldstop)(), (*oldstart)();
#endif

	/* figure out what shell we are using here */
	sh = ((options & SY_USER_SHELL) ? shell : "/bin/sh");
	dprint(2, (debugfile, "System Call: %s\n\t%s\n", sh, string));

	/* if we aren't reading a folder then a state dump is meaningless */
	if (!OPMODE_IS_READMODE(opmode))
	    options &= ~SY_DUMPSTATE;

	/* see if we need to dump out the folder state */
	if (options & SY_DUMPSTATE) {
	    if (create_folder_state_file() != 0)
		return -1;
	}

	/*
	 * Note the neat trick with close-on-exec pipes.
	 * If the child's exec() succeeds, then the pipe read returns zero.
	 * Otherwise, it returns the zero byte written by the child
	 * after the exec() is attempted.  This is the cleanest way I know
	 * to discover whether an exec() failed.   --CHS
	 */

	fflush(stdout);
	fflush(stderr);
	if (pipe(pfd) == -1) {
	    perror("pipe");
	    if (options & SY_DUMPSTATE)
		(void) remove_folder_state_file();
	    return -1;
	}
	fcntl(pfd[0], F_SETFD, 1);
	fcntl(pfd[1], F_SETFD, 1);

	/* if requested, kick terminal over to cooked mode */
	if (options & SY_COOKED) {
	    old_raw = !!(Term.status & TERM_IS_RAW);
	    if (old_raw)
		Raw(OFF);
	    EnableFkeys(OFF);
	    softkeys_off();
	}

	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
#ifdef SIGWINCH
	wstat = signal(SIGWINCH, SIG_DFL);
#endif
#ifdef SIGTSTP
	oldstop = signal(SIGTSTP, SIG_DFL);
	oldstart = signal(SIGCONT, SIG_DFL);
#endif

	for (iteration = 0; iteration < 5; ++iteration) {
	  if (iteration > 0)
	    sleep(2);

#ifdef VFORK
	  if (options&SY_ENV_SHELL)
	    pid = fork();
	  else
	    pid = vfork();
#else
	  pid = fork();
#endif

	  if (pid != -1)
	    break;
	}

	if (pid == -1) {
	  perror("fork");
	}
	else if (pid == 0) {
	  /*
	   * Set group and user back to their original values.
	   * Note that group must be set first.
	   */
	  setegid(groupid);
	  setuid(userid);

	  /*
	   * Program to exec may or may not be able to handle
	   * interrupt, quit, hangup and stop signals.
	   */
	  if (options&SY_ENAB_SIGINT)
		options |= SY_ENAB_SIGHUP;
	  (void) signal(SIGHUP,  (options&SY_ENAB_SIGHUP) ? SIG_DFL : SIG_IGN);
	  (void) signal(SIGINT,  (options&SY_ENAB_SIGINT) ? SIG_DFL : SIG_IGN);
	  (void) signal(SIGQUIT, (options&SY_ENAB_SIGINT) ? SIG_DFL : SIG_IGN);
#ifdef SIGTSTP
	  (void) signal(SIGTSTP, (options&SY_ENAB_SIGHUP) ? SIG_DFL : SIG_IGN);
	  (void) signal(SIGCONT, (options&SY_ENAB_SIGHUP) ? SIG_DFL : SIG_IGN);
#endif

	  /* Optionally override the SHELL environment variable. */
	  if (options&SY_ENV_SHELL) {
	    static char sheq[] = "SHELL=";
	    char *p = malloc(sizeof(sheq) + strlen(sh));
	    if (p) {
	      sprintf(p, "%s%s", sheq, sh);
	      putenv(p);
	    }
	  }

	  /* Go for it. */
	  if (string)
	    execl(sh, basename(sh), "-c", string, (char *) 0);
	  else
	    execl(sh, basename(sh), (char *) 0);
	  err = errno;

	  /* If exec fails, we write a byte to the pipe before exiting. */
	  perror(sh);
	  write(pfd[1], (char *)&err, sizeof(err));
	  _exit(127);
	}
	else {
	  int rd;

	  /* Try to read a byte from the pipe. */
	  close(pfd[1]);
	  rd = read(pfd[0], &err, sizeof(err));
	  close(pfd[0]);

	  while ((w = wait(&status)) != pid)
	      if (w < 0 && errno != EINTR)
		  break;

	  if (rd > 0) {
	    ; /* exec failed - errno was read from pipe */
	    retstat = -1;
	  } else if (w < 0) {
	    /* something went wrong in wait */
	    err = errno;
	    retstat = -1;
	  } else if (!WIFEXITED(status)) {
	    /* program probably was killed or signalled */
	    err = EINTR;
	    retstat = -1;
	  } else {
	    /* program terminated */
	    err = 0;
	    retstat = WEXITSTATUS(status);
	  }
        }

	(void) signal(SIGINT, istat);
	(void) signal(SIGQUIT, qstat);
#ifdef SIGWINCH
	(void) signal(SIGWINCH, wstat);
#endif
#ifdef SIGTSTP
	(void) signal(SIGTSTP, oldstop);
	(void) signal(SIGCONT, oldstart);
#endif

	/* restore terminal */
	if (options & SY_COOKED) {
	    if (old_raw == ON)
		Raw(ON);
	    EnableFkeys(ON);
	    softkeys_on();
	    InvalidateCursor();
	}

	/* cleanup any folder state file we made */
	if (options & SY_DUMPSTATE)
	    (void) remove_folder_state_file();

	errno = err;
	return(retstat);
}

int do_pipe(void)
{
	/** pipe the current message or tagged messages to
	    the specified sequence.. **/

	char command[SLEN], buffer[SLEN];
	register int  ret;
	int	old_raw;

        PutLine0(LINES-2, 0,
		    catgets(elm_msg_cat, ElmSet, ElmPipeTo, "Pipe to: "));
	if (enter_string(command, sizeof(command), -1, -1, ESTR_ENTER) < 0
		    || command[0] == '\0') {
	  MoveCursor(LINES-2,0);
	  CleartoEOLN();
	  return(0);
	}

	MoveCursor(LINES,0);
	CleartoEOLN();
	old_raw = !!(Term.status & TERM_IS_RAW);
	if (old_raw)
	  Raw(OFF);

	EnableFkeys(OFF);
	softkeys_off();
	
	sprintf(buffer, "%s -Ih|%s", readmsg, command);
	ret = system_call(buffer, SY_USER_SHELL|SY_ENAB_SIGINT|SY_DUMPSTATE);

	PutLine0(LINES, 0, catgets(elm_msg_cat, ElmSet, ElmPressAnyKeyToReturn,
		"\n\nPress any key to return to ELM: "));

	fflush(stdout);
	Raw(ON | NO_TITE);
	(void) getchar();
	NewLine();
	Raw(OFF | NO_TITE); /* Done even if old_raw == ON, to get ti/te right */
	if (old_raw == ON)
	  Raw(ON);

        softkeys_on();
	EnableFkeys(ON);

	if (ret != 0)
	  error1(catgets(elm_msg_cat, ElmSet, ElmReturnCodeWas,
		"Return code was %d."), ret);
	return(1);
}

int print_msg(int pause_on_scroll)
{
	/*
	 * Print the tagged messages, or the current message if none are
	 * tagged.  Message(s) are passed passed into the command specified
	 * by "printout".  An error is given if "printout" is undefined.
	 *
	 * Printing will be done through a pipe so we can print the number
	 * of lines output.  This is used to determine whether the screen
	 * got trashed by the print command.  One limitation is that only
	 * stdout lines are counted, not stderr output.  A nonzero result
	 * is returned if we think enough output was generated to trash
	 * the display, a zero result indicates the display is probably
	 * alright.  Further, if the display is trashed and "pause_on_scroll"
	 * is true then we'll give a "hit any key" prompt before returning.
	 *
	 * This routine has two modes of behavior, depending upon whether
	 * there is a "%s" embedded in the "printout" string.  If there,
	 * the old Elm behavior is used (a temp file is used, all output
	 * from the print command is chucked out).  If there isn't a "%s"
	 * then the new behavior is used (message(s) piped right into
	 * print command, output is left attached to the terminal).
	 *
	 * The old behaviour is bizarre.  I hope we can ditch it someday.
	 */

	char command[SLEN], filename[SLEN], buf[SLEN], *cp;
	int  nlines, retcode, old_raw;
	FILE *fp;

	/*
	 * Make sure we know how to print.
	 */
	if (printout[0] == '\0') {
	    error(catgets(elm_msg_cat, ElmSet, ElmPrintDontKnowHow,
		"Don't know how to print - option \"printmail\" undefined!"));
	    return 0;
	}

	/*
	 * Setup print command.
	 */
	sprintf(cp = command, "%s -Ip", readmsg);
	cp += strlen(cp);
	if (printhdrs[0] != '\0') {
	    sprintf(cp, " -w '%s'", printhdrs);
	    cp += strlen(cp);
	}
	if (strstr(printout, "%s") != NULL) {
	    sprintf(filename, "%s%s%d", temp_dir, temp_print, getpid());
	    sprintf(buf, printout, filename);
	    sprintf(cp, " >%s ; %s", filename, buf);
	} else {
	    sprintf(cp, " | %s", printout);
	}

	/*
	 * Create information for "readmsg" command.
	 */
	if (create_folder_state_file() != 0)
	    return 0;

	/*
	 * Put keyboard into normal state.
	 */
	old_raw = !!(Term.status & TERM_IS_RAW);
	if (old_raw)
	    Raw(OFF | NO_TITE);
	softkeys_off();
	EnableFkeys(OFF);

	/*
	 * Run the print command in a pipe and grab the output.
	 */
	putchar('\n');
	fflush(stdout);
	nlines = 0;
	if ((fp = popen(command, "r")) == NULL) {
	    error(catgets(elm_msg_cat, ElmSet, ElmPrintPipeFailed,
		"Cannot create pipe to print command."));
	    goto done;
	}
	while (fgets(buf, sizeof(buf), fp) != NULL) {
	    fputs(buf, stdout);
	    ++nlines;
	}

	/*
	 * See if there were enough lines printed to trash the screen.
	 */
	if (pause_on_scroll && nlines > 1) {
	    printf("\n%s ", catgets(elm_msg_cat, ElmSet, ElmPrintPressAKey,
		"Press any key to continue:"));
	    fflush(stdout);
	    Raw(ON | NO_TITE);
	    (void) getchar();
	}

	/*
	 * Display a status message.
	 */
	if ((retcode = pclose(fp)) == 0) {
	    error(catgets(elm_msg_cat, ElmSet, ElmPrintJobSpooled,
		"Print job has been spooled."));
	} else if ((retcode & 0xFF) == 0) {
	    error1(catgets(elm_msg_cat, ElmSet, ElmPrintFailCode,
		"Printout failed with return code %d."), (retcode>>8));
	} else {
	    error1(catgets(elm_msg_cat, ElmSet, ElmPrintFailStatus,
		"Printout failed with status 0x%04x."), (retcode>>8));
	}

	/*
	 * Hack alert:  The only place we use "pause_on_scroll" false is when
	 * printing while reading a mail message.  This newline prevents the
	 * above message from being wiped out by the command prompt.
	 */
	if (!pause_on_scroll)
		putchar('\n');

done:
	Raw(old_raw | NO_TITE);
	softkeys_on();
	EnableFkeys(ON);
	(void) unlink(filename);
	(void) remove_folder_state_file();
	return (nlines > 1);
}


static char folder_state_env_param[SLEN], *folder_state_fname;

/*
 * Setup a folder state file for external utilities (e.g. "readmsg").
 * Returns zero if the file was created, -1 if an error occurred.  A
 * diagnostic will have been printed on an error return.
 *
 * The state file contains the following:
 *
 * - An "F" record with the pathname to the current folder.
 *
 * - An "N" record with a count of the number of messages in the folder.
 *
 * - A set of "I" records indicating the seek index of the messages
 *   in the folder.  The first "I" record will contain the seek index
 *   of message number one, and so on.  The "I" records will be in
 *   sorting order and not necessarily mbox order.  The number of "I"
 *   records will match the value indicated in the "N" record.
 *
 * - A "C" record with a count of the total number of messages selected.
 *
 * - A set of "S" records indicating message number(s) which have been
 *   selected.  If messages have been tagged then there will be one
 *   "S" record for each selected message.  If no messages have been
 *   tagged then either:  there will be a single "S" record with the
 *   current message number, or there will be no "S" records if the
 *   folder is empty.  The number of "S" records will match the value
 *   indicated in the "C" record.
 */
int create_folder_state_file(void)
{
    int count, i;
    FILE *fp;

    /* format an environ param with the state file and pick out file name */
    sprintf(folder_state_env_param, "%s=%s%s%d",
	FOLDER_STATE_ENV, default_temp, temp_state, getpid());
    folder_state_fname = folder_state_env_param + strlen(FOLDER_STATE_ENV) +1;

    /* open up the folder state file for writing */
    if ((fp = file_open(folder_state_fname, "w")) == NULL) {
	error1(catgets(elm_msg_cat, ElmSet, ElmCannotCreateFolderState,
		"Cannot create folder state file \"%s\"."), folder_state_fname);
	return -1;
    }

    /* write out the pathname of the folder */
    fprintf(fp, "F%s\n",
	((curr_folder.flags & FOLDER_IS_SPOOL) ? curr_folder.tempname : curr_folder.filename));

    /* write out the folder size and message indices */
    fprintf(fp, "N%d\n", curr_folder.num_mssgs);
    for (i = 0 ; i < curr_folder.num_mssgs ; ++i) {
	fprintf(fp, "I%ld %ld\n",
	    curr_folder.headers[i]->offset, curr_folder.headers[i]->content_length);
    }

    /* count up the number of tagged messages */
    count = 0;
    for (i = 0 ; i < curr_folder.num_mssgs ; i++)  {
	if (curr_folder.headers[i]->status & TAGGED)
		++count;
    }

    /* write out selected messages */
    if (count > 0) {
	/* we found tagged messages - write them out */
	fprintf(fp, "C%d\n", count);
	for (i = 0 ; i < curr_folder.num_mssgs ; i++) {
	    if (curr_folder.headers[i]->status & TAGGED)
		fprintf(fp, "S%d\n", i+1);
	}
    } else if (curr_folder.curr_mssg > 0) {
	/* no tagged messages - write out the selected message */
	fprintf(fp, "C1\nS%d\n", curr_folder.curr_mssg);
    } else {
	/* hmmm...must be an empty mailbox */
	fprintf(fp, "C0\n");
    }

    /* file is done */
    (void) fclose(fp);

    /* put pointer to the file in the environment */
    if (putenv(folder_state_env_param) != 0) {
	error1(catgets(elm_msg_cat, ElmSet, ElmCannotCreateEnvParam,
	    "Cannot create environment parameter \"%s\"."), FOLDER_STATE_ENV);
	return -1;
    }

    return 0;
}


int remove_folder_state_file(void)
{
    /*
     * We simply leave the FOLDER_STATE_ENV environment variable set.
     * It's too much work trying to pull it out of the environment, and
     * the load_folder_state_file() does not mind if the environment
     * variable points to a non-existent file.
     */
    return unlink(folder_state_fname);
}

