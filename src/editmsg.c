

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
 * $Log: editmsg.c,v $
 * Revision 1.7  1996/03/14  17:27:58  wfp5p
 * Alpha 9
 *
 * Revision 1.6  1995/09/29  17:42:04  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.5  1995/09/11  15:19:05  wfp5p
 * Alpha 7
 *
 * Revision 1.4  1995/06/23  18:03:46  wfp5p
 * Missing semi-colon added.
 *
 * Revision 1.3  1995/06/21  15:27:07  wfp5p
 * editflush and confirmtagsave are new in the elmrc (Keith Neufeld)
 * The mlist code has a little bug fix.
 *
 * Revision 1.2  1995/06/12  20:33:33  wfp5p
 * Alpha 2 clean up
 *
 * Revision 1.1.1.1  1995/04/19  20:38:35  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/** This contains routines to do with starting up and using an editor (or two)
    from within Elm.  This stuff used to be in mailmsg2.c...
**/

#include "elm_defs.h"
#include "elm_globals.h"
#include "sndhdrs.h"
#include "s_elm.h"
#include <assert.h>
#include <setjmp.h>

/*
 * static values for the builtin editor
 */
static int builtin_active = FALSE;	/* routine is not re-entrant!	*/
static JMP_BUF builtin_jmpbuf;		/* interrupt handling return	*/
static int builtin_interrupt_count;	/* consecutive interrupt count	*/


static void tilde_help P_((void));
static void read_in_file P_((FILE *, const char *, int));
static void print_message_so_far(FILE *edit_fd, const SEND_HEADER *shdr);
static void read_in_messages P_((FILE *, char *));
static void get_with_expansion P_((const char *, char *, char *, const char *));
static SIGHAND_TYPE builtin_interrupt_handler P_((int));
static int builtin_editor P_((const char *, SEND_HEADER *));

#define IS_BUILTIN(s)	(streq((s), "builtin") || streq((s), "none"))


int edit_message(const char *filename, SEND_HEADER *shdr,
		 const char *sel_editor)
{
    /* Return 0 if successful, -1 on error. */

    char buffer[SLEN];
    int rc, return_value = 0, err;

    /* pick default editor on NULL */
    if (sel_editor == NULL)
	sel_editor = (IS_BUILTIN(editor) ? alternative_editor : editor);

    /* handle request for the builtin editor */
    if (IS_BUILTIN(sel_editor))
	return builtin_editor(filename, shdr);

    /* we will be running an external editor */
    PutLine(LINES, 0, catgets(elm_msg_cat, ElmSet, ElmInvokeEditor,
	    "Invoking editor..."));

    if (strstr(sel_editor, "%s") != NULL)
	sprintf(buffer, sel_editor, filename);
    else
	sprintf(buffer, "%s %s", sel_editor, filename);

    chown(filename, userid, groupid);

    if ((rc = system_call(buffer, SY_COOKED|SY_ENAB_SIGHUP|SY_DUMPSTATE)) < 0) {
	err = errno;
	dprint(1, (debugfile,
	    "System call failed with status %d (edit_message)\n", rc));
	dprint(1, (debugfile, "** %s **\n", strerror(err)));
	ClearLine(LINES-1);
	show_error(catgets(elm_msg_cat, ElmSet, ElmCantInvokeEditor,
	    "Can't invoke editor '%s' for composition."), sel_editor);
	if (sleepmsg > 0)
	    sleep(sleepmsg);
	return_value = -1;
    }

    /* Flush input buffer.  This is especially important under X,
    * where accidental keystrokes in the elm window could make
    * things messy.
    */
    if (edit_flush)
	FlushInput();

    return return_value;
}

