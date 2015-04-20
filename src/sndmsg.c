/*******************************************************************************
 *  The Elm Mail System  -  $Revision: 1.6 $   $State: Exp $
 *
 * This file and all associated files and documentation:
 *                      Copyright (c) 1988-1995 USENET Community Trust
 *			Copyright (c) 1986,1987 Dave Taylor
 *******************************************************************************
 * Bug reports, patches, comments, suggestions should be sent to:
 *
 *      Bill Pemberton, Elm Coordinator
 *      flash@virginia.edu
 *
 *******************************************************************************
 * $Log: sndmsg.c,v $
 * Revision 1.6  1996/10/28  16:58:11  wfp5p
 * Beta 1
 *
 * Revision 1.5  1996/08/08  19:49:31  wfp5p
 * Alpha 11
 *
 * Revision 1.4  1996/05/09  15:51:28  wfp5p
 * Alpha 10
 *
 * Revision 1.3  1996/03/14  17:29:55  wfp5p
 * Alpha 9
 *
 * Revision 1.2  1996/03/13  14:38:03  wfp5p
 * Alpha 9 before Chip's big changes
 *
 * Revision 1.1  1995/09/29  17:42:31  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 *
 ******************************************************************************/

#include "elm_defs.h"
#include "elm_globals.h"
#include "mime.h"
#include "sndhdrs.h"
#include "sndparts.h"
#include "s_elm.h"
#include <assert.h>

extern char *strip_commas();

static void display_subject P_((const char *));
static int get_subject P_((char *));
static int get_copies P_((char *, char *, char *, int));
static int verify_copy_msg P_((void));
static int recall_last_msg P_((const char *, int));
static int append_sig P_((FILE *, SEND_HEADER *));
static int verify_bounceback P_((void));
static void remove_hostbang P_((char *));
static int verify_transmission P_((const char *, SEND_HEADER *, SEND_MULTIPART **, int *, char *));


