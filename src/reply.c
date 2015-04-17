
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
 * $Log: reply.c,v $
 * Revision 1.7  1996/05/09  15:51:24  wfp5p
 * Alpha 10
 *
 * Revision 1.6  1996/03/14  17:29:47  wfp5p
 * Alpha 9
 *
 * Revision 1.5  1996/03/13  14:38:02  wfp5p
 * Alpha 9 before Chip's big changes
 *
 * Revision 1.4  1995/09/29  17:42:24  wfp5p
 * Alpha 8 (Chip's big changes)
 *
 * Revision 1.3  1995/07/18  19:00:05  wfp5p
 * Alpha 6
 *
 * Revision 1.2  1995/06/08  13:41:06  wfp5p
 * A few mostly cosmetic changes
 *
 * Revision 1.1.1.1  1995/04/19  20:38:38  wfp5p
 * Initial import of elm 2.4 PL0 as base for elm 2.5.
 *
 ******************************************************************************/

/*** routine allows replying to the sender of the current message

***/

#include "elm_defs.h"
#include "elm_globals.h"
#include "s_elm.h"

/** Note that this routine generates automatic header information
    for the subject and (obviously) to lines, but that these can
    be altered while in the editor composing the reply message!
**/

char *get_token();


/* Determine the subject to use for a reply.  */
void get_reply_subj(char *out_subj, char *in_subj, char *dflt_subj)
{
	if ( *in_subj == '\0' ) {
	  strcpy(out_subj,dflt_subj);
	  return;
	}
	if (
	  ( in_subj[0] == 'r' || in_subj[0] == 'R' ) &&
	  ( in_subj[1] == 'e' || in_subj[1] == 'E' ) &&
	  ( in_subj[2] == ':' )
	) {
	  for ( in_subj += 3 ; whitespace(*in_subj) ; ++in_subj ) ;
	}
	strcat( strcpy( out_subj, "Re: " ), in_subj);
}

int optimize_and_add(char *new_address, char *full_address)
{
	/** This routine will add the new address to the list of addresses
	    in the full address buffer IFF it doesn't already occur.  It
	    will also try to fix dumb hops if possible, specifically hops
	    of the form ...a!b...!a... and hops of the form a@b@b etc
	**/

	register int len, host_count = 0, i;
	char     hosts[MAX_HOPS][SLEN];	/* array of machine names */
	char     *host, *addrptr;

	if (in_list(full_address, new_address))
	  return(1);	/* duplicate address */

	/** optimize **/
	/*  break down into a list of machine names, checking as we go along */

	addrptr = (char *) new_address;

	while ((host = get_token(addrptr, "!", 1)) != NULL) {
	  for (i = 0; i < host_count && !streq(hosts[i], host); i++)
	      ;

	  if (i == host_count) {
	    strcpy(hosts[host_count++], host);
	    if (host_count == MAX_HOPS) {
	       dprint(2, (debugfile,
              "Error: hit max_hops limit trying to build return address (%s)\n",
		      "optimize_and_add"));
	       error(catgets(elm_msg_cat, ElmSet, ElmBuildRAHitMaxHops,
		"Can't build return address. Hit MAX_HOPS limit!"));
	       return(1);
	    }
	  }
	  else
	    host_count = i + 1;
	  addrptr = NULL;
	}

	/** fix the ARPA addresses, if needed **/

	if (qchloc(hosts[host_count-1], '@') > -1)
	  fix_arpa_address(hosts[host_count-1]);

	/** rebuild the address.. **/

	new_address[0] = '\0';

	for (i = 0; i < host_count; i++)
	  sprintf(new_address + strlen(new_address), "%s%s",
	          new_address[0] == '\0'? "" : "!",
	          hosts[i]);

	if (full_address[0] == '\0')
	  strcpy(full_address, new_address);
	else {
	  len = strlen(full_address);
	   if (len+strlen(new_address)+3 >= VERY_LONG_STRING)
	      return(0);
	  full_address[len  ] = ',';
	  full_address[len+1] = ' ';
	  full_address[len+2] = '\0';
	  strcat(full_address, new_address);
	}

	return(0);
}

void get_and_expand_everyone(char *return_address, char *full_address)
{
	/** Read the current message, extracting addresses from the 'To:'
	    and 'Cc:' lines.   As each address is taken, ensure that it
	    isn't to the author of the message NOR to us.  If neither,
	    prepend with current return address and append to the
	    'full_address' string.
	**/

    char ret_address[SLEN], buf[VERY_LONG_STRING], new_address[SLEN],
	 address[SLEN], comment[SLEN], next_line[SLEN], *acurr, *anext;
    int  lines, line_pending = 0, line_len, err;

    /*
     * The ret_address[] is used to hold a printf(3) format to route an
     * address through the originating host.  Initialize it as undefined
     * here, and we will fill it in if and when needed.
     */
    ret_address[0] = '\0';

    /** First off, get to the first line of the message desired **/

    if (fseek(curr_folder.fp, curr_folder.headers[curr_folder.curr_mssg-1]->offset, 0) == -1) {
	err = errno;
	dprint(1,(debugfile,"Error: seek %ld resulted in errno %s (%s)\n",
		 curr_folder.headers[curr_folder.curr_mssg-1]->offset,
		 strerror(err),
		 "get_and_expand_everyone"));
	error2(catgets(elm_msg_cat, ElmSet, ElmSeekFailedFile,
		"ELM [seek] couldn't read %d bytes into file (%s)."),
		curr_folder.headers[curr_folder.curr_mssg-1]->offset,
		strerror(err));
	return;
    }

    /** okay!  Now we're there!  **/

    /** now let's parse the actual message! **/

    for (lines = curr_folder.headers[curr_folder.curr_mssg-1]->lines;;) {

      /* read in another line if required - break out if end of mbox reached */
      if (!line_pending && (line_len = mail_gets(buf, SLEN, curr_folder.fp)) == 0)
	  return;
      line_pending = 0;

      /* break out at end of header or start of next message */
      if ( line_len < 2 )
	return;
      if (buf[line_len - 1] == '\n')
	lines--;
      if (lines <= 0)
	return;

      /* we only want lines with addresses */
      if (!header_cmp(buf, "To", (char *)NULL)
		&& !header_cmp(buf, "cc", (char *)NULL))
	continue;

      /* extract the addresses from this line and possible continuation lines */
      next_line[0] = '\0';

      /* read in another line - continuation lines start with whitespace */
      while ((line_len = mail_gets(next_line, SLEN, curr_folder.fp)) != 0 &&
	      whitespace(next_line[0])) {
	no_ret(buf);
	 if (strlen(buf)+strlen(next_line)+1 < VERY_LONG_STRING)
	    strcat(buf, next_line); /* Append continuation line */

	if (next_line[line_len - 1] == '\n')
	  lines--;
	next_line[0] = '\0';
      }

      dprint(2,(debugfile,"> %s\n",buf));

      /* locate the start of the header data */
      if ((acurr = strchr(buf, ':')) == NULL) {
	/* eh???  this cannot happen */
	dprint(1,(debugfile,
	  "Error: get_and_expand_everyone() failed to recognize valid header"));
	goto next_header;
      }
      ++acurr;

      /* go through all of the addresses in the line */
      while (*acurr != '\0') {

	/* extract the next address */
	if (parse_arpa_mailbox(acurr, address, sizeof(address),
			comment, sizeof(comment), &anext) != 0) {
	  dprint(2,(debugfile,
	      "get_and_expand_everyone() - ignore bad addr \"%s\"\n", acurr));
	  acurr = anext;
	  continue;
	}
	acurr = anext;

	/* ignore if this is us or the message sender */
	if (!okay_address(address, return_address))
	  continue;

	/**
	    Some mailers can emit unqualified addresses in the
	    headers, e.g. a Cc to a local user might appear as
	    just "user" and not "user@dom.ain".  We do a real
	    low-rent check here.  If it looks like a domain
	    address then we will pass it through.  Otherwise we
	    send it back through the originating host for routing.
	**/
	if (qchloc(address, '@') >= 0) {
	  if (optimize_and_add(address, full_address) == 0 &&
	      comment[0] != '\0')
	     if (strlen(full_address)+strlen(comment)+4 < VERY_LONG_STRING)
	        sprintf(full_address + strlen(full_address), " (%s)", comment);
	} else {
	  if (ret_address[0] == '\0')
	    translate_return(return_address, ret_address);
	  sprintf(new_address, ret_address, address);
	  if (optimize_and_add(new_address, full_address) == 0 &&
	      comment[0] != '\0')
	     if (strlen(full_address)+strlen(comment)+4 < VERY_LONG_STRING)
                 sprintf(full_address + strlen(full_address), " (%s)", comment);
	}

      }

next_header:
      if (next_line[0] != '\0') {
	strcpy(buf, next_line);
	line_pending++;
      }
    }
}

int reply(void)
{
    /** Reply to the current message.  Returns non-zero iff
	the screen has to be rewritten. **/

    char return_address[SLEN], subject[SLEN];

    if (curr_folder.headers[curr_folder.curr_mssg-1]->status & FORM_LETTER) {

	if (get_return(return_address, curr_folder.curr_mssg-1)) {
	  strcpy(subject,
		  curr_folder.headers[curr_folder.curr_mssg-1]->subject);
	} else {
	  get_reply_subj(subject,
		  curr_folder.headers[curr_folder.curr_mssg-1]->subject,
		  catgets(elm_msg_cat, ElmSet, ElmFilledInForm,
		  "Filled in form"));
	}
	return mail_filled_in_form(return_address, subject);

    } else {

	if (get_return(return_address, curr_folder.curr_mssg-1)) {
	  strcpy(subject,
		  curr_folder.headers[curr_folder.curr_mssg-1]->subject);
	} else {
	  get_reply_subj(subject,
		  curr_folder.headers[curr_folder.curr_mssg-1]->subject,
		  catgets(elm_msg_cat, ElmSet, ElmReYourMail,
		  "Re: your mail"));
	}
	return send_message(return_address, (char *)NULL, subject, SM_REPLY);

    }

    /*NOTREACHED*/
}

int reply_to_everyone(void)
{
	/** Reply to everyone who received the current message.
	    This includes other people in the 'To:' line and people
	    in the 'Cc:' line too.  Returns non-zero iff the screen
            has to be rewritten. **/

	char return_address[SLEN], subject[SLEN];
	char full_address[VERY_LONG_STRING];

	get_return(return_address, curr_folder.curr_mssg-1);

	full_address[0] = '\0';			/* no copies yet    */
	get_and_expand_everyone(return_address, full_address);
	dprint(2,(debugfile,
		"reply_to_everyone() - return_addr=\"%s\" full_addr=\"%s\"\n",
		return_address,full_address));

	get_reply_subj( subject,
		  curr_folder.headers[curr_folder.curr_mssg-1]->subject,
		  catgets(elm_msg_cat, ElmSet, ElmReYourMail, "Re: your mail"));

	return send_message(return_address, full_address, subject, SM_REPLY);
}

int forward(void)
{
    /** Forward the current message.
	Return TRUE if the main part of the screen has been changed
	(useful for knowing whether a redraw is needed.
    **/

    char subject[SLEN], *msg;
    int msgtype, len;
    struct header_rec *hdr = curr_folder.headers[curr_folder.curr_mssg-1];

    if (hdr->status & FORM_LETTER) {
	error(catgets(elm_msg_cat, ElmSet, ElmFormsCannotForward, /*(*/
"Forms cannot be forwarded.  You might want to try \"b)ounce\" instead."));
	return FALSE;
    }

    msg = catgets(elm_msg_cat, ElmSet, ElmEditOutgoingMessage,
		"Edit outgoing message?");
    if (enter_yn(msg, TRUE, LINES-3, FALSE))
	msgtype = SM_FORWARD;
    else
	msgtype = SM_FWDUNQUOTE;

    if ((len = strlen(hdr->subject)) > 0) {
	strcpy(subject, hdr->subject);
	if (len < 6 || strcmp(subject+len-(sizeof("(fwd)")-1), "(fwd)") != 0)
	    strcat(subject, " (fwd)");
    } else {
	strcpy(subject, catgets(elm_msg_cat, ElmSet, ElmForwardedMail,
	    "Forwarded mail..."));
    }

    return send_message((char *)NULL, (char *)NULL, subject, msgtype);
}

int get_return_address(char *address, char *single_address)
{
	char *sa;
	int i;

	for (sa = single_address; *address; ) {
	    i = len_next_part(address);
	    if (i > 1) {
		while (--i >= 0)
		    *sa++ = *address++;
	    } else if (*address == ',' || whitespace(*address))
		break;
	    else
		*sa++ = *address++;
	}
	*sa = '\0';
}

int get_return_name(char *address, char *name, int trans_to_lowercase)
{
	/** Given the address (either a single address or a combined list
	    of addresses) extract the login name of the first person on
	    the list and return it as 'name'.  Modified to stop at
	    any non-alphanumeric character. **/

	/** An important note to remember is that it isn't vital that this
	    always returns just the login name, but rather that it always
	    returns the SAME name.  If the persons' login happens to be,
	    for example, joe.richards, then it's arguable if the name
	    should be joe, or the full login.  It's really immaterial, as
	    indicated before, so long as we ALWAYS return the same name! **/

	/** Another note: modified to return the argument as all lowercase
	    always, unless trans_to_lowercase is FALSE... **/

	/**
	 *  Yet another note: Modified to return a reasonable name
	 *  even when double quoted addresses and DecNet addresses
	 *  are embedded in a domain style address.
	 **/

	char single_address[SLEN];
	register int	i, loc, iindex = 0,
			end, first = 0;
	register char	*c;

	dprint(6, (debugfile,"get_return_name called with (%s, <>, shift=%s)\n",
		   address, onoff(trans_to_lowercase)));

	/* First step - copy address up to a comma, space, or EOLN */

	get_return_address(address, single_address);

	/* Now is it an Internet address?? */

	if ((loc = qchloc(single_address, '@')) != -1) {	  /* Yes */

	    /*
	     *	Is it a double quoted address?
	     */

	    if (single_address[0] == '"') {
		first = 1;
		/*
		 *  Notice `end' will really be the index of
		 *  the last char in a double quoted address.
		 */
		loc = ((end = chloc (&single_address[1], '"')) == -1)
		    ? loc
		    : end;
	    }
	    else {
		first = 0;
	    }

	    /*
	     *	Hope it is not one of those weird X.400
	     *	addresses formatted like
	     *	/G=Jukka/S=Ukkonen/O=CSC/@fumail.fi
	     */

	    if (single_address[first] == '/') {
		/* OK then, let's assume it is one of them. */

		iindex = 0;

		if ((c = strstr (&single_address[first], "/G"))
		    || (c = strstr (&single_address[first], "/g"))) {

		    for (c += 2; *c && (*c++ != '='); );
		    for ( ;*c && (*c != '/'); c++) {
			name[iindex++] = trans_to_lowercase
					? tolower (*c) : *c;
		    }
		    if (iindex > 0) {
			name[iindex++] = '.';
		    }
		}
		if ((c = strstr (&single_address[first], "/S"))
		    || (c = strstr (&single_address[first], "/s"))) {

		    for (c += 2; *c && (*c++ != '='); );
		    for ( ;*c && (*c != '/'); c++) {
			name[iindex++] = trans_to_lowercase
					? tolower (*c) : *c;
		    }
		}
		name[iindex] = '\0';

		for (c = name; *c; c++) {
		    *c = ((*c == '.') || (*c == '-') || isalnum (*c))
			? *c : '_';
		}

		if (iindex == 0) {
		    strcpy (name, "X.400.John.Doe");
		}
		return 0;
	    }

	    /*
	     *	Is it an embedded DecNet address?
	     */

	    while (c = strstr (&single_address[first], "::")) {
		first = c - single_address + 2;
	    }


	    /*
	     *	At this point the algorithm is to keep shifting our
	     *	copy window left until we hit a '!'.  The login name
	     *	is then located between the '!' and the first meta-
	     *	character to it's right (ie '%', ':', '/' or '@').
	     */

	    for (i=loc; single_address[i] != '!' && i > first-1; i--)
		if (single_address[i] == '%' ||
		    single_address[i] == ':' ||
		    single_address[i] == '/' ||
		    single_address[i] == '@') loc = i-1;

	    if (i < first || single_address[i] == '!') i++;

	    for (iindex = 0; iindex < loc - i + 1; iindex++)
		if (trans_to_lowercase)
		    name[iindex] = tolower(single_address[iindex+i]);
		else
		    name[iindex] = single_address[iindex+i];
	    name[iindex] = '\0';

	}
	else {	/* easier - standard USENET address */

	    /*
	     *	This really is easier - we just cruise left from
	     *	the end of the string until we hit either a '!'
	     *	or the beginning of the line.  No sweat.
	     */

	    loc = strlen(single_address)-1; 	/* last char */

	    for (i = loc; i > -1 && single_address[i] != '!'
		 && single_address[i] != '.'; i--) {
		if (trans_to_lowercase)
		    name[iindex++] = tolower(single_address[i]);
		else
		    name[iindex++] = single_address[i];
	    }
	    name[iindex] = '\0';
	    reverse(name);
	}
	return 0;
}

int remail(void)
{
    /** remail a message... returns TRUE if new foot needed ... **/

    FILE *mailfd;
    char entered[VERY_LONG_STRING], expanded[VERY_LONG_STRING];
    char *filename, buffer[VERY_LONG_STRING], *msg;

    filename = NULL;
    entered[0] = '\0';

    if (!get_to(entered, expanded, SM_ORIGINAL))
	return FALSE;
    display_to(expanded);

    ClearLine(LINES);
    ClearLine(LINES-1);
    msg = catgets(elm_msg_cat, ElmSet, ElmSureYouWantToRemail,
	"Are you sure you want to remail this message?");
    if (!enter_yn(msg, TRUE, LINES-1, FALSE)) {
	set_error(catgets(elm_msg_cat, ElmSet, ElmBounceCancelled,
		    "Bounce of message canceled."));
        return TRUE;
    }

/* tempnam is bad.... but only if we open the file in a way that follows
 * symlinks.... file_open doesn't. */

    if((filename = tempnam(temp_dir, "snd.")) == NULL) {
	dprint(1, (debugfile, "couldn't make temp file nam! (remail)\n"));
	set_error(catgets(elm_msg_cat, ElmSet, ElmCouldntMakeTempFileName,
		    "Sorry - couldn't make file temp file name."));
	return TRUE;
    }

    if ((mailfd = file_open(filename, "w")) == NULL)
	goto failed;
    chown(filename, userid, groupid);
    copy_message(mailfd, curr_folder.curr_mssg, CM_REMOTE|CM_REMAIL);
    if (file_close(mailfd, filename) < 0)
	goto failed;

    build_mailer_command(buffer, filename,
	expanded, (char *)NULL, (char *)NULL);
    PutLine0(LINES, 0, catgets(elm_msg_cat, ElmSet, ElmResendingMail,
	    "Resending mail..."));
    (void) system_call(buffer, 0);
    set_error(catgets(elm_msg_cat, ElmSet, ElmMailResent, "Mail resent."));
    free((malloc_t)filename);
    return TRUE;

failed:
    if (filename != NULL) {
	(void) unlink(filename);
	free((malloc_t)filename);
    }
    return TRUE;
}