static void tilde_help(void)
{
	/* a simple routine to print out what is available at this level */

	PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmEditmsgAvailOpts,
	  "\n\r(Available options at this point are:\n\r\n\r"));
	PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmEditmsgHelpMenu,
	  "\t%c?\tPrint this help menu.\n\r"), escape_char);
	if (escape_char == '~') /* doesn't make sense otherwise... */
	  PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmEditmsgAddLine,
	      "\t~~\tAdd line prefixed by a single '~' character.\n\r"));
	PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmEditmsgBCC,
	  "\t%cb\tChange the addresses in the Blind-carbon-copy list.\n\r"),
	  escape_char);

	PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmEditmsgCC,
		"\t%cc\tChange the addresses in the Carbon-copy list.\n\r"),
		escape_char);
	PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmEditmsgEmacs,
	      "\t%ce\tInvoke the Emacs editor on the message, if possible.\n\r"),
		escape_char);
	PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmEditmsgAddMessage,
		"\t%cf\tAdd the specified message or current.\n\r"),
		escape_char);
	PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmEditmsgToCCBCC,
	      "\t%ch\tChange all available headers (to, cc, bcc, subject).\n\r"),
		escape_char);
	PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmEditmsgSameCurrentPrefix,
		"\t%cm\tSame as '%cf', but with the current 'prefix'.\n\r"),
		escape_char, escape_char);
	PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmEditmsgUserEditor,
		"\t%co\tInvoke a user specified editor on the message.\n\r"),
		escape_char);
	PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmEditmsgPrintMsg,
	      "\t%cp\tPrint out message as typed in so far.\n\r"), escape_char);
	PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmEditmsgReadFile,
		"\t%cr\tRead in the specified file.\n\r"), escape_char);
	PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmEditmsgSubject,
		"\t%cs\tChange the subject of the message.\n\r"), escape_char);
	PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmEditmsgTo,
		"\t%ct\tChange the addresses in the To list.\n\r"),
		escape_char);
	PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmEditmsgVi,
		"\t%cv\tInvoke the Vi visual editor on the message.\n\r"),
		escape_char);
	PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmEditmsgUnixCmd,
	  "\t%c!\tExecute a UNIX command (or give a shell if no command).\n\r"),
	  escape_char);
	PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmEditmsgAddUnixCmd,
      "\t%c<\tExecute a UNIX command adding the output to the message.\n\r"),
	  escape_char);
	PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmEditmsgEndMsg,
      "\t.  \tby itself on a line (or a control-D) ends the message.\n\r"));
	PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmEditmsgContinue,
	  "Continue.)\n\r"));
}

static void read_in_file(FILE *fd, const char *filename, int show_user_filename)
{
	/** Open the specified file and stream it in to the already opened
	    file descriptor given to us.  When we're done output the number
	    of lines and characters we added, if any... **/

	FILE *myfd;
	char exp_fname[SLEN], buffer[SLEN];
	int n;
	int lines = 0, nchars = 0;

	while (whitespace(*filename))
		++filename;

	/** expand any shell variables or leading '~' **/
	(void) expand_env(exp_fname, filename, sizeof(exp_fname));

	if (exp_fname[0] == '\0') {
	  PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmNoFilenameSpecified,
	      "\n\r(No filename specified for file read! Continue.)\n\r"));
	  return;
	}

	if ((myfd = fopen(exp_fname,"r")) == NULL) {
	  PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmCouldntReadFile,
	    "\n\r(Couldn't read file '%s'! Continue.)\n\r"), exp_fname);
	  return;
	}

	while ((n = mail_gets(buffer, SLEN, myfd))) {
	  if (buffer[n-1] == '\n')
		  lines++;
	  nchars += n;
  	  fwrite(buffer, 1, n, fd);
	}
	fflush(fd);

	fclose(myfd);

	if (lines == 1)
	  PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmAddedLine,
	    "\n\r(Added 1 line ["));
	else
	  PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmAddedLinePlural,
	    "\n\r(Added %d lines ["), lines);

	if (nchars == 1)
	  PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmAddedChar,
	    "1 char] "));
	else
	  PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmAddedCharPlural,
	    "%d chars] "), nchars);

	if (show_user_filename)
	  PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmAddedFromFile,
		"from file %s. Continue.)\n\r"), exp_fname);
	else
	  PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmAddedToMessage,
		"to message. Continue.)\n\r"));

	return;
}