int send_message(const char *given_to, const char *given_cc,
		 const char *given_subject, int mssgtype)
{
    SEND_HEADER *shdr;		/* headers for this message		*/
    SEND_BODYPART *mssg_parts;	/* contents of this message		*/
    SEND_MULTIPART *attachments;/* add'l user-specified attachments	*/
    SEND_MULTIPART *mp;         /* used to loop thru user attachments   */
    char fname_mssgbody[SLEN];	/* message composition file		*/
    char *fname_fullmssg; 	/* complete message to transmit		*/
    char fname_savecopy[SLEN];	/* selected save copy folder		*/
    FILE *fp_mssgbody;		/* file stream for fname_mssgbody[]	*/
    FILE *fp_fullmssg;		/* file stream for fname_fullmssg[]	*/
    int need_redraw;		/* have we scribbled on the scren?	*/
    int copy_msg;		/* copy mssg from folder into body?	*/
    int edit_msg;		/* edit the message body?		*/
    int form;			/* is the message a form?		*/
    int body_has_text;		/* any text been placed in mssg yet?	*/
    int want_signature;		/* should .signature file be added?	*/
    int send_attempts;		/* count of tries to send this message	*/
    int rc;			/* final return status			*/
    char tmpbuf[SLEN];
    char bigbuf[VERY_LONG_STRING];
    char *s;
    int i;

    /* is there a cancelled message laying around from last time? */
    static int cancelled_msg = FALSE;
    static int saved_want_sig = FALSE;

    /* initialize */
    shdr = sndhdr_new();
    mssg_parts = NULL;
    attachments = NULL;
    fp_mssgbody = NULL;
    fp_fullmssg = NULL;
    fname_fullmssg = NULL;
    need_redraw = FALSE;
    body_has_text = FALSE;
    want_signature = TRUE;
    send_attempts = 0;
    rc = -1;

    assert(mssgtype == SM_ORIGINAL || OPMODE_IS_READMODE(opmode));
    switch (mssgtype) {
    case SM_ORIGINAL:
	copy_msg = NO;
	edit_msg = TRUE;
	form = allow_forms;
	break;
    case SM_REPLY:
	copy_msg = verify_copy_msg();
	edit_msg = TRUE;
	form = NO;
	generate_in_reply_to(shdr, curr_folder.curr_mssg-1);
	break;
    case SM_FORWARD:
	copy_msg = YES;
	edit_msg = TRUE;
	form = NO;
	break;
    case SM_FWDUNQUOTE:
	copy_msg = YES;
	edit_msg = FALSE;
	form = NO;
	break;
    case SM_FORMRESP:
	copy_msg = FORM;
	edit_msg = FALSE;
	form = NO;
	generate_in_reply_to(shdr, curr_folder.curr_mssg-1);
	break;
    default:
	show_error("INTERNAL ERROR - bad mssgtype code %d in send_message().",
		    mssgtype);
	return FALSE;
    }

    /*
     * The tri-level logic (YES, NO, FORM) of some of the parameters
     * is driving me absolutely bonkers.
     */

    /* load argument values into headers */
    if (given_to && *given_to)
	strfcpy(shdr->to, given_to, sizeof(shdr->to));
    if (given_cc && *given_cc)
	strfcpy(shdr->cc, given_cc, sizeof(shdr->cc));
    if (shdr->cc[0] != '\0')
	build_address(strip_commas(shdr->cc), shdr->expanded_cc);
    if (given_subject && *given_subject) {
	strfcpy(shdr->subject, given_subject, sizeof(shdr->subject));
	/* in interactive mode, display Subject: if provided */
	if (OPMODE_IS_INTERACTIVE(opmode))
	    display_subject(given_subject);
    }

    /* get To: field */
    if (!get_to(shdr->to, shdr->expanded_to, mssgtype))
	goto done;

    /* prompt for other header information */
    if (OPMODE_IS_INTERACTIVE(opmode) && mssgtype != SM_FORMRESP) {
	display_to(shdr->expanded_to);
	if (!get_subject(shdr->subject))
	    goto done;
	if (prompt_for_cc && !get_copies(shdr->cc, shdr->expanded_to,
		    shdr->expanded_cc, copy_msg))
	    goto done;
	MoveCursor(LINES,0);	/* so you know you've hit <return> ! */
    }

    dprint(3, (debugfile, "\nsend_msg() ready to mail...\n"));
    dprint(3, (debugfile, "to=\"%s\" expanded_to=\"%s\"\n",
		shdr->to, shdr->expanded_to));
    dprint(4, (debugfile, "subject=\"%s\"\n",
		shdr->subject));
    dprint(5, (debugfile, "cc=\"%s\" expanded_cc=\"%s\"\n",
		shdr->cc, shdr->expanded_cc));
    dprint(5, (debugfile, "bcc=\"%s\" expanded_bcc=\"%s\"\n",
		shdr->bcc, shdr->expanded_bcc));

    /* initialize default for saved copy */
    if (!auto_cc)
	fname_savecopy[0] = '\0';	/* no default saved copy */
    else if (!(save_by_name || save_by_alias))
	strcpy(fname_savecopy, "<");	/* save to sentmail */
    else if (!force_name)
	strcpy(fname_savecopy, "=?");	/* conditional save by 'to' */
    else
	strcpy(fname_savecopy, "=");	/* save by 'to' logname */

    /* generate name of file used to compose message */
    sprintf(fname_mssgbody, "%s%s%d", temp_dir, temp_file, getpid());

    /* grab the completed form as the response */
    if (copy_msg == FORM) {
	sprintf(tmpbuf, "%s%s%d", temp_dir, temp_form_file, getpid());
	dprint(4, (debugfile,
		"-- renaming existing file %s to file %s --\n",
		tmpbuf, fname_mssgbody));
	if (file_rename(tmpbuf, fname_mssgbody) < 0)
	    goto done;
	want_signature = FALSE;
	body_has_text = TRUE;
	goto message_is_prepared;
    }

    /* if there is a cancelled message layaround, see if he wants to use that */
    if (cancelled_msg && user_level > 0) {
	cancelled_msg = FALSE;
	if (recall_last_msg(fname_mssgbody, copy_msg)) {
	    want_signature = saved_want_sig;
	    body_has_text = TRUE;
	    goto message_is_prepared;
	}
    }

    /* create the file to hold the message body */
    if ((fp_mssgbody = file_open(fname_mssgbody, "w")) == NULL)
	goto done;

    /* copy the message from standard input */
    if (opmode == OPMODE_SEND_BATCH) {
	if (file_copy(stdin, fp_mssgbody,
		    "standard input", fname_mssgbody) < 0)
	    goto done;
	body_has_text = TRUE;
    }

    /* add any included file specified on the command line */
    if (included_file != NULL) {
	FILE *fp;
	if ((fp = file_open(included_file, "r")) == NULL)
	    goto done;
	if (file_copy(fp, fp_mssgbody, included_file, fname_mssgbody) < 0) {
	    (void) fclose(fp);
	    goto done;
	}
	if (file_close(fp, fname_mssgbody) < 0)
	    goto done;
	body_has_text = TRUE;
    }

    /* retrieve copy of desired message */
    if (copy_msg == YES) {
	i = CM_ATTRIBUTION;
	if (edit_msg)
	    i |= CM_DECODE;
	if (mssgtype != SM_FWDUNQUOTE)
	    i |= CM_PREFIX;
	if (mssgtype == SM_FORWARD)
            i |= CM_FORWARDING;
        if (mssgtype == SM_REPLY && noheader)
	    i |= CM_REMOVE_HEADER;
	copy_message(fp_mssgbody, curr_folder.curr_mssg, i);
	body_has_text = TRUE;
    }

message_is_prepared:

    /* cleanup on retries */
    if (send_attempts++ > 0) {
	if (!OPMODE_IS_INTERACTIVE(opmode))
	    goto done;
	if (fp_fullmssg != NULL) {
	    (void) fclose(fp_fullmssg);
	    fp_fullmssg = NULL;
	}
	if (fname_fullmssg != NULL) {
	    (void) unlink(fname_fullmssg);
	    (void) free((malloc_t)fname_fullmssg);
	    fname_fullmssg = NULL;
	}
	edit_msg = FALSE;
    }

    /* add sig now unless we are using builtin editor */
    if (want_signature &&
	    !streq(editor, "builtin") && !streq(editor, "none")) {
	if (fp_mssgbody == NULL) {
	    if ((fp_mssgbody = file_open(fname_mssgbody, "a")) == NULL)
		goto done;
	}

	if (append_sig(fp_mssgbody, shdr))
 	    body_has_text = TRUE;
	want_signature = FALSE;
    }

    /* close out the body so it can be edited */
    if (fp_mssgbody != NULL) {
	if (file_close(fp_mssgbody, fname_mssgbody) < 0)
	    goto done;
	fp_mssgbody = NULL;

    }

    /* ask the user to confirm transmission of the message */
    if (OPMODE_IS_INTERACTIVE(opmode)) {
	need_redraw = TRUE; /* we are about to trounce the display */
	if (edit_msg) {
	    s = (body_has_text ? (char *)NULL : editor);
	    if (edit_message(fname_mssgbody, shdr, s) < 0)
		goto done;
	    body_has_text = TRUE;
	}
	if (verify_transmission(fname_mssgbody, shdr, &attachments, &form,
		fname_savecopy) != 0)
	    goto done;
	body_has_text = TRUE; /* it had better by this point! */
    }

    /* format the raw form for transmission */
    if (form == YES && format_form(fname_mssgbody) < 1)
	goto message_is_prepared;

    /* tack the .signature onto the end if needed */
    if (want_signature) {
	if ((fp_mssgbody = file_open(fname_mssgbody, "a")) == NULL)
	    goto done;
	(void) append_sig(fp_mssgbody, shdr);
	if (file_close(fp_mssgbody, fname_mssgbody) < 0)
	    goto done;
	fp_mssgbody = NULL;
	want_signature = FALSE;
    }

    /* on retries, cleanup from last attempt */
    if (mssg_parts != NULL) {
	bodypart_destroy(mssg_parts);
	mssg_parts = NULL;
    }

    /* scan the message for attachments, determine MIME headers, and such */
    if ((mssg_parts = newpart_mssgtext(fname_mssgbody)) == NULL)
	goto message_is_prepared;

    /* dup any user-specified attachments onto the message attachments */
    if (attachments != NULL) {
	mp = NULL;
	while ((mp = multipart_next(attachments, mp)) != NULL) {
	    multipart_insertpart(mssg_parts->subparts,
			MULTIPART_TAIL(mssg_parts->subparts),
			MULTIPART_PART(mp), MP_ID_ATTACHMENT);
	}
    }

    /* ask about bounceback if the user wants us to.... */
    if (bounceback > 0 && uucp_hops(shdr->to) > bounceback
		&& copy_msg != FORM && verify_bounceback()) {
	if (shdr->expanded_bcc[0] != '\0')
	    strcat(shdr->expanded_bcc, ", ");
	strcat(shdr->expanded_bcc, bounce_off_remote(shdr->to));
    }

    remove_hostbang(shdr->expanded_to);
    remove_hostbang(shdr->expanded_cc);
    remove_hostbang(shdr->expanded_bcc);

    /* create temp file in which to build entire message */
    if ((fname_fullmssg = tempnam(temp_dir, "xmt.")) == NULL) {
	dprint(1, (debugfile, "couldn't make temp file nam! (mail)\n"));
	show_error(catgets(elm_msg_cat, ElmSet, ElmCouldNotMakeTemp,
		"Sorry - couldn't make temp file name."));
	goto done;
    }

    dprint(2, (debugfile, "Composition file='%s' and mail buffer='%s'\n",
	    fname_mssgbody, fname_fullmssg));
    dprint(2,(debugfile,"--\nTo: %s\nCc: %s\nBcc: %s\nSubject: %s\n---\n",
	    shdr->expanded_to, shdr->expanded_cc, shdr->expanded_bcc,
	    shdr->subject));

    /* generate full message (headers and body) to transmit */
    if ((fp_fullmssg = file_open(fname_fullmssg, "w")) == NULL) /* 7! w+? */
	goto done;
    if (sndhdr_output(fp_fullmssg, shdr, (form == YES), FALSE) < 0)
	goto message_is_prepared;
    if (form == YES)
	; /* Content-Type was generated by sndhdr_output() */
    else if (emitpart_hdr(fp_fullmssg, mssg_parts) < 0)
	goto message_is_prepared;
    putc('\n', fp_fullmssg);
    if (emitpart_body(fp_fullmssg, mssg_parts) < 0)
	goto message_is_prepared;
    if (file_close(fp_fullmssg, fname_fullmssg) < 0)
	goto done;
    fp_fullmssg = NULL;

    /* ensure we have envelope recipients before trying to mail */
    if ((shdr->expanded_to == NULL || *shdr->expanded_to == '\0') &&
	    (shdr->expanded_cc == NULL || *shdr->expanded_cc == '\0') &&
	    (shdr->expanded_bcc == NULL || *shdr->expanded_bcc == '\0')) {
	if (bytes(fname_mssgbody) > 0)
	    show_error(catgets(elm_msg_cat, ElmSet, ElmNoRecipientsKeptMessage,
		    "No recipients specified!  Message kept."));
	else
	    show_error(catgets(elm_msg_cat, ElmSet, ElmNoRecipients,
		    "No recipients specified!"));
	goto done;
    } else {
	/* mail off the message */
	show_error(catgets(elm_msg_cat, ElmSet, ElmSendingMail, "Sending mail..."));
	(void) build_mailer_command(bigbuf, fname_fullmssg,
		shdr->expanded_to, shdr->expanded_cc, shdr->expanded_bcc);
	if ((i = system_call(bigbuf, SY_ENV_SHELL)) != 0) {
	    show_error(catgets(elm_msg_cat, ElmSet, ElmMailerReturnedError,
		    "Mail failed!  [mailer exit status %d]"), i);
	    goto done;
	}
    }

    /* grab a copy if the user so desires... */
/*  #ifdef SAVE_ATTACHMENT_NAMES this if we decide to make this optional */
    /* append attachment names to message body to remind user what they did */
        if (attachments != NULL) {
      if ((fp_mssgbody = file_open(fname_mssgbody, "a")) == NULL)
          goto done;

      fprintf(fp_mssgbody, "\n");  /* Blank line to separate */

      mp = NULL;
      while ((mp = multipart_next(attachments, mp)) != NULL) {
          fprintf(fp_mssgbody, "(%s)\n",
                  mp->part->content_header[BP_CONT_DISPOSITION]);
      }

      if (file_close(fp_mssgbody, fname_mssgbody) < 0)
          goto done;
      fp_mssgbody = NULL;
    }
/* #endif */

    if (fname_savecopy[0] != '\0'
		&& !save_copy(fname_savecopy, fname_mssgbody, shdr, form)
		&& sleepmsg > 0) {
	FlushOutput();
	sleep(sleepmsg);
    }

    /* mark the "replied" status of the message */
    if (mssgtype == SM_REPLY && !(curr_folder.headers[curr_folder.curr_mssg-1]->status & REPLIED_TO)) {
	curr_folder.headers[curr_folder.curr_mssg-1]->status |= REPLIED_TO;
	curr_folder.headers[curr_folder.curr_mssg-1]->status_chgd = TRUE;
    }

    /* prevent unlink below -- the background command will delete it */
    if (fname_fullmssg != NULL)
    {
       free((malloc_t)fname_fullmssg);
       fname_fullmssg = NULL;
    }


    set_error(catgets(elm_msg_cat, ElmSet, ElmMailSent, "Mail sent!"));
    rc = 0;

done:
    if (fname_fullmssg != NULL) {
	(void) unlink(fname_fullmssg);
	free((malloc_t)fname_fullmssg);
    }
    if (fp_fullmssg != NULL)
	(void) fclose(fp_fullmssg);
    if (fp_mssgbody != NULL)
	(void) fclose(fp_mssgbody);
    if (attachments != NULL) {
	multipart_destroy(attachments);
	attachments = NULL;
    }
    if (mssg_parts != NULL) {
	bodypart_destroy(mssg_parts);
	mssg_parts = NULL;
    }
    sndhdr_destroy(shdr);

    if (rc == 0) {
	/*
	 * Message sent ok -- blow away the body.  Beside crapping up
	 * the tmp directory, it could contain the cleartext to an
	 * encrypted message.
	 */
	(void) unlink(fname_mssgbody);
	cancelled_msg = FALSE;
    } else {
	/*
	 * The guy can try to recall this message next time.
	 */
	cancelled_msg = (copy_msg != FORM
		&& access(fname_mssgbody, EDIT_ACCESS) == 0
		&& bytes(fname_mssgbody) > 0);
	saved_want_sig = want_signature;
    }

    return need_redraw;
}