static void print_message_so_far(FILE *edit_fd, const SEND_HEADER *shdr)
{
	/** This prints out the message typed in so far.  We accomplish
	    this in a cheap manner - close the file, reopen it for reading,
	    stream it to the screen, then close the file, and reopen it
	    for appending.  Simple, but effective!

	    A nice enhancement would be for this to -> page <- the message
	    if it's sufficiently long.  Too much work for now, though.
	**/

	char buffer[SLEN];

	fflush(edit_fd);
	fseek(edit_fd, 0L, 0);

	NewLine();
	PutLine(-1, -1, "To: %s\r\n",		format_long(shdr->to, 4));
	PutLine(-1, -1, "Cc: %s\r\n",		format_long(shdr->cc, 4));
	PutLine(-1, -1, "Bcc: %s\r\n",		format_long(shdr->bcc, 5));
	PutLine(-1, -1, "Subject: %s\r\n",	shdr->subject);
	NewLine();

	while (fgets(buffer, SLEN, edit_fd) != NULL) {
	  PutLine(-1, -1, buffer);
	  NewLine();
	}

	PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmEditmsgPrintContinue,
	    "\n\r(Continue entering message.)\n\r"));
}

static void read_in_messages(FILE *fd, char *buffer)
{
	/** Read the specified messages into the open file.  If the
	    first character of "buffer" is 'm' then prefix it, other-
	    wise just stream it in straight...Since we're using the
	    pipe to 'readmsg' we can also allow the user to specify
	    patterns and such too...
	**/

	FILE *myfd;
	char  local_buffer[SLEN], *arg;
	int add_prefix=0, mindex;
	int n;
	int lines = 0, nchars = 0;

	add_prefix = tolower(buffer[0]) == 'm';

	/* strip whitespace to get argument */
	for(arg = &buffer[1]; whitespace(*arg); arg++)
		;

	/* a couple of quick checks */
	if(curr_folder.num_mssgs < 1) {
	  PutLine(-1, -1, catgets(elm_msg_cat,
	    ElmSet, ElmNoMessageReadContinue,
	    "(No messages to read in! Continue.)\n\r"));
	  return;
	}
	if (isdigit(*arg)) {
	  if((mindex = atoi(arg)) < 1 || mindex > curr_folder.num_mssgs) {
	    sprintf(local_buffer, catgets(elm_msg_cat, ElmSet, ElmValidNumbersBetween,
	      "(Valid message numbers are between 1 and %d. Continue.)\n\r"),
	      curr_folder.num_mssgs);
	    PutLine(-1, -1, local_buffer);
	    return;
	  }
	}

	/* dump state information for "readmsg" to use */
	if (create_folder_state_file() != 0)
	  return;

	/* go run readmsg and get output */
	sprintf(local_buffer, "%s -- %s", readmsg, arg);
	if ((myfd = popen(local_buffer, "r")) == NULL) {
	   PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmCantFindReadmsg,
	       "(Can't find 'readmsg' command! Continue.)\n\r"));
	   (void) remove_folder_state_file();
	   return;
	}

	dprint(5, (debugfile, "** readmsg call: \"%s\" **\n", local_buffer));

	while ((n = mail_gets(local_buffer, SLEN, myfd))) {
	  nchars += n;
	  if (local_buffer[n-1] == '\n') lines++;
	  if (add_prefix)
	    fprintf(fd, "%s", prefixchars);
	  fwrite(local_buffer, 1, n, fd);
	}

	pclose(myfd);
        (void) remove_folder_state_file();

	if (lines == 0) {
	  PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmEditmsgCouldntAdd,
	 	 "(Couldn't add the requested message. Continue.)\n\r"));
	  return;
	}

	if (lines == 1)
	  PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmAddedLine,
	    "\n\r(Added 1 line ["));
	else
	  PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmAddedLinePlural,
	    "\n\r(Added %d lines ["), lines);

	if (nchars == 1)
	  PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmAddedChar,
	    "1 char] "));
	else
	  PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmAddedCharPlural,
	    "%d chars] "), nchars);

	PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmAddedToMessage,
		"to message. Continue.)\n\r"));

	return;
}

static void get_with_expansion(const char *prompt, char *buffer,
			       char *expanded_buffer, const char *sourcebuf)
{
	char savecopy[SLEN];

	/** This is used to prompt for a new value of the specified field.
	    If expanded_buffer == NULL then we won't bother trying to expand
	    this puppy out!  (sourcebuf could be an initial addition)
	**/

	PutLine(-1, -1, prompt);

	if (sourcebuf != NULL) {
	  while (!whitespace(*sourcebuf) && *sourcebuf != '\0')
	    sourcebuf++;
	  if (*sourcebuf != '\0') {
	    while (whitespace(*sourcebuf))
	      sourcebuf++;
	    if (strlen(sourcebuf) > 0) {
	      strcat(buffer, " ");
	      strcat(buffer, sourcebuf);
	    }
	  }
	}

	(void) strfcpy(savecopy, buffer, sizeof(savecopy));
	if (enter_string(buffer, SLEN, -1, -1, ESTR_UPDATE) < 0) {
	    /* undo */
	    (void) strcpy(buffer, savecopy);
	    PutLine(-1, -1, prompt);
	    PutLine(-1, -1, buffer);
	    NewLine();
	    return;
	}

	if(expanded_buffer != NULL) {
	  build_address(strip_commas(buffer), expanded_buffer);
	  if(*expanded_buffer != '\0') {
	    if (*prompt != '\n')
	      NewLine();
	    PutLine(-1, -1, prompt);
	    PutLine(-1, -1, expanded_buffer);
	  }
	}
	NewLine();

	return;
}

/*
 * Interrupt handler for builtin_editor().
 */
static void builtin_interrupt_handler(int sig)
{
	signal(SIGINT, builtin_interrupt_handler);
	signal(SIGQUIT, builtin_interrupt_handler);

	++builtin_interrupt_count;

#if defined(SIGSET) && defined(HASSIGHOLD)
	/*
	 * During execution of a signal handler set with sigset(),
	 * the originating signal is held.  It must be released or
	 * it cannot recur.
	 */
	sigrelse(sig);
#endif /* SIGSET and HASSIGHOLD */

	LONGJMP(builtin_jmpbuf, 1);
}


/*
 * The editor used by edit_message() when "builtin" or "none" are selected.
 * Return 0 if successful, -1 on error.
 */