void display_to(char *address)
{
    char *ap;
    char *dp, dispbuf[SLEN], ret_addr[SLEN], ret_name[SLEN];
    int to_line, to_col, first, displen, dispsiz, len;

    assert(OPMODE_IS_INTERACTIVE(opmode));
    if (OPMODE_IS_READMODE(opmode)) {
	to_line = LINES-3;
	to_col = 20;
    } else {
	to_line = 3;
	to_col = 0;
    }

    (void) strcpy(dispbuf, "To: ");
    displen = 4;
    dp = dispbuf + displen;
    dispsiz = ((COLS-2) - to_col) + 1;
    first = TRUE;

    ap = address;
    while (displen < dispsiz && *ap != '\0' && parse_arpa_mailbox(ap,
		ret_addr, sizeof(ret_addr), ret_name, sizeof(ret_name),
		&ap) == 0) {
	if (!first) {
	    (void) strcpy(dp, ", ");
	    dp += 2;
	    displen += 2;
	}
	first = FALSE;

	if (*ret_name == '\0')
	    (void) strcpy(dp, ret_addr);
	else if (names_only)
	    (void) strcpy(dp, ret_name);
	else
	    sprintf(dp, "%s (%s)", ret_addr, ret_name);
	len = strlen(dp);
	displen += len;
	dp += len;
    }

    if (displen > dispsiz) {
	(void) strcpy(dispbuf+dispsiz-4, " ...");
	displen = dispsiz;
    }

    MoveCursor(to_line, to_col);
    CleartoEOLN();
    if (to_col > 0) {
	/* fixup column position to be right justified */
	MoveCursor(to_line, COLS - (displen+1));
    }
    PutLine(-1, -1, dispbuf);
}