static int builtin_editor(const char *filename, SEND_HEADER *shdr)
{
    char linebuf[SLEN];		/* line input buffer			*/
    char wrapbuf[SLEN];		/* wrapped line overflow buffer		*/
    char tmpbuf[SLEN];		/* scratch buffer			*/
    FILE *fp;			/* output stream to "filename"		*/
    int rc;			/* return code from this procedure	*/
    int is_wrapped;		/* wrapped line flag			*/
    int err;			/* temp holder for errno		*/
    SIGHAND_TYPE (*oldint)();	/* previous value of SIGINT		*/
    SIGHAND_TYPE (*oldquit)();	/* previous value of SIGQUIT		*/
    SIGHAND_TYPE builtin_interrupt_handler();

    /* the built-in editor is not re-entrant! */
    assert(!builtin_active);

    /* initialize return code to failure */
    rc = -1;

    if ((fp = fopen(filename, "r+")) == NULL) {
	err = errno;
	sprintf(tmpbuf, catgets(elm_msg_cat, ElmSet, ElmCouldntOpenAppend,
	    "Couldn't open %s for update [%s]."),
	    filename, strerror(err));
	PutLine(-1, -1, tmpbuf);
	dprint(1, (debugfile,
	    "Error encountered trying to open file %s;\n", filename));
	dprint(1, (debugfile, "** %s **\n", strerror(err)));
	return rc;
    }

    /* skip past any existing text */
    fseek(fp, 0, SEEK_END);

    /* prompt user, depending upon whether file already has text */
    if (fsize(fp) > 0L)
	strcpy(tmpbuf, catgets(elm_msg_cat, ElmSet, ElmContinueEntering,
	    "\n\rContinue entering message."));
    else
	strcpy(tmpbuf, catgets(elm_msg_cat, ElmSet, ElmEnterMessage,
	    "\n\rEnter message."));
    strcat(tmpbuf, catgets(elm_msg_cat, ElmSet, ElmTypeElmCommands,
	"  Type Elm commands on lines by themselves.\n\r"));
    sprintf(tmpbuf+strlen(tmpbuf),
	catgets(elm_msg_cat, ElmSet, ElmCommandsInclude,
	"Commands include:  ^D or '.' to end, %cp to list, %c? for help.\n\r\n\r"),
	escape_char, escape_char);
    CleartoEOS();
    PutLine(-1, -1, tmpbuf);

    builtin_active = TRUE;
    builtin_interrupt_count = 0;

    oldint  = signal(SIGINT,  builtin_interrupt_handler);
    oldquit = signal(SIGQUIT, builtin_interrupt_handler);

    /* return location for interrupts */
    while (SETJMP(builtin_jmpbuf) != 0) {
	if (builtin_interrupt_count == 1) {
	    PutLine(-1, -1, catgets(elm_msg_cat, ElmSet,
		ElmEditmsgOneMoreCancel,
		"(Interrupt. One more to cancel this letter.)\n\r"));
	} else {
	    PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmEditmsgCancelled,
		"(Interrupt. Letter canceled.)\n\r"));
	    goto done;
	}
    }

    for (;;) {

	/* re-open file if it was closed out on a call to an external editor */
	if (fp == NULL) {
	    if ((fp = fopen(filename, "a+")) == NULL) {
		err = errno;
		sprintf(tmpbuf, catgets(elm_msg_cat, ElmSet,
		    ElmCouldntOpenAppend,
		    "Couldn't open %s for update [%s]."),
		    filename, strerror(err));
		PutLine(-1, -1, tmpbuf);
		dprint(1, (debugfile,
		    "Error encountered trying to open file %s;\n", filename));
		dprint(1, (debugfile, "** %s **\n", strerror(err)));
		goto done;
	    }
	    PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmPostEdContinue,
		"(Continue entering message.  Type ^D or '.' on a line by itself to end.)\n\r"));
	}

	linebuf[0] = '\0';
	wrapbuf[0] = '\0';
	is_wrapped = 0;