int get_to(char *to_field, char *address, int mssgtype)
{
    char *prompt;
    int line;

    if (*to_field == '\0') {
	line = (OPMODE_IS_READMODE(opmode)) ? LINES - 2 : 3;

	if (user_level < 2) {
	    prompt = catgets(elm_msg_cat, ElmSet,
			ElmSendTheMessageTo, "Send the message to: ");
	} else {
	    prompt = catgets(elm_msg_cat, ElmSet, ElmTo, "To: ");
	}
	PutLine(line, 0, prompt);
	if (enter_string(to_field, LONG_STRING, -1, -1, ESTR_REPLACE) < 0
		    || *to_field == '\0') {
	    ClearLine(line);
	    return FALSE;
	}
    }

    if (mssgtype == SM_REPLY || mssgtype == SM_FORMRESP) {
	/* use the literal address values */
	(void) strcpy(address, to_field);
    } else {
	/* perform alias expansion of addresses */
	(void) build_address(strip_commas(to_field), address);
	if (*address == '\0') {	/* bad address!  Removed!! */
	    ClearLine(line);
	    return FALSE;
	}
    }

    return TRUE;
}


static void display_subject(const char *subject_field)
{
    int prompt_line;

    assert(OPMODE_IS_INTERACTIVE(opmode));
    prompt_line = (OPMODE_IS_READMODE(opmode) ? LINES-2 : 4);

    if (user_level == 0) {
	PutLine(prompt_line, 0,
		catgets(elm_msg_cat, ElmSet, ElmSubjectOfMessage,
		"Subject of message: "));
    }
    else
	PutLine(prompt_line, 0,
		catgets(elm_msg_cat, ElmSet, ElmSubject, "Subject: "));

    PutLine(-1, -1, subject_field);
}

static int get_subject(char *subject_field)
{
	char *msg;

	/** get the subject and return non-zero if all okay... **/
	int prompt_line;

	assert(OPMODE_IS_INTERACTIVE(opmode));
	prompt_line = (OPMODE_IS_READMODE(opmode) ? LINES-2 : 4);

	if (user_level == 0) {
	  PutLine(prompt_line,0, catgets(elm_msg_cat, ElmSet, ElmSubjectOfMessage,
		"Subject of message: "));
	}
	else
	  PutLine(prompt_line,0, catgets(elm_msg_cat, ElmSet, ElmSubject, "Subject: "));

	if (enter_string(subject_field, SLEN, -1, -1, ESTR_UPDATE) < 0) {
	  MoveCursor(prompt_line,0);
	  CleartoEOLN();
	  show_error(catgets(elm_msg_cat, ElmSet, ElmMailNotSent, "Mail not sent."));
	  return FALSE;
	}

	if (strlen(subject_field) == 0) {	/* zero length subject?? */
	  msg = catgets(elm_msg_cat, ElmSet, ElmNoSubjectContinue,
	    "No subject - Continue with message?");

	  if (!enter_yn(msg, FALSE, prompt_line, FALSE)) {
	    ClearLine(prompt_line);
	    show_error(catgets(elm_msg_cat, ElmSet, ElmMailNotSend, "Mail not sent."));
	    return FALSE;
	  }
	  PutLine(prompt_line,0,"Subject: <none>");
	  CleartoEOLN();
	}

	return TRUE;
}

static int get_copies(char *cc_field, char *address, char *addressII,
		      int copy_message)
{
	/** Get the list of people that should be cc'd, returning ZERO if
	    any problems arise.  Address and AddressII are for expanding
	    the aliases out after entry!
	    If copy-message, that means that we're going to have to invoke
	    a screen editor, so we'll need to delay after displaying the
	    possibly rewritten Cc: line...
	**/
	int prompt_line;

	assert(OPMODE_IS_INTERACTIVE(opmode));
	prompt_line = (OPMODE_IS_READMODE(opmode) ? LINES-1 : 5);
	PutLine(prompt_line,0,
		catgets(elm_msg_cat, ElmSet, ElmCopiesTo, "Copies to: "));

	if (enter_string(cc_field, VERY_LONG_STRING, -1, -1, ESTR_REPLACE) < 0) {
	  ClearLine(prompt_line-1);
	  ClearLine(prompt_line);

	  show_error(catgets(elm_msg_cat, ElmSet, ElmMailNotSend, "Mail not sent."));
	  return FALSE;
	}

	/** The following test is that if the build_address routine had
	    reason to rewrite the entry given, then, if we're mailing only
	    print the new Cc line below the old one.  If we're not, then
	    assume we're in screen mode and replace the incorrect entry on
	    the line above where we are (e.g. where we originally prompted
	    for the Cc: field).
	**/

	if (build_address(strip_commas(cc_field), addressII)) {
	  PutLine(prompt_line, 11, "%s", addressII);
	  if ((strcmp(editor, "builtin") != 0 && strcmp(editor, "none") != 0)
	      || copy_message)
	    if (sleepmsg > 0) {
		FlushOutput();
		sleep(sleepmsg);
	    }
	}

	if (strlen(address) + strlen(addressII) > VERY_LONG_STRING) {
	  dprint(2, (debugfile,
		"String length of \"To:\" + \"Cc\" too long! (get_copies)\n"));
	  show_error(catgets(elm_msg_cat, ElmSet, ElmTooManyPeople, "Too many people. Copies ignored."));
	  if (sleepmsg > 0)
		sleep(sleepmsg);
	  cc_field[0] = '\0';
	}

	return TRUE;
}

static int verify_copy_msg(void)
{
	char *msg;

	if (!ask_reply_copy)
	    return reply_copy;

	if (user_level < 2) {
	    msg = catgets(elm_msg_cat, ElmSet, ElmCopyMessageIntoReplyYN,
		    "Copy message into reply?");
	} else {
	    msg = catgets(elm_msg_cat, ElmSet, ElmCopyMessageYN,
		    "Copy message?");
	}
	return enter_yn(msg, reply_copy, LINES-3, FALSE);
}

static int recall_last_msg(const char *filename, int copy_msg)
{
	char *msg;

	if (access(filename, EDIT_ACCESS) != 0)
	  return FALSE;

	if (copy_msg)
	    msg = catgets(elm_msg_cat, ElmSet, ElmRecallLastInstead,
		"Recall last kept message instead?");
	else
	    msg = catgets(elm_msg_cat, ElmSet, ElmRecallLastKept,
		"Recall last kept message?");

	ClearLine(LINES-1);
	return enter_yn(msg, -1, LINES-1, FALSE);

	/*NOTREACHED*/
}