more_wrap:
	if (wrapped_enter(linebuf, wrapbuf, -1, -1, fp, &is_wrapped) != 0)
	    break;

	if (is_wrapped) {
	    fprintf(fp, "%s\n", linebuf);
	    NewLine();
	    (void) strcpy(linebuf, wrapbuf);
	    wrapbuf[0] = '\0';
	    goto more_wrap;
	}

	/* reset consecutive interrupt counter */
	builtin_interrupt_count = 0;

	/* a lone "." signals end of text */
	if (strcmp(linebuf, ".") == 0)
	    break;

	/* process line of text */
	if (linebuf[0] != escape_char) {
	   fprintf(fp, "%s\n", linebuf);
	   NewLine();
	   continue;
	}

	/* command character was escaped */
	if (linebuf[1] == escape_char) {
	    fprintf(fp, "%s\n", linebuf+1);
	    continue;
	}

	switch (tolower(linebuf[1])) {

	case '?':
	    tilde_help();
	    break;

	case 't':
	    get_with_expansion("\n\rTo: ",
			shdr->to, shdr->expanded_to, linebuf);
	    break;

	case 'b':
	    get_with_expansion("\n\rBcc: ",
			shdr->bcc, shdr->expanded_bcc, linebuf);
	    break;

	case 'c':
	    get_with_expansion("\n\rCc: ",
			shdr->cc, shdr->expanded_cc, linebuf);
	    break;

	case 's':
	    get_with_expansion("\n\rSubject: ",
			shdr->subject, (char *)NULL, linebuf);
	    break;

	case 'h':
	    get_with_expansion("\n\rTo: ",
			shdr->to, shdr->expanded_to, (char *)NULL);
	    get_with_expansion("Cc: ",
			shdr->cc, shdr->expanded_cc, (char *)NULL);
	    get_with_expansion("Bcc: ",
			shdr->bcc, shdr->expanded_bcc, (char *)NULL);
	    get_with_expansion("Subject: ",
			shdr->subject, (char *)NULL, (char *)NULL);
	    break;

	case 'r':
	    read_in_file(fp, linebuf+2, 1);
	    break;

	case 'e':
	    if (e_editor[0] == '\0') {
		PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmDontKnowEmacs,
		    "\n\r(Don't know where Emacs would be. Continue.)\n\r"));
		break;
	    }
	    NewLine();
	    fclose(fp);
	    fp = NULL;
	    (void) edit_message(filename, shdr, e_editor);
	    break;

	case 'v':
	    NewLine();
	    fclose(fp);
	    fp = NULL;
	    (void) edit_message(filename, shdr, v_editor);
	    break;

	case 'o':
	    PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmEnterNameEditor,
		"\n\rPlease enter the name of the editor: "));
	    if (enter_string(tmpbuf, sizeof(tmpbuf), -1, -1, ESTR_ENTER) < 0
			|| tmpbuf[0] == '\0') {
		PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmSimpleContinue,
		    "(Continue.)\n\r"));
		break;
	    }
	    NewLine();
	    fclose(fp);
	    fp = NULL;
	    (void) edit_message(filename, shdr, tmpbuf);
	    break;

	case '<':
	    NewLine();
	    if (strlen(linebuf) < 3) {
		PutLine(-1, -1, catgets(elm_msg_cat,
		    ElmSet, ElmUseSpecificCommand,
		   "(You need to use a specific command here. Continue.)\n\r"));
		break;
	    }
	    sprintf(tmpbuf, "%s%s.%d", temp_dir, temp_edit, getpid());
	    sprintf(linebuf+strlen(linebuf), " >%s 2>&1", tmpbuf);
	    (void) system_call(linebuf+2, SY_COOKED|SY_ENAB_SIGINT|SY_DUMPSTATE);
	    read_in_file(fp, tmpbuf, 0);
	    (void) unlink(tmpbuf);
	    break;

	case '!':
	    NewLine();
	    (void) system_call(
		(strlen(linebuf) < 3 ? (char *)NULL : linebuf+2),
		SY_COOKED|SY_USER_SHELL|SY_ENAB_SIGINT|SY_DUMPSTATE);
	    PutLine(LINES, 0, catgets(elm_msg_cat, ElmSet, ElmSimpleContinue,
		"(Continue.)\n\r"));
	    break;

	case 'm': /* same as 'f' but with leading prefix added */
	case 'f': /* this can be directly translated into a
			 'readmsg' call with the same params! */
	    NewLine();
	    read_in_messages(fp, linebuf+1);
	    break;

	case 'p': /* print out message so far */
	    print_message_so_far(fp, shdr);
	    break;

	default:
	    sprintf(tmpbuf, catgets(elm_msg_cat, ElmSet, ElmDontKnowChar,
		"\n\r(Don't know what %c%c is. Try %c? for help.)\n\r"),
		 escape_char, linebuf[1], escape_char);
	    PutLine(-1, -1, tmpbuf);
	    break;

	}

    }

    PutLine(-1, -1, catgets(elm_msg_cat, ElmSet, ElmEndOfMessage,
	"\n\r<end-of-message>\n\r\n\r\n\r\n\r"));
    rc = 0;

done:
    (void) signal(SIGINT,  oldint);
    (void) signal(SIGQUIT, oldquit);
    if (fp != NULL)
	fclose(fp);
    builtin_active = FALSE;
    return rc;
}