static int append_sig(FILE *fp_mssg, SEND_HEADER *shdr)
{
    /* Append the correct signature file to file.  Return TRUE if
    we append anything.  */

    /* Look at the to and cc list to determine which one to use */

    /* We could check the bcc list too, but we don't want people to
    know about bcc, even indirectly */

    /* Some people claim that  user@anything.same_domain should be
    considered local.  Since it's not the same machine, better be
    safe and use the remote sig (presumably it has more complete
    information).  You can't necessarily finger someone in the
    same domain. */

    FILE *fp_sig;
    char filename2[SLEN], *sig;

    if (!OPMODE_IS_INTERACTIVE(opmode))
	return FALSE;	/* FOO - why are we doing this?? */
    if (local_signature[0] == '\0' && remote_signature[0] == '\0')
	return FALSE;

    if (strchr(shdr->expanded_to, '!') || strchr(shdr->expanded_cc,'!')) {
	sig = remote_signature;		/* ! always means remote */
    } else {
	/* check each @ for @thissite.domain */
	/* if any one is different than this, then use remote sig */
	int len;
	char *ptr;
	char sitename[SLEN];
	sprintf(sitename, "@%s", host_fullname);
	len = strlen(sitename);
	sig = local_signature;
	for (ptr = strchr(shdr->expanded_to,'@'); ptr;  /* check To: list */
		    ptr = strchr(ptr+1,'@')) {
	    if (strncasecmp(ptr,sitename,len) != 0
			|| (*(ptr+len) != ',' && *(ptr+len) != 0
			&& *(ptr+len) != ' ')) {
		sig = remote_signature;
		break;
	    }
	}
	if (sig == local_signature) {		   /* still local? */
	    for (ptr = strchr(shdr->expanded_cc,'@'); ptr;   /* check Cc: */
			ptr = strchr(ptr+1,'@')) {
		if (strncasecmp(ptr,sitename,len) != 0
			    || (*(ptr+len) != ',' && *(ptr+len) != 0
			    && *(ptr+len) != ' ')) {
		    sig = remote_signature;
		    break;
		}
	    }
	}
    }

    switch (sig[0]) {
    case '\0':
	return FALSE;
    case '/':
	(void) strcpy(filename2, sig);
	break;
    default:
	sprintf(filename2, "%s/%s", user_home, sig);
	break;
    }

    /*
     * FOO - this displays an error if the .sig doesn't exist.
     * Is that really what we want to do?
     */
    if ((fp_sig = file_open(filename2, "r")) == NULL)
	return FALSE;
    if (sig_dashes)
	fputs("\n-- \n", fp_mssg);
    if (file_copy(fp_sig, fp_mssg, filename2, "message file") < 0) {
	(void) fclose(fp_sig);
	return FALSE;
    }
    if (file_close(fp_sig, filename2) < 0)
	return FALSE;
    return TRUE;
}

static int verify_bounceback(void)
{
    char *msg;

    ClearLine(LINES);
    msg = catgets(elm_msg_cat, ElmSet, ElmBounceOffRemote,
		"\"Bounce\" a copy off the remote machine?");
    return enter_yn(msg, FALSE, LINES, FALSE);
}


/*
 * remove_hostbang - Given an expanded list of addresses, remove all
 * occurrences of "thishost!" at the beginning of addresses.
 * This hack is useful in itself, but it is required now because of the
 * kludge disallowing alias expansion on return addresses.
 */
static void remove_hostbang(char *addrs)
{
    int hlen, flen;
    char *src, *dest;

    if ((hlen = strlen(host_name)) < 1)
	return;
    flen = strlen(host_fullname);
    src = dest = addrs;

    while (*src != '\0') {
	if (strncmp(src, host_name, hlen) == 0 && src[hlen] == '!')
	    src += hlen+1;
	if (strncmp(src, host_fullname, flen) == 0 && src[flen] == '!')
	    src += flen+1;
	while (*src != '\0' && !isspace(*src))
	    *dest++ = *src++;
	if (isspace(*src)) {
	    while (isspace(*src))
		++src;
	    *dest++ = ' ';
	}
    }

    *dest = '\0';
}


#define VT_REDRAW_NONE		0
#define VT_REDRAW_HEADER	(1<<0)
#define VT_REDRAW_FOOTER	(1<<1)
#define VT_REDRAW_ALL		(~0)

/*
 * verify_transmission() - Ask the user to confirm transmission of the
 * message.  Returns 0 to send it, -1 to forget it.
 */
/* const char *filename;	/\* pathname to mail mssg composition file	*\/ */
/* SEND_HEADER *shdr;	/\* headers for the message being sent		*\/ */
/* SEND_MULTIPART **attachments_p; /\* attachments to message being sent	*\/ */
/* int  *form_p;		/\* pointer to form message state		*\/ */
/* char *copy_file;	/\* pointer to buffer holding copy file name	*\/ */

static int verify_transmission(const char *filename, SEND_HEADER *shdr,
			       SEND_MULTIPART **attachments_p, int *form_p,
			       char *copy_file)
{
    char *prompt_mssg;		/* message to display prompting for cmd	*/
    char prompt_menu1[SLEN];	/* menu of available commands		*/
    char prompt_menu2[SLEN];	/* menu of available commands		*/
    int bad_cmd;		/* set TRUE to bitch about user's entry	*/
    int prev_form;		/* "*form_p" value last time thru loop	*/
    int do_redraw;		/* portions of display to update	*/
    int cmd;			/* command to perform			*/
    int max_hdrline;		/* bottommost line used in hdr display	*/
    char lbuf[VERY_LONG_STRING];
    int curr_line, curr_col;

    bad_cmd = FALSE;		/* nothing to complain about yet	*/
    prev_form = *form_p + 1;	/* force build of prompt strings	*/
    do_redraw = VT_REDRAW_ALL;	/* force screen display first time	*/

    for (;;) {

	/* see if the prompts need to be built */
	if (prev_form == *form_p) {
	    ; /* not changed - no need to rebuild the strings */
	} else {
	    if (user_level == 0) {
		prompt_mssg = catgets(elm_msg_cat, ElmSet,
			    ElmVfyPromptSendTheMsg,
"Send the message now? y");
		strcpy(prompt_menu1, catgets(elm_msg_cat, ElmSet,
			    ElmVfyMenu1User0,
"Select letter of header to edit, 'e' to edit the message,"));
		strcpy(prompt_menu2, catgets(elm_msg_cat, ElmSet,
			    ElmVfyMenu2User0,
"'a' to make attachments, 'y' to send message, or 'n' to cancel."));
	    } else {
		prompt_mssg = catgets(elm_msg_cat, ElmSet,
			    ElmVfyPromptSendMessage,
			    "Send message now? y");
		strcpy(prompt_menu1, catgets(elm_msg_cat, ElmSet,
			    ElmVfySelectLetter,
			    "Select letter of header to edit, "));
		*prompt_menu2 = '\0';
		switch (*form_p) {
		case PREFORMATTED:
		    break;
		case YES:
		    strcat(prompt_menu1, catgets(elm_msg_cat, ElmSet,
				ElmVfyMenuEditForm, "e)dit form,"));
		    break;
		case MAYBE:
		    strcat(prompt_menu1, catgets(elm_msg_cat, ElmSet,
				ElmVfyMenuEditMake, "e)dit msg, m)ake form,"));
		    break;
		default:
		    strcat(prompt_menu1, catgets(elm_msg_cat, ElmSet,
				ElmVfyMenuEditMsg, "e)dit message,"));
		    break;
		}
		strcat(prompt_menu2, catgets(elm_msg_cat, ElmSet,
			    ElmVfyMenuVfyCpy,
			    "all h)eaders, a)ttachments, co(p)y, "));
#ifdef ISPELL
		strcat(prompt_menu2, catgets(elm_msg_cat, ElmSet,
			    ElmVfyMenuIspell,
			    "i)spell, "));
#endif
#ifdef ALLOW_SUBSHELL
		strcat(prompt_menu2, catgets(elm_msg_cat, ElmSet,
			    ElmVfyMenuShell,
			    "!)shell, "));
#endif
		strcat(prompt_menu2, catgets(elm_msg_cat, ElmSet,
			    ElmVfyMenuForget,
			    "f)orget, or:"));
	    }
	    prev_form = *form_p;
	    do_redraw |= VT_REDRAW_FOOTER;
	}

	/* complain if last entry was bad */
	if (bad_cmd) {
	    PutLine(-1, -1, "\07??");	/* beep! */
	    if (sleepmsg > 0) {
		FlushOutput();
		sleep((sleepmsg + 1) / 2);
	    }
	    bad_cmd = FALSE;
	}

	/* update the screen */
	if (do_redraw == VT_REDRAW_ALL)
	    ClearScreen();
	if (do_redraw & VT_REDRAW_HEADER) {
	    if (do_redraw != VT_REDRAW_ALL) {
		MoveCursor(0, 0);
		CleartoEOLN();
	    }
	    CenterLine(0, catgets(elm_msg_cat, ElmSet, ElmVfyTitle,
			"Send Message"));
	}
	if (do_redraw == VT_REDRAW_ALL)
	    max_hdrline = show_msg_headers(shdr, "tcbsr");
	if (do_redraw & VT_REDRAW_FOOTER) {
	    if (do_redraw != VT_REDRAW_ALL) {
		MoveCursor(max_hdrline+1, 0);
		CleartoEOS();
	    }
	    CenterLine(LINES-5, prompt_menu1);
	    CenterLine(LINES-4, prompt_menu2);
	    show_last_error();
	}
	do_redraw = VT_REDRAW_NONE;

	/* prompt for command */
	PutLine(LINES-2, 0, prompt_mssg);
	GetCursorPos(&curr_line, &curr_col);
	curr_col--; /* backspace over default answer */
	CleartoEOLN();
	MoveCursor(curr_line, curr_col);
	if ((cmd = GetKey(0)) == KEY_REDRAW) {
	    do_redraw = VT_REDRAW_ALL;
	    continue;
	}
	clear_error();

	/* handle command */
	switch (cmd) {

	case 'y':
	case '\n':
	case '\r':
	    PutLine(-1, -1, "Send");
	    return 0;
	    /*NOTREACHED*/

	case 'n':
	case 'f':		/* old menu used "f)orget" */
	    PutLine(-1, -1, "Forget");
	    if (bytes(filename) <= 0) {
		; /* forget about empty files */
	    } else if (!OPMODE_IS_READMODE(opmode)) {
		sprintf(lbuf, "%s/%s", user_home, dead_letter);
		(void) save_mssg(lbuf, filename, shdr, *form_p);
	    } else if (user_level > 0) {
		set_error(catgets(elm_msg_cat, ElmSet, ElmVfyMessageKept,
		    "Message kept.  Can be restored at next f)orward, m)ail or r)eply."));
	    }
	    return -1;
	    /*NOTREACHED*/

	case 'a':
	    if (attachment_menu(attachments_p) < 0)
		return -1;
	    do_redraw = VT_REDRAW_ALL;
	    break;

	case 'p':
	    PutLine(-1, -1, "Copy file");
	    do_redraw |= (name_copy_file(copy_file)
			? VT_REDRAW_ALL : VT_REDRAW_FOOTER);
	    break;

	case 'e':
	    PutLine(-1, -1, "Edit");
	    if (*form_p == PREFORMATTED) {
		bad_cmd = TRUE;
	    } else {
		if (*form_p == YES) {
		    *form_p = MAYBE;
		    show_error(catgets(elm_msg_cat, ElmSet, ElmVfyFormToPlaintext,
"Converting form back to plaintext.  Do \"m)ake form\" to re-make form."));
		    if (sleepmsg > 0)
			sleep(sleepmsg);
		}
		if (edit_message(filename, shdr, (char *)NULL) < 0)
		    return -1;
		do_redraw = VT_REDRAW_ALL;
	    }
	    break;

	case 'h':
	    PutLine(-1, -1, "Headers");
	    edit_headers(shdr);
	    do_redraw |= (VT_REDRAW_HEADER|VT_REDRAW_FOOTER);
	    break;

	case 'm':
	    if (*form_p != MAYBE) {
		bad_cmd = TRUE;
	    } else {
		switch (check_form_file(filename)) {
		case -1:
		    /* couldn't open file??? */
		    return -1;
		case 0:
		    PutLine(-1, -1, catgets(elm_msg_cat, ElmSet,
			ElmVfyNoFieldsInForm, "No fields in form!\007"));
		    if (sleepmsg > 0) {
			FlushOutput();
			sleep(sleepmsg);
		    }
		    break;
		default:
		    /* looks like a good form */
		    *form_p = YES;
		    break;
		}
	    }
	    break;

#ifdef ISPELL
	case 'i':
	    PutLine(-1, -1, "Ispell");
	    if (*form_p == PREFORMATTED) {
		bad_cmd = TRUE;
	    } else {
		if (*form_p == YES) {
		    *form_p = MAYBE;
		    show_error(catgets(elm_msg_cat, ElmSet, ElmVfyFormToPlaintext,
"Converting form back to plaintext.  Do \"m)ake form\" to re-make form."));
		    if (sleepmsg > 0)
			sleep(sleepmsg);
		}
		sprintf(lbuf, "%s %s %s",
		    ISPELL_PATH, ISPELL_OPTIONS, filename);
		system_call(lbuf, SY_COOKED|SY_ENAB_SIGHUP);
		do_redraw = VT_REDRAW_ALL;
	    }
	    break;
#endif

#ifdef ALLOW_SUBSHELL
	case '!':
	    if (subshell() != 0)
		do_redraw = VT_REDRAW_ALL;
	    break;
#endif

	/* these letters should match those given to show_msg_headers() */
	/* And we allow uppercase also, since it's what's parenthesized
	 * on the display. */
	case 't': case 'T':	/* T)o */
	case 'c': case 'C':	/* C)c */
	case 'b': case 'B':	/* B)cc */
	case 's': case 'S':	/* S)ubject */
	case 'r': case 'R':	/* R)eply-To */
	    MoveCursor(LINES-5, 0);
	    CleartoEOS();
	    if (!edit_header_char(shdr, tolower(cmd)))
		bad_cmd = TRUE;
	    do_redraw |= VT_REDRAW_FOOTER;
	    break;

	case ctrl('L'):
	    do_redraw = VT_REDRAW_ALL;
	    break;

	default:
	    bad_cmd = TRUE;
	    break;

	}

    }

}




/*
 * Construct a command string to mail a message.
 * This sanitizes the recipient lists as a side effect.
 */
char *build_mailer_command(char *cmdbuf, const char *fname_mssg,
			   char *to_recip, char *cc_recip, char *bcc_recip)
{
    char recipients[VERY_LONG_STRING], mflags[SLEN], *cp;

    if (streq(sendmail, mailer)) {
	strcpy(mflags, (sendmail_verbose ? smflagsv : smflags));
	if (metoo)
	    strcat(mflags, smflagmt);
    } else if (streq(submitmail, mailer)) {
	strcpy(mflags, submitflags_s);
    } else if (streq(execmail, mailer)) {
	strcpy(mflags, (sendmail_verbose ? emflagsv : emflags));
	if (metoo)
	    strcat(mflags, emflagmt);
    } else {
	mflags[0] ='\0';
    }

    if (streq(submitmail, mailer)) {

	strcpy(mflags, submitflags_s);
	recipients[0] = '\0';

    } else {

	if (streq(sendmail, mailer)) {
	    strcpy(mflags, (sendmail_verbose ? smflagsv : smflags));
	    if (metoo)
		strcat(mflags, smflagmt);
	} else if (streq(execmail, mailer)) {
	    strcpy(mflags, (sendmail_verbose ? emflagsv : emflags));
	    if (metoo)
		strcat(mflags, emflagmt);
	} else {
	    mflags[0] ='\0';
	}

	*(cp = recipients) = '\0';
	if (to_recip != NULL && *to_recip != '\0') {
	    *cp++ = ' ';
	    quote_args(cp, strip_commas(strip_parens(to_recip)));
	    cp += strlen(cp);
	}
	if (cc_recip != NULL && *cc_recip != '\0') {
	    *cp++ = ' ';
	    quote_args(cp, strip_commas(strip_parens(cc_recip)));
	    cp += strlen(cp);
	}
	if (bcc_recip != NULL && *bcc_recip != '\0') {
	    *cp++ = ' ';
	    quote_args(cp, strip_commas(strip_parens(bcc_recip)));
	    cp += strlen(cp);
	}

    }

    sprintf(cmdbuf, "(%s %s%s <%s;%s %s)&",
	mailer, mflags, recipients, fname_mssg, remove_cmd, fname_mssg);
    return cmdbuf;
}
